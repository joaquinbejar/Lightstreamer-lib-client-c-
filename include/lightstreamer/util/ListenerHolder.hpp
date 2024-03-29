/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 29/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_LISTENERHOLDER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_LISTENERHOLDER_HPP
#include <functional>
#include <mutex>
#include <set>
#include <vector>
#include <memory>

namespace lightstreamer::util {

    // Placeholder for the EventsThread class
    class EventsThread {
    public:
        template<typename Func>
        void queue(Func&& func) {
            // Implementation that queues the function for execution in an event thread
        }
    };

    template<typename T>
    class ListenerHolder {
    protected:
        std::shared_ptr<EventsThread> eventThread;
        std::set<T> listeners;
        mutable std::mutex mtx; // For thread-safety

    public:
        explicit ListenerHolder(std::shared_ptr<EventsThread> eventThread)
                : eventThread(std::move(eventThread)) {}

        void addListener(const T& listener, const std::function<void(T)>& visitor) {
            std::lock_guard<std::mutex> lock(mtx);
            auto isNew = listeners.insert(listener).second;
            if (isNew) {
                eventThread->queue([visitor, listener]() {
                    visitor(listener);
                });
            }
        }

        void removeListener(const T& listener, const std::function<void(T)>& visitor) {
            std::lock_guard<std::mutex> lock(mtx);
            bool contained = listeners.erase(listener) > 0;
            if (contained) {
                eventThread->queue([visitor, listener]() {
                    visitor(listener);
                });
            }
        }

        std::vector<T> getListeners() const {
            std::lock_guard<std::mutex> lock(mtx);
            return std::vector<T>(listeners.begin(), listeners.end());
        }

        void forEachListener(const std::function<void(T)>& visitor) {
            std::lock_guard<std::mutex> lock(mtx);
            for (const auto& listener : listeners) {
                eventThread->queue([visitor, listener]() {
                    visitor(listener);
                });
            }
        }
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_LISTENERHOLDER_HPP
