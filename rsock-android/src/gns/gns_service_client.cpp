/**
	This code acts as the GNS Client for RSock. This provides the interface between RSock and Java based GNS client.
	Author: Suman
	Suman downloaded a C++ client example code from https://www.binarytides.com/code-a-simple-socket-client-class-in-c/
*/


//#define GNS_JAVA_IP		"0.0.0.0"



#include "gns_service_client.h"

namespace hrp {
  gns_service_client::gns_service_client()
  {
      port = GNS_JAVA_PORT;
      address = GNS_JAVA_IP;

      errorMessage ="ERROR";
      successMessage = "SUCCESS";
      translate_command ="TRANSLATE";
      commandSeparator = "\t";
     /* _logger = std::make_shared<spdlog::logger>("TxrxEngine", file_sink);
      if (debug_mode)
            _logger->set_level(spdlog::level::debug);
*/
  }

  /**
      Connect to a host on a certain port number
  */
  bool gns_service_client::conn()
  {
      //Create socket
      sock = socket(AF_INET , SOCK_STREAM , 0);
      if (sock == -1)
      {
          perror("Could not create socket");
          return false;
      }

      //cout<<"Socket created\n";

      //setup address structure

      server.sin_addr.s_addr = inet_addr( address.c_str() );

      server.sin_family = AF_INET;
      server.sin_port = htons( port );

      //Connect to remote server
      if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
      {
          perror("connect failed. Error");
          return false;
      }

      //cout<<"Connected\n";
      return true;
  }


  // This function sends some request and waits for reply
  string gns_service_client::send_receive(string msg)
  {

      //char* buffer = (char*)malloc(sizeof(char)*BUFFER_SIZE);
      char buffer[1000];
      string reply;
      string req = msg+"\n\n";

      // try to connect
      if (! conn()){
      	perror("Can not connect to GNS JAVA Client");
      	return errorMessage+commandSeparator+"could not connect with local GNS service";
      }


      //Send the request
      if( send(sock , req.c_str() , strlen( req.c_str() ) , 0) < 0)
      {
          perror ("Send failed : ");
          return ("ERROR: send error");
      }
      //cout<<"Data send\n";


      //Receive a reply from the server
      if( recv(sock , buffer , sizeof(buffer) , 0) < 0)
      {
          perror("recv failed");
          return "ERROR: receive error";
      }
      reply = buffer;
      //cout<<"Server reply: "<<buffer<<endl;
      //close the socket
      close(sock);

      //discard the last few garbled characters
      return reply.substr(0,reply.find('\n'));
  }

  // This function splits a string using delimeter
  vector<string> gns_service_client::split_string(string s, string delimiter){
    size_t pos = 0;
    vector<string>tokens;
    while ((pos = s.find(delimiter)) != string::npos) {
        tokens.push_back( s.substr(0, pos));
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);
    return tokens;
  }



  vector<string> gns_service_client::communicate_and_parse_vector(string req){
    string rep = send_receive(req);

    vector<string> tokens = split_string(rep, commandSeparator);

    if (successMessage.compare(tokens[0])==0){
      tokens.erase(tokens.begin()+0);
      return tokens;
    }
    else if (errorMessage.compare(tokens[0])==0){
      //cout<<"Error occured in GNS: "<<tokens[1]<<endl;
      //_logger->error("Error occured in GNS:: {}", tokens[1]);
      return vector<string>();
    }
    else{
      //cout<<"Unknown Error occured in GNS"<<endl;
      //_logger->error("Unknown Error occured in GNS");
      return vector<string>();
    }
  }

  string gns_service_client::communicate_and_parse_str(string req){
    vector<string> reply_vector = communicate_and_parse_vector(req);
    if (reply_vector.empty())
      return "";
    else return reply_vector[0];
  }

  bool gns_service_client::updateIP(){
    string rep = send_receive("UPDATE_IP");

    vector<string> tokens = split_string(rep, commandSeparator);

    if (successMessage.compare(tokens[0])==0){
      return true;
    }
    else if (errorMessage.compare(tokens[0])==0){
      //cout<<"Error occured in GNS: "<<tokens[1]<<endl;
      //_logger->error("Error occured in GNS:: {}", tokens[1]);
      return false;
    }
    else{
      //cout<<"Unknown Error occured in GNS"<<endl;
      //_logger->error("Unknown Error occured in GNS");
      return false;
    }
  }

  string gns_service_client::getOwnAccountName(){
    return communicate_and_parse_str("GET_OWN_ACCOUNTNAME");
  }
  string gns_service_client::getOwnGUID(){
    return communicate_and_parse_str( "GET_OWN_GUID");
  }


  vector<string> gns_service_client::getIPbyGUID(string guid){
      return communicate_and_parse_vector ("GET_IP_BY_GUID"+commandSeparator+guid);
  }

  vector<string> gns_service_client::getIPbyAccountName(string accountName){
      return communicate_and_parse_vector ("GET_IP_BY_ACCOUNTNAME"+commandSeparator+accountName);
  }


  string gns_service_client::getGUIDbyAccountName(string accountName){
    return communicate_and_parse_str("GET_GUID_BY_ACCOUNTNAME"+commandSeparator+accountName);
  }
  string gns_service_client::getAccountNamebyGUID(string guid){
    return communicate_and_parse_str("GET_ACCOUNTNAME_BYGUID"+commandSeparator+guid);
  }
}
