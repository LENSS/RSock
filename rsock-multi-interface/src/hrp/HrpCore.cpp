//
// Cretaed by Chen Yang and Ala Altaweel
//

#include <cassert>
#include <hrpMessages.h>
#include "HrpCore.h"


std::shared_ptr<spdlog::sinks::stdout_sink_st> file_sink;
bool debug_mode = false;

namespace hrp
{


	// Commented and replaced by Ala
	/*
	HrpCore::HrpCore(const hrpNode &nid, const std::map<std::string, std::string> &cfg)
	: _thisNode(nid), _mveCore(nid),_mveEstimator(_mveCore, _mutex_mveCore), _engine(NULL), _pi_timer(NULL)
	*/
	// End Ala
	HrpCore::HrpCore(const std::map<std::string, std::string> &cfg)
	:_mveCore(_thisNode),_mveEstimator(_mveCore, _mutex_mveCore), _engine(NULL), _pi_timer(NULL)
	{
		std::string val;
		if (cfg.find(HRP_CFG_ALPHA) != cfg.end())
		{
			val = cfg.find(HRP_CFG_ALPHA)->second;
			_alpha = std::stod(val);
		}
		
		else
		_alpha = HRP_DEFAULT_ALPHA;

		if (cfg.find(HRP_CFG_ITA) != cfg.end())
		{
			val = cfg.find(HRP_CFG_ITA)->second;
			_ita = std::stod(val);
		}
		else
			_ita = HRP_DEFAULT_ITA;

		if (cfg.find(HRP_CFG_ICT_PROBE) != cfg.end())
		{
			val = cfg.find(HRP_CFG_ICT_PROBE)->second;
			_inter_probe_duration = std::stod(val);
		}
		else
		_inter_probe_duration = HRP_DEFAULT_ICT_PROBE;

		if (cfg.find(HRP_CFG_RMAX) != cfg.end())
		{
			val = cfg.find(HRP_CFG_RMAX)->second;
			_rmax = std::stoul(val);
		}
		else
			_rmax = HRP_DEFAULT_RMAX;

		if (cfg.find(HRP_CFG_MVE_EPOCH) != cfg.end())
		{
			val = cfg.find(HRP_CFG_MVE_EPOCH)->second;
			_mve_epoch_duration = std::stod(val);
		}
		else
			_mve_epoch_duration = HRP_DEFAULT_MVE_EPOCH_DURATION;

		_logger = std::make_shared<spdlog::logger>("HrpCore", file_sink);

		if (debug_mode)
			_logger->set_level(spdlog::level::debug);
	}

	HrpCore::~HrpCore()
	{
		/// release resources in the _neighbors_db
		for (auto itr = _neighbors_db.begin(); itr != _neighbors_db.end(); )
		{
			db_node_entry &entry = itr->second;
			delete entry._fi_learner;
			delete entry._fi_rm_core;
			delete entry._pi_learner;
			delete entry._pi_rm_core;
			delete entry._mve_core;
			delete entry._mutex_mve;
			itr = _neighbors_db.erase(itr);
		}

		delete _pi_timer;
	}

	void HrpCore::scheduleProbes(const std::error_code &error)
	{

		//_logger->debug("scheduleProbes()");
		if (error)
		{
			_logger->error("ERROR in HrpCore::scheduleProbes, message = {}", error.message());
			return;
		}

		{


			std::lock_guard<std::mutex> lg(_mutex_ngbr_db);

			for (auto itr : _neighbors_db)
			{
				//_logger->debug("scheduleProbes inside for");
				db_node_entry& entry = itr.second;
				// Only try to probe if the neighbor is connected
				if (entry.isConnected && entry._pi_learner->shouldProbe())
				{
					pi_probe_meta probe_meta = entry._pi_learner->prepareProbe();
					auto pkt = std::make_shared<HrpDataHeader>(HrpDataHeader(_thisNode, itr.first, probe_meta.repl, pi_probe_request));
					pkt->set_payload(&probe_meta, sizeof(probe_meta));
					// Commented and Replaced by Ala
					/*
					_logger->debug("scheduleProbes: sending pi_probes_request to {}", itr.first);
					*/
					//_logger->debug("scheduleProbes: sending pi_probes_request to {}", printHrpNode("",itr.first));
					// End Ala

					_engine->scheduleTransfer(pkt, std::bind(&HrpCore::TxrxEngineCallback, this, std::placeholders::_1, std::placeholders::_2));
				}
				// Commented and Replaced by Ala
				/*
				_logger->debug("scheduleProbes: updating pi_learner for {}", itr.first);
				*/
				//_logger->debug("scheduleProbes: updating pi_learner for {}", printHrpNode("",itr.first));
				// End Ala
				entry._pi_learner->update();
				// _logger->info("scheduleProbes: pi_probs to {} is {}", itr.first, entry._pi_learner->format_probs());
			}
		}


		// schedule next probe event
		schedulePIProbeTimer();
	}

