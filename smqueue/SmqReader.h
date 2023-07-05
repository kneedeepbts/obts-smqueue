#ifndef SMQREADER_H_
#define SMQREADER_H_

#include "SmqMessageHandler.h"

namespace kneedeepbts::smqueue {
    class SmqReader {
    public:
        static std::string MQ_NAME;

        explicit SmqReader() {
            // create message handler
            mqueHan = new SmqMessageHandler(SmqReader::MQ_NAME);
            //smq_manager = smq;
            // Start reader thread
            //pthread_create(&mthread_ID, nullptr, SmqReaderThread, (void *) nullptr);
        }

        ~SmqReader();

        void run();

        static void *SmqReaderThread(void *ptr);

        // get handle to message queue
        SmqMessageHandler *getqueHan() {
            return mqueHan;
        }

    private:
        pthread_t mthread_ID{};
        SmqMessageHandler *mqueHan;
        bool please_re_exec{};
        //static SmqManager * smq_manager;
    };
}

#endif /* SMQREADER_H_ */
