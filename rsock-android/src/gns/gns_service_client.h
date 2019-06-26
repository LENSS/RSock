#include<iostream>    //cout
#include<cstdlib>
#include<unistd.h>
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<string>  //string
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<netdb.h> //hostent
#include<vector>
#include "common.h"


using namespace std;

#define GNS_JAVA_IP		"0.0.0.0"
#define GNS_JAVA_PORT	22223
#define BUFFER_SIZE 1024
namespace hrp {
  class gns_service_client
  {
  private:
      int sock;
      string address;
      int port;
      struct sockaddr_in server;
      string errorMessage ;
      string successMessage ;
      string translate_command ;
      string commandSeparator;
      //std::shared_ptr<spdlog::logger> _logger;




  public:
      gns_service_client();
      bool conn();
      string send_receive(string);
      vector<string> split_string(string s, string delimiter);
      vector<string> communicate_and_parse_vector(string req);
      string communicate_and_parse_str(string req);
      bool updateIP();
      string getOwnAccountName();
      string getOwnGUID();
      vector<string> getIPbyGUID(string guid);
      vector<string> getIPbyAccountName(string guid);
      string getGUIDbyAccountName(string AccountName);
      string getAccountNamebyGUID(string guid);
  };
}
