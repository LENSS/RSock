//
// Cretaed by Chen Yang and Ala Altaweel
//

#include "TestAPI.h"

namespace hrp {
    int TestAPI::send(void *buf, size_t size, const hrpNode &dst, unsigned long ttl) {
        std::shared_ptr<HrpDataHeader> pkt = _router->newMessage(buf, size, dst, "testApp", ttl);
        if (pkt) return pkt->get_payload_size();
        return -1;
    }

    size_t TestAPI::recv(void *buf, size_t max_size, hrpNode &src, long &delay) {

        std::unique_lock<std::mutex> lk(_mutex_queue);
        _cv.wait(lk, [this]{ return !_queue.empty();});

        std::shared_ptr<HrpDataHeader> &pkt = _queue.front();
        if (pkt->get_payload_size() > max_size) throw hrpException("TestAPI::recv received data is larger than buffer");
        memcpy(buf, pkt->get_payload(), pkt->get_payload_size());

        long now = s_from_epoch();
        delay = now - pkt->get_gen_time();

        lk.unlock();
        return pkt->get_payload_size();
    }

    void TestAPI::recv_cb(std::shared_ptr<HrpDataHeader> pkt) {
        {
            std::lock_guard<std::mutex> lk(_mutex_queue);
            _queue.push(pkt);
        }
        _cv.notify_one();
    }
}
