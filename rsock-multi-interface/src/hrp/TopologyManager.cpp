//
// Cretaed by Chen Yang and Ala Altaweel
//

#include <asio/connect.hpp>
#include <hrpEexception.h>
#include "TopologyManager.h"
#include "common.h"
rapidjson::Document guidDoc;
namespace hrp
{

	// initialize _thisNode and _olsrConnector when creates an object
	// commented and replaced by Ala	
	/*
	TopologyManager::TopologyManager(const hrpNode &thisNode, asio::io_service &io_service) : _thisNode(thisNode), _olsrConnector(new OlsrConnector(io_service, this))
	*/
	// End Ala
	TopologyManager::TopologyManager(asio::io_service &io_service) : _olsrConnector(new OlsrConnector(io_service, this))
	{
		// share the pointer for _logger
		_logger = std::make_shared<spdlog::logger>("TopologyManager", file_sink);
		if (debug_mode)
			_logger->set_level(spdlog::level::debug);
		//_logger->info("TopologyManager:: Constructor()");
	}

	TopologyManager::~TopologyManager()
	{
		delete _olsrConnector;
	}


	void TopologyManager::registerNodeEventListener(std::function<void(const hrp::hrpNode &, bool)> cb)
	{
		_nodeListeners.push_back(cb);
	}

	void TopologyManager::init()
	{
		_olsrConnector->init();
	}

	void TopologyManager::notifyHrpReadyNodeConnected(const hrpNode &node)
	{
	}

	void TopologyManager::notifyHrpReadyNodeDisconnected(const hrpNode &node)
	{
	}

	HrpGraph::succMap TopologyManager::findKShortestPath(const hrpNode &src, const hrpNode &dst, int K)
	{
		//_logger->debug("findKShortestPath, called");
		if (_kPathCache.count(dst) != 0)
		{
			//_logger->debug("findKShortestPath, inside if (_kPathCache.count(dst) != 0)");
			if (_kPathCache[dst].size() >= K)
			{
				//_logger->debug("findKShortestPath, inside if (_kPathCache[dst].size() >= K)");
				std::vector<HrpGraph::hrpPathEntry> paths(_kPathCache[dst].begin(), _kPathCache[dst].begin() + K); 	
				return HrpGraph::convertKPathToSuccMap(paths);
			}
		}
		try
		{
			//_logger->debug("findKShortestPath, inside try");
			_kPathCache[dst] = _hrpGraph.findKShortestPath(src, dst, K);
		}
		catch (const hrpException &e)
		{	
			throw e;
		}

		return HrpGraph::convertKPathToSuccMap(_kPathCache[dst]);
	}


	void TopologyManager::topologyChange(std::set<hrpNode> nodes, std::set<hrpEdge> edges)
	{

		std::map<hrpNode, lemon::ListDigraphBase::Node> &m = _hrpGraph.get_mutable_vertices();
		lemon::ListDigraph &g = _hrpGraph.get_mutable_graph();
		std::set<hrpNode> oldNodes;
		for (auto itr : m)
			oldNodes.insert(itr.first);

		// get difference
		std::set<hrpNode> availableNodes, unavailableNodes;
		std::set_difference(nodes.begin(), nodes.end(), oldNodes.begin(), oldNodes.end(), std::inserter(availableNodes, availableNodes.begin()));
		std::set_difference(oldNodes.begin(), oldNodes.end(), nodes.begin(), nodes.end(), std::inserter(unavailableNodes, unavailableNodes.begin()));

		// reconstruct the graph
		g.clear();
		m.clear();
		_kPathCache.clear();
		for (const hrpNode& node : nodes)
			_hrpGraph.addNode(node);
	
		for (hrpEdge edge : edges)
			_hrpGraph.addArc(edge.peer1, edge.peer2, edge.length);
	
		// notify others of node changes
		for (const hrpNode& node : unavailableNodes)
		{
			if (node == _thisNode)
				continue;
			for (std::function<void(const hrp::hrpNode &, bool)> cb : _nodeListeners)
				cb(node, false);
		}
	
		for (const hrpNode& node : availableNodes)
		{
			if (node == _thisNode)
				continue;
			for (std::function<void(const hrp::hrpNode &, bool)> cb : _nodeListeners)
				cb(node, true);
		}
		// pretty print of the graph
		/*
		_logger->debug("\npretty_print:");
		_logger->debug(_hrpGraph.pretty_print());
		_logger->debug("\n");
		*/



		if(unavailableNodes.size()!=0)
		{
			_logger->debug("\nUnavailable:");
			for (const hrpNode& node : unavailableNodes)
			{
				_logger->debug(printHrpNode("node: ", node));
			}
		}	
		if(availableNodes.size()!=0)
		{
		
			_logger->debug("\nAvailable:");
			for (const hrpNode& node : availableNodes)
			{
				_logger->debug(printHrpNode("node: ", node));
			}
		}	
		
	}
	
