//
// Cretaed by Chen Yang and Ala Altaweel
//

#include <thread>
#include <asio.hpp>
#include "api/HrpAPIClient.h"
#include "gns/GnsServiceClient.h"

using namespace hrp;
using namespace std;

void recv_thd(void *argv);
void send_thd(void *argv);

int nrofMsg = 100, msgInterval = 5000, ttl = 60;
	bool sender = false, receiver = false, olsr = false;
	hrpNode node;
	string GUID;

	string app_name = "test";
	size_t data_sz = 1048576;//1 MB


int main(int argc, char **argv)
{

	long now_time = s_from_epoch();
	cout << "now_time: "<<now_time<<endl;

	if (argc < 2)
	{
		cout << "usage: ./rsockd [-o (use olsr)] [-a app_name] [-s] [-n nrofMsg] [-d dst] [-i interval (ms)] [-l ttl (s)]" << endl;
		return 1;
	}

	int opt = 0;
	while ((opt = getopt(argc, argv, "orsi:n:d:a:z:l:")) != -1)
	{
		switch(opt)
		{
			case 'a':
				app_name = optarg;
			break;
			case 'r':
				receiver = true;
			break;
			case 's':
				sender = true;
			break;
			case 'n':
				nrofMsg = stoi(optarg);
			break;
			case 'd':
				GUID = optarg;
			break;
			case 'i':
				msgInterval = stoi(optarg);
			break;
			case 'l':
				ttl = stoi(optarg);
			break;
			case 'z':
				data_sz = stoul(optarg);
			break;
			case 'o':
				olsr = true;
			break;
			default:
				std::cerr << "Unknown Command Line Argument\n";
			return 1;
		}
	}
	

	if (!olsr)
	{
		HrpAPIClient client(app_name);
		if (!client.init())
			return 1;
		if (receiver)
			recv_thd(&client);
		else if (sender) 
		{
			send_thd(&client);
		}
	}

	return 0;
}

void recv_thd(void *argv)
{
	auto *client = (HrpAPIClient *)argv;

	char buf[HRP_DEFAULT_DATA_BUF_SIZE];
	memset(buf, 0, HRP_DEFAULT_DATA_BUF_SIZE);
	hrpNode peer;
	long delay;
	uint32_t returned_val;
	std::chrono::duration<double> elapsed_seconds;
	bool firstTime = true;
	auto last = std::chrono::system_clock::now();
	while (true)
	{

		// code below is for testing the request topology from rsockd
		/*
		elapsed_seconds = std::chrono::system_clock::now()-last;
		//cout << "elapsed_seconds: " << elapsed_seconds.count() <<endl;
		if(firstTime)
		{
			returned_val = client->requestTopology();
			last = std::chrono::system_clock::now();
			firstTime = false;
		}
		else if(elapsed_seconds > std::chrono::seconds(2))
		{
			returned_val = client->requestTopology();
			last = std::chrono::system_clock::now();
		}
		*/

		
		long ret = client->recv(buf, HRP_DEFAULT_DATA_BUF_SIZE, peer, delay);
		if (ret == 0)
		{	
			cout << "error: client->recv returned zero" << endl;
			break;
		}
		string contents (buf, ret);
		
		
		//cout << "received with delay=" << delay << " (ms) size: " << ret <<" \ncontent: \n"<< contents << "\nfrom: " << peer.at(0) <<endl;
		cout << "received with delay=" << delay << " (ms) size: " << ret << "\nfrom: " << peer.at(0) <<endl;
			
		/*
		if (peer.size() == 1)
			cout << "received data from " << peer.at(0) << " with delay=" << delay << " (ms) size: " << ret << endl;
		else if (peer.size() == 2)
			cout << "received data from " << peer.at(0) << " with delay=" << peer.at(1) << " delay=" << delay << " (ms) size: " << ret << endl;
		else if (peer.size() == 3)
			cout << "received data from " << peer.at(0) << " with delay=" << peer.at(1) << " " << peer.at(2) << " delay=" << delay << " (ms) size: " << ret << endl;
		*/
	}
}



void send_thd(void *argv) {
	auto *client = (HrpAPIClient *)argv;
	int i=0,j=0,k=0;
	uint32_t returned_val;
	char *send_buf;
	int bytes = data_sz+1;
	send_buf = (char *) malloc(bytes);

	// create an object of gns client
	GnsServiceClient gnsObj;
	node.push_back(GUID);		
	// get GUID by IP
	// No need to add IPs since we represtn the graph using GUIDs
	
	/*
	std::vector<std::string> IPAddresses = gnsObj.getIPbyGUID(GUID, false);
	if(IPAddresses.size() != 0)			
	{	

		for(std::vector<std::string>::size_type i = 0; i < IPAddresses.size(); i++)
			node.push_back(IPAddresses.at(i));
		cout<<printHrpNode("dst node: ", node)<<endl;
	}
	else
	{
		cout<<"no IP address returned for node with GUID: "<<GUID<<endl;			
	}
	*/


	while(true)
	{

		for (; i<nrofMsg; i++)
		{
			j=0;	
			for(;j<bytes-1;j++)
				send_buf[j] = (char) rand();

			send_buf[j] = '\0';
			returned_val = client->send(send_buf, data_sz, node, ttl);
			//cout << "returned_val = " << returned_val << endl;
			cout << "Data packet sent with: "<< data_sz<< " Bytes" << endl;
			this_thread::sleep_for(chrono::milliseconds(msgInterval));
		}
		
				
	}
	
	cout << "Done with sending: " <<nrofMsg << " packets!"<< endl;
}


