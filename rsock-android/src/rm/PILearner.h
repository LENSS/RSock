//
// Created by Chen on 7/19/17.
//

#ifndef HRP_PILEARNER_H
#define HRP_PILEARNER_H

#include <chrono>
#include <mutex>
#include <queue>
#include <iostream>
#include <spdlog/spdlog.h>
#include <string>
#include <sstream>
#include "FILearner.h"
#include "common.h"
#include "hrpMessages.h"

namespace hrp {

    class PRDCompare {
    public:
        bool operator() (const pi_response_local& lhs, const pi_response_local& rhs) const {
            return (lhs.meta.seq > rhs.meta.seq);
        }
    };

    class PILearner {
    public:
        PILearner(RMCore &core, double alpha);

        /**
         * Need a way to
         *  1) send out probe,
         *  2) a callback function to collect delay
         *  3) a table to track outstanding probes
         *  4) a timeout mechanism to purge expired outstanding probes
         */

        /**
         * Prepare a probe, setup the meta data.
         * @return The struct that contains the meta data for next probe
         */
        pi_probe_meta prepareProbe();

        /**
         * Notify the PILearner that a new probe with the meta data is sent
         * @param meta The struct of meta data of the sent out probe
         */
        void sentProbe(const pi_probe_meta& meta);

        /**
         * Public interface for outside class. Receive the pi_response
         * @param data The data received
         */
        void recvResponse(const pi_response_tx& data);

        /**
         * Notify the PILearner that a new response is received for the probe
         * with meta data as parameter
         * @param meta The struct of meta data of the response
         * @param latency The latency reported by the peer
         */
        void recvResponse(const pi_response_tx& meta, double latency);

        bool shouldProbe() { return _curr_nr_of_round >= _nrof_round_per_epoch; }

        /**
         * Notify the PILearner that it can now update its internal data structures.
         * This includes:
         *  1) purge outdated outstanding probes
         *  2) update the core if necessary
         * This method should be invoked periodically by an external object
         */
        void update();

        /**
         * Getters and setters
         */
        unsigned int get_seq() const;
        unsigned int get_pi_epoch() const;
        const std::map<unsigned int, std::pair<unsigned int, std::chrono::steady_clock::time_point>> &get_track_table() const;
        unsigned int get_pi_action() const;
        const std::vector<pi_response_local> &get_loss_aggr() const;
        const std::priority_queue<pi_response_local, std::vector<pi_response_local>, PRDCompare> &
        get_out_order_buf() const;
        unsigned int get_response_seq() const;

        void set_pi_epoch(unsigned int _pi_epoch);
        void set_pi_timeout(double _pi_timeout);

        std::string format_probs() const { return _core.format_probs(); }

        std::string format_track_table() const;
        std::string format_out_of_order_buf() const;

    private:

        /**
         * Update the RMCore this learner is maintaining. When this method
         * is called, a _loss_aggr vector should be ready, i.e. _loss_aggr.size()
         * should equal to _pi_epoch. Then this method will construct appropriate
         * loss vector, update the RMCore, and clear _loss_aggr.
         */
        void updateCore();

        /**
         * Notify the PILearner that expired probes can now be deleted.
         */
        void purgeOutdatedProbes();

        /**
         * Move anything in the _out_of_order buf that is now in-order to
         * the _loss_aggr, and updateCore if necessary
         */
        void moveOutOfOrderBuf();

        /**
         * Draw next testing action. If _pi_action < _pi_epoch, return it
         * otherwise, return a random action drew from the _core
         * @return The action to be tested
         */
        unsigned int drawNextAction();

    private:
        /// Logger
        std::shared_ptr<spdlog::logger> _logger;

        RMCore &_core;

        unsigned int _seq;          /// current sequence number
        unsigned int _pi_epoch;     /// PI epoch, how many rounds until we update RMCore
        std::map<unsigned int, std::pair<unsigned int, std::chrono::steady_clock::time_point>> _track_table;
                                    /// A tracking table which maps sequence number of the time it sent out probe
        double      _pi_timeout;    /// PI timeout value. Outstanding probes are purged after this time
        unsigned int _pi_action;    /// Next test action

        int _nrof_round_per_epoch;  /// How many rounds until we send out next probe, used to stop probe if network is stable
                                    /// This build on the fact that update() must be called at each round
        int _curr_nr_of_round;      /// Current round

        unsigned int _response_seq; /// The expected next response seq
        std::vector<pi_response_local> _loss_aggr;   /// Buffer for in-order responses
        std::priority_queue<pi_response_local, std::vector<pi_response_local>, PRDCompare> _out_order_buf;
                                    /// Temporary buffer for out-of-order responses, sorted by the seq.

        double      _one_copy_latency;  /// average one copy latency baseline
        unsigned int _oc_latency_cnt;   /// number of received one copy latency

        double      _alpha;         /// alpha, the parameter for weighing delay and resource

        std::mutex  _mutex;
    };
}



#endif //HRP_PILEARNER_H
