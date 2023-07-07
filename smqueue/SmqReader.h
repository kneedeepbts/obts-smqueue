#ifndef SMQREADER_H_
#define SMQREADER_H_

#include <thread>

//#include "SmqMessageHandler.h"

#include "QueuedMsgHdrs.h"
#include "smqthreadsafequeue.h"

namespace kneedeepbts::smqueue {
    class SmqReader {
    public:
        //static std::string MQ_NAME;

//        explicit SmqReader() {
//            // create message handler
//            //mqueHan = new SmqMessageHandler(SmqReader::MQ_NAME);
//            //smq_manager = smq;
//            // Start reader thread
//            //pthread_create(&mthread_ID, nullptr, SmqReaderThread, (void *) nullptr);
//        }

        //~SmqReader();

        std::thread run();
        void stop();

        void reader_thread();

        static void *SmqReaderThread(void *ptr);

//        // get handle to message queue
//        SmqMessageHandler *getqueHan() {
//            return mqueHan;
//        }

    private:
        //pthread_t mthread_ID{};
        //SmqMessageHandler *mqueHan;
        //bool please_re_exec{};
        //static SmqManager * smq_manager;

        SmqThreadSafeQueue<SimpleWrapper> m_tsq{};
        bool m_stop_thread = false;
    };
}

#endif /* SMQREADER_H_ */
