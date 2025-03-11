#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class AsyncTaskQueue {
public:
    static void init();
    static void shutdown();
    static AsyncTaskQueue background;
    static AsyncTaskQueue main;

private:
    static std::vector<std::thread> threadpool;

public:
    void run();
    void run_blocking();
    void push_task(std::function<void()>);
    void close();
    bool is_open();
    std::size_t num_queued_tasks();
    std::size_t num_total_queued_tasks();

private:
    std::queue<std::function<void()>> m_queue;
    std::mutex m_queue_mutex;
    std::condition_variable m_convar;
    bool m_is_open{true};
    std::size_t m_total_queued_tasks{0};
};
