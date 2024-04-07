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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_JOINABLEPOOLEXECUTOR_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_JOINABLEPOOLEXECUTOR_HPP
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>
#include <memory>
#include <lightstreamer/util/threads/providers/JoinableExecutor.hpp>

namespace lightstreamer::util::threads {

    class JoinablePoolExecutor : public providers::JoinableExecutor {
    private:
        std::mutex currentThreadLock;
        std::condition_variable cv;
        std::unique_ptr<std::thread> currentThread;

    public:
        void execute(std::function<void()> task) override {
            std::lock_guard<std::mutex> lock(currentThreadLock);
            if (currentThread && currentThread->joinable()) {
                // Task execution is pending. Real implementation should queue tasks for execution.
                throw std::runtime_error("Execution not implemented. Task execution is already in progress.");
            } else {
                // Start the task in a new thread.
                currentThread = std::make_unique<std::thread>(task);
                cv.notify_one();
            }
        }

        void join() override {
            std::unique_lock<std::mutex> lock(currentThreadLock);
            cv.wait(lock, [this] { return currentThread != nullptr; }); // Wait until a task is being executed

            if (currentThread->joinable()) {
                currentThread->join(); // Wait for the task to complete
            }

            while (currentThread && currentThread->joinable()) {
                // Loop until the current thread has finished execution
            }
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_JOINABLEPOOLEXECUTOR_HPP
