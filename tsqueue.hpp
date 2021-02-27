#pragma once
#include <queue>
#include <mutex>
#include <optional>

namespace cge
{
    template<typename T>
    class tsqueue 
    {
        std::queue<T> queue;
        mutable std::mutex mutex;

        bool empty() const {
            return queue.empty();
        }        

    public:
        tsqueue() = default;
        tsqueue(const tsqueue<T>&) = delete;
        tsqueue& operator=(const tsqueue<T>&) = delete;

        tsqueue(tsqueue<T>&& other) {
            std::lock_guard<std::mutex> lock(mutex);
            queue = std::move(other.queue);
        }
        virtual ~tsqueue() {}

        unsigned long size() const 
        {
            std::lock_guard<std::mutex> lock(mutex);
            return queue.size();
        }
        std::optional<T> pop() 
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (queue.empty()) {
                return {};
            }
            T tmp = queue.front();
            queue.pop();
            return tmp;
        }
        void push(const T& item)
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(item);
        }
    };
}