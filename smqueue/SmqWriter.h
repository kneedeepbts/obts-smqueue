#ifndef SMQWRITER_H_
#define SMQWRITER_H_

#include <cstdint>
#include <string>

#include "smqthread.h"
#include "smqrmq.h"
#include "netsocket.h"

#include "rawmessage.h"
#include "smqthreadsafequeue.h"

namespace kneedeepbts::smqueue {
    class SmqWriter : public SmqThread, public SmqRmq, public NetSocket {
    public:
        SmqWriter();

    private:
        void thread() override;
        void process_message();
    };
}
#endif /* SMQWRITER_H_ */