	const HrpGraph &TopologyManager::get_hrpGraph() const
	{
		return _hrpGraph;
	}	



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// This class is handling all communications with olsr daemon on local device:9090
	OlsrConnector::OlsrConnector(asio::io_service &io_service, TopologyManager *manager) : _topologyManager(manager), _io_service(io_service), _socket(_io_service), _endpoint(asio::ip::address::from_string("127.0.0.1"), 9090)
	{
		_logger = std::make_shared<spdlog::logger>("OlsrConnector", file_sink);
		if (debug_mode)
			_logger->set_level(spdlog::level::debug);
		//_logger->info("OlsrConnector:: Constructor()");
	}

	void OlsrConnector::init()
	{

		using namespace asio::ip;

		// conenct to the olsr server
		try
		{
			_socket.connect(_endpoint);
			_logger->info("init, connected to olsr daemon");
			_socket.close();
		}
		catch (std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			exit(1);
		}


		// initialize the timer
		_fetch_timer = new asio::steady_timer(_io_service);
		if (_fetch_timer == NULL)
		{
			_logger->error("init, cannot initialize _fetch_timer");
			exit(1);
		}
		// calling the scheduleTopologyFetchTimer to fetch topology from olsr 
		scheduleTopologyFetchTimer();
	}

	std::string OlsrConnector::fetchJsonByCmd(const std::string &cmd)
	{
		std::string str;
		try
		{
			_socket.connect(_endpoint);
			_socket.send(asio::buffer(cmd, cmd.length()));
			std::vector<char> buf(1024);
			asio::error_code socket_err;

			for (;;)
			{
				size_t len = _socket.read_some(asio::buffer(buf), socket_err);
				if (socket_err == asio::error::eof)		
					break; // Connection closed cleanly by peer.
				if (socket_err)
					throw asio::system_error(socket_err); // Some other error.

				std::string nstr(buf.begin(), buf.begin() + len);

				/*
				// this if statement was used when using olsrv2 OONF
				if (nstr.find(">") != std::string::npos) {
				str = str + nstr.substr(0, nstr.find(">"));
				break;
				}
				*/

				str += nstr;
			}
			_socket.close();
		}
		catch (std::exception &e)
		{
			_logger->error("{}", e.what());
			exit(1);
		}
		
		return str;
	}
	
