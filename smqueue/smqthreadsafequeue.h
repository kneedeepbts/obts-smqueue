#ifndef OBTS_SMQUEUE_SMQTHREADSAFEQUEUE_H
#define OBTS_SMQUEUE_SMQTHREADSAFEQUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

namespace kneedeepbts::smqueue {
    template <typename T>
    class SmqThreadSafeQueue {
    public:
        void push(T&& item)  {
            {
                std::lock_guard lock(m_mutex);
                m_queue.push(item);
            }
            m_cond_var.notify_one();
        }

        T& front() {
            std::unique_lock lock(m_mutex);
            // FIXME: Add a timeout using the "wait_for" version.
            m_cond_var.wait(lock, [&]{ return !m_queue.empty(); });
            return m_queue.front();
        }

        void pop() {
            std::lock_guard lock(m_mutex);
            m_queue.pop();
        }

        bool empty() {
            return m_queue.empty();
        }

        uint32_t count() {
            return m_queue.size();
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_cond_var;
        std::queue<T> m_queue;
    };
}

#endif //OBTS_SMQUEUE_SMQTHREADSAFEQUEUE_H
