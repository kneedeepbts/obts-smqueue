#ifndef SMQREADER_H_
#define SMQREADER_H_

#include <cstdint>
#include <string>
#include <thread>

#include <enet/enet.h>

//#include "SmqMessageHandler.h"

#include "QueuedMsgHdrs.h"
#include "smqthreadsafequeue.h"

/* It appears as though the SmqReader thread was originally used to pull packets
 * from the network and place them into a queue.  This queue was managed-ish by
 * the SmqMessageHandler class (under the SmqWriter, the instance in the
 * SmqReader doesn't appear to be used) and accessed by calling across to the
 * SmqWriter through a mess of global pointers.  At some point, the logic for
 * pulling from the network was moved to the SmqManager::main_loop method, but
 * that method was still called/controlled in the SmqReader thread.
 *
 * Now, the SmqReader class will pull the packets from the network and place
 * them into a queue.  That queue will be made accessible to a "processor"
 * thread, which will process them and route the messages.
 */
namespace kneedeepbts::smqueue {
    class SmqReader {
    public:
        SmqReader(std::string listen_address, uint16_t listen_port);

        std::thread run();
        void stop();

        void reader_thread();

        static void *SmqReaderThread(void *ptr);

    private:
        void process_event(ENetEvent * event);
        uint32_t validate_msg(ShortMsgPending * smp);
        uint32_t verify_ack(ShortMsgPending * smp);
        uint32_t verify_msg(ShortMsgPending * smp);

        SmqThreadSafeQueue<ShortMsgPending> m_tsq{};
        bool m_stop_thread = false;

        // NOTE: These are being put here so they can be pulled out into the
        //       configuration at some point.
        uint32_t m_timeout_ms = 100; // Short timeout to keep the thread responsive.
        uint32_t m_num_connections = 100; // Allow 100 active connections.

        ENetAddress m_net_address{};
        ENetHost * m_net_server = nullptr;
    };
}

#endif /* SMQREADER_H_ */