	std::set<hrpNode> OlsrConnector::fetchNodesFromJson(rapidjson::Document &j)
	{

		// Commented and Replaced by Ala
		/*
		std::set<hrpNode> nodes;
		const rapidjson::Value& routes = j["routes"];
		for (const auto& v : routes.GetArray())
		{
			nodes.insert(v["destination"].GetString());
		}
		// fetch the routes first, ensure that all nodes are routable
		// also add this node
		nodes.insert(_topologyManager->_thisNode);
		return nodes;
		*/

		//_logger->info("OlsrConnector::fetchNodesFromJson");
		// create an object of gns client
		GnsServiceClient gnsObj;
			
		std::set<hrpNode> nodes;
		const rapidjson::Value& routes = j["routes"];
		for (const auto& v : routes.GetArray())
		{
			// get GUID by IP

			std::vector<std::string> GUID = gnsObj.getGUIDbyIP(v["destination"].GetString());
			if(GUID.size() == 1)			
			{	
				//_logger->info("Got this GUID: {} for ip: {}",GUID.at(0), v["destination"].GetString());
				hrpNode hrpNodeObj;
				hrpNodeObj.push_back(GUID.at(0));
					
				//_logger->debug(printHrpNode("selected node and IP by olsr: ", hrpNodeObj));
				
				//_logger->info("selected node and IP by olsr: {} - {}", hrpNodeObj.size()!=0? hrpNodeObj.at(0): "empty hrpNode", v["destination"].GetString());
				nodes.insert(hrpNodeObj);
			}
			else
			{
				_logger->warn("fetchNodesFromJson destination no GUID returned for node with ip: {}", v["destination"].GetString());			

			}
			
		}
		// fetch the routes first, ensure that all nodes are routable
		// also add this node
		// Commented and replaced by Ala		
		/*
		nodes.insert(_topologyManager->_thisNode);
		return nodes;
		*/
	
		// update my current node to handle if interface(s) changed
		
		string ownGUID = gnsObj.getOwnGUID();
		hrpNode hrpNodeObj;
		if(ownGUID.size() != 0)			
		{	
			hrpNodeObj.push_back(ownGUID);
		}
		else
		{
			_logger->warn("no GUID returned from getOwnGUID() function");			
		}
			
		//_topologyManager->_thisNode = hrpNodeObj;	
		//nodes.insert(_topologyManager->_thisNode);
		// we need to add a lock here so other readers wait until we update this node
		{
			std::lock_guard<std::mutex> lck (_mutex_table_for_thisNode);
			_thisNode = hrpNodeObj;	
		}
		nodes.insert(_thisNode);
				
		return nodes;
		// End Ala
	}

	std::set<hrpEdge> OlsrConnector::fetchLinksFromJson(rapidjson::Document &j, const std::set<hrpNode> &nodes)
	{
		// Commented and Replaced by Ala
		/*
		std::set<hrpEdge> edges;
		const rapidjson::Value& routes = j["topology"];
		for (const auto& v : routes.GetArray())
		{
			hrpNode p1(v["lastHopIP"].GetString());
			hrpNode p2(v["destinationIP"].GetString());
			if (nodes.count(p1)!=0 && nodes.count(p2)!=0)
				edges.insert(hrpEdge(p1, p2, v["linkQuality"].GetDouble()));
		}
		// fetch the topology, i.e. edges
		return edges;
		*/
		//_logger->info("OlsrConnector::fetchLinksFromJson");
		// create an object of gns client	
		GnsServiceClient gnsObj;
		std::set<hrpEdge> edges;
		const rapidjson::Value& routes = j["topology"];
		for (const auto& v : routes.GetArray())
		{

			hrpNode p1, p2;
			// get GUID1 
			std::vector<std::string> GUID1 = gnsObj.getGUIDbyIP(v["lastHopIP"].GetString());
			if(GUID1.size() == 1)			
			{	
				//_logger->info("Got this GUID: {} for ip: {}",GUID1.at(0), v["lastHopIP"].GetString());

				p1.push_back(GUID1.at(0));	
				//_logger->info(printHrpNode("p1: ", p1));


			}
			else
			{
				_logger->warn("fetchLinksFromJson lastHopIP no GUID returned for node with ip: {}", v["lastHopIP"].GetString());			

			}

			
			
			// get GUID2 
			
			std::vector<std::string> GUID2 = gnsObj.getGUIDbyIP(v["destinationIP"].GetString());
			if(GUID2.size() == 1)			
			{
				//_logger->info("Got this GUID: {} for ip: {}",GUID2.at(0), v["destinationIP"].GetString());

				p2.push_back(GUID2.at(0));
				//_logger->info(printHrpNode("p2: ", p2));
			}
			else
			{
				_logger->warn("fetchLinksFromJson destinationIP no GUID returned for node with ip: {}", v["destinationIP"].GetString());			

			}
			
			
			
			if (nodes.count(p1)!=0 && nodes.count(p2)!=0)
			{
				edges.insert(hrpEdge(p1, p2, v["linkQuality"].GetDouble()));
				//_logger->info("Add p1 and p2 edge to the links");
			}
		}
		// fetch the topology, i.e. edges
		return edges;
		// End Ala
	}

