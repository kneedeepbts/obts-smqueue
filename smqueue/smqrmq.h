#ifndef OBTS_SMQUEUE_SMQRMQ_H
#define OBTS_SMQUEUE_SMQRMQ_H

#include <cstdint>

#include "rawmessage.h"
#include "smqthreadsafequeue.h"

namespace kneedeepbts::smqueue {
    class SmqRmq {
    public:
        std::uint32_t queue_count();
        RawMessage queue_front();
        void queue_pop();
        void queue_push(RawMessage rmsg);

    private:
        SmqThreadSafeQueue<RawMessage> m_tsq{};
    };
}

#endif //OBTS_SMQUEUE_SMQRMQ_H
