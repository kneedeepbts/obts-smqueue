#ifndef OBTS_SMQUEUE_Z100TIMER_H
#define OBTS_SMQUEUE_Z100TIMER_H

#include <cstdint>
#include <chrono>

namespace kneedeepbts::gsm {
    /**
    * CCITT Z.100 activity timer, as described in GSM 04.06 5.1
    */
    class Z100Timer {
    public:
        explicit Z100Timer(std::uint32_t duration_ms) : m_duration_ms(duration_ms) {}

        bool is_expired();

        [[nodiscard]] bool is_active() const;
        [[nodiscard]] std::uint32_t remaining_ms() const;

        void expire();
        void set();
        void set(std::uint32_t new_duration_ms);
        void reset();
        void wait();

    private:
        std::chrono::milliseconds m_duration_ms;
        bool m_active = false;
        bool m_expired = false;
        std::chrono::time_point<std::chrono::steady_clock> m_start;
    };
}

#endif //OBTS_SMQUEUE_Z100TIMER_H
