//
// Cretaed by Chen Yang and Ala Altaweel
//

#ifndef HRP_TESTAPI_H
#define HRP_TESTAPI_H

#include <condition_variable>
#include <string.h>
#include "hrp/HrpRouter.h"
#include "hrpMessages.h"
#include "hrpNode.h"

namespace hrp {
    class TestAPI {
    public:
        explicit TestAPI(HrpRouter *router) : _router(router) {
            _router->registerCallback(std::bind(&TestAPI::recv_cb, this, std::placeholders::_1));
        }

        /**
         * Async send function. Push the data into hrp router.
         * @param buf The buffer address
         * @param size Size of the data
         * @param dst Destination of the data
         * @param ttl Time-to-live of the data
         * @return
         */
        int send(void *buf, size_t size, const hrpNode &dst, unsigned long ttl);

        /**
         * Sync call of the reception function. Will block until a data is received.
         * @param buf   The receiving buffer
         * @param max_size  The size of the buffer
         * @param src   The source of the data
         * @param delay The delay of the data (for statistic purpose)
         * @return  The size of the data being read
         */
        size_t recv(void *buf, size_t max_size, hrpNode &src, long &delay);

        /**
         * Callback function for the router to notify this api that a data is received.
         * @param pkt
         */
        void recv_cb(std::shared_ptr<HrpDataHeader> pkt);

    private:
        HrpRouter *_router;

        std::queue<std::shared_ptr<HrpDataHeader>> _queue;
        std::mutex _mutex_queue;
        std::condition_variable _cv;
    };
}

#endif //HRP_TESTAPI_H