	void HrpCore::schedulePIProbeTimer()
	{
		//_logger->debug("schedulePIProbeTimer()");
		_pi_timer->expires_from_now(std::chrono::seconds((long long)this->_inter_probe_duration));
		_pi_timer->async_wait(std::bind(&HrpCore::scheduleProbes, this, std::placeholders::_1));
	}

	void HrpCore::init()
	{

		_logger->debug("init()");

		// check if TxrxEngine is initialized
		if (_engine == NULL)
		{
			_logger->error("ERROR in HrpCore::init(), TxrxEngine is not set");
			exit(1);
		}

		// register receiver handler
		_engine->registerHandler(pi_probe_response, std::bind(&HrpCore::rxReceived, this, std::placeholders::_1));
		_engine->registerHandler(pi_probe_request, std::bind(&HrpCore::rxReceived, this, std::placeholders::_1));

		// initialize the pi_timer
		asio::io_service &_io = _engine->get_io_service();
		_pi_timer = new asio::steady_timer(_io);
		if (_pi_timer == NULL)
		{
			_logger->error("ERROR in init(), cannot initialize _pi_timer");
			exit(1);
		}

		_mveEstimator.set_io_service(&_io);
		_mveEstimator.init();

		// schedule the first PI probe event
		schedulePIProbeTimer();
	}

	void HrpCore::shutdown()
	{
	}

	void HrpCore::TxrxEngineCallback(std::shared_ptr<HrpDataHeader> pkt, bool success)
	{
		if (success)
			txStarted(*pkt);
		else
			txAborted(*pkt);
	}

	void HrpCore::txStarted(HrpDataHeader &header)
	{

		try
		{
		
			/// if this is a pi_probe_request, notify the learner that the transmission is started
			if (header.get_type() == hrp_packet_type::pi_probe_request)
			{
				auto *req = (pi_probe_meta *)(header.get_payload());
				std::lock_guard<std::mutex> lg(_mutex_ngbr_db);

				const hrpNode &dst = header.get_dst_by_guid();				
				if (_neighbors_db.find(dst) == _neighbors_db.end())
				{
					/// something is wrong
					_logger->error( "ERROR: txStarted, could not find the dst in neighbor_db");
				}
				else
				{
					auto itr = _neighbors_db.find(dst);
					db_node_entry &entry = itr->second;
					entry._pi_learner->sentProbe(*req);
				}
			}
		}
		catch (hrpException &e)
		{
			_logger->error("HrpCore::txStarted cath exception: {}", e.what());
			_logger->info("skipping the exception");
		}
		catch (...)
		{
			throw;
		}


	}

	void HrpCore::txCompleted(HrpDataHeader &header)
	{
	}

	void HrpCore::txAborted(HrpDataHeader &header)
	{
	}

