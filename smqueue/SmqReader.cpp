#include "SmqReader.h"

#include <string>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"

//#include "QueuedMsgHdrs.h"
//#include "SmqMessageHandler.h"

namespace kneedeepbts::smqueue {
//    std::string SmqReader::MQ_NAME = "/SmqReader";

//    SmqReader::~SmqReader() {
//        delete mqueHan;
//    }

    std::thread SmqReader::run() {
        return std::thread(&SmqReader::reader_thread, this);
    }

    void SmqReader::stop() {
        SPDLOG_DEBUG("Stopping reader thread.");
        m_stop_thread = true;
    }

    void SmqReader::reader_thread() {
        SPDLOG_DEBUG("Starting the reader thread.");

        while(!m_stop_thread) {
            SPDLOG_DEBUG("Pushing a SimpleWrapper onto the queue.");
            m_tsq.push(SimpleWrapper(nullptr));
            sleep(1);
            SPDLOG_DEBUG("Fronting a SimpleWrapper from the queue.");
            SimpleWrapper tmp = m_tsq.front();
            sleep(1);
            SPDLOG_DEBUG("Popping a SimpleWrapper from the queue.");
            m_tsq.pop();
            sleep(1);
        }

        SPDLOG_DEBUG("Reader thread finishing.");
    }

    // FIXME: This whole structure is bass-ackwards.  The smq_manager should run the main loop, not the reader.
    void *SmqReader::SmqReaderThread(void *ptr) {
        LOG(DEBUG) << "Start SMQ reader thread";

        //char msgBuffer[MQ_MESSAGE_MAX_SIZE + 10];  // Must be larger than size speced in attr
        //int bytesRead = 0;
        //int msgCount = 0;

//        smq_manager->InitBeforeMainLoop();
//        smq_manager->InitInsideReaderLoop(); // Updates configuration  do here to make sure everything is setup for threads

        // Queue opened  Process messages
//        LOG(DEBUG) << "Enter reader thread loop";
//        while (!smq_manager->stop_main_loop) {
//
//            smq_manager->main_loop(60000);

// FIXME: Is the below in the main_loop?
#if 0
            // Put this back in if process messages in reader thread
            LOG(DEBUG) <<"Start SMQ reader thread loop";
            bytesRead = smqReader->getqueHan()->SmqWaitforMessage(2000, msgBuffer, sizeof(msgBuffer));  // Wait for message

            //LOG(DEBUG) << "Got return from SmqWaitforMessage in reader thread status:%d", bytesRead);
            if (bytesRead < 0) {
                // GOT ERROR
                LOG(DEBUG) << "Reader failed to get message:" << bytesRead;

            } else if (bytesRead == 0) {
                // TIMEOUT
                LOG(DEBUG) << "Got timeout waiting for reader message";
                //Add timeout stuff here

                //LOG(DEBUG) << "Enter InitInsideReaderLoop";
                smq.InitInsideReaderLoop(); // Updates configuration  Moved this to writer queue
                //LOG(DEBUG) << "Return from InitInsideReaderLoop";

                // This is where all the work is done
                // Message are read in here
                //LOG(DEBUG) << "Enter main_loop";
                smq.main_loop(2000);
                //LOG(DEBUG) << "Return from main_loop";

            } else if (bytesRead > 0) {
                // GOT MESSAGE
                LOG(DEBUG) << "Received message in reader thread length;" << bytesRead;
                // PROCESS MESSAGES HERE
#if 0
                switch (msgType) {
                case QueuedMsgHdrs::TestMessage:

                    break;

                case QueuedMsgHdrs::SendResponse:   // this is a writer message

                    break;
                } // switch
#endif
            }
#endif
        //} // while

//        smq_manager->CleaupAfterMainreaderLoop();
//
//        LOG(DEBUG) << "End SMQ reader thread";
        return nullptr;
    }

}
