#include "z100timer.h"

#include <unistd.h>

namespace kneedeepbts::gsm {
    bool Z100Timer::is_expired() {
        // Make sure this timer is active.
        if(!m_active) {
            return false;
        }

        // If already expired, return true
        if(m_expired) {
            return true;
        }

        // If not already expired, check to see if now expired
        std::chrono::time_point<std::chrono::steady_clock> end_time = m_start + m_duration_ms;
        if(std::chrono::steady_clock::now() > end_time) {
            // timer has expired, update and return true
            m_expired = true;
            return true;
        }

        return false;
    }

    bool Z100Timer::is_active() const {
        return m_active;
    }

    std::uint32_t Z100Timer::remaining_ms() const {
        // If the timer is not active, return 0
        if(!m_active) {
            return 0;
        }

        std::chrono::time_point<std::chrono::steady_clock> end_time = m_start + m_duration_ms;
        std::chrono::milliseconds remaining_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - std::chrono::steady_clock::now());
        std::int64_t remaining = remaining_ms.count();
        if(remaining < 0) {
            return 0;
        }
        return (uint32_t)remaining;
    }

    void Z100Timer::expire() {
        // Set the expiration time to now and make the timer active
        m_expired = true;
        m_active = true;
    }

    void Z100Timer::set() {
        // Make sure the timer is inactive
        reset();

        // Set the start time to now
        m_start = std::chrono::steady_clock::now();

        // Set active
        m_active = true;
    }

    void Z100Timer::set(std::uint32_t new_duration_ms) {
        // Update the duration and call the set() method
        m_duration_ms = std::chrono::milliseconds(new_duration_ms);
        set();
    }

    void Z100Timer::reset() {
        // Set the timer to inactive
        m_active = false;
    }

    void Z100Timer::wait() {
        while(!is_expired()) {
            std::uint32_t remaining = remaining_ms();
            if(remaining > 1000) {
                usleep(1000000); // sleep one second
            } else if(remaining > 100) {
                usleep(100000); // sleep 0.1 second
            } else {
                usleep(remaining_ms() * 1000); // sleep the remaining milliseconds (less than 0.1 second)
            }
        }
    }
}