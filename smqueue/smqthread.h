#ifndef OBTS_SMQUEUE_SMQTHREAD_H
#define OBTS_SMQUEUE_SMQTHREAD_H

#include <cstdint>
#include <thread>

namespace kneedeepbts::smqueue {
    class SmqThread {
    public:
        std::thread run();
        void stop();

    protected:
        bool m_stop_thread = false;
        std::uint32_t m_timeout_ms = 100; // Short timeout to keep the thread responsive.

    private:
        virtual void thread() = 0;
    };
}

#endif //OBTS_SMQUEUE_SMQTHREAD_H
