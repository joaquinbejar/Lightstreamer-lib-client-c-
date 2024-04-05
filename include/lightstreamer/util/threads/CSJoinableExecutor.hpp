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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CSJOINABLEEXECUTOR_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CSJOINABLEEXECUTOR_HPP
#include <functional>
#include <queue>
#include <mutex>
#include <thread>
#include <future>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <lightstreamer/util/threads/providers/JoinableExecutor.hpp>

namespace lightstreamer::util::threads {

    struct Task {
        std::function<void()> func;
        bool done;

        Task(const std::function<void()>& f) : func(f), done(false) {}
        void operator()() { func(); done = true; }
    };

    class CSJoinableExecutor  : providers::JoinableExecutor {
    private:
        std::queue<std::shared_ptr<Task>> tasks;
        std::mutex lock;
        std::condition_variable cv;
        long keepAliveTime;
        bool running;
        std::thread worker;

    public:

        void execute(std::function<void()> task) {
            std::lock_guard<std::mutex> guard(lock);
            if(!running) {
                running = true;
                worker = std::thread([this]{ this->work(); });
            }
            tasks.push(std::make_shared<Task>(task));
            cv.notify_all();
        }

        void join() {
            std::unique_lock<std::mutex> ul(lock);
            cv.wait(ul, [this]{ return tasks.empty(); });
            running = false;
            if(worker.joinable()) worker.join();
        }

    private:
        void work() {
            while(running) {
                std::shared_ptr<Task> task = nullptr;
                {
                    std::unique_lock<std::mutex> ul(lock);
                    cv.wait_for(ul, std::chrono::milliseconds(keepAliveTime), [&]{ return !tasks.empty(); });
                    if(!tasks.empty() && running) {
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                }
                if(task) (*task)();
            }
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CSJOINABLEEXECUTOR_HPP
