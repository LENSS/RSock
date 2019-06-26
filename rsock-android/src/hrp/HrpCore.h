//
// Created by Chen on 7/24/17.
//

#ifndef HRP_HRPCORE_H
#define HRP_HRPCORE_H

#include <string>
#include <mutex>
#include <map>
#include <asio.hpp>
#include <asio/deadline_timer.hpp>
#include "common.h"
#include "hrpNode.h"
#include "mve/MVECore.h"
#include "mve/MVEEstimator.h"
#include "rm/RMCore.h"
#include "rm/FILearner.h"
#include "rm/PILearner.h"
#include "hrpMessages.h"
#include "TxrxEngine.h"
#include "spdlog/spdlog.h"

namespace hrp {

    /**
     * A node entry in the neighborhood database
     */
    typedef struct struct_db_node_entry{

        struct_db_node_entry() {
            _mutex_mve = new std::mutex();
        }

        /// node id
        hrpNode    _nid;

        /// mve core distribution
        MVECore *_mve_core;

        /// bloom filter
        bloom_filter *_bloom_filter;

        /// mutex for locking _mve_core
        std::mutex* _mutex_mve;

        /// Regret-minimization core
        RMCore  *_fi_rm_core;
        RMCore  *_pi_rm_core;

        /// Learners
        FILearner *_fi_learner;
        PILearner *_pi_learner;

        /// Peer's current pi_seq. Any request with less than this seq number are ignored
        unsigned int _pi_seq;

        /// The time when first met with this node
        std::chrono::steady_clock::time_point _start_time;

        /// The total duration when this node is connected
        std::chrono::duration<double>   _ct_duration;

        /// The time when we last met with this node
        std::chrono::steady_clock::time_point _last_ct_time;

        /// flag for in contact
        bool isConnected;


    } db_node_entry;

    /**
     * HRP library's main class of various interfaces
     */
    class HrpCore {
    public:
        HrpCore(const hrpNode &nid, const std::map<std::string, std::string> &cfg);

        ~HrpCore();

        /**
         * Initialize/shutdown the HrpCore.
         */
        void init();
        void shutdown();

        void set_engine(TxrxEngine *_engine);

        /// Tx/Rx related incoming interfaces
        /**
         * Callback function for scheduling transmission
         * @param pkt The packet that was scheduled
         * @param success If it was a success
         */
        void TxrxEngineCallback(std::shared_ptr<HrpDataHeader> pkt, bool success);

        /**
         * Notify the HRPCore that a trans. has started. The HRPCore
         * should notify PILearner if the message is a pi_probe
         * @param header The header of the message being transmitted
         */
        void txStarted(HrpDataHeader &header);

        /**
         * Notify the HRPCore that a trans. has finished successfully.
         * This means that the intended next carrier has received the packet.
         * HRPCore should now modify the header with proper information.
         * Note: this method is probably unnecessary here?
         * @param header The header of the message having been transmitted
         */
        void txCompleted(HrpDataHeader &header);

        /**
         * Notify the HRPCore that a trans. was aborted, possibily due to
         * the break of a link. (What should the HRPCore do here?)
         * @param header The header of the message being aborted
         */
        void txAborted(HrpDataHeader &header);

        /**
         * Notify the HRPCore that a message was successfully received.
         * If the message is a response to a probe, HRPCore should notify
         * the PILearner. If the message is a data message, the HRPCore
         * should modify the header properly.
         * @param header The header of the message received
         */
        void rxReceived(std::shared_ptr<HrpDataHeader> header);

        /// Forwarding decision
        /**
         * Receive a quiry for whether the message should be transmitted. Return
         * true if it should be transmitted based on the HRP forwarding policy.
         * @param header The header of the message being considered
         * @return  The decided next carrier, or nullptr if should not forward
         */
        std::shared_ptr<hrpNode> calcRoutingDecision(std::shared_ptr<HrpDataHeader> header);

        /// Draw replication factors
        /**
         * Draw a replication factor.
         * @param dst The destination
         * @return The repl. factor drawed from the distribution learned by RM module
         */
        unsigned int drawReplFactor(const hrpNode& dst);

        /// Node related interfaces
        /**
         * Notify the HRPCore that a new node is connected, while passing in the
         * collected MVECore data exchanged from the peer.
         * @param peer The new node connected
         * @param core The MVECore collected from the peer
         */
        void nodeConnected(const hrpNode &peer, const MVECore &core, const bloom_filter &filter);

        /**
         * Notify the HRPCore that the node is disconnected.
         * @param peer The node that is disconnected
         */
        void nodeDisconnected(const hrpNode &peer);

        /**
         * Get a copy of the MVECore that this node is maintaining
         * @return  The MVECore copy
         */
        MVECore get_mveCore() {
            std::lock_guard<std::mutex> lg(_mutex_mveCore);
            MVECore ret(_mveCore);
            return ret;
        }

        const hrpNode &get_thisNode() const;

    private:

        /**
         * Schedule the transmissions of PI probes for each neighbor, and
         * schedule the next timer to fire
         * @param error
         */
        void scheduleProbes(const std::error_code &error);

        /**
         * Schedule the next timer for a PI probe event
         */
        void schedulePIProbeTimer();

        /**
         * Return the calculated ACR metric
         * @param candidate The node being considered
         * @param pkt   The packet being considered
         * @param mveCore   The dst MVECore
         * @return  The calculated ACR metric
         */
        double acrCalc(const hrpNode &candidate, std::shared_ptr<HrpDataHeader> pkt, const MVECore &mveCore);

//        /**
//         * Return the total contact rate to the destination
//         * @param candidate The candidate node being considered
//         * @param pkt The packet
//         * @param mveCore The MVECore of the candidate
//         * @return The total rate
//         */
//        double ttrCalc(const hrpNode &candidate, std::shared_ptr<HrpDataHeader> pkt, const MVECore &mveCore);

        void updateFILearners();

    public:
        const std::map<hrpNode, db_node_entry> &get_neighbors_db() const;

    private:
        /// Logger
        std::shared_ptr<spdlog::logger> _logger;

        hrpNode _thisNode;

        /// Various parameters
        unsigned int _rmax;             /// maximum number of replication
        double _alpha;                  /// parameter for weighing the performance v.s. replication
        double _ita;                    /// learning rate
        double _inter_probe_duration;   /// duration between two probes for RM Partial info learner
        double _mve_epoch_duration;     /// duration for one time window for mve estimation


        /// Flags
        //bool    _is_probe;            /// flag for whether we should continue probing for repl. (maybe this should be per destination?)


        /// MVE Module objects
        MVECore _mveCore;
        std::mutex _mutex_mveCore;
        MVEEstimator _mveEstimator;

        /// Neighborhood database
        std::map<hrpNode, db_node_entry> _neighbors_db;
        std::mutex _mutex_ngbr_db;

        TxrxEngine *_engine;

        // Timers
        asio::steady_timer  *_pi_timer;    /// Regret-min timer for PI learner
    };
}

#endif //HRP_HRPCORE_H
