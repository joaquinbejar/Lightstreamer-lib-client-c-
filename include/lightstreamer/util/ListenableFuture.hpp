/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 14/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_LISTENABLEFUTURE_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_LISTENABLEFUTURE_HPP

#include <vector>
#include <mutex>
#include <memory>

namespace lightstreamer::util {

    class IRunnable {
    public:
        virtual ~IRunnable() = default;
        virtual void run() = 0;
    };

    class ListenableFuture {
    public:
        enum class State {
            NOT_RESOLVED = 0,
            FULFILLED = 1,
            REJECTED = 2,
            ABORTED = 3
        };

    private:
        std::vector<std::shared_ptr<IRunnable>> onFulfilledListeners;
        std::vector<std::shared_ptr<IRunnable>> onRejectedListeners;
        State state = State::NOT_RESOLVED;
        std::mutex mtx;

    public:
        static std::shared_ptr<ListenableFuture> fulfilled() {
            auto future = std::make_shared<ListenableFuture>();
            future->fulfill();
            return future;
        }

        static std::shared_ptr<ListenableFuture> rejected() {
            auto future = std::make_shared<ListenableFuture>();
            future->reject();
            return future;
        }

        ListenableFuture& onFulfilled(std::shared_ptr<IRunnable> listener) {
            std::lock_guard<std::mutex> lock(mtx);
            onFulfilledListeners.push_back(listener);
            if (state == State::FULFILLED) {
                listener->run();
            }
            return *this;
        }

        ListenableFuture& onRejected(std::shared_ptr<IRunnable> listener) {
            std::lock_guard<std::mutex> lock(mtx);
            onRejectedListeners.push_back(listener);
            if (state == State::REJECTED) {
                listener->run();
            }
            return *this;
        }

        ListenableFuture& fulfill() {
            std::lock_guard<std::mutex> lock(mtx);
            if (state == State::NOT_RESOLVED) {
                state = State::FULFILLED;
                for (auto& runnable : onFulfilledListeners) {
                    runnable->run();
                }
            }
            return *this;
        }

        ListenableFuture& reject() {
            std::lock_guard<std::mutex> lock(mtx);
            if (state == State::NOT_RESOLVED) {
                state = State::REJECTED;
                for (auto& runnable : onRejectedListeners) {
                    runnable->run();
                }
            }
            return *this;
        }

        ListenableFuture& abort() {
            std::lock_guard<std::mutex> lock(mtx);
            state = State::ABORTED;
            return *this;
        }

        State getState() const {
            std::lock_guard<std::mutex> lock(mtx);
            return state;
        }
    };

} // namespace lightstreamer::util

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_LISTENABLEFUTURE_HPP
