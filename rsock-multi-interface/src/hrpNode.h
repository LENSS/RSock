//
// Cretaed by Chen Yang and Ala Altaweel
//

#ifndef HRP_NODE_H
#define HRP_NODE_H

#include <string>
#include <set>
#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include "gns/GnsServiceClient.h"
using namespace std;

namespace hrp
{


	// Commented and Replaced by Ala, change hrpNode to vector instead of string
	/*
	typedef std::string hrpNode;
	*/
	// End Ala
	// Define the hrpNode as a vector of strings
	// hrpNode[0] is the GUID obtained from GNS
	// we use vector for future extensions such that use more than one GUID for a node or with IP, etc.
	
	 
	typedef std::vector<std::string> hrpNode;
	typedef std::set<hrpNode> ect_set; /// Encounter set

	inline string printHrpNode(string prefix, const hrpNode &hrpNodeObj)
	{

		string toPrint = "";
		if (hrpNodeObj.size() == 0)
			toPrint = prefix + string("An HrpNode with no GUID!");
		else if (hrpNodeObj.size() == 1)
			toPrint = prefix + string(" GUID: ") + hrpNodeObj.at(0);
		else if (hrpNodeObj.size() == 2)
			toPrint = prefix + string(" GUID: ") + hrpNodeObj.at(0) + string("\tip1: ") + hrpNodeObj.at(1);
		else if (hrpNodeObj.size() == 3)
			toPrint = prefix + string(" GUID: ") + hrpNodeObj.at(0) + string("\tip1: ") + hrpNodeObj.at(1) + string("\tip2: ") + hrpNodeObj.at(2);
	
		return toPrint;

	}
		
	// check if hrpNode has at least a GUID and one IP address
	inline bool isHrpNodeValid(const hrpNode &hrpNodeObj)
	{
		//if(hrpNodeObj.size() == 0 || hrpNodeObj.size() == 1)
		if(hrpNodeObj.size() == 0)
			return false;
		else
			return true;

	}

	/*
	inline int node_to_ip(const hrpNode &n)
	{
		//cout<<"node_to_ip called for: "<<printHrpNode("",n)<<endl;

		// Commented and Replaced by by Ala
		//
		//int ret = 0;
		//inet_pton(AF_INET, n.c_str(), &ret);
		//return ret;
		

		//
		//int ret = 0;
		//if(n.size() == 1)
		//{
		//	cout<<"error in node_to_ip func: node has no ip address!, ret = " << ret<<endl;
		//		return ret;
		//}
		//string ipAddress = n.at(1);
		//if(ipAddress.size() == 0)
		//{
		//	cout<<"error in node_to_ip func: node with empty ip addres, ret = " << ret<<endl;
		//	return ret;
		//}
		//inet_pton(AF_INET, ipAddress.c_str(), &ret);
		//cout<<"node_to_ip successfully called, ret = " << ret<<endl;;
		//return ret;
		
		int ret = 0;
		if(n.size() == 0)
		{
			cout<<"error in node_to_ip func: empty node!, ret = " << ret<<endl;
			return 0;
		}

		else if(n.size() == 1)
		{
			hrp::GnsServiceClient gnsObj;
			//cout<<"node_to_ip func: node with empty ip address, add ip from GNS"<<endl;
			std::vector<std::string> IPAddresses = gnsObj.getIPbyGUID(n.at(0));
			if(IPAddresses.size() != 0)
			{
				inet_pton(AF_INET, IPAddresses.at(0).c_str(), &ret);
			}
			else
			{
				cout<<"warn in node_to_ip: no IP addresses returned for GUID: " << n.at(0) <<endl;	
			}	
			return ret;
		}
		// n has size > 2
		// just use one IP address		
		string ipAddress = n.at(1);
		inet_pton(AF_INET, ipAddress.c_str(), &ret);
		return ret;
		// End Ala
	}
	//
	*/
	// the fromCache flag decides whether to fetch the info from GNS cache or the server

