/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 5/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CSJOINABLESCHEDULER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CSJOINABLESCHEDULER_HPP

#include <stack>
#include <lightstreamer/util/threads/CSJoinableScheduler.hpp>
#include <lightstreamer/util/threads/providers/JoinableExecutor.hpp>
#include <lightstreamer/util/threads/providers/JoinableScheduler.hpp>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <map>
#include <chrono>
#include <future>

namespace lightstreamer::util::threads {
    class CSJoinableScheduler : public providers::JoinableScheduler {
    private:
//        std::stack<std::future<void>> currentTasks;
        std::stack<std::shared_ptr<std::future<void>>> currentTasks;
        std::string threadName;
        long keepAliveTime;
//        providers::JoinableExecutor executor;
        std::shared_ptr<providers::JoinableExecutor> executor;
        std::map<std::future<void>, std::shared_ptr<std::promise<void>>> cancs;
        std::mutex m;

    public:
        CSJoinableScheduler(std::string threadName, long keepAliveTime) : threadName(threadName),
                                                                          keepAliveTime(keepAliveTime) {}

        CSJoinableScheduler(const std::string& threadName, long keepAliveTime, std::shared_ptr<providers::JoinableExecutor> executor)
                : threadName(threadName), keepAliveTime(keepAliveTime), executor(executor) {
        }

        void join() {
            std::lock_guard<std::mutex> lock(m);
            for (auto &entry: cancs) {
                if (entry.second != nullptr) {
                    entry.second->set_value();
                }
            }
            cancs.clear();
        }

        void Dequeue(std::future<void> tsk) {
            std::lock_guard<std::mutex> lock(m);
            cancs.erase(tsk);
        }

        std::shared_ptr<std::promise<void>> schedule(std::function<void()> task, long delayInMillis) {
            std::shared_ptr<std::promise<void>> source = std::make_shared<std::promise<void>>();
            std::lock_guard<std::mutex> lock(m);
            std::future<void> tsk_p = std::async(std::launch::async, [=]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(delayInMillis));
                if (source->get_future().wait_for(std::chrono::seconds(0)) == std::future_status::timeout) {
                    executor.execute(task);
                }
            });
            cancs[tsk_p] = source;
            tsk_p.share().wait();
            this->Dequeue(tsk_p);
            return source;
        }
    };
}
#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CSJOINABLESCHEDULER_HPP
