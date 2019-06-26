//
// Cretaed by Chen Yang and Ala Altaweel
//

#include "MetaDataExchanger.h"

namespace hrp
{

	void MetaDataExchanger::init()
	{

		//check if TxrxEngine is initialized
		if (_engine == NULL)
		{
			_logger->error("ERROR in init(), TxrxEngine is not set");
			exit(1);
		}

		if (_hrp_core == NULL)
		{
			_logger->error("ERROR in init(), HrpCore is not set");
			exit(1);
		}

		// register handlers
		_engine->registerHandler(meta_request, std::bind(&MetaDataExchanger::rxReceived, this, std::placeholders::_1));
		_engine->registerHandler(meta_response, std::bind(&MetaDataExchanger::rxReceived, this, std::placeholders::_1));

		// schedule timers
		_request_timer = new asio::steady_timer(_engine->get_io_service());
		if (_request_timer == NULL)
		{
			_logger->error("ERROR in init(), cannot initialize _request_timer");
			exit(1);
		}

		scheduleMveRequestTimer();
	}


	void MetaDataExchanger::nodeAvailable(const hrpNode &n)
	{
		std::lock_guard<std::mutex> lg(_mutex_nodes);
		auto itr = _connected_nodes.find(n);
		if (itr == _connected_nodes.end())
		{
			itr = _discovered_nodes.find(n);
			if (itr == _discovered_nodes.end())
			{
				_discovered_nodes.insert(n);
				//scheduleMveRequest(n);
			}
		}
	}


	void MetaDataExchanger::nodeUnavailable(const hrpNode &n)
	{
		std::lock_guard<std::mutex> lg(_mutex_nodes);
		auto itr = _discovered_nodes.find(n);
		if (itr == _discovered_nodes.end())
		{
			itr = _connected_nodes.find(n);
			if (itr != _connected_nodes.end())
			{
				_hrp_core->nodeDisconnected(n);
				_router->nodeDisconnected(n);
				_connected_nodes.erase(itr);
			}
		}
		else
		{
			_discovered_nodes.erase(itr);
		}
	}

	
	void MetaDataExchanger::scheduleMveRequest(const hrpNode &peer)
	{
		// Commented and Replaced by Ala
		/*
		_logger->debug("MetaDataExchanger::scheduleMveRequest, schedule mve request to {}", peer);
		*/
		//_logger->debug("MetaDataExchanger::scheduleMveRequest, schedule mve request to peer {}", printHrpNode("",peer));
		// End Ala
	
		HrpDataHeader header(_hrp_core->get_thisNode(), peer, 1, meta_request);
	

		// Added by Ala
		/*
		hrpNode nextCarrier1 = header.get_next_carrier();
		hrpNode dst1 = header.get_dst();
		_logger->info(printHrpNode("MetaDataExchanger::scheduleMveRequest nextCarrier from header", nextCarrier1));
		_logger->info(printHrpNode("MetaDataExchanger::scheduleMveRequest dst from header", dst1));
		*/
		// End Ala
	

		std::shared_ptr<HrpDataHeader> header_ptr = std::make_shared<HrpDataHeader>(header);
	

		// Added by Ala
		/*
		hrpNode nextCarrier = header_ptr->get_next_carrier();
		hrpNode dst = header_ptr->get_dst();
		_logger->debug(printHrpNode("MetaDataExchanger::scheduleMveRequest nextCarrier from header_ptr", nextCarrier));
		_logger->debug(printHrpNode("MetaDataExchanger::scheduleMveRequest dst from header_ptr", dst));
		*/
		// End Ala

		_engine->scheduleTransfer(header_ptr, std::bind(&MetaDataExchanger::TxrxEngineCallback, this, std::placeholders::_1, std::placeholders::_2));

		//_logger->debug("MetaDataExchanger::scheduleMveRequest after calling scheduleTransfer()");
	}

	void MetaDataExchanger::scheduleMveRequestTimer()
	{
		_request_timer->expires_from_now(std::chrono::seconds((long long)2));
		_request_timer->async_wait(std::bind(&MetaDataExchanger::scheduleMveRequestForDiscovered, this));
	}

	
	void MetaDataExchanger::scheduleMveRequestForDiscovered()
	{
		std::lock_guard<std::mutex> lg(_mutex_nodes);
		for (const hrpNode &node : _discovered_nodes)
			scheduleMveRequest(node);
		scheduleMveRequestTimer();
	}

	
	void MetaDataExchanger::rxReceived(std::shared_ptr<HrpDataHeader> header)
	{
		switch (header->get_type())
		{
			case meta_request:
				// Commented and Replaced by Ala
				/*
				_logger->debug("rxReceived, receive mve request from {}", header->get_src());
				*/
				//_logger->debug("rxReceived, receive mve request from {}", printHrpNode("",header->get_src()));
				// End Ala
				processMveRequest(header);
			break;
			case meta_response:
				// Commented and Replaced by Ala
				/*
				_logger->debug("rxReceived, receive mve response from ", header->get_src());
				*/
				//_logger->debug("MetaDataExchanger::rxReceived, receive mve response from {}", printHrpNode("",header->get_src()));
				// End Ala
				processMveResponse(header);
			break;
			default:
			break;
		}
	}


