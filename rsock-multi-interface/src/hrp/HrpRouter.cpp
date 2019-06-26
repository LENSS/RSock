//
// Cretaed by Chen Yang and Ala Altaweel
//

#include "HrpRouter.h"

namespace hrp
{
	void HrpRouter::init()
	{

		// set up bloom filter
		bloom_parameters parameters;

		// How many elements roughly do we expect to insert?
		parameters.projected_element_count = 2000;

		// Maximum tolerable false positive probability? (0,1)
		parameters.false_positive_probability = 0.0001; // 1 in 10000

		// Simple randomizer (optional)
		parameters.random_seed = 0xA5A5A5A5;

		if (!parameters)
		{
			_logger->error("Error - Invalid set of bloom filter parameters!");
			exit(1);
		}

		parameters.compute_optimal_parameters();

		//Instantiate Bloom Filter
		filter = bloom_filter(parameters);

		// register packet reception callback
		_engine->registerHandler(hrp_packet_type::data, std::bind(&HrpRouter::rxReceived, this, std::placeholders::_1));
		_engine->registerHandler(hrp_packet_type::data_ack, std::bind(&HrpRouter::rxReceived, this, std::placeholders::_1));

		_update_timer = new asio::steady_timer(_io_service);
		if (_update_timer == nullptr)
		{
			_logger->error("init, cannot initialize _update_timer");
			exit(1);
		}

		scheduleUpdateTimer();

		// Added by Ala to send buffered packet frequently
		
		_checkBuffer_timer = new asio::steady_timer(_io_service);
		if (_checkBuffer_timer == nullptr)
		{
			_logger->error("init, cannot initialize _checkBuffer_timer");
			exit(1);
		}


		scheduleclearBufferMsgsTimer();
		
		// End by Ala
	}

	void HrpRouter::update(const std::error_code &error)
	{
		if (!error)
		{
			std::lock_guard<std::mutex> lg(_mutex_buffer);
			std::chrono::time_point<std::chrono::system_clock> point = std::chrono::system_clock::now();
			long now_time = s_from_epoch();
			for (auto itr = _buffer.begin(); itr != _buffer.end(); )
			{
				auto &pkt = itr->second.pkt;
				long gen_time = pkt->get_gen_time();
				long ttl = pkt->get_ttl();
				// first purge outdated messages
				if (now_time - gen_time > ttl)
				{
					_logger->info("update, message {} dropped due to ttl", pkt->get_hash());

					_logger->debug("now_time: {} ", now_time);
					_logger->debug("gen_time: {}", gen_time);
					_logger->debug("ttl: {} ", ttl);
					itr = _buffer.erase(itr);
				}
				else
				{
					if (itr->second.status == outbound || itr->second.status == scheduled)					
					{
						// otherwise, if the packet is outbound for too long, rebuffer it						
						std::chrono::duration<double> duration = point - itr->second.outbound_t;
						// if packet has been outbound for 60 seconds with no ack back						
						if (duration.count() > 5)
						{
							//_logger->debug("update, message {} move back to buffered state from outbound", pkt->get_hash());
							
							// Added by Ala for logging
							//_logger->debug("update, duration.count(): {}", duration.count());
							if (itr->second.status == outbound)
								_logger->debug("update, message {} move back to buffered state from outbound", pkt->get_hash());
							else
								_logger->debug("update, message {} move back to buffered state from scheduled", pkt->get_hash());
			
							// End by Ala
							itr->second.status = buffered;
						}
					}
					++itr;
				}
			}
			scheduleUpdateTimer();
		}
		else
		{
			_logger->error("update, called with error {}", error.message());
		}
	}

	void HrpRouter::scheduleUpdateTimer()
	{
		// call update function each 1 seconds
		_update_timer->expires_from_now(std::chrono::seconds((long long)1));
		_update_timer->async_wait(std::bind(&HrpRouter::update, this, std::placeholders::_1));
	}


