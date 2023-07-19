#include "smqrmq.h"

namespace kneedeepbts::smqueue {
    std::uint32_t SmqRmq::queue_count() {
        return m_tsq.count();
    }

    RawMessage SmqRmq::queue_front() {
        return m_tsq.front();
    }

    void SmqRmq::queue_pop() {
        m_tsq.pop();
    }

    void SmqRmq::queue_push(RawMessage rmsg) {
        m_tsq.push(std::move(rmsg));
    }
}