#include "smqthread.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

namespace kneedeepbts::smqueue {
    std::thread SmqThread::run() {
        return std::thread(&SmqThread::thread, this);
    }

    void SmqThread::stop() {
        SPDLOG_DEBUG("Stopping reader thread.");
        m_stop_thread = true;
    }
}