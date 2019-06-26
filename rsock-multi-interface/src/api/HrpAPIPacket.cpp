//
// Cretaed by Chen Yang and Ala Altaweel
//

#include <assert.h>
#include "HrpAPIPacket.h"
#include <hrpEexception.h>
#include "common.h"
namespace hrp
{


	// Added by Ala 
	char *HrpAPIPacketHelper::requestTopology(uint32_t &out_size)
	{
		
		uint64_t sz_of_data = sizeof(char) + 40*sizeof(char) + sizeof(int64_t) + sizeof(uint32_t);

		// create an object of gns client
		hrp::GnsServiceClient gnsObj;
		hrpNode hrpNodeObj;
		// get GUID by IP
		string ownGUID =  gnsObj.getOwnGUID();

		if(ownGUID.size() != 0)			
		{	
			hrpNodeObj.push_back(ownGUID);
			// get IPs for this GUID
			// No need to add IPs since we represtn the graph using GUIDs
			/*
			std::vector<std::string> IPAddresses = gnsObj.getIPbyGUID(ownGUID);
			if(IPAddresses.size() != 0)
				for(std::vector<std::string>::size_type i = 0; i < IPAddresses.size(); i++)
					hrpNodeObj.push_back(IPAddresses.at(i));
			else
			{
				cout<<" error: no IP addresses returned for GUID: " << ownGUID <<endl;	
			}
			*/
		}
		else
		{
			cout<<"no GUID returned from getOwnGUID() function"<<endl;			
		}

		auto *buf = helper(sz_of_data, hrpNodeObj, 0, 0, "", out_size);
		// t for requesting the topology	
		buf[sizeof(uint64_t)] = 't';
		return buf;
	}


	char *HrpAPIPacketHelper::replyTopologyPacket(const hrpNode &node, int64_t time, uint32_t sz_of_payload, const char *payload, uint32_t &out_size)
	{
		uint64_t sz_of_data = sizeof(char) + 40*sizeof(char) + sizeof(int64_t) + sizeof(uint32_t) + sz_of_payload;
		auto *buf = helper(sz_of_data, node, time, sz_of_payload, payload, out_size);
		buf[sizeof(uint64_t)] = 't';
		return buf;
	}

	// End Ala	


	char *HrpAPIPacketHelper::registerPacket(const std::string &name, uint32_t &out_size)
	{

		//cout<< "registerPacket starts"<<endl;	

		assert(!name.empty());
		// Commented and Replaced by Ala		
		/*
		uint64_t sz_of_data = sizeof(char) + sizeof(int32_t) + sizeof(int64_t) + sizeof(uint32_t) + name.length();
		*/
		uint64_t sz_of_data = sizeof(char) + 40*sizeof(char) + sizeof(int64_t) + sizeof(uint32_t) + name.length();
		// End Ala

		// Commented and Replaced by Ala
		/*
		auto *buf = helper(sz_of_data, "10.10.10.10", 0, name.length(), name.c_str(), out_size);
		*/


		
		// create an object of gns client
		hrp::GnsServiceClient gnsObj;
		hrpNode hrpNodeObj;
		// get GUID by IP
		string ownGUID =  gnsObj.getOwnGUID();

		if(ownGUID.size() != 0)			
		{	
			hrpNodeObj.push_back(ownGUID);			
		}
		else
		{
			cout<<"no GUID returned from getOwnGUID() function"<<endl;			
		}
		
		auto *buf = helper(sz_of_data, hrpNodeObj, 0, name.length(), name.c_str(), out_size);
		// End Ala
	
		buf[sizeof(uint64_t)] = 'r';
		return buf;
	}


	char *HrpAPIPacketHelper::dataPacket(const hrpNode &node, int64_t time, uint32_t sz_of_payload, const char *payload, uint32_t &out_size)
	{
		// Commented and Replaced by Ala		
		/*
		uint64_t sz_of_data = sizeof(char) + sizeof(int32_t) + sizeof(int64_t) + sizeof(uint32_t) + sz_of_payload;
		*/
		uint64_t sz_of_data = sizeof(char) + 40*sizeof(char) + sizeof(int64_t) + sizeof(uint32_t) + sz_of_payload;
		// End Ala
		auto *buf = helper(sz_of_data, node, time, sz_of_payload, payload, out_size);
		buf[sizeof(uint64_t)] = 's';
		return buf;
	}

