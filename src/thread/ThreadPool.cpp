#include "sese/thread/ThreadPool.h"
#include "sese/thread/Locker.h"

namespace sese {
    ThreadPool::ThreadPool(std::string threadPoolName, size_t threads)
        : name(std::move(threadPoolName)),
          threads(threads),
          data(std::make_shared<RuntimeData>()) {

        auto proc = [data = data] {
            while (true) {
                std::unique_lock<std::mutex> locker(data->mutex);
                if (data->isShutdown) {
                    locker.unlock();
                    break;
                } else if (data->tasks.empty()) {
                    data->conditionVariable.wait(locker);
                } else {
                    auto task = std::move(data->tasks.front());
                    data->tasks.pop();
                    locker.unlock();
                    if (task) {
                        task->content();
                    }
                }
            }
        };

        for (size_t i = 0; i < threads; i++) {
            auto pthread = new Thread(proc, name + std::to_string(i));
            threadGroup.emplace_back(pthread);
            pthread->start();
        }
    }

    ThreadPool::~ThreadPool() {
        if (!data->isShutdown) {
            shutdown();
        }
    }

    void ThreadPool::postTask(const Task::Ptr &task) {
        {
            Locker locker(data->mutex);
            data->tasks.emplace(task);
        }
        data->conditionVariable.notify_one();
    }

    void ThreadPool::postTask(const std::vector<Task::Ptr> &tasks) {
        {
            Locker locker(data->mutex);
            for (const auto &task: tasks) {
                data->tasks.emplace(task);
            }
        }
        data->conditionVariable.notify_all();
    }

    void ThreadPool::shutdown() {
        data->isShutdown = true;
        data->conditionVariable.notify_all();
        for (auto pthread: threadGroup) {
            if (pthread->joinable()) {
                pthread->join();
                delete pthread;
            }
        }
    }

    size_t ThreadPool::size() noexcept {
        Locker locker(data->mutex);
        return data->tasks.size();
    }

    bool ThreadPool::empty() noexcept {
        Locker locker(data->mutex);
        return data->tasks.empty();
    }
}// namespace sese