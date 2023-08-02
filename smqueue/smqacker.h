#ifndef OBTS_SMQUEUE_SMQACKER_H
#define OBTS_SMQUEUE_SMQACKER_H

#include <cstdint>
#include <string>
#include <thread>

#include "smqthreadsafequeue.h"
#include "smqthread.h"
#include "SmqReader.h"
#include "SmqWriter.h"
#include "shortmsgpending.h"

namespace kneedeepbts::smqueue {
    class SmqAcker : public SmqThread {
    public:
        SmqAcker(SmqReader * reader, SmqWriter * writer) : SmqThread(), m_reader(reader), m_writer(writer) {}

        uint32_t queue_count();
        ShortMsgPending queue_front();
        void queue_pop();

    private:
        void thread() override;

        void process_message();
        uint32_t validate_msg(ShortMsgPending * smp);
        uint32_t verify_ack(ShortMsgPending * smp);
        uint32_t verify_msg(ShortMsgPending * smp);

        void respond_sip_ack(int errcode, ShortMsgPending *smp, char *netaddr, size_t netaddrlen);

        SmqReader * m_reader;
        SmqWriter * m_writer;

        SmqThreadSafeQueue<ShortMsgPending> m_tsq{};
    };
}

#endif //OBTS_SMQUEUE_SMQACKER_H