	char *HrpAPIPacketHelper::helper(uint64_t sz_of_data, const hrpNode &node, int64_t time, uint32_t sz_of_payload, const char *payload, uint32_t &out_size)
	{

		//cout<< "helper starts"<<endl;	
		out_size = sizeof(sz_of_data) + sz_of_data;
		auto *buf = new char[out_size];
		auto *mv_buf = buf;
		// Commented and Replaced by Ala		
		/*
		int32_t raw_ip = node_to_ip(node);
		*/
		const char* guid;
		string guidTemp;
		if(node.size() != 0)
			guidTemp = node.at(0);
		else
			guidTemp="";
		guid = guidTemp.c_str();
		//cout<< "guid value in helper: "<< guid <<endl;	
		// End Ala

		// copy sz_of_data
		memcpy(mv_buf, &sz_of_data, sizeof(sz_of_data));
		mv_buf += sizeof(sz_of_data);
		// reserve for command
		mv_buf += 1;
		
		// Commented and Replaced by Ala		
		/*
		// copy raw ip
		memcpy(mv_buf, &raw_ip, sizeof(int32_t));
		mv_buf += sizeof(int32_t);
		*/
		// copy guid instead of ip
		memcpy(mv_buf, guid, 40*sizeof(char));
		mv_buf += 40*sizeof(char);
		//cout<< "copy guid successfully"<<endl;	
		// End Ala

		// copy time
		memcpy(mv_buf, &time, sizeof(time));
		mv_buf += sizeof(time);
		// copy sz_of_payload
		memcpy(mv_buf, &sz_of_payload, sizeof(sz_of_payload));
		mv_buf += sizeof(sz_of_payload);
		// copy payload
		memcpy(mv_buf, payload, sz_of_payload);
		//cout<< "helper ends"<<endl;	
		return buf;
	}

	HrpAPIPacket HrpAPIPacketHelper::parseFromBuffer(const char *buf, uint64_t size)
	{

		try
		{

			//cout<<"parseFromBuffer starts"<<endl;			
			const char* mv_buf = buf;
			char command;
			//int32_t ip_raw;
			int64_t time;
			uint32_t sz_of_payload;
			char* payload;
			
			command = mv_buf[0];
			mv_buf += 1;
		
			// Commented and Replaced by Ala		
			/*
			memcpy(&ip_raw, mv_buf, sizeof(int32_t));
			mv_buf += sizeof(int32_t);
			*/
			char* guid ;
			guid = new char[40*sizeof(char)];
			memcpy(guid, mv_buf, 40*sizeof(char));
			mv_buf += 40*sizeof(char);

			//cout<< "40*sizeof(char) value in parseFromBuffer: "<< 40*sizeof(char) <<endl;	
			//cout<< "guid value in parseFromBuffer: "<< guid <<endl;	

			// End Ala


			memcpy(&time, mv_buf, sizeof(int64_t));
			mv_buf += sizeof(int64_t);
			
			memcpy(&sz_of_payload, mv_buf, sizeof(uint32_t));
			mv_buf += sizeof(uint32_t);
	
			payload = new char[sz_of_payload];
			memcpy(payload, mv_buf, sz_of_payload);
			
			// Commented and Replaced by Ala
			/*
			hrp_api_pkt_type type = command=='r'?api_register:api_data;
			End Ala	
			*/

			hrp_api_pkt_type type;
			if (command=='r')
			{
				cout<<"registeration received"<<endl;
				type = api_register;
			}
			else if (command=='s')
			{
				cout<<"data received"<<endl;						
				type = api_data;
			}
			else // it should be t
			{
				cout<<"topology request received"<<endl;			
				type = api_topology;
			}	

			// Commented and Replaced by Ala
			/*
			HrpAPIPacket pkt(type, ip_to_node(ip_raw), time, sz_of_payload, payload);
			*/	

			

			// create an object of gns client
			hrp::GnsServiceClient gnsObj;
			hrpNode hrpNodeObj;
			string guidStr = string(guid,40*sizeof(char));
			//cout<< "guidStr value in parseFromBuffer: "<< guidStr <<endl;	
			if(guidStr.size() != 0)			
			{	
				hrpNodeObj.push_back(guidStr);
				
				if(isHrpNodeValid(hrpNodeObj))
				{	
					HrpAPIPacket pkt(type, hrpNodeObj, time, sz_of_payload, payload);
					return pkt;					
				}			
				else
				{
					cout<<"error: Invalid hrpNode. Skipping"<<endl;	
					throw hrpException("HrpAPIPacketHelper::parseFromBuffer(). Invalid hrpNode .. skipping!");					
				}				
			}
			else
			{
				cout<<"HrpAPIPacketHelper::parseFromBuffer guidStr is empty"<<endl;		
				throw hrpException("HrpAPIPacketHelper::parseFromBuffer(). Invalid hrpNode .. skipping!");					
			}

			// End Ala
		}

		catch (const hrpException &e)
		{
			cout<<"HrpAPIPacketHelper::parseFromBuffer(). Invalid hrpNode exception caught!"<<endl;	
			hrpNode hrpNodeObj;
			char * temp;
			HrpAPIPacket pkt(api_undefined, hrpNodeObj, 0, 0, temp);			
			return pkt;
		}
	}
}
