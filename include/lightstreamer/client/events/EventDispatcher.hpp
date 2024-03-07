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

#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H


#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <lightstreamer/client/events/Event.hpp>
#include "Event.hpp"
#include <lightstreamer/client/events/EventsThread.hpp>
#include "Logger.hpp"
#include <vector>
#include <lightstreamer/client/Constants.hpp>


namespace com::lightstreamer::client::events {



template<typename T>
class EventDispatcher {
    std::unordered_map<T, std::shared_ptr<ListenerWrapper>> listeners;
    std::unique_ptr<EventsThread> eventThread;
    Logger log; // Assume Logger is a class similar to ILogger
    auto log = ConsoleLogger::getInstance(Level::TRACE, Constants::THREADS_LOG);

    class ListenerWrapper {
    public:
        T listener;
        bool alive = true;

        ListenerWrapper(T listener) : listener(listener) {}
    };

public:
    EventDispatcher(std::unique_ptr<EventThread> thread) : eventThread(std::move(thread)) {
        if (!eventThread) {
            throw std::runtime_error("An EventThread is required");
        }
    }

    void addListener(const T& listener, const Event<T>& startEvent) {
        std::lock_guard<std::mutex> lock(mutex);
        if (listeners.find(listener) != listeners.end()) {
            return;
        }

        auto wrapper = std::make_shared<ListenerWrapper>(listener);
        listeners[listener] = wrapper;
        dispatchEventToListener(startEvent, wrapper, true);
    }

    void removeListener(const T& listener, const Event<T>& endEvent) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = listeners.find(listener);
        if (it == listeners.end()) {
            return;
        }

        auto wrapper = it->second;
        listeners.erase(it);
        wrapper->alive = false;
        dispatchEventToListener(endEvent, wrapper, true);
    }

    void dispatchEvent(const Event<T>& event) {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto& [key, wrapper] : listeners) {
            dispatchEventToListener(event, wrapper, false);
        }
    }

    int size() const {
        std::lock_guard<std::mutex> lock(mutex);
        return listeners.size();
    }

    std::vector<T> getListeners() const {
        std::lock_guard<std::mutex> lock(mutex);
        std::vector<T> result;
        for (const auto& [key, wrapper] : listeners) {
            result.push_back(wrapper->listener);
        }
        return result;
    }

private:
    mutable std::mutex mutex;

    void dispatchEventToListener(const Event<T>& event, std::shared_ptr<ListenerWrapper> wrapper, bool forced) {
        eventThread->queue([event, wrapper, forced, this]() {
            if (wrapper->alive || forced) {
                try {
                    event.applyTo(wrapper->listener);
                } catch (const std::exception& e) {
                    log.error("Exception caught while executing event on custom code: " + std::string(e.what()));
                }
            }
        });
    }
};

}



}
#endif //EVENTDISPATCHER_H
