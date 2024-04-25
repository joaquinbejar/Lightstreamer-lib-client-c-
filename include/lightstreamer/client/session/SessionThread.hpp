/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 17/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONTHREAD_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONTHREAD_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <lightstreamer/util/threads/ThreadShutdownHook.hpp>
#include <lightstreamer/util/threads/ThreadMultiplexer.hpp>
#include <lightstreamer/client/session/SessionManager.hpp>
#include <lightstreamer/util/threads/SingleThreadMultiplexer.hpp>


namespace lightstreamer::client::session {
    using namespace util::threads;

    class SessionThread {
    private:
        static ILogger *log;
        std::shared_ptr<ThreadMultiplexer<SessionThread>> threads;
        std::queue<std::function<void()>> stackTasks;
        std::shared_ptr<ThreadShutdownHook> shutdownHookReference;
        std::shared_ptr<ThreadShutdownHook> wsShutdownHookReference;
        std::shared_ptr<SessionManager> sessionManager;
        std::string clientId;

        class SessionThreadFactory {
        private:
            static SessionThreadFactory INSTANCE;
            bool dedicatedSessionThread;
            std::shared_ptr<ThreadMultiplexer<SessionThread>> singletonSessionThread;
            std::mutex mutex;

            // Private constructor for the singleton pattern
            SessionThreadFactory() : dedicatedSessionThread(false) {
                // Optionally load the configuration for dedicatedSessionThread from system properties or a configuration file
                // dedicatedSessionThread = readConfiguration("com.lightstreamer.client.session.thread") == "dedicated";
            }

        public:
            // Deleted copy constructor and assignment operator to prevent copying of the singleton instance
            SessionThreadFactory(const SessionThreadFactory&) = delete;
            SessionThreadFactory& operator=(const SessionThreadFactory&) = delete;

            // Static method to access the singleton instance
            static SessionThreadFactory& getInstance() {
                return INSTANCE;
            }

            // Method to get the ThreadMultiplexer instance
            std::shared_ptr<ThreadMultiplexer<SessionThread>> getSessionThread() {
                std::lock_guard<std::mutex> lock(mutex);
                if (dedicatedSessionThread) {
                    return std::make_shared<SingleThreadMultiplexer<SessionThread>>();
                } else {
                    if (!singletonSessionThread) {
                        singletonSessionThread = std::make_shared<SingleThreadMultiplexer<SessionThread>>();
                    }
                    return singletonSessionThread;
                }
            }
        };

        // Initialization of the singleton instance
//        SessionThreadFactory SessionThreadFactory::INSTANCE;
//        SessionThreadFactory &instance = SessionThreadFactory::getInstance();

    public:
        SessionThread() {
            threads = SessionThreadFactory::getInstance().getSessionThread();
            clientId = std::to_string(reinterpret_cast<std::uintptr_t>(this));
        }

        void registerShutdownHook(ThreadShutdownHook *shutdownHook) {
            shutdownHookReference.CompareAndSet(nullptr, shutdownHook); // TODO: fix this
        }

        void registerWebSocketShutdownHook(ThreadShutdownHook *shutdownHook) {
            wsShutdownHookReference.CompareAndSet(nullptr, shutdownHook);
        }

        void await() {
            threads->await();
            ThreadShutdownHook *hook = shutdownHookReference.Value();
            if (hook != nullptr) {
                hook->onShutdown();
            } else {
                log->Info("No HTTP Shutdown Hook provided");
            }
            ThreadShutdownHook *wsHook = wsShutdownHookReference.Value();
            if (wsHook != nullptr) {
                wsHook->onShutdown();
            } else {
                log->Info("No WebSocket Shutdown Hook provided");
            }
        }

        void queue(std::function<void()> task) {
            threads->execute(this, decorateTask(task));
        }

        std::chrono::milliseconds schedule(std::function<void()> task, long delayMillis) {
            return threads->schedule(this, decorateTask(task), delayMillis);
        }

        void setSessionManager(SessionManager *sessionManager) {
            this->sessionManager = sessionManager;
        }

    private:
        std::function<void()> decorateTask(std::function<void()> task) {
            return [this, task]() {
                try {
                    task();
                } catch (const std::exception &e) {
                    log->Error("Uncaught exception", e);
                    sessionManager->onFatalError(e);
                }
            };
        }

        void queueCompleted() {
            // Do nothing
        }
    };



} // namespace lightstreamer




#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONTHREAD_HPP