	/*
	inline hrpNode ip_to_node(int addr)
	{
		//cout<<"ip_to_node called for: "<<addr<<endl;
		// Commented and Replaced by Ala
		//
		//char str[INET_ADDRSTRLEN];
		//inet_ntop(AF_INET, &addr, str, INET_ADDRSTRLEN);
		//return hrp::hrpNode(str);
		//

		// get IP address from int addr
		char IP_CharArr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &addr, IP_CharArr, INET_ADDRSTRLEN);
		
		//cout<<"ip_to_node IP_CharArr: "<<IP_CharArr<<endl;

		// create an object of gns client
		hrp::GnsServiceClient gnsObj;
		hrpNode hrpNodeObj;
		
		string IP_str(IP_CharArr);

		if(IP_str.compare(string("0.0.0.0")) != 0) // not called for current node
		{
			// get GUID by IP
			std::vector<std::string> GUID = gnsObj.getGUIDbyIP(IP_str);
			if(GUID.size() != 0)			
			{	
				hrpNodeObj.push_back(GUID.at(0));
				// get IPs for this GUID
				// No need to add IPs since we represtn the graph using GUIDs
				//
				//std::vector<std::string> IPAddresses = gnsObj.getIPbyGUID(GUID.at(0));
				//if(IPAddresses.size() != 0)
				//	for(std::vector<std::string>::size_type i = 0; i < IPAddresses.size(); i++)
				//		hrpNodeObj.push_back(IPAddresses.at(i));
				//else
				//{
				//	cout<<"warn: no IP addresses returned for GUID: " << GUID.at(0) <<endl;	
				//}
				//
			}
			else
			{
				cout<<"warn in ip_to_node func: no GUID returned for node with ip: "<<IP_str<< endl;	
			}
		}				
		else // set hrpNodeObj to current node
		{
			//cout<<"0.0.0.0 called"<<endl;
			// get current node GUID
			std::string GUID = gnsObj.getOwnGUID();
			if(GUID.size() != 0)			
			{
				hrpNodeObj.push_back(GUID);
		
				// get IPs for this GUID
				//
				//std::vector<std::string> IPAddresses = gnsObj.getIPbyGUID(GUID);
				//if(IPAddresses.size() != 0)
				//	for(std::vector<std::string>::size_type i = 0; i < IPAddresses.size(); i++)
				//		hrpNodeObj.push_back(IPAddresses.at(i));
				//else
				//{
				//	cout<<"warn: no IP addresses returned for GUID: " << GUID <<endl;	
				//}
				
			}
			else
			{
				cout<<"warn in ip_to_node func: getOwnGUID return no GUID"<< endl;				
			}
		}		
			
		return hrpNodeObj;
		// End Ala
	}
	*/


	
	/*
	* convert string to unint64
	* src: http://jsteemann.github.io/blog/2016/06/02/fastest-string-to-uint64-conversion-method/
	*/
	uint64_t str_to_unint64(std::string const& value);



	/**
	* Utility function. Convert the sockaddr_in into hrpNode
	* @param sa
	* @return
	*/

	/*
	* splt the guid into two halves and convert each to uint64_t, aad them as a hash
	*/

	inline uint64_t guid_to_unint64(std::string const& value)
	{

		//cout<<"info guid_to_unint64 passed guid: "<<value <<  endl;				
		if(value.size() == 40)
		{
			int64_t first_half_hash = str_to_unint64(value.substr(0,20));	
			int64_t second_half_hash = str_to_unint64(value.substr(20,20));	
			//cout<<"info guid_to_unint64 func returned: "<<(first_half_hash + second_half_hash) <<  endl;				
			return (first_half_hash + second_half_hash);
		}
		else
		{
				//cout<<"warn guid_to_unint64 func: guid length != 40 return 0"<< endl;				
				return 0;
		}

	}

	
	

	class hrpEdge
	{
    		public:
	        hrpEdge(hrpNode& p1, hrpNode& p2, double l) : peer1(p1), peer2(p2), length(l) {}
		
		friend bool operator<(const hrpEdge &e1, const hrpEdge &e2)
		{
			if (e1.peer1 < e2.peer1) return true;
			else if (e1.peer1 > e2.peer1) return false;
			else return e1.peer2 < e2.peer2;
	        }
	
	        friend std::ostream& operator<<(std::ostream &stream, const hrpEdge &e)
		{
			// Commented and Replaced by Ala
			//stream << "[" << e.peer1) << " -> " << e.peer2 << ", lq = " << e.length << "]";	
			stream << "[" << e.peer1.at(0) << " -> " << e.peer2.at(0) << ", lq = " << e.length << "]";
			// End Ala
			return stream;
		}

		hrpNode peer1, peer2;
		double length;
	};
}

#endif //HRP_NODE_H
