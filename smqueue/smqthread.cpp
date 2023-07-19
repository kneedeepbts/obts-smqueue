#include "smqthread.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"

namespace kneedeepbts::smqueue {
    std::thread SmqThread::run() {
        return std::thread(&SmqThread::thread, this);
    }

    void SmqThread::stop() {
        SPDLOG_DEBUG("Stopping this thread");
        m_stop_thread = true;
    }
}