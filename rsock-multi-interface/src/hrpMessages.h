//
// Cretaed by Chen Yang and Ala Altaweel
//

#ifndef HRP_HRPMESSAGES_H
#define HRP_HRPMESSAGES_H

#include <mve/MVECore.h>
#include "common.h"
#include "hrpNode.h"
#include "hrp/HrpGraph.h"
#include "proto/HrpMsg.pb.h"
#include "bloom/bloom_filter.hpp"

namespace hrp
{
	enum hrp_packet_type
	{
		pi_probe_request = 0,
		pi_probe_response = 1,
		meta_request = 3,
		meta_response = 4,
		data = 5,
		data_ack = 6
	};

	/**
	* Probe message header metadata, for evaluating a particular repl. factor
	*/

	typedef struct
	{
		unsigned int seq;   /// sequence number
		unsigned short repl;  /// replication factor
	}pi_probe_meta;

	/**
	* Probe message response received from the peer.
	*/
	typedef struct
	{
		unsigned int seq;   /// sequence number
		unsigned short repl;  /// replication factor
	}pi_response_tx;

	/**
	* Probe message response received from the peer.
	*/
	typedef struct {
		pi_response_tx meta;
		double latency;
	}pi_response_local;

	/**
	* Main class for HRP message header
	*/
	class HrpDataHeader
	{
		public:

		/// Constructor
		HrpDataHeader(const hrpNode &src, const hrpNode &dst, unsigned int repl, hrp_packet_type type)
		{
			// Added by Ala
			/*
			_logger = std::make_shared<spdlog::logger>("HrpDataHeader", file_sink);
			_logger->info("HrpDataHeader::Constructor 4 inputs");
			_logger->info(printHrpNode("HrpDataHeader::Constructor node_to_ip for src", src));
			_logger->info(printHrpNode("HrpDataHeader::Constructor node_to_ip for dst", dst));
			*/
			// End Ala

			_proto_msg.set_src_guid(src.at(0));
			//_proto_msg.set_dst(node_to_ip(dst));
			_proto_msg.set_remain_repl(repl);
			_proto_msg.set_type(type);
			// Added by Ala
			_proto_msg.set_dst_guid(dst.at(0));
			// End Ala
			
		}

		HrpDataHeader(const hrpNode &src, const hrpNode &dst, unsigned int repl, hrp_packet_type type, unsigned int seq)
		{
	
			//int src_ip = node_to_ip(src);
			//_proto_msg.set_src(node_to_ip(src));
			_proto_msg.set_src_guid(src.at(0));
			//_proto_msg.set_dst(node_to_ip(dst));
			_proto_msg.set_remain_repl(repl);
			_proto_msg.set_type(type);
			//_proto_msg.set_current_carrier(src_ip);
			_proto_msg.set_current_carrier_guid(src.at(0));
			//_proto_msg.set_next_carrier(node_to_ip(dst));
			_proto_msg.set_next_carrier_guid(dst.at(0));
			_proto_msg.set_ttl(HRP_DEFAULT_PKT_TTL);
			_proto_msg.set_seqnum(seq);
			_proto_msg.set_gen_time(s_from_epoch());

			
			//int64_t hash = src_ip;
			// get the hash of the 40 chars of guid
			int64_t hash = guid_to_unint64(src.at(0));
			hash = (hash << 32) | seq;
			_proto_msg.set_hash(hash);
			// Added by Ala
			_proto_msg.set_dst_guid(dst.at(0));
			// End Ala
		}

		/// Constructor
		explicit HrpDataHeader(const std::string &data)
		{
			_proto_msg.ParseFromString(data);
		}
	
		/// Copy constructor
		HrpDataHeader(const HrpDataHeader &header) : _carriers(header._carriers)
		{
			_proto_msg.CopyFrom(header._proto_msg);
		}

		bool SerializeToString(std::string* output) const
		{
			return _proto_msg.SerializeToString(output);
		}

		bool ParseFromString(const std::string& data)
		{
			return _proto_msg.ParseFromString(data);
		}

		/// Utility methods
		// Commented and Replaced by Ala
		/*
		void add_carriers(const hrpNode &new_carrier)
		{
			if (_carriers.insert(new_carrier).second)
				_proto_msg.add_carriers(node_to_ip(new_carrier));
		}
		*/
		void add_carriers(const hrpNode &new_carrier)
		{
			if (_carriers.insert(new_carrier).second)
				_proto_msg.add_carriers(new_carrier.at(0));
		}


