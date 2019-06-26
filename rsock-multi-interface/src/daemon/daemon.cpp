//
// Cretaed by Chen Yang and Ala Altaweel
//

#include "daemon.h"

#include <unwind.h>
#include <dlfcn.h>
#include <iomanip>

using namespace hrp;
using namespace std;

//a smart pointer that retains shared ownership of an object through a pointer.
std::shared_ptr<spdlog::logger> _logger;
TxrxEngine *txrxGlb_engineObj = NULL;
// create an object of gns client
GnsServiceClient gnsObj;

// global variable of this node
hrpNode _thisNode;

//pid_t is a signed integer representing a process ID
pid_t pid = 0;
pid_t proc_find();
string getIpAddr(const char *);
void sighandler(int signum);
void dumpBacktrace(std::ostream& os, void** buffer, size_t count);

string firstIpAddr, firstInterface;
string secondIpAddr, secondInterface;

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
		//cout << "After setvbuf(stderr, 0, _IONBF, 0);"<< endl;
	
		/* create the pipe and redirect stdout and stderr */
		pipe(pfd);
		dup2(pfd[1], 1);
		dup2(pfd[1], 2);
	
		/* spawn the logging thread */
		if(pthread_create(&thr, 0, thread_func, 0) == -1)
			return -1;
	
		pthread_detach(thr);
		return 0;
	}

	void *thread_func(void*)
	{
		ssize_t rdsz;
		char buf[128];
		while((rdsz = read(pfd[0], buf, sizeof buf - 1)) > 0)
		{
			if(buf[rdsz - 1] == '\n')
				--rdsz;
			buf[rdsz] = 0;
			/* add null-terminator */
		        __android_log_write(ANDROID_LOG_DEBUG, tag, buf);
		}
		return 0;
	}
#endif

