//
// Cretaed by Ala Altaweel
//

#include "GnsServiceClient.h"

/**
 * This code acts as the GNS Client for RSock. This provides the interface
 * between RSock and Java based GNS client.	A C++ client example code is used from
 * https://www.binarytides.com/code-a-simple-socket-client-class-in-c/
 *
 *
 * @author sbhunia
 */




namespace hrp
{

	/**
	 * Define the static variable
	 */
	GUIDTable GnsServiceClient::guidTableCache;
	
	/**
	 * The default constructor
	 */
	GnsServiceClient::GnsServiceClient()
	{
		port = GNS_JAVA_PORT;
		address = GNS_JAVA_IP;
		errorMessage ="ERROR";
		successMessage = "SUCCESS";
		translate_command ="TRANSLATE";
		commandSeparator = "\t";


		// share the pointer for _logger
	
		_logger = spdlog::get("GnsServiceClient");		
		if (!_logger)
			_logger = std::make_shared<spdlog::logger>("GnsServiceClient", file_sink);
		if (debug_mode)
			_logger->set_level(spdlog::level::debug);		
	}
	


	/**
	 * This function delas with the socket communication.
	 * @return
	 */
	int GnsServiceClient::conn()
	{
		//Create socket
		//_logger->debug("conn() starts");

		int sock = socket(AF_INET , SOCK_STREAM , 0);
		if (sock == -1)
		{
			_logger->error("Could not create socket");
			return -1;
		}

		//_logger->debug("Socket created");
		//setup address structure

		server.sin_addr.s_addr = inet_addr( address.c_str() );

		server.sin_family = AF_INET;
		server.sin_port = htons( port );

		//Connect to GNS service
		if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
		{
			_logger->error("connect failed. Error");
			return -1;
		}

		//_logger->debug("Connected");
		return sock;
	}


	/**
	 *  This function sends some request and waits for reply
	 */
	string GnsServiceClient::send_receive(string msg)
	{


		//char* buffer = (char*)malloc(sizeof(char)*BUFFER_SIZE);
		//_logger->debug("send_receive() starts");
		char buffer[1000];

		string req = msg+"\n\n";

		int sock = conn();
		// try to connect
		if (sock==-1 )
		{
			_logger->error("Can not connect to GNS JAVA Client");
			return errorMessage+commandSeparator+"could not connect with local GNS service";
		}


		//Send the request
		if( send(sock , req.c_str() , strlen( req.c_str() ) , 0) < 0)
		{
			_logger->error("send failed");
			return ("ERROR: send error");
		}


		//Receive a reply from the server
		//_logger->debug("before recv from GNS service");
		if( recv(sock , buffer , sizeof(buffer) , 0) < 0)
		{
			_logger->error("recv failed");
			return "ERROR: receive error";
		}
		//_logger->debug("after recv from GNS service");
		string reply(buffer);
		//close the socket
		//_logger->debug("Server reply: {}",  reply);
		close(sock);

		//discard the last few garbled characters
		return reply.substr(0,reply.find('\n'));
	}


	/**
	* This function splits a string using delimeter
	* @param input_string
	* @param delimiter
	* @return
	*/
	vector<string> GnsServiceClient::split_string(string input_string, string delimiter)
	{
		size_t pos = 0;
		vector<string>tokens;
		while ((pos = input_string.find(delimiter)) != string::npos)
		{
			tokens.push_back( input_string.substr(0, pos));
			input_string.erase(0, pos + delimiter.length());
		}
		tokens.push_back(input_string);
		return tokens;
	}


	/**
	 * This function is used for dealing with service queries that reply vector of string
	 * such as IP addresses or GUIDs
	 *
	 * @param request query string
	 * @return vector of string
	 */
	vector<string> GnsServiceClient::communicate_and_parse_vector(string req)
	{

		//_logger->debug("communicate_and_parse_vector starts");
		string rep = send_receive(req);
		//_logger->debug("trying to vectorize this reply: {}", rep);
		vector<string> tokens = split_string(rep, commandSeparator);

		if (successMessage.compare(tokens[0])==0)
		{
			tokens.erase(tokens.begin()+0);
			return tokens;
		}
		else if (errorMessage.compare(tokens[0])==0)
		{
			_logger->error("Error occured in GNS: {}",tokens[1]);
			return vector<string>();
		}
		else
		{
			_logger->error("Unknown Error occured in GNS");
			return vector<string>();
		}
	}

	/**
	* This function deals with queries that returns one string literal
	* such as IP address or GUID
	*
	* @param request query string
	* @return String
	*/
	string GnsServiceClient::communicate_and_parse_str(string req)
	{
		vector<string> reply_vector = communicate_and_parse_vector(req);
		if (reply_vector.empty())
			return "";
		else
			return reply_vector[0];
	}

	/**
	* This function deals with queries that returns true or false (e.g. update_ip)
	*	
	* @param request query string
	* @return bool
	*/

	bool GnsServiceClient::communicate_and_parse_bool(string req)
	{

		string rep = send_receive(req);
		vector<string> tokens = split_string(rep, commandSeparator);

		if (successMessage.compare(tokens[0])==0)
		{
			return true;
		}
		else if (errorMessage.compare(tokens[0])==0)
		{
			_logger->error("Error occured in GNS: {}",tokens[1]);
			return false;
		}
		else
		{
			_logger->error("Unknown Error occured in GNS");
			return false;
		}
	}

