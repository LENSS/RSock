//
// Cretaed by Chen Yang and Ala Altaweel
//

#ifndef HRP_HRPAPIPACKET_H
#define HRP_HRPAPIPACKET_H

#include <string>
#include <cstring>
#include "hrpNode.h"

namespace hrp
{

	enum hrp_api_pkt_type
	{
		api_register,
		api_data,
		// Added by Ala to handle topology requests
		api_topology,
		api_undefined
		// End Ala
	
	};


	class HrpAPIPacket
	{

		public:
		HrpAPIPacket(hrp_api_pkt_type t, hrpNode n, int64_t tm, uint32_t sz, char* p):
		type(t), node(std::move(n)), time(tm), sz_of_payload(sz), payload(p)
		{	
		}
		hrp_api_pkt_type getType() const
		{
			return type;
		}

		const hrpNode &getNode() const
		{
			return node;
		}

		int64_t getTime() const
		{
			return time;
		}

		uint32_t getSz_of_payload() const
		{
			return sz_of_payload;
		}

		char *getPayload() const
		{
			return payload;
		}

		private:
		std::shared_ptr<spdlog::logger> _logger;
		hrp_api_pkt_type type;
		hrpNode node;
		int64_t time;
		uint32_t sz_of_payload;
		char*   payload;
	};


	class HrpAPIPacketHelper
	{
		public:
		static char* registerPacket(const std::string &name, uint32_t &out_size);
		static char* dataPacket(const hrpNode &node, int64_t time, uint32_t sz_of_payload, const char* payload, uint32_t &out_size);
		static HrpAPIPacket parseFromBuffer(const char* buf, uint64_t size);
		// Added by Ala
		static 	char * requestTopology(uint32_t &out_size);
		static char * replyTopologyPacket(const hrpNode &node, int64_t time, uint32_t sz_of_payload, const char *payload, uint32_t &out_size);
		// End Ala
		private:
		static char* helper(uint64_t sz_of_data, const hrpNode &node, int64_t time, uint32_t sz_of_payload, const char* payload, uint32_t &out_size);
	};
}



#endif //HRP_HRPAPIPACKET_H
