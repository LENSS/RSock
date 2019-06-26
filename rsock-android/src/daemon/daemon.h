//
// Created by Chen on 2/5/18.
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

// if we are in linux/apple, then use this to print backtrace
//#define _DESKTOP_

// if we want to use google profiling tool
//#define _PROFILING_

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

#ifdef _PROFILING_
#include <gperftools/profiler.h>
#endif

int daemon_main(int argc, char** argv);

#ifdef __ANDROID__
int start_logger(const char *app_name);
void *thread_func(void*);
#endif

#endif //HRP_DAEMON_H