		// End Ala
		/// Setters
		// Commented and Replaced by Ala
		/*
		void set_current_carrier(const hrpNode &_current_carrier)
		{
			_proto_msg.set_current_carrier(node_to_ip(_current_carrier));
		}
		*/
		void set_current_carrier_guid(const hrpNode &_current_carrier)
		{


			if(_current_carrier.at(0).size() == 0)
			{
				cout<<printHrpNode("node: ",_current_carrier)<<endl;	
				throw hrpException("set_current_carrier_guid. Invalid hrpNode with no GUID .. skipping!");				
			}
			_proto_msg.set_current_carrier_guid(_current_carrier.at(0));
		}
		// End Ala	


		// Commented and Replaced by Ala
		/*
		
		void set_next_carrier(const hrpNode &_next_carrier)
		{
			_proto_msg.set_next_carrier(node_to_ip(_next_carrier));
		}
		*/

		void set_next_carrier_guid(const hrpNode &_next_carrier)
		{
			//_proto_msg.set_next_carrier(node_to_ip(_next_carrier));
			if(_next_carrier.at(0).size() == 0)
			{
				cout<<printHrpNode("node: ",_next_carrier)<<endl;	
				throw hrpException("set_next_carrier_guid. Invalid hrpNode with no GUID .. skipping!");				
			}
			_proto_msg.set_next_carrier_guid(_next_carrier.at(0));


		}

		// End Ala	

		void set_remain_repl(unsigned int _remain_repl)
		{
			_proto_msg.set_remain_repl(_remain_repl);
		}

		void set_payload(const void *_pkt, size_t size)
		{
			_proto_msg.set_payload(_pkt, size);
		}

		// Commented and Replaced by Ala
		/*
		void set_routes(const HrpGraph::succMap &routes)
		{
			auto m = _proto_msg.mutable_routes();
			for (auto itr : routes)
			{
				hrp_message::HrpPacket::NextHop hops;
				for (const hrpNode &hop : itr.second)
					hops.add_hops(node_to_ip(hop));
				(*m)[node_to_ip(itr.first)] = hops;
			}
		}
		*/

		void set_routes(const HrpGraph::succMap &routes)
		{
			auto m = _proto_msg.mutable_routes();
			for (auto itr : routes)
			{
				hrp_message::HrpPacket::NextHop hops;
				for (const hrpNode &hop : itr.second)
					hops.add_hops(hop.at(0));
				(*m)[itr.first.at(0)] = hops;
			}
		}
		// End Ala
		void set_ttl(int ttl)
		{
			_proto_msg.set_ttl(ttl);
		}

		void set_seqnum(unsigned long seqnum)
		{
			_proto_msg.set_seqnum(seqnum);
		}

		void set_app(const std::string& app)
		{
			_proto_msg.set_app(app);
		}


		/// Hash
		std::string get_hash() const
		{
			int64_t hash = _proto_msg.hash();
			int src_ip = ((hash >> 32) & 0xFFFFFFFF);
			unsigned int seq = hash & 0xFFFFFFFF;
			return std::to_string(src_ip) + "-" + std::to_string(seq);
		}
	
		/// Getters
		// Commented and Replaced by Ala
		/*
		hrpNode get_src() const
		{
			// Commented and Replaced by Ala
			//
			//return ip_to_node(_proto_msg.src());
			//
			hrpNode node = ip_to_node(_proto_msg.src());
			if(!isHrpNodeValid(node))
			{				
				cout<<printHrpNode("node: ",node)<<endl;
				throw hrpException("HrpDataHeader::get_src(). Invalid hrpNode .. skipping!");	//				
			}
			return node;				
			
		}
		*/
		hrpNode get_src_by_guid() const
		{

			hrpNode node;
			string GUID = _proto_msg.src_guid();
			if(GUID.size() == 0)
			{
				cout<<printHrpNode("node: ",node)<<endl;	
				throw hrpException("get_src_by_guid. Invalid hrpNode with no GUID .. skipping!");				
			}
			node.push_back(GUID);
			return node;				
		}

	
		// Commented and Replaced by Ala
		/*
		hrpNode get_dst_byIP() const
		{
			// Commented and Replaced by Ala
			//
			//return ip_to_node(_proto_msg.dst());
			//
			hrpNode node = ip_to_node(_proto_msg.dst());
			if(!isHrpNodeValid(node))
			{
				cout<<printHrpNode("node: ",node)<<endl;	
				throw hrpException("HrpDataHeader::get_dst_byIP(). Invalid hrpNode .. skipping!");				
			}
			return node;				
		}
		// End Ala
		*/

