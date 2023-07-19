#include "SmqWriter.h"

#include <cerrno>
#include <chrono>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

namespace kneedeepbts::smqueue {
    SmqWriter::SmqWriter() : SmqThread(), SmqRmq(), NetSocket(SocketType::TYPE_DGRAM) {}

    void SmqWriter::thread() {
        SPDLOG_DEBUG("Starting the writer thread");

        // Initialize things

        // Loop through:
        //    Get a message off the queue
        //    Create the address and peer
        //    Send the message
        while(!m_stop_thread) {
            //if(m_tsq.count() != 0) {
            if(queue_count() != 0) {
                process_message();
            } else {
                // FIXME: Service the host so the messages actually get sent.
                std::this_thread::sleep_for(std::chrono::milliseconds(m_timeout_ms));
            }
        }

        // Clean up before closing the thread
    }

    void SmqWriter::process_message() {
        SPDLOG_DEBUG("Processing message");
        // Get a message off the queue
        RawMessage rmsg = queue_front();
        queue_pop();

        // Create the address and peer
        set_address(rmsg.m_to_address);
        set_port(rmsg.m_to_port);

        // Send the message
        // FIXME: Error handling?
        sendto(m_socket, rmsg.m_bytes.data(), rmsg.m_bytes.size(), 0, reinterpret_cast<sockaddr*>(&m_addr), sizeof(m_addr));
    }

//    void *SmqWriter::SmqWriterThread(void *ptr) {
//        SPDLOG_DEBUG("Start SMQ writer thread");
//
//        SmqWriter * smqw = (SmqWriter *) ptr;
//
//        char msgBuffer[MQ_MESSAGE_MAX_SIZE + 10];
//        int bytesRead = 0;
//        int msgCount = 0;
//        SimpleWrapper *pWrap;
//        QueuedMsgHdrs *pMsg;
//        unsigned long currentSeconds;
//        unsigned long lastRunSeconds;
//        ShortMsgPending pendingMsg;
//
//        // Moving into the Thread to try to solve pointer non-sense
//        SmqMessageHandler * mqueHan = new SmqMessageHandler(SmqWriter::MQ_NAME);
//
//        // Queue opened  Process messages
//        currentSeconds = getCurrentSeconds();
//        lastRunSeconds = currentSeconds;
//        SPDLOG_DEBUG("Enter writer thread loop");
//        while (!smqw->stop_thread) {
//
//            //LOG(DEBUG) <<"Start SMQ writer thread loop";
//            // FIXME: Pretty sure there's a seg fault happening here.  Probably because of the pointer non-sense.
//            //bytesRead = smqw->getqueHan()->SmqWaitforMessage(200, msgBuffer, (int) sizeof(msgBuffer));
//            bytesRead = mqueHan->SmqWaitforMessage(200, msgBuffer, (int) sizeof(msgBuffer));
//            currentSeconds = getCurrentSeconds();
//            SPDLOG_DEBUG("Got return from SmqWaitforMessage in writer, status: {}", bytesRead);
//            if (bytesRead < 0) {
//                SPDLOG_DEBUG("Writer failed to get message:", bytesRead);
//
//            } else if (bytesRead == 0) {
//                // ******** Got timeout ***************
//                SPDLOG_DEBUG("Got timeout in writer thread");
//
//                // NOTE: This appears to be getting bytes from a buffer (or mqueue?)
//                //       and placing them into messages.  If no bytes are gathered,
//                //       then the queue is processed.  This is a very naive loop,
//                //       and appears to be placed in the completely wrong place.
//
////                smq_manager->process_timeout();  // Process entries in queue
//                //LOG(DEBUG) << "Return from process_timeout";
//
////                if ((currentSeconds - lastRunSeconds) > 60) {
////                    LOG(DEBUG) << "Run once a minute stuff";
////                    // NOTE: Why would this need to be re-run?  We can restart if the config changes.
////                    //smq_manager->InitInsideReaderLoop(); // Updates configuration
////
//////                    int queueSize = smqw->smq_manager->time_sorted_list.size();
//////                    if (queueSize > 0) {
//////                        SPDLOG_DEBUG("Queue size: {}", queueSize);
//////                    }
////
////                    // Save queue to file on timeout
////                    //LOG(DEBUG) << "Enter save_queue_to_file";
//////                    if (!smq_manager->save_queue_to_file()) {  // Save queue file each timeout  may want to slow this down
//////                        //LOG(WARNING) << "Failed to read queue file on timeout file:" << smqw->smq_manager->savefile;
//////                    }
////                    lastRunSeconds = currentSeconds;
////                }
//
//            } else if (bytesRead > 0) {
//                // ********* Got message *************
//                SPDLOG_DEBUG("Received message in writer thread length: {}", bytesRead);
//                pWrap = (SimpleWrapper *) msgBuffer;
//                pMsg = pWrap->getMsgPtr();
//
//                // PROCESS MESSAGES HERE
//                pMsg->ProcessMessage();
//
//                delete pMsg;
//            } // Got message
//        } // while
//
//        SPDLOG_DEBUG("End SMQ writer thread");
//        return nullptr;
//    }
//
//
//    /*
//     * Send a QueuedMsgHdrs to writer queue
//     * Deleted SimpleWrapper receiver deletes attached message
//     */
//    int SmqWriter::SendWriterMsg(SimpleWrapper* pMsg) {
//        int iret;
//        iret = getqueHan()->SmqSendMessage(pMsg);
//        delete pMsg;  // Delete SimpleWrapper
//        return iret;
//    }
//
//
//    /*
//        Send a message to the writer thread asking for a sip ack to be sent
//     */
//    void SmqWriter::queue_respond_sip_ack(int errcode, kneedeepbts::smqueue::ShortMsgPending *shortmsg, char * netaddr, size_t netaddrlen) {
//        LOG(DEBUG) << "Send SIP ACK queue request";
//        QueuedMsgHdrs* pMsg = new SIPAckMessage(errcode, shortmsg, netaddr, netaddrlen);
//        SimpleWrapper* sWrap = new SimpleWrapper(pMsg);
//        SendWriterMsg(sWrap);  // Message gets deleted in here
//    }
//
//
//    // Signal writer thread to process incoming message
//    void SmqWriter::ProcessReceivedMsg() {
//        LOG(DEBUG) << "Signal writer thread ProcessReceivedMsg";
//        QueuedMsgHdrs* pMsg = new ProcessIncommingMsg();
//        SimpleWrapper* sWrap = new SimpleWrapper(pMsg);
//        SendWriterMsg(sWrap);
//    }
}
