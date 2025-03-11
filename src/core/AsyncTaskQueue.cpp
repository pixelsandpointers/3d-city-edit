#include "core/AsyncTaskQueue.hpp"

#include <optional>

AsyncTaskQueue AsyncTaskQueue::background;
AsyncTaskQueue AsyncTaskQueue::main;
std::vector<std::thread> AsyncTaskQueue::threadpool;

void AsyncTaskQueue::init()
{
    auto const thread_count = 4;

    for (std::size_t i = 0; i < thread_count; ++i) {
        threadpool.emplace_back([]() {
            while (background.is_open()) {
                background.run_blocking();
            }
        });
    }
}

void AsyncTaskQueue::shutdown()
{
    background.close();

    for (auto& thread : threadpool) {
        thread.join();
    }
}

void AsyncTaskQueue::run()
{
    while (true) {
        std::optional<std::function<void()>> task;

        {
            auto lock = std::lock_guard<std::mutex>{m_queue_mutex};

            if (!m_is_open || m_queue.empty()) {
                return;
            }

            task = std::move(m_queue.front());
            m_queue.pop();
        }

        if (task.has_value()) {
            task.value()();
        }
    }
}

void AsyncTaskQueue::run_blocking()
{
    while (true) {
        std::optional<std::function<void()>> task;

        {
            auto lock = std::unique_lock<std::mutex>{m_queue_mutex};
            m_convar.wait(lock, [&]() { return !m_queue.empty() || !m_is_open; });

            if (!m_is_open) {
                return;
            }

            if (!m_queue.empty()) {
                task = std::move(m_queue.front());
                m_queue.pop();
            }
        }

        if (task.has_value()) {
            task.value()();
        }
    }
}

void AsyncTaskQueue::push_task(std::function<void()> task)
{
    {
        auto lock = std::lock_guard<std::mutex>{m_queue_mutex};
        m_queue.push(std::move(task));
        ++m_total_queued_tasks;
    }

    m_convar.notify_one();
}

void AsyncTaskQueue::close()
{
    m_is_open = false;
    m_convar.notify_all();
}

bool AsyncTaskQueue::is_open()
{
    return m_is_open;
}

std::size_t AsyncTaskQueue::num_queued_tasks()
{
    auto lock = std::lock_guard<std::mutex>{m_queue_mutex};
    return m_queue.size();
}

std::size_t AsyncTaskQueue::num_total_queued_tasks()
{
    auto lock = std::lock_guard<std::mutex>{m_queue_mutex};
    return m_total_queued_tasks;
}