	// Added by Ala to send buffered packet frequently
	void HrpRouter::scheduleclearBufferMsgsTimer()
	{
		_checkBuffer_timer->expires_from_now(std::chrono::seconds((long long)11));
		_checkBuffer_timer->async_wait(std::bind(&HrpRouter::clearBufferMsgs, this, std::placeholders::_1));
	}

	void HrpRouter::clearBufferMsgs(const std::error_code &error)
	{
		if (!error)
		{
			tryBufferedMessages();
			scheduleclearBufferMsgsTimer();
		}
		else
		{
			_logger->error("clearBufferMsgs(), called with error {}", error.message());
		}
	}
	

	void HrpRouter::tryBufferedMessages()
	{
		//_logger->debug("tryBufferedMessages()");
		//_logger->debug("tryBufferedMessages() buffer has: {} messages", _buffer.size());
		tryAllMessages();
	}
	// End by Ala

	std::shared_ptr<HrpDataHeader> HrpRouter::newMessage(void *buf, size_t size, const hrpNode &dst, const std::string &app, unsigned long ttl)
	{

		//_logger->debug("HrpRouter::newMessage to dst={}", printHrpNode("", dst));
		HrpDataHeader pkt(_core->get_thisNode(), dst, 1, data, _send_seq++);
		pkt.set_app(app);
		pkt.set_remain_repl(_core->drawReplFactor(dst));
		pkt.set_payload(buf, size);
		pkt.set_ttl(ttl);
		std::shared_ptr<HrpDataHeader> pkt_ptr = std::make_shared<HrpDataHeader>(pkt);
		std::lock_guard<std::mutex> lg(_mutex_buffer);
		addMessageToBuffer(pkt_ptr);
		return pkt_ptr;
	}

	void HrpRouter::addMessageToBuffer(std::shared_ptr<HrpDataHeader> pkt)
	{
		HrpBufEntry entry;
		entry.pkt = pkt;
		entry.status = buffered;
		entry.outbound_t = std::chrono::time_point<std::chrono::system_clock>::max();
		_buffer.insert(std::pair<std::string, HrpBufEntry>(pkt->get_hash(), entry));
		filter.insert(pkt->get_hash());
		_logger->debug("HrpRouter::addMessageToBuffer new message added: {}, repl = {}", pkt->get_hash(), pkt->get_remain_repl());
		tryMessage(pkt);
	}


	

	void HrpRouter::tryAllMessages()
	{
		_logger->debug("tryAllMessages()");
		_logger->debug("tryAllMessages() buffer has: {} messages", _buffer.size());
		std::lock_guard<std::mutex> lg(_mutex_buffer);
		for (auto itr : _buffer)
		{
			auto &pkt = itr.second.pkt;
			tryMessage(pkt);
		}
	}

		
	bool HrpRouter::tryMessage(std::shared_ptr<HrpDataHeader> pkt)
	{
		//_logger->debug("HrpRouter::tryMessage()");
		auto nextHop = _core->calcRoutingDecision(pkt);
		if (nextHop != nullptr)
		{
			auto &entry = _buffer.find(pkt->get_hash())->second;
			entry.status = scheduled;

			pkt->set_current_carrier_guid(_core->get_thisNode());
			pkt->set_next_carrier_guid(*nextHop);
			_engine->scheduleTransfer(pkt, std::bind(&HrpRouter::TxrxEngineCallback, this, std::placeholders::_1, std::placeholders::_2));
			return true;
		}
		else
			_logger->debug("HrpRouter::tryMessage() calcRoutingDecision return false for nextHop");
		return false;
	}

	void HrpRouter::TxrxEngineCallback(std::shared_ptr<HrpDataHeader> pkt, bool success)
	{
		if (success)
			txStarted(*pkt);
		else
			txAborted(*pkt);
	}

	
	void HrpRouter::txStarted(HrpDataHeader &header)
	{
		//_logger->debug("HrpRouter::txStarted");
		std::lock_guard<std::mutex> lg(_mutex_buffer);
		const auto &hash = header.get_hash();
		auto itr = _buffer.find(hash);
		if (itr != _buffer.end())
		{
			//_logger->debug("txStarted, a packet {} is transferred", hash);
			auto &entry = itr->second;
			if (entry.status == scheduled)
			{
				entry.status = outbound;
				entry.outbound_t = std::chrono::system_clock::now();
			}
			else
				_logger->debug("txStarted, packet status is not scheduled, but {}", entry.status);
		}
		else
			_logger->debug("txStarted, a packet is transferred but no longer in buffer");
	}