	/**
	* Handler function for the topology fetch timer
	*/
	void OlsrConnector::scheduleTopologyFetch(const std::error_code &error)
	{

		//_logger->debug("OlsrConnector::scheduleTopologyFetch");
		//char sendbuf[] = "netjsoninfo filter graph ipv4_0\x0d\x0a";
		std::string str = fetchJsonByCmd("/topology/routes\x0d\x0a");
		
		// print what is fetched from olsr
		//_logger->debug("retruned json from olsr: {}", str);
		



		rapidjson::Document j;
		j.Parse(str.c_str());
		// call refresh GNS function to refresh the cache
		refreshGNS(j);
		std::set<hrpNode> nodes = fetchNodesFromJson(j);
		std::set<hrpEdge> edges = fetchLinksFromJson(j, nodes);



		// Modify Json doc
		//rapidjson::Document guidDoc;
		{
			std::lock_guard<std::mutex> lck (_mutex_table);
			// assign the global json object
			guidDoc.Parse(str.c_str());
			// translate IPs in json to guids
			replaceIPstoGUIDsJsonDoc();		
		}
		// End Ala

		// call the topologyChange function 		
		_topologyManager->topologyChange(nodes, edges);
		// repeat by calling the scheduleTopologyFetchTimer 
		scheduleTopologyFetchTimer();
	}


	void OlsrConnector::replaceIPstoGUIDsJsonDoc()
	{


		// create an object of gns client
		hrp::GnsServiceClient gnsObj;
		hrpNode hrpNodeObj;

		if(guidDoc.HasMember("routes"))
		{
			//_logger->debug("guidDoc has routes array");

			for (unsigned int i = 0; i < guidDoc["routes"].Size(); ++i)
			{
				std::string destinationStr = guidDoc["routes"][i]["destination"].GetString();
				//_logger->debug("destinationStr: {}", destinationStr);				
				// get GUID by IP for destination from the cache since its updated by refreshGNS
				std::vector<std::string> GUIDDest = gnsObj.getGUIDbyIP(destinationStr,true);
				if(GUIDDest.size() != 0)			
				{	// replace the destination with corressponding GUID	
					//_logger->debug("Set destination GUID: {} with length: {}", GUIDDest.at(0).c_str(), GUIDDest.at(0).size());					
					guidDoc["routes"][i]["destination"].SetString(GUIDDest.at(0).c_str(), GUIDDest.at(0).size(), guidDoc.GetAllocator());
				}
				else
				{
					_logger->warn("no GUID returned for node with ip: {}", destinationStr);	
					// if IP is not translatable, skip the whole record
					continue;
				}




				std::string gatewayStr = guidDoc["routes"][i]["gateway"].GetString();
				//_logger->debug("gatewayStr: {}", gatewayStr);
				// get GUID by IP for gateway from the cache since its updated by refreshGNS
				std::vector<std::string> GUIDGateway = gnsObj.getGUIDbyIP(gatewayStr,true);
				if(GUIDGateway.size() != 0)			
				{	// replace the gateway with corressponding GUID
					//_logger->debug("Set gateway GUID: {} with length: {}", GUIDGateway.at(0).c_str(), GUIDGateway.at(0).size());					
					guidDoc["routes"][i]["gateway"].SetString(GUIDGateway.at(0).c_str(), GUIDGateway.at(0).size(), guidDoc.GetAllocator());
				}
				else
				{
					_logger->warn("no GUID returned for node with ip: {}", gatewayStr);	
					// if IP is not translatable, skip the whole record
					continue;
				}
			}			
		}
		else
			_logger->warn("guidDoc has no routes array");
		
		if(guidDoc.HasMember("topology"))
		{
			//_logger->debug("guidDoc has topology array");
			
			for (unsigned int i = 0; i < guidDoc["topology"].Size(); ++i)
			{
				std::string lastHopIPStr = guidDoc["topology"][i]["lastHopIP"].GetString();
				//_logger->debug("lastHopIPStr: {}", lastHopIPStr);
				// get GUID by IP for lastHopIP from the cache since its updated by refreshGNS
				std::vector<std::string> GUIDLastHop = gnsObj.getGUIDbyIP(lastHopIPStr,true);
				if(GUIDLastHop.size() != 0)			
				{	// replace the destination with corressponding GUID
					//_logger->debug("Set lastHopIP GUID: {} with length: {}", GUIDLastHop.at(0).c_str(), GUIDLastHop.at(0).size());
					guidDoc["topology"][i]["lastHopIP"].SetString(GUIDLastHop.at(0).c_str(), GUIDLastHop.at(0).size(), guidDoc.GetAllocator());
				}
				else
				{
					_logger->warn("no GUID returned for node with ip: {}", lastHopIPStr);	
					// if IP is not translatable, skip the whole record
					continue;
				}


				std::string destinationIPStr = guidDoc["topology"][i]["destinationIP"].GetString();
				//_logger->debug("destinationIPStr: {}", destinationIPStr);
				// get GUID by IP for destinationIP from the cache since its updated by refreshGNS
				std::vector<std::string> GUIDDestIP = gnsObj.getGUIDbyIP(destinationIPStr,true);
				if(GUIDDestIP.size() != 0)			
				{	// replace the destination with corressponding GUID
					//_logger->debug("Set destinationIP GUID: {} with length: {}", GUIDDestIP.at(0).c_str(), GUIDDestIP.at(0).size());
					guidDoc["topology"][i]["destinationIP"].SetString(GUIDDestIP.at(0).c_str(), GUIDDestIP.at(0).size(), guidDoc.GetAllocator());
				}
				else
				{
					_logger->warn("no GUID returned for node with ip: {}", destinationIPStr);
					// if IP is not translatable, skip the whole record	
					continue;
				}
			}			
		}
		else
			_logger->warn("guidDoc has no topology array");

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		guidDoc.Accept(writer);
		//_logger->debug("updated json with GUIDSs: {}", buffer.GetString());	
	}

