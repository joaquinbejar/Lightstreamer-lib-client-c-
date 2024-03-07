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

/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 7/3/24
 ******************************************************************************/

#ifndef EVENTSTHREAD_HPP
#define EVENTSTHREAD_HPP


#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <Logger.hpp>
#include <ConsoleLogLevel.hpp>


namespace lightstreamer::client::events {
class EventsThread {
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::thread worker_thread;
    auto logger = Logger::ConsoleLogger::getInstance(ConsoleLogLevel::Level::TRACE, "category");


    void worker() {
        while (!stop) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                condition.wait(lock, [this] { return stop || !tasks.empty(); });
                if (stop && tasks.empty()) return;
                task = std::move(tasks.front());
                tasks.pop();
            }
            task();
        }
    }

public:
    EventsThread() : stop(false), worker_thread(&EventsThread::worker, this) {}

    ~EventsThread() {
        stop = true;
        condition.notify_all();
        if (worker_thread.joinable()) worker_thread.join();
    }

    void queue(const std::function<void()>& task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            tasks.emplace(task);
        }
        condition.notify_one();
    }
};
}
#endif //EVENTSTHREAD_HPP