	void HrpCore::rxReceived(std::shared_ptr<HrpDataHeader> header)
	{


		try
		{
			//_logger->debug("HrpCore::rxReceived starts");
			const hrpNode& peer = header->get_src_by_guid();			
			std::lock_guard<std::mutex> lg(_mutex_ngbr_db);
			auto itr = _neighbors_db.find(peer);
			if (itr == _neighbors_db.end())
			{
				// Commented and Replaced by Ala
				/*
				_logger->error("ERROR: rxReceived, could not find the peer: {} in neighbor_db", peer);
				*/
				_logger->error("ERROR: rxReceived, could not find the peer: {} in neighbor_db", printHrpNode("",peer));
				// End Ala
				return;
			}

			db_node_entry& entry = itr->second;
			if (header->get_type() == pi_probe_response)
			{
	
				auto res = (pi_response_tx *) header->get_payload();
				// Commented and Replaced by Ala
				/*
				_logger->debug("rxReceived, receive pi_probe_response ({}, {}) from {}", res->seq, res->repl, peer);
				*/
				//_logger->debug("rxReceived, receive pi_probe_response ({}, {}) from {}", res->seq, res->repl, printHrpNode("",peer));
			
				// End Ala

				entry._pi_learner->recvResponse(*res);
			}
	
			
			if (header->get_type() == pi_probe_request)
			{
				auto res = (pi_probe_meta *) header->get_payload();
				// Commented and Replaced by Ala
				/*
				_logger->debug("rxReceived, receive pi_probe_request ({}, {}) from {}, repl = {}", res->seq, res->repl, peer, header->get_remain_repl());
				*/
				//_logger->debug("rxReceived, receive pi_probe_request ({}, {}) from {}, repl = {}", res->seq, res->repl, printHrpNode("",peer), header->get_remain_repl());
				// End Ala	
				if (entry._pi_seq < res->seq)
				{
					entry._pi_seq = res->seq;
					pi_response_tx response_data;
					response_data.repl = res->repl;
					response_data.seq = res->seq;
					HrpDataHeader resPkt(_thisNode, peer, res->repl, pi_probe_response);
					resPkt.set_payload(&response_data, sizeof(response_data));
					std::shared_ptr<HrpDataHeader> response_ptr = std::make_shared<HrpDataHeader>(resPkt);
					// Commented and Replaced by Ala
					/*
					_logger->debug("rxReceived, send back pi_probe_response ({}, {}) to {}", response_data.seq, response_data.repl, peer);
					*/
					//_logger->debug("rxReceived, send back pi_probe_response ({}, {}) to {}", response_data.seq, response_data.repl, printHrpNode("",peer));
					//End Ala
					_engine->scheduleTransfer(response_ptr, std::bind(&HrpCore::TxrxEngineCallback, this, std::placeholders::_1, std::placeholders::_2));
				}
			}
		}

		catch (hrpException &e)
		{
			_logger->error("HrpCore::rxReceived cath exception: {}", e.what());
			_logger->info("skipping the exception");
		}
		catch (...)
		{
			throw;
		}
	}


	double HrpCore::acrCalc(const hrpNode &candidate, std::shared_ptr<HrpDataHeader> pkt, const MVECore &mveCore)
	{
		double ret = 0;
		const std::map<ect_set, double> &params = mveCore.get_params();
		ect_set intersection;
		for (auto itr = params.begin(); itr != params.end(); itr++)
		{
			intersection.clear();
			std::set_intersection(pkt->get_carriers().begin(), pkt->get_carriers().end(), itr->first.begin(), itr->first.end(), std::inserter(intersection, intersection.begin()));
			if (itr->first.find(candidate) != itr->first.end() && intersection.empty())
				ret += itr->second;
		}
		return ret;
	}