	void HrpRouter::txCompleted(HrpDataHeader &header)
	{
	}


	void HrpRouter::txAborted(HrpDataHeader &header)
	{
		//_logger->debug("HrpRouter::txAborted");
		std::lock_guard<std::mutex> lg(_mutex_buffer);
		auto hash = header.get_hash();
		auto itr = _buffer.find(hash);
		if (itr != _buffer.end())
		{
			_logger->debug("txAborted, a packet {} is aborted", hash);
			auto &entry = itr->second;
			if (entry.status == scheduled)
			{
				entry.status = buffered;
			}
			else
				_logger->debug("txAborted, packet status is not scheduled, but {}", entry.status);
		}
		else
			_logger->debug("txAborted, a packet is aborted but no longer in buffer");
	}


	void HrpRouter::rxReceived(std::shared_ptr<HrpDataHeader> header)
	{
		//_logger->debug("HrpRouter::rxReceived");
		std::lock_guard<std::mutex> lg(_mutex_buffer);
		if (header->get_type() == data)
		{
			processData(header);
		}
		else if (header->get_type() == data_ack)
		{
			processDataAck(header);
		}
		else
			_logger->error("rxReceived received unknown packet type {}", header->get_type());
	}

	void HrpRouter::processData(std::shared_ptr<HrpDataHeader> header)
	{
		
		try
		{

			//_logger->debug("HrpRouter::processData");
			auto hash = header->get_hash();
			auto itr = _buffer.find(hash);
			if (itr == _buffer.end() && !filter.contains(header->get_hash()))
			{
	
				// Commented and Replaced by Ala
				/*
				_logger->info("rxReceived new data packet received, id={}, repl={}, last_carrier={}, seqnum={}", hash, header->get_remain_repl(), header->get_current_carrier(),header->get_seqnum());
				*/
	
				//_logger->debug("rxReceived new data packet received, id={}, repl={}, last_carrier={}, seqnum={}", hash, header->get_remain_repl(), printHrpNode("",header->get_current_carrier_by_guid()),header->get_seqnum());
				
				//_logger->debug("rxReceived new data packet received, id = {}", hash);
				
				// End Ala
			
				// send back ack
				//_logger->debug("Before building the ack");
				HrpDataHeader ack(header->get_src_by_guid(), header->get_dst_by_guid(), 1, data_ack, header->get_seqnum());
				//_logger->debug("After building the ack");
				ack.set_current_carrier_guid(_core->get_thisNode());
				ack.set_next_carrier_guid(header->get_current_carrier_by_guid());
				std::shared_ptr<HrpDataHeader> ack_ptr = std::make_shared<HrpDataHeader>(ack);
				_engine->scheduleTransfer(ack_ptr, [](std::shared_ptr<HrpDataHeader> pkt, bool success){});
		
				// modify the packet
				header->set_current_carrier_guid(_core->get_thisNode());
		
		
				// Commented and Replaced by Ala
				/*
				//header->set_next_carrier("0.0.0.0");
				*/
	
				// set current node GUID
				
				// create an object of gns client
				GnsServiceClient gnsObj;
				// get GUID by IP
				string ownGUID =  gnsObj.getOwnGUID();
				hrpNode hrpNodeObj;
				if(ownGUID.size() != 0)			
				{	
					hrpNodeObj.push_back(ownGUID);
				}
				else
				{
					_logger->error("no GUID returned from getOwnGUID() function");			
				}
				header->set_next_carrier_guid(hrpNodeObj);
				
				// End Ala
			
		
				header->add_carriers(_core->get_thisNode());
				if (header->get_remain_repl() > 1)
					header->set_remain_repl(header->get_remain_repl()-1);
				//_logger->debug("before getting the dest");
				if (header->get_dst_by_guid() == _core->get_thisNode())
				{
					// if we are the destination, added it to the filter first, then call api layer
					_logger->debug("rxReceived new data packet received for me, id = {}", hash);
					filter.insert(header->get_hash());
					_recv_cb(header);
				}
				else
				{
					// Add to buffer
					_logger->debug("rxReceived new data packet received for node = {}, id= {}", printHrpNode("",header->get_dst_by_guid()), hash);
					addMessageToBuffer(header);
				}
			}
			else
			{
				_logger->debug("rxReceived received duplicate data packet {}", hash);
				// Added by Ala to send back ack
				/*
				HrpDataHeader ack(header->get_src_by_guid(), header->get_dst_by_guid(), 1, data_ack, header->get_seqnum());
				ack.set_current_carrier_guid(_core->get_thisNode());
				ack.set_next_carrier_guid(header->get_current_carrier_by_guid());
				std::shared_ptr<HrpDataHeader> ack_ptr = std::make_shared<HrpDataHeader>(ack);
				_engine->scheduleTransfer(ack_ptr, [](std::shared_ptr<HrpDataHeader> pkt, bool success){});
				*/
				// End Ala
			}
		}
		catch (hrpException &e)
		{
			_logger->error("HrpRouter::processData cath exception: {}", e.what());
			_logger->info("skipping the exception");
		}
		catch (...)
		{
			throw;
		}

	}	


