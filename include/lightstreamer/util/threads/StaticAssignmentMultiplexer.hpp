/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 7/4/24
 ******************************************************************************/

/*******************************************************************************
 Copyright (c) 2024.

 This program is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation, either version 3 of the License, or (at your
 option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program. If not, see <https://www.gnu.org/licenses/>..
 ******************************************************************************/

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_STATICASSIGNMENTMULTIPLEXER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_STATICASSIGNMENTMULTIPLEXER_HPP

#include <lightstreamer/util/threads/ThreadMultiplexer.hpp>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <future>
#include <chrono>
#include <unordered_map>

namespace lightstreamer::util::threads {

    template<typename S>
    class StaticAssignmentMultiplexer : public ThreadMultiplexer<S> {
    private:
        static std::vector<std::thread> workers;
        static std::atomic<int> nextWorkerIndex;
        static std::mutex mtx;
        std::unordered_map<S, size_t> sourceToWorkerMap;

    public:
        StaticAssignmentMultiplexer() {
            static std::once_flag flag;
            std::call_once(flag, []() {
                unsigned int cores = std::thread::hardware_concurrency();
                workers.reserve(cores);
                for (int i = 0; i < cores; ++i) {
                    // TODO: Implement SingleThreadEventExecutor
                }
                nextWorkerIndex.store(0);
            });
        }

        ~StaticAssignmentMultiplexer() override {
            for (auto& worker : workers) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }

        void execute(S source, std::function<void()> runnable) override {
            size_t workerIndex = assignWorkerToSource(source);
            // Execute the task immediately in a new thread for simplicity
            std::thread([runnable]() {
                runnable();
            }).detach();
        }

        std::shared_ptr<std::promise<void>> schedule(S source, std::function<void()> task, long delayMillis) override {
            size_t workerIndex = assignWorkerToSource(source);
            auto promise = std::make_shared<std::promise<void>>();
            auto future = promise->get_future();
            // Schedule task with delay
            std::thread([task, delayMillis, promise]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMillis));
                if (future.wait_for(std::chrono::seconds(0)) == std::future_status::timeout) {
                    task();
                }
                promise->set_value();
            }).detach();
            return promise;
        }

        void await() override {
            // Implementation depends on the actual task queuing and completion tracking mechanism
        }

    private:
        size_t assignWorkerToSource(const S& source) {
            std::lock_guard<std::mutex> lock(mtx);
            if (sourceToWorkerMap.find(source) == sourceToWorkerMap.end()) {
                size_t workerIndex = nextWorkerIndex.fetch_add(1) % workers.size();
                sourceToWorkerMap[source] = workerIndex;
                return workerIndex;
            }
            return sourceToWorkerMap[source];
        }
    };

    template<typename S>
    std::vector<std::thread> StaticAssignmentMultiplexer<S>::workers;

    template<typename S>
    std::atomic<int> StaticAssignmentMultiplexer<S>::nextWorkerIndex(0);

    template<typename S>
    std::mutex StaticAssignmentMultiplexer<S>::mtx;

}



#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_STATICASSIGNMENTMULTIPLEXER_HPP
