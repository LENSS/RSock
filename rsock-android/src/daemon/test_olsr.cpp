//
// Created by chen on 9/11/17.
//

#include "hrp/TopologyManager.h"
#include <chrono>
#include <thread>
#include <exception>

using namespace asio::ip;
using namespace std;
using namespace hrp;

int main(int argc, char **argv) {

    try {
        asio::io_service io_service;
        TopologyManager manager("10.211.55.2", io_service);
        OlsrConnector connector(io_service, &manager);
        connector.init();
        io_service.run();

        std::this_thread::sleep_for(std::chrono::seconds(10));
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        exit(1);
    }


    return 0;
}