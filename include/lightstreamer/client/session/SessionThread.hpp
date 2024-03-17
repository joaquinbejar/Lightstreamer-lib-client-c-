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


namespace lightstreamer::client::session {


    class SessionThread {
    private:
        static ILogger *log;
        ThreadMultiplexer *threads;
        std::queue<std::function<void()>> stackTasks;
        AtomicReference<ThreadShutdownHook *> shutdownHookReference;
        AtomicReference<ThreadShutdownHook *> wsShutdownHookReference;
        SessionManager *sessionManager;
        std::string clientId;

        class SessionThreadFactory {
        private:
            static SessionThreadFactory INSTANCE;
            bool dedicatedSessionThread;
            ThreadMultiplexer <SessionThread> *singletonSessionThread;
            std::mutex mtx;

            SessionThreadFactory() {
                dedicatedSessionThread = false;
                singletonSessionThread = nullptr;
            }

        public:
            static SessionThreadFactory &getInstance() {
                return INSTANCE;
            }

            ThreadMultiplexer <SessionThread> *getSessionThread() {
                std::lock_guard<std::mutex> lock(mtx);
                ThreadMultiplexer <SessionThread> *sessionThread;
                if (dedicatedSessionThread) {
                    sessionThread = new SingleThreadMultiplexer<SessionThread>();
                } else {
                    if (singletonSessionThread == nullptr) {
                        singletonSessionThread = new SingleThreadMultiplexer<SessionThread>();
                    }
                    sessionThread = singletonSessionThread;
                }
                return sessionThread;
            }
        };

    public:
        SessionThread() {
            threads = SessionThreadFactory::SessionThread;
            clientId = std::to_string(reinterpret_cast<std::uintptr_t>(this));
        }

        void registerShutdownHook(ThreadShutdownHook *shutdownHook) {
            shutdownHookReference.CompareAndSet(nullptr, shutdownHook);
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
