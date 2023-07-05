#ifndef SMQWRITER_H_
#define SMQWRITER_H_

#include <string>

#include "SmqMessageHandler.h"

namespace kneedeepbts::smqueue {
    class SmqWriter {
    public:
        static std::string MQ_NAME;

        explicit SmqWriter(std::string savefile) {
            mqueHan = new SmqMessageHandler(SmqWriter::MQ_NAME);
            m_savefile = savefile;
            //smq_manager = smq;
            pthread_create(&mthread_ID, nullptr, SmqWriterThread, (void *) this);
        }

        ~SmqWriter();

        static void *SmqWriterThread(void *ptr);

        SmqMessageHandler *getqueHan() {
            return mqueHan;
        }

    private:
        pthread_t mthread_ID;
        SmqMessageHandler *mqueHan;
        std::string m_savefile;
        //static SmqManager * smq_manager; // FIXME: Work this one out of the picture.

    };
}
#endif /* SMQWRITER_H_ */
