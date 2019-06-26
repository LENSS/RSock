//
//

#ifndef HRP_COMMON_H
#define HRP_COMMON_H

#include <string>
#include <sstream>
#include <spdlog/sinks/file_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
///
/// Configuration strings
#define HRP_CFG_ALPHA   "hrp_alpha"
#define HRP_CFG_ITA     "hrp_ita"
#define HRP_CFG_RMAX    "hrp_rmax"
#define HRP_CFG_ICT_PROBE   "hrp_interprobe"
#define HRP_CFG_MVE_EPOCH   "hrp_mve_epoch"

#define HRP_NULL_NODE "null_node"   /// Null node representation

#define HRP_DEFAULT_ALPHA 0.5               /// Default alpha for RM module
#define HRP_DEFAULT_ITA 0.8                 /// Default ita for RM module
//#define HRP_DEFAULT_RMAX 3                  /// Default rmax for RM module
#define HRP_DEFAULT_RMAX 1                  /// Default rmax for RM module
#define HRP_DEFAULT_ICT_PROBE 2             /// Default probe interval for MVE estimation
#define HRP_DEFAULT_MVE_EPOCH_DURATION 10   /// Default MVE estimator epoch duration, 30 seconds
#define HRP_DEFAULT_PI_PROBE_EXPIRATION 10  /// Default PI learner probe expiration time, 10 seconds
#define HRP_DEFAULT_OLSR_FETCH_EPOCH   2    /// Default timer value for fetching olsr information
#define HRP_DEFAULT_PKT_TTL             10  /// Default ttl for a packet, 10 seconds

#define HRP_DEFAULT_API_PORT    19999
// Commented out and modified by Ala Altaweel
//#define HRP_DEFAULT_BUF_SIZE    1024
//#define HRP_DEFAULT_DATA_BUF_SIZE    1048576   /// 1 MB Default buffer size for data transfer between API server and client
#define HRP_DEFAULT_DATA_BUF_SIZE    5242880   /// 5 MB, the Default buffer size for data transfer between API server and client
extern std::string log_file_name;           /// Global variable for the name of the log file this daemon is to use
extern std::shared_ptr<spdlog::sinks::stdout_sink_st> file_sink;
extern bool debug_mode;

inline long ms_from_epoch() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    long duration = value.count();
    return duration;
}

#endif //HRP_COMMON_H