	void HrpRouter::processDataAck(std::shared_ptr<HrpDataHeader> header)
	{

		try
		{
			auto hash = header->get_hash();
			auto itr = _buffer.find(hash);
			long now = s_from_epoch();
	
	        	if (itr != _buffer.end())
			{
				auto &entry = itr->second;
				if (entry.status == outbound)
				{
					long rtt = now - entry.pkt->get_gen_time();
					_logger->info("rxReceived received data_ack for packet {}, rtt = {}", hash, rtt);
					auto &pkt = entry.pkt;
					if (pkt->get_dst_by_guid() == pkt->get_next_carrier_by_guid())
					{
						//_logger->info("data packet {} succesfully delivered, delay = {} ms", hash, rtt/2);
						_buffer.erase(itr);
						_logger->info("data packet {} removed from buffer since it's received", hash);
					}	
					else
					{
	
						// Commented and Replaced by Ala
						/*
						_logger->info("data packet {} replicated to next carrier {}", hash, pkt->get_next_carrier());
						*/
						//_logger->debug("data packet {} replicated to next carrier {}", hash, printHrpNode("",pkt->get_next_carrier_by_guid()));
						// End Ala
	
 						if (pkt->get_remain_repl() == 1)
						{
							_buffer.erase(itr);
							_logger->debug("data packet {} removed from buffer since it's forwarded", hash);
						}
						else
						{

							pkt->add_carriers(pkt->get_next_carrier_by_guid());
							pkt->set_remain_repl(1);
							entry.status = buffered;
							entry.outbound_t = std::chrono::time_point<std::chrono::system_clock>::max();
							_logger->debug("data packet {} remain repl set to 1 and outbound_t to max", hash);
						}
					}
				}
				else
					_logger->debug("rxReceived received data_ack for packet {}, but pkt not in outbound status", hash);
			}
			else
				_logger->debug("rxReceived data_ack for {}, but no packet found in buffer", hash);
		}
		catch (hrpException &e)
		{
			_logger->error("HrpRouter::processDataAck cath exception: {}", e.what());
			_logger->info("skipping the exception");
		}
		catch (...)
		{
			throw;
		}
	
	}
	
	void HrpRouter::nodeConnected(const hrpNode &n)
	{
		tryAllMessages();
	}

	void HrpRouter::nodeDisconnected(const hrpNode &n)
	{
	}

	const bloom_filter &HrpRouter::getFilter() const
	{
		return filter;
	}
}