	std::shared_ptr<hrpNode> HrpCore::calcRoutingDecision(std::shared_ptr<HrpDataHeader> pkt)
	{


		try
		{
			//_logger->debug("calcRoutingDecision()");
			double self_acr = 0, peer_acr = 0, max_acr = 0;
			std::shared_ptr<hrpNode> retptr = nullptr;
			MVECore *dstCore = nullptr;
			std::vector<hrpNode> incontacts;
			std::vector<double> acrs;

			// Commented and replaced by Ala
			/*
			const hrpNode &dst = pkt->get_dst_by_guid();
			*/
			const hrpNode &dst = pkt->get_dst_by_guid();
			// End Ala			
	
			{
				std::lock_guard<std::mutex> lg(_mutex_ngbr_db);
				//_logger->debug("calcRoutingDecision(), got the lock!");
				// if we have met with the dst before
				if (_neighbors_db.find(dst) != _neighbors_db.end())
				{
					//_logger->debug("calcRoutingDecision(), we met with the dest before!");
					const auto &entry = _neighbors_db.find(dst)->second;
					dstCore = entry._mve_core;
					std::lock_guard<std::mutex> lg_entry(*entry._mutex_mve);
					self_acr = acrCalc(_thisNode, pkt, *dstCore);
				}
	
				for (auto itr : _neighbors_db)
				{
					//_logger->debug("calcRoutingDecision(), inside looping for _neighbors_db");
					if (itr.second.isConnected)
					{
						//_logger->debug("calcRoutingDecision(), itr.second Is Connected");
						incontacts.push_back(itr.first);
						if (!itr.second._bloom_filter->contains(pkt->get_hash()))
						{
							// if we found the destination, just return it
							if (itr.first == dst)
							{
								retptr = std::make_shared<hrpNode>(itr.first);
								acrs.push_back(-2);
								//_logger->debug("calcRoutingDecision(), inside itr.first == dst");
								break;
							}
	
							std::lock_guard<std::mutex> lg_entry(*itr.second._mutex_mve);
							// if self_acr == 0
							if (self_acr == 0)
							{
								//_logger->debug("calcRoutingDecision(), inside if self_acr == 0");
								ect_set tmp;
								tmp.insert(dst);
								peer_acr = itr.second._mve_core->getMarginal(tmp);
							}
							// else self_acr != 0
							else
							{
								//_logger->debug("calcRoutingDecision(), inside else self_acr != 0");
								assert(dstCore != nullptr);
								peer_acr = acrCalc(itr.first, pkt, *dstCore);
							}
							// if peer_acr > max_acr 						
							// Commented and Replaced by Ala
							/*
							if (peer_acr > max_acr)	
							*/			
							//End Ala
							if (peer_acr >= max_acr)	
							{	
								//_logger->debug("calcRoutingDecision(), inside if peer_acr > max_acr");
								//_logger->debug("calcRoutingDecision(), inside if peer_acr >= max_acr");		
								max_acr = peer_acr;
								retptr = std::make_shared<hrpNode>(itr.first);
							}
							acrs.push_back(peer_acr);
	
						}
						else
						{
							// Commented and Replaced by Ala
							/*
							_logger->info("calcRoutingDecision peer {}'s bloom filter contains {}", itr.first, pkt->get_hash());
							*/
							_logger->debug("calcRoutingDecision(), peer {}'s bloom filter contains {}", printHrpNode("",itr.first), pkt->get_hash());	
							// End Ala
					
							acrs.push_back(-1);
							if (itr.first == dst)
							{
								//_logger->debug("calcRoutingDecision(), itr.first: {}",printHrpNode("",itr.first));
								//_logger->debug("calcRoutingDecision(), dst: {}",printHrpNode("",dst));
								// commented and replaced by Ala
								/*
								retptr = nullptr;								
								*/
								retptr = std::make_shared<hrpNode>(itr.first);
								// End by Ala
								break;
							}
						}
					}
					else
					{
						_logger->debug("calcRoutingDecision(), node: {} in _neighbors_db Is Not Connected", printHrpNode("",itr.second._nid));					
						;
					}
				}
				//_logger->debug("calcRoutingDecision(), Finish looping for _neighbors_db");
			}
	
			// comment the below block out if you dont run in debug mode
			/////////////////////////////
			
			string toBePrinted;
			toBePrinted = string ("[");
			for (int i=0; i<acrs.size(); i++)
			{
				toBePrinted += string("[") + printHrpNode("",incontacts[i]) + string(", ") + to_string (acrs[i]) + string ("], ");
			}
			toBePrinted += "]";
			_logger->debug("calcRoutingDecision() decides to forward to {}, metrics = {}", retptr == nullptr? "null" : printHrpNode("",*retptr), toBePrinted);
			
			/////////////////////////////
			return retptr;
		}
		catch (hrpException &e)
		{
			_logger->error("HrpCore::calcRoutingDecision cath exception: {}", e.what());
			_logger->info("skipping the exception");
			return nullptr;
		}
		catch (...)
		{
			throw;
		}


	}	
	
	unsigned int HrpCore::drawReplFactor(const hrpNode& dst)
	{
		double rho = 0;
		unsigned int a1, a2;
		{
			std::lock_guard<std::mutex> lg(_mutex_ngbr_db);
			if (_neighbors_db.find(dst) == _neighbors_db.end())
			{
				// if dst is not seen before
				return _rmax;
			}

			std::chrono::steady_clock::time_point now_time = std::chrono::steady_clock::now();
			db_node_entry &entry = _neighbors_db.find(dst)->second;
			double in_ct_duration = entry._ct_duration.count();
			if (entry.isConnected)
			{
				std::chrono::duration<double> duration = now_time - entry._last_ct_time;
				in_ct_duration += duration.count();	
			}

			std::chrono::duration<double> total_chrono_duration = now_time - entry._start_time;
			double total_duration = total_chrono_duration.count();
			assert(total_duration != 0);

			rho = in_ct_duration/total_duration;
			a1 = entry._fi_rm_core->drawAction();
			a2 = entry._pi_rm_core->drawAction();
		}

		double r = (double)std::rand()/RAND_MAX;
		if (r < rho)
		{
			//commented out by Ala	
			_logger->debug("drawReplFactor, rho = {}, pick r = {}, using pi_learner", rho, r);
			return a2;
		}
		else
		{
			//commented out by Ala
			_logger->debug("drawReplFactor, rho = {}, pick r = {}, using fi_learner", rho, r);
			return a1;
		}
	}

