#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

template <typename T>
class ThreadSafeQueue
{
public:
    void push(const T& value)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (shutdown_)
            {
                return;
            }

            queue_.push(value);
        }

        condition_.notify_one();
    }

    // Blocks until an item is available. Returns std::nullopt once the
    // queue has been shut down and every remaining item has been consumed.
    std::optional<T> waitAndPop()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        condition_.wait(lock, [this]() {
            return !queue_.empty() || shutdown_;
        });

        if (queue_.empty())
        {
            return std::nullopt;
        }

        T value = std::move(queue_.front());
        queue_.pop();

        return value;
    }

    // Wakes all waiting consumers. Items already queued are still delivered;
    // new pushes are ignored.
    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }

        condition_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable condition_;
    bool shutdown_ = false;
};