		// this function can be called after a node is connected since its IP(s) might be changed
		hrpNode get_dst_by_guid() const
		{			
			hrpNode node;
			string GUID = _proto_msg.dst_guid();
			if(GUID.size() == 0)
			{
				cout<<printHrpNode("node: ",node)<<endl;	
				throw hrpException("get_dst_by_guid. Invalid hrpNode with no GUID .. skipping!");				
			}
			node.push_back(GUID);
			return node;				
		}


		// Commented and Replaced by Ala
		/*
		hrpNode get_current_carrier() const
		{
			// Commented and Replaced by Ala
			//
			//return ip_to_node(_proto_msg.current_carrier());
			//
			hrpNode node = ip_to_node(_proto_msg.current_carrier());
			if(!isHrpNodeValid(node))
			{
				cout<<printHrpNode("node: ",node)<<endl;	
				throw hrpException("HrpDataHeader::get_current_carrier(). Invalid hrpNode .. skipping!");				
			}
			return node;
			// End Ala	
	
		}
		*/
		hrpNode get_current_carrier_by_guid() const
		{
			hrpNode node;
			string GUID = _proto_msg.current_carrier_guid();

			if(GUID.size() == 0)
			{
				cout<<printHrpNode("node: ",node)<<endl;	
				throw hrpException("get_current_carrier_by_guid. Invalid hrpNode with no GUID .. skipping!");				
			}
			node.push_back(GUID);
			return node;		
			// End Ala	
	
		}

		// End Ala	
	

		// Commented and Replaced by Ala
		/*
        	hrpNode get_next_carrier() const
		{
			// Commented and Replaced by Ala
			//
			//return ip_to_node(_proto_msg.next_carrier());
			//
			hrpNode node = ip_to_node(_proto_msg.next_carrier());
			if(!isHrpNodeValid(node))
			{				
				cout<<printHrpNode("node: ",node)<<endl;	
				throw hrpException("HrpDataHeader::get_next_carrier(). Invalid hrpNode with no GUID .. skipping!");				
									
			}
			return node;
			// End Ala	
		}
		*/
        	hrpNode get_next_carrier_by_guid() const
		{
			hrpNode node;
			string GUID = _proto_msg.next_carrier_guid();
			if(GUID.size() == 0)
			{
				//cout<<"empty next carrier node. set myself as default"<<endl;	
				GnsServiceClient gnsObj;
				GUID =  gnsObj.getOwnGUID();				
			}
			node.push_back(GUID);
			return node;		
		}


		// End Ala	
		
		const ect_set &get_carriers() const
		{
			return _carriers;
		}
	
        	unsigned long get_ttl() const
		{
			return _proto_msg.ttl();
		}
	
		long get_gen_time() const
		{
			return _proto_msg.gen_time();
		}
	
		unsigned int get_remain_repl() const
		{
			return _proto_msg.remain_repl();
		}
	
		const void* get_payload() const
		{
			return _proto_msg.payload().c_str();
		}
	
		const hrp_packet_type get_type() const
		{
			return hrp_packet_type(_proto_msg.type());
		}
	
		const size_t get_payload_size()
		{
			return _proto_msg.payload().size();
		}
	

		// Commented and Replaced by Ala
		/*

		std::set<hrpNode> get_next_hop_for(const hrpNode &node)
		{
			std::set<hrpNode> ret;
			int node_raw_ip = node_to_ip(node);
			auto m = _proto_msg.routes();
			if (m.count(node_raw_ip) != 0)
			{
				hrp_message::HrpPacket::NextHop n = m[node_raw_ip];
				for (int i=0; i<n.hops_size(); i++)
					ret.insert(ip_to_node(n.hops(i)));
			}
	
			return ret;
		}
		*/