	void HrpCore::nodeConnected(const hrpNode &peer, const MVECore &core, const bloom_filter &filter)
	{
		{
			// Commented and Replaced by Ala
			/*
			_logger->info("nodeConnected, peer: {} connected", peer);
			*/
			_logger->info("nodeConnected, peer: {} connected", printHrpNode("",peer));
			// End Ala
			std::lock_guard<std::mutex> lg(_mutex_ngbr_db);
			if (_neighbors_db.find(peer) != _neighbors_db.end())
			{
				// Commented and Replaced by Ala
				/*
				_logger->debug("nodeConnected, peer: {} seen before, updating", peer);
				*/
				_logger->debug("nodeConnected, peer: {} seen before, updating", printHrpNode("",peer));
				// End Ala
		
				// We have seen this node before, just need to update
				auto itr = _neighbors_db.find(peer);
				db_node_entry &entry = itr->second;
				
				// update _last_ct_time
				entry._last_ct_time = std::chrono::steady_clock::now();

				// update peer mve core
				std::lock_guard<std::mutex> clg(*entry._mutex_mve);
				delete entry._mve_core;
				delete entry._bloom_filter;
				entry._mve_core = new MVECore(core);
				entry._bloom_filter = new bloom_filter(filter);
			
				/// set isConnected to true
				entry.isConnected = true;
			}
			else
			{

				// Commented and Replaced by Ala
				/*
				_logger->debug("nodeConnected, peer: {} never seen before, add to neighbor_db", peer);
				*/
				_logger->debug("nodeConnected, peer: {} never seen before, add to neighbor_db", printHrpNode("",peer));
				// End Ala

				/// This is a new neighbor, we need to create a new entry;
		
				auto entry_itr = _neighbors_db.insert(std::pair<hrpNode, db_node_entry>(peer, db_node_entry()));
				db_node_entry &entry = entry_itr.first->second;
				entry._nid = peer;
				entry._mve_core = new MVECore(core);
				entry._bloom_filter = new bloom_filter(filter);
				entry._pi_rm_core = new RMCore(_rmax, _ita);
				entry._pi_learner = new PILearner(*entry._pi_rm_core, _alpha);
				entry._fi_rm_core = new RMCore(2*_rmax, _ita);
				entry._fi_learner = new FILearner(*entry._fi_rm_core, _alpha, _mveCore, _mutex_mveCore, &entry._mve_core, *entry._mutex_mve);
				entry._ct_duration = std::chrono::duration<double>(0);
				entry._start_time = std::chrono::steady_clock::now();
				entry._last_ct_time = std::chrono::steady_clock::now();
				entry._pi_seq = 0;
				entry.isConnected = true;
				//_neighbors_db.insert(std::pair<hrpNode, db_node_entry>(peer, entry));
			}
		}	

		// Now we need to update this node's MVECore
		_mveEstimator.addNode(peer);

		// Now we update the FI learners
		updateFILearners();
	}

	
	
	void HrpCore::nodeDisconnected(const hrpNode &peer)
	{
		// Commented and Replaced by Ala
		/*
		_logger->info("nodeDisconnected, peer: {} disconnected", peer);
		*/	
		_logger->info("nodeDisconnected, peer: {} disconnected", printHrpNode("",peer));
		// End Ala

		std::lock_guard<std::mutex> lg(_mutex_ngbr_db);
		if (_neighbors_db.find(peer) != _neighbors_db.end())
		{
			auto itr = _neighbors_db.find(peer);
			db_node_entry &entry = itr->second;
			// Update the total contact duration
			entry._ct_duration += (std::chrono::steady_clock::now() - entry._last_ct_time);
			// Set isConnected to false
			entry.isConnected = false;
		}
		else
		{
			// the node is not found, this is an error
			// Commented and Replaced by Ala
			/*	
			_logger->error("ERROR: nodeDisconnected, peer {} not found in _neighbor_d", peer);
			*/
			_logger->error("ERROR: nodeDisconnected, peer {} not found in _neighbor_d", printHrpNode("",peer));
			// End Ala
		}
	}

	void HrpCore::updateFILearners()
	{
		std::lock_guard<std::mutex> lg(_mutex_ngbr_db);
		// _logger->info("updateFILearners, self mve core: {}", _mveCore.format_string());
		for (auto itr : _neighbors_db)
		{
			auto &entry = itr.second;
			entry._fi_learner->updateCore();
			// _logger->info("updateFILearners, to node {}, probs = {}, delays = {}, mve_core = {}", itr.first, entry._fi_learner->format_probs(), delays, entry._mve_core->format_string());
		}
	}

	
	void HrpCore::set_engine(TxrxEngine *_engine)
	{
		HrpCore::_engine = _engine;
	}

	
	const hrpNode &HrpCore::get_thisNode() const
	{
		return _thisNode;
	}

	const std::map<hrpNode, db_node_entry> &HrpCore::get_neighbors_db() const
	{	
		return _neighbors_db;
	}
}