	void OlsrConnector::refreshGNS(rapidjson::Document &j)
	{
		const rapidjson::Value& topology = j["topology"];
		set<string> _IPsSet;
		// create an object of gns client	
		GnsServiceClient gnsObj;
		for (const auto& v : topology.GetArray())
		{
			_IPsSet.insert(v["destinationIP"].GetString());
			_IPsSet.insert(v["lastHopIP"].GetString());

			//_IPsSeenbyOLSR.push_back(v["destinationIP"].GetString());
			//_IPsSeenbyOLSR.push_back(v["lastHopIP"].GetString());
		}

		const rapidjson::Value& routes = j["routes"];
		for (const auto& v : routes.GetArray())
		{
			_IPsSet.insert(v["destination"].GetString());
			_IPsSet.insert(v["gateway"].GetString());
		}

		// conver set to list to remove redundancies 
		vector <string> _IPsSeenbyOLSR(_IPsSet.begin(), _IPsSet.end());


		
	
		if (_IPsSeenbyOLSR.size() == 0)
			_logger->info("no neighbors seen by olsr");			
		else
		{
		//	_logger->info("IPs seen by olsr:");			
		//	for(int i=0; i<_IPsSeenbyOLSR.size(); i++)
		//		_logger->info(" {}", _IPsSeenbyOLSR.at(i));
			
			// refresh the cache at GNS	
			gnsObj.refreshTranslationTable(_IPsSeenbyOLSR);
		}
	}
	


	/**
	* Schedule the topology fetch timer
	*/	
	void OlsrConnector::scheduleTopologyFetchTimer()
	{
		// sets the expiry time
		_fetch_timer->expires_from_now(std::chrono::seconds((long long)HRP_DEFAULT_OLSR_FETCH_EPOCH));
		// initiate an asynchronous wait against the timer
		// the handler function i.e., scheduleTopologyFetch will be called when the timer has expired
		_fetch_timer->async_wait(std::bind(&OlsrConnector::scheduleTopologyFetch, this, std::placeholders::_1));
	}
}
