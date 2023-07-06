/*
* Copyright 2008 Free Software Foundation, Inc.
* Copyright 2011, 2013, 2014 Range Networks, Inc.
*
* This software is distributed under multiple licenses;
* see the COPYING file in the main directory for licensing
* information for this specific distribuion.
*
* This use of this software may be subject to additional restrictions.
* See the LEGAL file in the main directory for details.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
 * SmqWriter.cpp
 *
 *  Created on: Nov 16, 2013
 *      Author: Scott Van Gundy
 */
#include "SmqWriter.h"

#include <string>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"

#include "QueuedMsgHdrs.h"
#include "SmqMessageHandler.h"

namespace kneedeepbts::smqueue {
    std::string SmqWriter::MQ_NAME = "/SmqWriter";

    SmqWriter::~SmqWriter() {
        delete mqueHan;
    }

    void SmqWriter::run() {
        pthread_create(&mthread_ID, nullptr, SmqWriterThread, (void *) nullptr);
    }

    void SmqWriter::stop() {
        stop_thread = true;
    }

    void *SmqWriter::SmqWriterThread(void *ptr) {
        SPDLOG_DEBUG("Start SMQ writer thread");

        SmqWriter * smqw = (SmqWriter *) ptr;

        char msgBuffer[MQ_MESSAGE_MAX_SIZE + 10];
        int bytesRead = 0;
        int msgCount = 0;
        SimpleWrapper *pWrap;
        QueuedMsgHdrs *pMsg;
        unsigned long currentSeconds;
        unsigned long lastRunSeconds;
        ShortMsgPending pendingMsg;

        // Queue opened  Process messages
        currentSeconds = getCurrentSeconds();
        lastRunSeconds = currentSeconds;
        SPDLOG_DEBUG("Enter writer thread loop");
        while (!smqw->stop_thread) {

            //LOG(DEBUG) <<"Start SMQ writer thread loop";
            bytesRead = smqw->getqueHan()->SmqWaitforMessage(200, msgBuffer, (int) sizeof(msgBuffer));
            currentSeconds = getCurrentSeconds();
            SPDLOG_DEBUG("Got return from SmqWaitforMessage in writer, status: {}", bytesRead);
            if (bytesRead < 0) {
                SPDLOG_DEBUG("Writer failed to get message:", bytesRead);

            } else if (bytesRead == 0) {
                // ******** Got timeout ***************
                SPDLOG_DEBUG("Got timeout in writer thread");

                // NOTE: This appears to be getting bytes from a buffer (or mqueue?)
                //       and placing them into messages.  If no bytes are gathered,
                //       then the queue is processed.  This is a very naive loop,
                //       and appears to be placed in the completely wrong place.

//                smq_manager->process_timeout();  // Process entries in queue
                //LOG(DEBUG) << "Return from process_timeout";

//                if ((currentSeconds - lastRunSeconds) > 60) {
//                    LOG(DEBUG) << "Run once a minute stuff";
//                    // NOTE: Why would this need to be re-run?  We can restart if the config changes.
//                    //smq_manager->InitInsideReaderLoop(); // Updates configuration
//
////                    int queueSize = smqw->smq_manager->time_sorted_list.size();
////                    if (queueSize > 0) {
////                        SPDLOG_DEBUG("Queue size: {}", queueSize);
////                    }
//
//                    // Save queue to file on timeout
//                    //LOG(DEBUG) << "Enter save_queue_to_file";
////                    if (!smq_manager->save_queue_to_file()) {  // Save queue file each timeout  may want to slow this down
////                        //LOG(WARNING) << "Failed to read queue file on timeout file:" << smqw->smq_manager->savefile;
////                    }
//                    lastRunSeconds = currentSeconds;
//                }

            } else if (bytesRead > 0) {
                // ********* Got message *************
                SPDLOG_DEBUG("Received message in writer thread length: {}", bytesRead);
                pWrap = (SimpleWrapper *) msgBuffer;
                pMsg = pWrap->getMsgPtr();

                // PROCESS MESSAGES HERE
                pMsg->ProcessMessage();

                delete pMsg;
            } // Got message
        } // while

        SPDLOG_DEBUG("End SMQ writer thread");
        return nullptr;
    }


    /*
     * Send a QueuedMsgHdrs to writer queue
     * Deleted SimpleWrapper receiver deletes attached message
     */
    int SmqWriter::SendWriterMsg(SimpleWrapper* pMsg) {
        int iret;
        iret = getqueHan()->SmqSendMessage(pMsg);
        delete pMsg;  // Delete SimpleWrapper
        return iret;
    }


    /*
        Send a message to the writer thread asking for a sip ack to be sent
     */
    void SmqWriter::queue_respond_sip_ack(int errcode, kneedeepbts::smqueue::ShortMsgPending *shortmsg, char * netaddr, size_t netaddrlen) {
        LOG(DEBUG) << "Send SIP ACK queue request";
        QueuedMsgHdrs* pMsg = new SIPAckMessage(errcode, shortmsg, netaddr, netaddrlen);
        SimpleWrapper* sWrap = new SimpleWrapper(pMsg);
        SendWriterMsg(sWrap);  // Message gets deleted in here
    }


    // Signal writer thread to process incoming message
    void SmqWriter::ProcessReceivedMsg() {
        LOG(DEBUG) << "Signal writer thread ProcessReceivedMsg";
        QueuedMsgHdrs* pMsg = new ProcessIncommingMsg();
        SimpleWrapper* sWrap = new SimpleWrapper(pMsg);
        SendWriterMsg(sWrap);
    }
}