/**
* A main function to startup the daemon
* @param argc input arguments
* @param argv number of arguments
**/
int daemon_main(int argc, char** argv)
{
	// initialize loggers
	file_sink = make_shared<spdlog::sinks::stdout_sink_st>();
	_logger = std::make_shared<spdlog::logger>("Main", file_sink);

	string log_folder = "./";
	const char *homedir;
	if ((homedir = getenv("HOME")) == NULL)
	{
		homedir = getpwuid(getuid())->pw_dir;
		log_folder = string(homedir) + "/";
	}


	
	map<string, string> cfg;
	bool daemonize = false;


	if (argc < 2)
	{
		cout << "usage: ./rsockd -1 firstInterface -2 secondInterface [-a alpha] [-r rmax] [-t ita] [-p ict_probe] [-e mve_epoch] [-g debug] " << endl;
		return 1;
	}

	int opt = 0;   
	while ((opt = getopt(argc, argv, "w:1:2:a:r:t:p:e:gd")) != -1)
	{
		switch(opt)
		{
			case 'w':
				firstIpAddr = optarg;
			break;
			case '1':
				firstInterface = optarg;
			break;
			case '2':
				secondInterface = optarg;
			break;
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
			case 'g':
				debug_mode = true;
			break;
			case 'd':
			daemonize = true;
			break;
			default:
				std::cerr << "Unknown Command Line Argument\n";
			return 1;
		}
	}

	if (daemonize)
	{
		// make sure that the process runs in the background as system daemon
		// input parameters are for change working directory and to redirect standard input/output
		// if input parameters are non zeros, no change
		// the function returns zero on success
		if (daemon(1, 1) != 0)
		{
			_logger->error("error while daemonizing rsockd: {}", errno);
			exit(1);
		}
	}

	firstIpAddr = getIpAddr(firstInterface.c_str());
        //_logger->info("First IpAddr: {}", firstIpAddr);
	//cout << "First IpAddr: " << firstIpAddr << endl;

	secondIpAddr = getIpAddr(secondInterface.c_str());
        //_logger->info("Second IpAddr: {}", secondIpAddr);
	//cout << "Second IpAddr: " << secondIpAddr << endl;

	

	// get account name and GUID
	//string ownAccountName = gnsObj.getOwnAccountName();
        //_logger->info("Own account Name: {}", ownAccountName);
	//cout<<"Own account Name: "<< ownAccountName <<endl;
	string ownGUID =  gnsObj.getOwnGUID();
	if(ownGUID.size() != 0)			
		_logger->info("Own GUID: {}", ownGUID);
	else
	{
		_logger->error("empty GUID returned for me, check GNS exiting");
		exit(1);
	}
		
	/*
	// modify the olsrd config file. i.e., the PlParam "name" "" should be replaced to PlParam "name" "GUID"
	// open olsrd config file for reading
	std::ifstream olsrd_conf_file_r("olsrd.conf");
	vector<string> olsrd_conf_Lines;
	string olsrd_conf_line;
	// check if file found and opended
	if(!olsrd_conf_file_r)
	{
		_logger->error("Cannot open olsrd config file for reading, exiting");
		//cout << "Cannot open olsrd config file for reading, exiting" << endl;
		exit(1);
	}

	while (std::getline(olsrd_conf_file_r, olsrd_conf_line))
	{
		olsrd_conf_Lines.push_back(olsrd_conf_line);
	}
	// close the olsrd config file
	olsrd_conf_file_r.close();

	// find the line that contains the PlParam "name" ""
	// print olsrd config file contents
	// cout<<"olsrd config contnets:"<<endl;
	for(vector<string>::size_type i = 0; i < olsrd_conf_Lines.size(); i++)
	{
		// cout<<olsrd_conf_Lines.at(i)<<endl;		
		if (olsrd_conf_Lines.at(i).find("PlParam") != string::npos  && olsrd_conf_Lines.at(i).find("name") != string::npos)
		{
			//cout<<"PlParam name found:"<<olsrd_conf_Lines.at(i)<<endl;
			//ownGUID = "584CB67DC344691F212FFAF1CB20DF4D8F882CCD";
			olsrd_conf_Lines[i] = string("\t") + string("PlParam") + string(" ") + string("\"name\"") + string("\t\t\t") + string("\"") + ownGUID + string("\""); 
			//cout<<"PlParam name line replced:"<<olsrd_conf_Lines.at(i)<<endl;			
		}		
	}

	// open olsrd config file for writing
	std::ofstream olsrd_conf_file_w("olsrd.conf");
	// check if file found and opended
	if(!olsrd_conf_file_w)
	{
		_logger->error("Cannot open olsrd config file for writing, exiting");
		//cout << "Cannot open olsrd config file for writing, exiting" << endl;
		exit(1);
	}

	// overwite the contents of olsrd config file
	for(vector<string>::size_type i = 0; i < olsrd_conf_Lines.size(); i++)
	{
		//cout<<"write line:"<<olsrd_conf_Lines[i]<<endl;
		olsrd_conf_file_w << olsrd_conf_Lines[i];
		olsrd_conf_file_w<< endl;
	}
	// close the olsrd config file
	olsrd_conf_file_w.close();
	_logger->info("Update olsrd config file with GUID");
	//cout << "Update olsrd config file with GUID" << endl;
	*/

	//Specifies a function to handle the below signals
	signal(SIGINT, sighandler);
	signal(SIGSEGV, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGABRT, sighandler);

	

	//cout << "rsockd initiating... \n";
	//cout << "1st Interface = " << firstInterface << ", 1st IpAddr = " << firstIpAddr << endl;
	//cout << "2nd Interface = " << secondInterface << ", 2nd IpAddr = " << secondIpAddr << endl;


	_logger->info("checking if olsrd running...");
	//cout << "checking if olsrd running... \n";
	// check if olsrd is running
	pid = proc_find();
	if (pid == -1)
	{
#ifdef __ANDROID__

		_logger->info("Android");
		_logger->info("olsrd is not running, initiating...");
		//cout << "Android\n";
		//cout << "olsrd is not running, initiating..." << endl;

		//system("export LD_LIBRARY_PATH=/data/hrpd/lib/");
		system("/data/hrpd/bin/olsrd -f /data/hrpd/bin/olsrd.conf -d 0");
		//system("/data/hrpd/bin/olsrd -f /data/hrpd/bin/olsrd.conf -d 1 > olsr.log &");

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		pid = proc_find();
		if (pid == -1)
		{
			_logger->error("Cannot initiate olsrd, exiting");
			//cout << "Cannot initiate olsrd, exiting" << endl;
			exit(1);
		}
		else
		{
			_logger->info("olsrd successfully started");		
			//cout << "olsrd successfully started" << endl;
		}
#else
	
		_logger->info("A non-Andriod OS .. maybe Unix");
		_logger->info("olsrd is not running, initiating...");	
		//cout << "A non-Andriod OS .. maybe Unix\n";
		//cout << "olsrd is not running, initiating..." << endl;
		
		// start our modified olsrd
		//system("sudo ./olsrd -f ./olsrd.conf -d 0");
		//system("sudo ./olsrd -f ./olsrd.conf -d 9 > olsr.log &");

		// start original code olsrd
		system("sudo olsrd -f ./olsrd.conf -d 0");
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		pid = proc_find();
		if (pid == -1)
		{
			_logger->error("Cannot initiate olsrd, exiting");
			//cout << "Cannot initiate olsrd, exiting" << endl;
			exit(1);
		}
		else
		{
			_logger->info("olsrd successfully started");		
			//cout << "olsrd successfully started" << endl;
		}
#endif
	}

	else
	{
		_logger->info("found running olsrd, pid = {}", pid);		
		//cout << "found running olsrd, pid = " << pid << endl;
	}

	///////////////////////////// Start Up ///////////////////////////
	// Commented and Replaced by Ala
	/*
	//asynchronous I/O service
	asio::io_service io_service_Obj;
	TopologyManager topologyManagerObj(hrpNode(firstIpAddr), io_service_Obj);
	TxrxEngine txrxEngineObj(hrp::hrpNode(firstIpAddr), io_service_Obj, HRP_DEFAULT_DAEMON_PORT, &topologyManagerObj);
	HrpCore hrpCoreObj(firstIpAddr, cfg);
	HrpRouter hrpRouterObj(io_service_Obj, &hrpCoreObj, &txrxEngineObj);
	MetaDataExchangerWithTpManager metaDataExchangeObj(&hrpRouterObj, &hrpCoreObj, &txrxEngineObj, &topologyManagerObj);
	HrpAPIServer hrpAPIServerObj(io_service_Obj, HRP_DEFAULT_API_PORT, hrpRouterObj);
	
	hrpCoreObj.set_engine(&txrxEngineObj);
	txrxGlb_engineObj = &txrxEngineObj;

	topologyManagerObj.init();
	txrxEngineObj.init();
	hrpCoreObj.init();
	hrpRouterObj.init();
	metaDataExchangeObj.init();
	hrpAPIServerObj.init();

	if (debug_mode)
		spdlog::set_level(spdlog::level::debug);

	thread &threadRef = txrxEngineObj.get_thread_io_service();
	threadRef.join();
	
	return 0;
	*/

	_thisNode.push_back(ownGUID);

	_logger->info("rsockd initiating");
        
	if(firstIpAddr.size() != 0)
	{
	        _logger->info("1st Interface = {}, 1st IpAddr = {}", firstInterface, firstIpAddr);
		// No need to add IPs since we represtn the graph using GUIDs
		//_thisNode.push_back(firstIpAddr);
	}
	if(secondIpAddr.size() != 0)
	{
		_logger->info("2nd Interface = {}, 2nd IpAddr = {}", secondInterface, secondIpAddr);
		// No need to add IPs since we represtn the graph using GUIDs
		//_thisNode.push_back(secondIpAddr);
	}

	_logger->info(printHrpNode("This node", _thisNode));		

	//asynchronous I/O service
	asio::io_service io_service_Obj;
	//TopologyManager topologyManagerObj(_thisNode, io_service_Obj);
	TopologyManager topologyManagerObj(io_service_Obj);
	//TxrxEngine txrxEngineObj(_thisNode, io_service_Obj, HRP_DEFAULT_DAEMON_PORT, &topologyManagerObj);
	TxrxEngine txrxEngineObj(io_service_Obj, HRP_DEFAULT_DAEMON_PORT, &topologyManagerObj);
	//HrpCore hrpCoreObj(_thisNode, cfg);
	HrpCore hrpCoreObj(cfg);
	HrpRouter hrpRouterObj(io_service_Obj, &hrpCoreObj, &txrxEngineObj);
	MetaDataExchangerWithTpManager metaDataExchangeObj(&hrpRouterObj, &hrpCoreObj, &txrxEngineObj, &topologyManagerObj);
	HrpAPIServer hrpAPIServerObj(io_service_Obj, HRP_DEFAULT_API_PORT, hrpRouterObj);
	
	hrpCoreObj.set_engine(&txrxEngineObj);
	txrxGlb_engineObj = &txrxEngineObj;

	topologyManagerObj.init();
	txrxEngineObj.init();
	hrpCoreObj.init();
	hrpRouterObj.init();
	metaDataExchangeObj.init();
	hrpAPIServerObj.init();

	if (debug_mode)
		spdlog::set_level(spdlog::level::debug);

	thread &threadRef = txrxEngineObj.get_thread_io_service();
	threadRef.join();
	
	return 0;
	// End Ala	
}


