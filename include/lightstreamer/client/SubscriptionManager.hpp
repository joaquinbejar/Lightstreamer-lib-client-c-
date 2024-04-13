/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 13/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONMANAGER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONMANAGER_HPP

#include <unordered_map>
#include <set>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>
#include <lightstreamer/client/Subscription.hpp>
#include <lightstreamer/client/session/SessionThread.hpp>
#include <lightstreamer/client/session/SessionManager.hpp>
#include <lightstreamer/client/session/InternalConnectionOptions.hpp>
#include <lightstreamer/client/session/SubscriptionsListener.hpp>

namespace lightstreamer::client {


    class SubscriptionManager {
    private:
        bool instanceFieldsInitialized = false;

        std::shared_ptr<ILogger> log = LogManager::GetLogger(Constants::SUBSCRIPTIONS_LOG);

        std::unordered_map<int, std::shared_ptr<Subscription>> subscriptions;
        /**
         * @brief Set recording unsubscription requests which have been sent but whose corresponding REQOK/SUBOK messages
         * have not yet been received.
         */
        std::unordered_set<int> pendingDelete;
        /**
         * @brief Set recording unsubscription requests which have not yet been sent because the corresponding items are still subscribing.
         */
        std::unordered_set<int> pendingUnsubscribe;
        std::unordered_map<int, int> pendingSubscriptionChanges;

        bool sessionAlive = false;
        std::shared_ptr<session::SessionThread> sessionThread;
        std::shared_ptr<session::InternalConnectionOptions> options;

        std::shared_ptr<session::SubscriptionsListener> eventsListener;

        std::shared_ptr<session::SessionManager> manager;

        void InitializeInstanceFields() {
            eventsListener = std::make_shared<EventsListener>(this);
        }

        std::mutex mutex;


    public:
        /**
         * @brief Constructs a SubscriptionManager object.
         *
         * @param sessionThread A shared pointer to a SessionThread object.
         * @param manager A shared pointer to a SessionManager object.
         * @param options A shared pointer to InternalConnectionOptions object.
         */
        SubscriptionManager(std::shared_ptr<session::SessionThread> sessionThread, std::shared_ptr<session::SessionManager> manager, std::shared_ptr<session::InternalConnectionOptions> options)
                : sessionThread(sessionThread), options(options), manager(manager) {
            if (!instanceFieldsInitialized) {
                InitializeInstanceFields();
                instanceFieldsInitialized = true;
            }

            manager->setSubscriptionsListener(eventsListener);
        }


        void add(std::shared_ptr<Subscription> subscription);

        void remove(std::shared_ptr<Subscription> subscription);

        void changeFrequency(std::shared_ptr<Subscription> subscription);

        void subscribe(std::shared_ptr<Subscription> subscription);

        void unsubscribe(int subscriptionId);

        void pauseAllSubscriptions();

        void clearAllPending();
        // Other methods...

    private:
        void doAdd(std::shared_ptr<Subscription> subscription);

        void doRemove(std::shared_ptr<Subscription> subscription);

        void sendAllSubscriptions();
    };

    // Methods would include threading and networking logic
    void SubscriptionManager::add(std::shared_ptr<Subscription> subscription) {
        // Add subscription, potentially starting a new thread or posting to a thread pool
    }

    void SubscriptionManager::remove(std::shared_ptr<Subscription> subscription) {
        // Remove subscription, handle asynchronously
    }

    // Additional methods need to be fleshed out based on their actual logic in C#
}


#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONMANAGER_HPP
