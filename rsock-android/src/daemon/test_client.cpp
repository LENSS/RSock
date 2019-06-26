//
//

#include <thread>
#include <asio.hpp>
#include "api/HrpAPIClient.h"

using namespace hrp;
using namespace std;

void recv_thd(void *argv);
void send_thd(void *argv);

int nrofMsg = 100, msgInterval = 5000, ttl = 30;
    bool sender = false, receiver = false, olsr = false;
    hrpNode node = "10.10.10.10";
    string app_name = "test";
    size_t data_sz = 1048576;//1 MB


int main(int argc, char **argv) {

    if (argc < 2) {
        cout << "usage: ./rsockd [-o (use olsr)] [-a app_name] [-s] [-n nrofMsg] [-d dst] [-i interval (ms)] [-l ttl (s)]" << endl;
        return 1;
    }

    int opt = 0;
    while ((opt = getopt(argc, argv, "orsi:n:d:a:z:l:")) != -1){
        switch(opt) {
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
                node = optarg;
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
    if (!olsr){
        HrpAPIClient client(app_name);
        if (!client.init())
            return 1;
        if (receiver)
            recv_thd(&client);
        else if (sender) 
	    send_thd(&client);
   }
    return 0;
}

void recv_thd(void *argv) {
    auto *client = (HrpAPIClient *)argv;

    char buf[HRP_DEFAULT_DATA_BUF_SIZE];
    memset(buf, 0, HRP_DEFAULT_DATA_BUF_SIZE);
    hrpNode peer;
    long delay;

    while (true) {
        long ret = client->recv(buf, HRP_DEFAULT_DATA_BUF_SIZE, peer, delay);
        if (ret == 0) break;
        cout << "received data from " << peer << " delay=" << delay << " (ms) size: " << ret << endl;
    }
}

void send_thd(void *argv) {
    auto *client = (HrpAPIClient *)argv;
    int i=0,j=0;
    uint32_t returned_val;
    char *send_buf;
    int bytes = data_sz+1;
    send_buf = (char *) malloc(bytes);
    while(true) {

           for (; i<nrofMsg; i++) {
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
