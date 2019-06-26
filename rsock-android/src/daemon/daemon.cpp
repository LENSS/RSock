//
// Created by Chen on 2/5/18.
//

#include "daemon.h"

using namespace hrp;
using namespace std;

std::shared_ptr<spdlog::logger> _logger;
TxrxEngine *glb_engine = NULL;
pid_t pid = 0;
pid_t proc_find();
string getIpAddr(const char *);
void sighandler(int signum);


#ifdef __ANDROID__
static int pfd[2];
static pthread_t thr;
static const char *tag = "myapp";

int start_logger(const char *app_name)
{
    tag = app_name;

    /* make stdout line-buffered and stderr unbuffered */
    setvbuf(stdout, 0, _IOLBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);
    cout << "After setvbuf(stderr, 0, _IONBF, 0);"<< endl;
	
    /* create the pipe and redirect stdout and stderr */
    pipe(pfd);
    cout << "After pipe(pfd);"<< endl;
    dup2(pfd[1], 1);
    cout << "After dup2(pfd[1], 1);"<< endl;
    dup2(pfd[1], 2);
    cout << "After dup2(pfd[1], 2);"<< endl;

    /* spawn the logging thread */
    if(pthread_create(&thr, 0, thread_func, 0) == -1)
        return -1;

    cout << "Before pthread_detach(thr);"<< endl;
    pthread_detach(thr);
    cout << "After pthread_detach(thr);"<< endl;
    return 0;
}

void *thread_func(void*)
{
    ssize_t rdsz;
    char buf[128];
    while((rdsz = read(pfd[0], buf, sizeof buf - 1)) > 0) {
        if(buf[rdsz - 1] == '\n') --rdsz;
        buf[rdsz] = 0;  /* add null-terminator */
        __android_log_write(ANDROID_LOG_DEBUG, tag, buf);
    }
    return 0;
}
#endif


int daemon_main(int argc, char** argv) {

#ifdef __ANDROID__
    // commented since it causes some problems while running on Android    
    // start_logger("rsock");
#endif

    string log_folder = "./";
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
        log_folder = string(homedir) + "/";
    }

    log_file_name = "basic_log.txt";

    string ipAddr, interface;
    map<string, string> cfg;
    bool daemonize = false;


    if (argc < 2) {
        cout << "usage: ./rsockd -i interface [-a alpha] [-r rmax] [-t ita] [-p ict_probe] [-e mve_epoch] [-g debug] [-l log_file_name]" << endl;
        return 1;
    }

    int opt = 0;   
    while ((opt = getopt(argc, argv, "w:i:a:r:t:p:e:l:gd")) != -1){
        switch(opt) {
            case 'w':
                ipAddr = optarg; break;
            case 'i':
                interface = optarg;break;
            case 'a':
                cfg.insert(pair<string,string>(HRP_CFG_ALPHA, string(optarg)));
                break;
            case 'r':
                cfg.insert(pair<string,string>(HRP_CFG_RMAX, string(optarg)));
                break;
            case 't':
                cfg.insert(pair<string,string>(HRP_CFG_ITA, string(optarg)));
                break;
            case 'p':
                cfg.insert(pair<string,string>(HRP_CFG_ICT_PROBE, string(optarg)));
                break;
            case 'e':
                cfg.insert(pair<string,string>(HRP_CFG_MVE_EPOCH, string(optarg)));
                break;
            case 'l':
                log_file_name = string(optarg);
                break;
            case 'g':
                debug_mode = true;
                break;
            case 'd':
                daemonize = true; break;
            default:
                std::cerr << "Unknown Command Line Argument\n";
                return 1;
        }
    }

    if (daemonize) {
	// Commented out by Ala and change 0 to 1 
	//if (daemon(1, 0) != 0) {	       
	if (daemon(1, 1) != 0) {
	    cout << "error: " << errno << endl;
            exit(1);
        }
    }

    log_file_name = log_folder + log_file_name;

    cout << "log_file_name and folder: "<<log_file_name<< endl;

    ipAddr = getIpAddr(interface.c_str());
    cout << "ip address = " << ipAddr << endl;

    signal(SIGINT, sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGABRT, sighandler);

    cout << "hrpd initiating... ";
    cout << "interface = " << interface << ", ip address = " << ipAddr << endl;

    cout << "detecting olsrd... ";
    pid = proc_find();
    if (pid == -1) {
#ifdef __APPLE__
        cout << "APPLE\n";
        cout << "failed. olsrd is not running!" << endl;
        exit(1);
#elif defined(__ANDROID__)
        cout << "Android\n";
        cout << "not running, initiating..." << endl;
        system("su -c export LD_LIBRARY_PATH=/data/data/com.lenss.cmy.myapplication/files:$LD_LIBRARY_PATH && su -c /data/data/com.lenss.cmy.myapplication/files/olsrd -f /data/data/com.lenss.cmy.myapplication/files/olsrd.conf -d 0");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        pid = proc_find();
        if (pid == -1) {
            cout << "Cannot initiate olsrd, exiting" << endl;
            exit(1);
        }
#else
        cout << "OTHER\n";
        cout << "not running, initiating..." << endl;
        system("olsrd -f /etc/olsrd.conf -d 0");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        pid = proc_find();
        if (pid == -1) {
            cout << "Cannot initiate olsrd, exiting" << endl;
            exit(1);
        }
#endif
    }
    else cout << "found running olsrd, pid = " << pid << endl;