/**
* A function to handle the signals	
* @param signum signal number
**/
void sighandler(int signum)
{
	//cout << "Caught signal " << strsignal(signum) << ", comming out\n";
	_logger->error("Caught signal {}, coming out", strsignal(signum));
#ifdef __ANDROID__
	const size_t max = 30;
	void* buffer[max];
	//std::ostringstream oss;
	//dumpBacktrace(oss, buffer, max);
#endif
	
	
#ifdef _DESKTOP_
	void *array[10];
	size_t size;
	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", signum);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif

	txrxGlb_engineObj->shutdown();
	exit(1);
}

/**
* A function to get Ip Address of current device 
* @param iface interface
* source: https://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine
**/
string getIpAddr(const char *iface)
{

	struct ifaddrs * ifAddrStruct=NULL;
	struct ifaddrs * ifa=NULL;
	void * tmpAddrPtr=NULL;
	string ret;

#ifdef __ANDROID__
	android_getifaddrs(&ifAddrStruct);
#else
	getifaddrs(&ifAddrStruct);
#endif

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (!ifa->ifa_addr)
		{
			continue;
		}
		if (ifa->ifa_addr->sa_family == AF_INET)
		{// check if it is IP4
			if (strcmp(ifa->ifa_name, iface) == 0)
			{
				// is a valid IP4 Address
				tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
				char addressBuffer[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
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
	// return IP address as string
	return ret;
}


#ifdef __ANDROID__
// backtrace for Android
void dumpBacktrace(std::ostream& os, void** buffer, size_t count)
{
    for (size_t idx = 0; idx < count; ++idx)
	{
		const void* addr = buffer[idx];
		const char* symbol = "";
		Dl_info info;
		
		if (dladdr(addr, &info) && info.dli_sname)
		{
			symbol = info.dli_sname;
		}
		//os << "  #" << std::setw(2) << idx << ": " << addr << "  " << symbol << "\n";
		string errorMsg = string(" #") + to_string(idx) + string(": ") + (char*)addr + string("  ") + symbol + string("\n");
		_logger->error("backtrace:\n {}",errorMsg);
	}
}
#endif


/**
* A function to process ID of olsrd daemon
* @return pid_t is a signed integer representing a process ID
**/
pid_t proc_find()
{
	std::array<char, 128> buffer;
	std::string result;
	std::shared_ptr<FILE> pipe(popen("pidof olsrd", "r"), pclose);
	if (!pipe)
		throw std::runtime_error("popen() failed!");
	while (!feof(pipe.get()))
	{
		if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
			result += buffer.data();
	}
	int ret = -1;
	if (result.empty())
		return ret;
	ret = std::stoi(result);
	return (pid_t)ret;
}
