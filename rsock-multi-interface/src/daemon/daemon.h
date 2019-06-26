//
// Cretaed by Chen Yang and Ala Altaweel
//

#ifndef HRP_DAEMON_H
#define HRP_DAEMON_H

#include "common.h"
#include <string>
#include <sys/types.h>
#include <string>
#include <string.h>
#include <api/HrpAPIServer.h>
#include <pwd.h>
#include <unistd.h>
#include "gns/GnsServiceClient.h"
#include <fstream>



// if we are in linux, then use this to print backtrace
//#define _DESKTOP_



#ifdef _DESKTOP_
	#include <execinfo.h>
#endif

#include "hrp/HrpCore.h"
#include "hrp/MetaDataExchanger.h"
#include "hrp/TxrxEngine.h"
#include "api/TestAPI.h"

// Android does not have getifaddr() in NDK, so we have to provide it.
// Code is from a Mozila site.
#ifdef __ANDROID__
	#include "ifaddrs-android.h"
	#include <android/log.h>
#else
	#include <ifaddrs.h>
#endif


int daemon_main(int argc, char** argv);

#ifdef __ANDROID__
	int start_logger(const char *app_name);
	void *thread_func(void*);
#endif

#endif //HRP_DAEMON_H