#ifdef _PROFILING_
    ProfilerStart("hrp_profile.log");
#endif

    // initialize loggers
    //file_sink = make_shared<spdlog::sinks::stdout_sink_st>(log_file_name);
    file_sink = make_shared<spdlog::sinks::stdout_sink_st>();
    _logger = std::make_shared<spdlog::logger>("Main", file_sink);

    asio::io_service io_service;
    TopologyManager manager(hrpNode(ipAddr), io_service);

    TxrxEngine engine(hrp::hrpNode(ipAddr), io_service, 18888, &manager);

    HrpCore core(ipAddr, cfg);

    HrpRouter router(io_service, &core, &engine);

    MetaDataExchangerWithTpManager metaDataExchangerWithTpManager(&router, &core, &engine, &manager);

    HrpAPIServer apiServer(io_service, HRP_DEFAULT_API_PORT, router);

    core.set_engine(&engine);
    glb_engine = &engine;
    manager.init();
    engine.init();
    core.init();
    router.init();
    metaDataExchangerWithTpManager.init();
    apiServer.init();

    if (debug_mode)
        spdlog::set_level(spdlog::level::debug);

    //this_thread::sleep_for(chrono::seconds(10));
    thread &thread1 = engine.get_thread_io_service();
    thread1.join();
    return 0;
}


void sighandler(int signum)
{
    cout << "Caught signal " << strsignal(signum) << ", comming out\n";
    _logger->error("Caught signal {}, coming out", strsignal(signum));

#ifndef __APPLE__
    cout << "Terminating olsrd..." << endl;
    kill(pid, SIGTERM);
#endif

    void *array[10];
    size_t size;

#ifdef _DESKTOP_
    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", signum);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif

#ifdef _PROFILING_
    ProfilerStop();
#endif

    glb_engine->shutdown();

    exit(1);
}


string getIpAddr(const char *iface) {
    // source: https://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine

    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;
    string ret;

#ifdef __ANDROID__
    android_getifaddrs(&ifAddrStruct);
#else
    getifaddrs(&ifAddrStruct);
#endif


    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            if (strcmp(ifa->ifa_name, iface) == 0) {
                // is a valid IP4 Address
                tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                // printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
                ret = addressBuffer;
            }
        }
    }

    if (ifAddrStruct!=NULL)
#ifdef __ANDROID__
        android_freeifaddrs(ifAddrStruct);
#else
        freeifaddrs(ifAddrStruct);
#endif

    return ret;
}

pid_t proc_find() {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen("pidof olsrd", "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    int ret = -1;
    if (result.empty()) return ret;
    ret = std::stoi(result);
    return (pid_t)ret;
}