		std::set<hrpNode> get_next_hop_for(const hrpNode &node)
		{
			std::set<hrpNode> ret;
			auto m = _proto_msg.routes();
			if (m.count(node.at(0)) != 0)
			{
				hrp_message::HrpPacket::NextHop n = m[node.at(0)];
				for (int i=0; i<n.hops_size(); i++)
				{
					hrpNode node;
					string GUID = n.hops(i);
					if(GUID.size() == 0)
					{			
						cout<<printHrpNode("node: ",node)<<endl;	
						throw hrpException("get_next_hop_for. Invalid hrpNode with no GUID .. skipping!");				
					}
					ret.insert(node);
				}
			}
	
			return ret;
		}

		// End Ala        	
		unsigned long get_seqnum() const
		{
			return _proto_msg.seqnum();
		}
	
		std::string get_app() const
		{
			return _proto_msg.app();
		}

	
		private:
		/// Logger
		std::shared_ptr<spdlog::logger> _logger;
		ect_set _carriers;          /// All carriers that carry this message
		hrp_message::HrpPacket _proto_msg;
	};

	class HrpMveCoreMessage
	{
		public:
	        explicit HrpMveCoreMessage(const MVECore &core) : _thisNode(core.get_thisNode())
		{
			for (auto itr : core.get_params())
			{
				hrp_message::MveCoreMessage_EctSet *ectSet = _proto_msg.add_keys();
				for (auto n : itr.first)
				{
					// Commented and Replaced by by Ala
					/*
					ectSet->add_node(n);
					*/
					// Add the GUID of the node
					ectSet->add_node(n.at(0));
					// End Ala
				}
				double val = itr.second;
				_proto_msg.add_vals(val);
			}
		}

		HrpMveCoreMessage(const hrpNode &node, const void *buf, int size)
		{
			_proto_msg.ParseFromArray(buf, size);
			_thisNode = node;
		}
		
		HrpMveCoreMessage(const hrpNode &node, const hrp_message::MveCoreMessage &msg): _thisNode(node), _proto_msg(msg) {}

		/**
		* Fill the string buffer
		* @param str The buffer to fill
		*/
	        void inflate_payload(std::string *str)
		{
			_proto_msg.SerializeToString(str);
		}
		
		/**
		* Inflate the MVECore from the _proto_msg this object is holding
		* @param core Output parameter, the MVECore object to inflate
		*/

		void inflate_mvecore(MVECore &core)
		{
			core.set_thisNode(_thisNode);
			// create an object of gns client
			GnsServiceClient gnsObj;
			string GUID;
			for (int i=0; i<_proto_msg.keys_size(); i++)
			{
				const hrp_message::MveCoreMessage_EctSet &ectSetMsg = _proto_msg.keys(i);
				std::set<hrpNode> ectSet;
				for (int j=0; j<ectSetMsg.node_size(); j++)
				{
					// Commented and Replaced by by Ala
					/*
					ectSet.insert(ectSetMsg.node(j));
					*/
					// 
					// get GUID
					GUID = ectSetMsg.node(j);
					hrpNode hrpNodeObj;
		
					// get IPs for this GUID
					if(GUID.size() != 0)			
					{	
						hrpNodeObj.push_back(GUID);
					}
					else
					{
						cout<<"no GUID returned from getOwnGUID() function"<<endl;			
					}




					ectSet.insert(hrpNodeObj);			
					// End Ala
					
				}
				double val = _proto_msg.vals(i);
				core.updateParam(ectSet, val);
			}
		}
		
		const hrp_message::MveCoreMessage &get_proto_msg() const
		{
			return _proto_msg;
		}


		hrp_message::MveCoreMessage &get_proto_msg()
		{
			return _proto_msg;
		}
	
		private:
		hrp_message::MveCoreMessage _proto_msg;
		hrpNode _thisNode;
	};

	class HrpBloomFilterMessage : public bloom_filter
	{
		public:
		explicit HrpBloomFilterMessage(const std::string& buf);
		explicit HrpBloomFilterMessage(const hrp_message::BloomFilter &msg);
		HrpBloomFilterMessage(const bloom_filter& filter);
		HrpBloomFilterMessage(const HrpBloomFilterMessage& msg);
		void inflate_payload(std::string *str)
		{
			_proto_msg.SerializeToString(str);
		}

		const hrp_message::BloomFilter &get_proto_msg() const;
		hrp_message::BloomFilter &get_proto_msg()
		{
			return _proto_msg;
		}

		protected:
		void init_from_proto_msg();
	
		private:
		hrp_message::BloomFilter _proto_msg;
	};
}

#endif //HRP_HRPMESSAGES_H
