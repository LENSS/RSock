//
// Cretaed by Ala Altaweel
//

#ifndef GNS_SERVICE_CLIENT
#define GNS_SERVICE_CLIENT

#include<unistd.h>
#include<cstdlib>
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<string>  //string
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<netdb.h> //hostent
#include<vector>
#include <map>
#include <set>
#include<mutex>
#include<iostream>    //cout

#include "common.h"
#include <spdlog/logger.h>
#include "spdlog/spdlog.h"




typedef std::map<string, vector<string> > GUIDTable;


/**
 * This class handles the communication with GNS-service through socket communication.
 * This can be used for clients running in both Desktop and Android.
 */
namespace hrp
{
	class GnsServiceClient
	{
		public:
		string address;
		GnsServiceClient();
		bool updateIP();
		string getOwnAccountName();
		string getOwnGUID();
		vector<string> getIPbyGUID(string guid, bool fromCache = true);
		vector<string> getIPbyAccountName(string guid);
		string getGUIDbyAccountName(string AccountName);
		string getAccountNamebyGUID(string guid);
		vector<string> getGUIDbyIP(string ip, bool fromCache = true);
		vector<string> getAccountNamebyIP(string guid);
		void refreshTranslationTable(vector<string> ipList);
	
		void print_map(GUIDTable table);
		void print_vector(vector<string> ips);
		void printTableCache();

		private:
		//int sock;
		int port;
		struct sockaddr_in server;
		string errorMessage ;
		string successMessage ;
		string translate_command ;
		string commandSeparator;
		static GUIDTable guidTableCache;
		int conn();
		string send_receive(string);
		vector<string> split_string(string s, string delimiter);
		vector<string> communicate_and_parse_vector(string req);
		string communicate_and_parse_str(string req);
		bool communicate_and_parse_bool(string req);
		void updateCache(GUIDTable);
		std::mutex  _mutex_table;
		/// Logger
		std::shared_ptr<spdlog::logger> _logger;
	
	};
}
#endif //GNS_SERVICE_CLIENT
