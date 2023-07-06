#ifndef SMQWRITER_H_
#define SMQWRITER_H_

#include <string>

#include "SmqMessageHandler.h"

namespace kneedeepbts::smqueue {
    class SmqWriter {
    public:
        static std::string MQ_NAME;

        explicit SmqWriter() {
            mqueHan = new SmqMessageHandler(SmqWriter::MQ_NAME);
            //m_savefile = savefile;
            //smq_manager = smq;
            //pthread_create(&mthread_ID, nullptr, SmqWriterThread, (void *) this);
        }

        ~SmqWriter();

        void run();
        void stop();

        static void *SmqWriterThread(void *ptr);

        SmqMessageHandler *getqueHan() {
            return mqueHan;
        }

        // NOTE: Moving from SmqMessageHandler
        int SendWriterMsg(SimpleWrapper* pMsg);
        void queue_respond_sip_ack(int errcode, kneedeepbts::smqueue::ShortMsgPending *shortmsg, char * netaddr, size_t netaddrlen);
        void ProcessReceivedMsg();

    private:
        pthread_t mthread_ID;
        SmqMessageHandler *mqueHan;
        //std::string m_savefile;
        //static SmqManager * smq_manager; // FIXME: Work this one out of the picture.

        bool stop_thread = false;

    };
}
#endif /* SMQWRITER_H_ */