	void MetaDataExchanger::processMveRequest(std::shared_ptr<HrpDataHeader> request)
	{

		try
		{
			//_logger->debug("processMveRequest starts");
			hrpNode peer = request->get_src_by_guid();
			std::string buf;
			HrpMveCoreMessage mveCoreMessage(_hrp_core->get_mveCore());
			HrpBloomFilterMessage filterMessage(_router->getFilter());
			hrp_message::HrpMetaData metaData;
			metaData.set_allocated_bloomfilter(&filterMessage.get_proto_msg());
			metaData.set_allocated_mvecore(&mveCoreMessage.get_proto_msg());
			metaData.SerializeToString(&buf);
			metaData.release_bloomfilter();
			metaData.release_mvecore();
	
			HrpDataHeader response(_hrp_core->get_thisNode(), peer, 1, meta_response);
			response.set_payload(buf.c_str(), buf.size() * sizeof(char));
	
			std::shared_ptr<HrpDataHeader> response_ptr = std::make_shared<HrpDataHeader>(response);
			// Commented and Replaced by Ala
			/*
			_logger->debug("processMveRequest, reply with mve response to {}", peer);
			*/
			//_logger->debug("MetaDataExchanger::processMveRequest, reply with mve response to {}", printHrpNode("",peer));
			// End Ala
	
			_engine->scheduleTransfer(response_ptr, std::bind(&MetaDataExchanger::TxrxEngineCallback, this, std::placeholders::_1, std::placeholders::_2));
		}
		catch (hrpException &e)
		{
			_logger->error("MetaDataExchanger::processMveRequest cath exception: {}", e.what());
			_logger->info("skipping the exception");
		}
		catch (...)
		{
			throw;
		}

	}

	
	void MetaDataExchanger::processMveResponse(std::shared_ptr<HrpDataHeader> response)
	{

		try
		{
			//_logger->debug("processMveResponse starts");
			hrpNode peer = response->get_src_by_guid();
			std::lock_guard<std::mutex> lg(_mutex_nodes);
			auto itr = _connected_nodes.find(peer);
			if (itr != _connected_nodes.end())
			{
				// We already have this hrpNode in the connected set, what should we do here?
				// Commented and Replaced by Ala
				/*
				_logger->debug("processMveResponse, receive MveResponse from already connected node {}, ignored ", peer);
				*/
				//_logger->debug("processMveResponse, receive MveResponse from already connected node {}, ignored ", printHrpNode("",peer));
				// End Ala
	
			}
			else
			{
				itr = _discovered_nodes.find(peer);
				if (itr != _discovered_nodes.end())
				{
					// this hrpNode was in _discovered_nodes set, we should promote it
					hrp_message::HrpMetaData metaData;
					metaData.ParseFromArray(response->get_payload(), response->get_payload_size());
					HrpMveCoreMessage mveCoreMessage(peer, metaData.mvecore());
					HrpBloomFilterMessage filterMessage(metaData.bloomfilter());
	
					// Commented and Replaced by Ala
					/*
					MVECore mveCore("tmp");
					*/
					hrpNode hrpNodeObj;
					GnsServiceClient gnsObj;
					string GUID =  gnsObj.getOwnGUID();	
					if(GUID.size() == 0)
					{
						throw hrpException("processMveResponse. Invalid hrpNode with no own GUID .. skipping!");				
					}

					hrpNodeObj.push_back(GUID);
					MVECore mveCore(hrpNodeObj);
					// End Ala
	
					mveCoreMessage.inflate_mvecore(mveCore);
	
					// promote the peer to _connected
					_discovered_nodes.erase(peer);
					_connected_nodes.insert(peer);
	
					// notify HrpCore
					//_logger->debug("processMveResponse, notify hrpCore that a node is connected");
					_hrp_core->nodeConnected(peer, mveCore, filterMessage);
					_router->nodeConnected(peer);
	
				}
				else
				{
					// this hrpNode was not even in the _discovered_nodes set, something is wrong
					// just ignore it for now
					// Commented and Replaced by Ala
					/*
					_logger->debug("processMveResponse, receive MveResponse from non-discovered node {}, ignored ", peer);
					*/
					_logger->debug("processMveResponse, receive MveResponse from non-discovered node {}, ignored ", printHrpNode("",peer));
					// End Ala
				}
			}
		}
		catch (hrpException &e)
		{
			_logger->error("MetaDataExchanger::processMveResponse cath exception: {}", e.what());
			_logger->info("skipping the exception");
		}
		catch (...)
		{
			throw;
		}

	}
	
		
	void MetaDataExchanger::TxrxEngineCallback(std::shared_ptr<HrpDataHeader> pkt, bool success)
	{
		// Added by Ala

		if(!success)
		{
		 	
			//_logger->error("error in TxrxEngineCallback");
			return;
		}
		else
		{
			//_logger->debug("successfully called TxrxEngineCallback");
			;
		}

	}
}