	bool GnsServiceClient::updateIP()
	{
		return communicate_and_parse_bool("UPDATE_IP");
	}

	string GnsServiceClient::getOwnAccountName()
	{
		return communicate_and_parse_str("GET_OWN_ACCOUNTNAME");
	}
	
	string GnsServiceClient::getOwnGUID()
	{
		return communicate_and_parse_str( "GET_OWN_GUID");
	}


	vector<string> GnsServiceClient::getIPbyAccountName(string accountName)
	{
		return communicate_and_parse_vector ("GET_IP_BY_ACCOUNTNAME"+commandSeparator+accountName);
	}

	string GnsServiceClient::getGUIDbyAccountName(string accountName)
	{
		return communicate_and_parse_str("GET_GUID_BY_ACCOUNTNAME"+commandSeparator+accountName);
	}

	string GnsServiceClient::getAccountNamebyGUID(string guid)
	{
		return communicate_and_parse_str("GET_ACCOUNTNAME_BYGUID"+commandSeparator+guid);
	}

	vector<string> GnsServiceClient::getAccountNamebyIP(string s)
	{
		return communicate_and_parse_vector("GET_ACCOUNTNAME_BY_IP"+commandSeparator+s);
	}

	vector<string> GnsServiceClient::getGUIDbyIP(string ip, bool fromCache )
	{
		if(fromCache)
		{
			if(guidTableCache.empty())
				return getGUIDbyIP(ip, false);
			vector<string> guidList ;
			for ( GUIDTable::iterator it = guidTableCache.begin(); it != guidTableCache.end(); ++it  )
			{
				string g = it->first;
				vector<string> ipList = it->second; // iplist
				for(string s: ipList)
					if(ip.compare(s)==0)
						guidList.push_back(g);
			}
			
			return guidList;
		}
		else
		{
			return communicate_and_parse_vector("GET_GUID_BY_IP"+commandSeparator+ip);
		}
	}



	vector<string> GnsServiceClient::getIPbyGUID(string guid, bool fromCache)
	{
	  
		if(fromCache )
		{
			if(guidTableCache.empty())
				return getIPbyGUID(guid, false);
			GUIDTable::iterator it = guidTableCache.find(guid);
			if(it != guidTableCache.end())
			{ 
				// i.e. the guid is found in the cache
				return guidTableCache[guid];
			}
			else
			{
				return vector<string>();				
			}
		}
		else
		{
			return communicate_and_parse_vector ("GET_IP_BY_GUID"+commandSeparator+guid);
		}
	}


	void GnsServiceClient::refreshTranslationTable(vector<string> ipList)
	{

		//_logger->debug("Refreshing GUID cache for:");
		//print_vector(ipList);
		//updateIP();
		//_logger->debug("Refreshing the GUID cache map");
		
		if(ipList.empty())
		{
			_logger->error("Empty IP list");
			return;
		}
		std::set<string> guidSet ;
		// First thing is to gather all the GUID for these IPS
		for(vector<string>::size_type i = 0; i < ipList.size(); i++)
		{
			// Fetches the GUIDs and add to the set
			vector<string> guids = getGUIDbyIP(ipList[i], false);
			for(string g: guids)
				guidSet.insert(g);
		}
		GUIDTable  tmpTable;
		//_logger->debug("Refreshing the GUID cache...");

		guidSet.insert(getOwnGUID());		

		for(string g: guidSet)
		{
			//_logger->debug("Now, working with GUID: {}", g);
			vector<string> ips=getIPbyGUID(g, false); // fetch it from the server
			//_logger->debug("Inserting: {} to table", g);
			
			//print_vector(ips);
			tmpTable.insert(std::make_pair (g, ips));
		}

		updateCache(tmpTable);
		//_logger->debug("Done with Refreshing the GUID cache");
	}

	/**
	*
	* @param t
	*/

	void GnsServiceClient::updateCache(GUIDTable t)
	{

		//_logger->debug("Updating the cache...");
		{
			std::lock_guard<std::mutex> lck (_mutex_table);
			guidTableCache = t;
		}
		//_logger->debug("Done with updating");
		// uncomment the below line if you want to print the cache contents
		//print_map(guidTableCache);
	}


	void GnsServiceClient::print_map(GUIDTable table)
	{
		if(table.empty())
		{
			_logger->error("Cache is empty");
			return;
		}
		_logger->debug("Cache contents:");
		for ( auto it = table.begin(); it != table.end(); ++it  )
		{
			string guid = it->first;
			vector<string> ipList = it->second;
			_logger->debug("GUID: {}:",guid);
			for(string ip:ipList)
			{
				_logger->debug("\t {}",ip);
				
			}
		}
	}

	void GnsServiceClient::print_vector(vector<string> ips)
	{
		if (! ips.empty())
			for(vector<string>::size_type i = 0; i < ips.size(); i++)
			{
				_logger->debug(" {}",ips[i]);
			}
		else
			{
				_logger->error("The list is empty");
			}
	}

	void GnsServiceClient::printTableCache()
	{
		_logger->debug("The cache table contains:");
		print_map(guidTableCache);
		//print_map(guidCache.table);
	}
}
