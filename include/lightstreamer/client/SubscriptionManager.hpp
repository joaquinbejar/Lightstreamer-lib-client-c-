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


#include <exception>
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
#include <lightstreamer/client/requests/ChangeSubscriptionRequest.hpp>
#include <lightstreamer/client/requests/SubscribeRequest.hpp>
#include <lightstreamer/client/requests/RequestTutor.hpp>

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
        SubscriptionManager(std::shared_ptr<session::SessionThread> sessionThread,
                            std::shared_ptr<session::SessionManager> manager,
                            std::shared_ptr<session::InternalConnectionOptions> options)
                : sessionThread(sessionThread), options(options), manager(manager) {
            if (!instanceFieldsInitialized) {
                InitializeInstanceFields();
                instanceFieldsInitialized = true;
            }

            manager->setSubscriptionsListener(eventsListener);
        }

        /**
         * @brief Adds a subscription.
         * This method is called from the eventsThread.
         *
         * @param subscription The subscription to add.
         */
        void add(std::shared_ptr<Subscription> subscription) {
            sessionThread->queue([this, subscription]() {
                doAdd(subscription);
            });
        }

        /**
         * @brief Removes a subscription.
         * This method is called from the eventsThread.
         *
         * @param subscription The subscription to remove.
         */
        void remove(std::shared_ptr<Subscription> subscription) {
            sessionThread->queue([this, subscription]() {
                doRemove(subscription);
            });
        }

        /**
         * @brief Initiates a frequency change for a subscription.
         *
         * @param subscription The subscription for which the frequency change is initiated.
         */
        void changeFrequency(std::shared_ptr<Subscription> subscription) {
            log->Info("Preparing subscription frequency change: " + std::to_string(subscription->getSubscriptionId()));

            std::shared_ptr<requests::ChangeSubscriptionRequest> request = subscription->generateFrequencyRequest();
            auto tutor = std::make_shared<ChangeSubscriptionTutor>(this, 0, sessionThread, options, request);

            pendingSubscriptionChanges[subscription->getSubscriptionId()] = request->getReconfId(); // if reconfId is newer we don't care about the older one

            manager->sendSubscriptionChange(request, tutor);
        }

        /**
         * @brief Initiates a frequency change for a subscription with a specified timeout and reconfiguration ID.
         *
         * @param subscription The subscription for which the frequency change is initiated.
         * @param timeoutMs Timeout in milliseconds for the frequency change.
         * @param reconfId Reconfiguration ID for the frequency change.
         */
        void changeFrequency(std::shared_ptr<Subscription> subscription, long timeoutMs, int reconfId) {
            log->Info("Preparing subscription frequency change again: " +
                      std::to_string(subscription->getSubscriptionId()));

            std::shared_ptr<requests::ChangeSubscriptionRequest> request = subscription->generateFrequencyRequest(
                    reconfId);
            auto tutor = std::make_shared<ChangeSubscriptionTutor>(this, timeoutMs, sessionThread, options, request);

            pendingSubscriptionChanges[subscription->getSubscriptionId()] = request->getReconfId(); // if reconfId is newer we don't care about the older one

            manager->sendSubscriptionChange(request, tutor);
        }

        /**
         * @brief Subscribes to a given subscription.
         *
         * @param subscription The subscription to be subscribed.
         */
        void subscribe(std::shared_ptr<Subscription> subscription) {
            log->Info("Preparing subscription: " + std::to_string(subscription->getSubscriptionId()));

            std::shared_ptr<requests::SubscribeRequest> request = subscription->generateSubscribeRequest();

            log->Debug("Preparing subscription 2: " + request->toString() + ", " +
                       std::to_string(subscription->getPhase()) + ", " +
                       sessionThread->getSessionManager()->getSessionId() + ", " + options->toString());

            try {
                auto tutor = std::make_shared<SubscribeTutor>(this, subscription->getSubscriptionId(),
                                                              subscription->getPhase(), sessionThread, 0L);

                log->Debug("Preparing subscription 3: " + sessionThread->getSessionManager()->getSessionId());

                manager->sendSubscription(request, tutor);

                log->Debug("Preparing subscription 4.");
            } catch (std::exception &e) {
                log->Error("Something wrong in Subscription preparation: " + std::string(e.what()));
                log->Debug(e.what());  // Assuming the existence of a method to get the stack trace or similar
            }
        }

        /**
         * @brief Attempts to resubscribe a subscription with a timeout.
         *
         * @param subscription The subscription to resubscribe.
         * @param timeoutMs The timeout in milliseconds for the subscription attempt.
         */
        void resubscribe(std::shared_ptr<Subscription> subscription, long timeoutMs) {
            log->Info("Preparing to send subscription again: " + std::to_string(subscription->getSubscriptionId()));

            std::shared_ptr<requests::SubscribeRequest> request = subscription->generateSubscribeRequest();

            auto tutor = std::make_shared<SubscribeTutor>(this, subscription->getSubscriptionId(),
                                                          subscription->getPhase(), sessionThread, timeoutMs);

            manager->sendSubscription(request, tutor);
        }

        /**
         * @brief Pauses all active subscriptions. This avoids unexpected behavior when altering collections during iteration.
         */
        void pauseAllSubscriptions() {
            try {
                log->Debug("pauseAllSubscriptions: " + std::to_string(subscriptions.size()));

                // Cloning to avoid modifications during iteration
                std::unordered_map<int, std::shared_ptr<Subscription>> copy(subscriptions);

                for (auto &subscriptionPair: copy) {
                    auto subscription = subscriptionPair.second;

                    if (subscription->isSubTable()) {
                        // No need to pause these, will be removed soon
                        continue;
                    }

                    subscription->onPause();
                }

                log->Debug("pauseAllSubscriptions done!");
            } catch (std::exception &e) {
                log->Error("pauseAllSubscriptions error: " + std::string(e.what()));
                log->Debug(" - " + std::string(e.what()));
                try {
                    log->Debug("pauseAllSubscriptions try recovery.");

                    for (auto &subscriptionPair: subscriptions) {
                        auto subscription = subscriptionPair.second;

                        if (subscription->isSubTable()) {
                            // No need to pause these, will be removed soon
                            continue;
                        }

                        subscription->onPause();
                    }

                    log->Debug("pauseAllSubscriptions recovery done.");
                } catch (std::exception &ex) {
                    log->Error("pauseAllSubscriptions recovery try error: " + std::string(ex.what()));
                    log->Debug(" - " + std::string(ex.what()));
                }
            }
        }

        /**
         * @brief Clears all pending operations regarding subscription changes, deletions, and unsubscribes.
         */
        void clearAllPending() {
            pendingSubscriptionChanges.clear();
            pendingDelete.clear();
            pendingUnsubscribe.clear();
        }

        /**
         * @brief Initiates the unsubscription process for a given subscription ID.
         *
         * @param subscriptionId The ID of the subscription to unsubscribe.
         */
        void unsubscribe(int subscriptionId) {
            log->Info("Preparing to send unsubscription: " + std::to_string(subscriptionId));
            pendingDelete.insert(subscriptionId);
            pendingUnsubscribe.erase(subscriptionId);

            std::shared_ptr<requests::SubscribeRequest> request = std::make_shared<requests::SubscribeRequest>(
                    subscriptionId);
            auto tutor = std::make_shared<UnsubscribeTutor>(this, subscriptionId, sessionThread, 0L);

            manager->sendUnsubscription(request, tutor);
        }


    private:

        /**
         * @brief Actual implementation of the add operation.
         *
         * @param subscription The subscription to add.
         */
        void doAdd(std::shared_ptr<Subscription> subscription) {
            try {
                int subId = IdGenerator::NextSubscriptionId();

                subscriptions[subId] = subscription;

                log->Info("Adding subscription " + std::to_string(subId));

                subscription->onAdd(subId, shared_from_this(), sessionThread);

                log->Debug("Do Add for subscription " + std::to_string(subId) + " completed.");

                if (sessionAlive) {
                    subscribe(subscription);
                } else {
                    subscription->onPause();
                }
            } catch (std::exception &e) {
                log->Warn("Something wrong in doAdd for a subscription - " + e.what());
            }
        }

        /**
         * @brief Actual implementation of the remove operation.
         *
         * @param subscription The subscription to remove.
         */
        void doRemove(std::shared_ptr<Subscription> subscription) {
            int subId = subscription->getSubscriptionId();
            log->Info("Removing subscription " + std::to_string(subId));
            if (sessionAlive) {
                if (subscription->isSubscribing()) {
                    pendingUnsubscribe.insert(subId);
                } else if (subscription->isSubscribed()) {
                    unsubscribe(subId);
                }
            }
            subscriptions.erase(subId);
            subscription->onRemove();
        }

        /**
         * @brief Sends all subscriptions managed by this SubscriptionManager.
         */
        void sendAllSubscriptions() {
            try {
                // Cloning just to avoid unexpected issues as in the pauseAllSubscriptions case.
                log->Debug("sendAllSubscriptions: " + std::to_string(subscriptions.size()));

                std::unordered_map<int, std::shared_ptr<Subscription>> copy(subscriptions);

                for (auto &subscriptionPair: copy) {
                    auto subscription = subscriptionPair.second;

                    log->Debug("sendAllSubscriptions - " + std::to_string(subscriptionPair.first) + " - " +
                               subscription->toString());

                    if (subscription->isSubTable()) {
                        log->Error("Second level subscriptions should not be in the list of paused subscriptions");
                        return;
                    }

                    subscription->onStart(); // Wake up

                    subscribe(subscription);
                }

                log->Debug("sendAllSubscriptions done!");
            } catch (std::exception &e) {
                log->Error("SendAllSubscriptions error: " + std::string(e.what()));
                log->Debug(" - " + std::string(e.what()));
                try {
                    log->Debug("sendAllSubscriptions try recovery.");

                    for (auto &subscriptionPair: subscriptions) {
                        auto subscription = subscriptionPair.second;

                        if (subscription->isSubTable()) {
                            log->Error("Second level subscriptions should not be in the list of paused subscriptions");
                            return;
                        }

                        subscription->onStart(); // Wake up

                        subscribe(subscription);
                    }

                    log->Debug("sendAllSubscriptions recovery done.");
                } catch (std::exception &ex) {
                    log->Error("SendAllSubscriptions recovery try error: " + std::string(ex.what()));
                    log->Debug(" - " + std::string(ex.what()));
                }
            }
        }

        /**
        *
        */
        class EventsListener : public session::SubscriptionsListener {
            SubscriptionManager* outerInstance; // Se asume que SubscriptionManager es una clase definida previamente

        public:
            explicit EventsListener(SubscriptionManager* outerInstance) : outerInstance(outerInstance) {}

            void onSessionStart() override {
                outerInstance->log.Debug("SubscriptionManager sessionAlive set to true.");
                outerInstance->sessionAlive = true;
                outerInstance->sendAllSubscriptions();
            }

            void onSessionClose() override {
                outerInstance->log.Debug("SubscriptionManager sessionAlive set to false.");
                outerInstance->sessionAlive = false;
                outerInstance->pauseAllSubscriptions();
                outerInstance->clearAllPending();
            }

            std::shared_ptr<Subscription> extractSubscriptionOrUnsubscribe(int subscriptionId) {
                try {
                    auto it = outerInstance->subscriptions.find(subscriptionId);
                    if (it != outerInstance->subscriptions.end()) {
                        return it->second;
                    }
                } catch (const std::out_of_range&) {
                    // the subscription was removed
                }

                if (outerInstance->pendingDelete.find(subscriptionId) == outerInstance->pendingDelete.end()) {
                    outerInstance->unsubscribe(subscriptionId);
                }
                return nullptr;
            }

            void onUpdateReceived(int subscriptionId, int item, const std::vector<std::string>& args) override {
                auto subscription = extractSubscriptionOrUnsubscribe(subscriptionId);
                if (!subscription) {
                    outerInstance->log.Debug(std::to_string(subscriptionId) + " missing subscription, discarding update");
                    return;
                }

                if (outerInstance->log.IsDebugEnabled()) {
                    outerInstance->log.Info(std::to_string(subscriptionId) + " received an update");
                }

                subscription->update(args, item, false);
            }

            void onEndOfSnapshotEvent(int subscriptionId, int item) override {
                auto subscription = extractSubscriptionOrUnsubscribe(subscriptionId);
                if (!subscription) {
                    outerInstance->log.Debug(std::to_string(subscriptionId) + " missing subscription, discarding end of snapshot event");
                    return;
                }

                if (outerInstance->log.IsDebugEnabled()) {
                    outerInstance->log.Info(std::to_string(subscriptionId) + " received end of snapshot event");
                }

                subscription->endOfSnapshot(item);
            }

            void onClearSnapshotEvent(int subscriptionId, int item) override {
                auto subscription = extractSubscriptionOrUnsubscribe(subscriptionId);
                if (!subscription) {
                    outerInstance->log.Debug(std::to_string(subscriptionId) + " missing subscription, discarding clear snapshot event");
                    return;
                }

                if (outerInstance->log.IsDebugEnabled()) {
                    outerInstance->log.Info(std::to_string(subscriptionId) + " received clear snapshot event");
                }

                subscription->clearSnapshot(item);
            }

            void onLostUpdatesEvent(int subscriptionId, int item, int lost) override {
                auto subscription = extractSubscriptionOrUnsubscribe(subscriptionId);
                if (!subscription) {
                    outerInstance->log.Debug(std::to_string(subscriptionId) + " missing subscription, discarding lost updates event");
                    return;
                }

                if (outerInstance->log.IsDebugEnabled()) {
                    outerInstance->log.Info(std::to_string(subscriptionId) + " received lost updates event");
                }

                subscription->lostUpdates(item, lost);
            }

            void onConfigurationEvent(int subscriptionId, const std::string& frequency) override {
                auto subscription = extractSubscriptionOrUnsubscribe(subscriptionId);
                if (!subscription) {
                    outerInstance->log.Debug(std::to_string(subscriptionId) + " missing subscription, discarding configuration event");
                    return;
                }

                if (outerInstance->log.IsDebugEnabled()) {
                    outerInstance->log.Info(std::to_string(subscriptionId) + " received configuration event");
                }

                subscription->configure(frequency);
            }

            void onUnsubscriptionAck(int subscriptionId) override {
                outerInstance->pendingDelete.erase(subscriptionId);
                if (outerInstance->pendingUnsubscribe.find(subscriptionId) != outerInstance->pendingUnsubscribe.end()) {
                    outerInstance->unsubscribe(subscriptionId);
                }
            }

            void onUnsubscription(int subscriptionId) override {
                outerInstance->log.Info(std::to_string(subscriptionId) + " successfully unsubscribed");
                outerInstance->pendingDelete.erase(subscriptionId);
                if (outerInstance->pendingUnsubscribe.find(subscriptionId) != outerInstance->pendingUnsubscribe.end()) {
                    outerInstance->unsubscribe(subscriptionId);
                }

                if (outerInstance->subscriptions.find(subscriptionId) != outerInstance->subscriptions.end()) {
                    outerInstance->log.Error("Unexpected unsubscription event");
                }
            }

            void onSubscriptionAck(int subscriptionId) override {
                auto subscription = extractSubscriptionOrUnsubscribe(subscriptionId);
                if (!subscription) {
                    outerInstance->log.Debug(std::to_string(subscriptionId) + " missing subscription, discarding subscribed event");
                    return;
                }
                subscription->onSubscriptionAck();
            }

            void onSubscription(int subscriptionId, int totalItems, int totalFields, int keyPosition, int commandPosition) override {
                auto subscription = extractSubscriptionOrUnsubscribe(subscriptionId);
                if (!subscription) {
                    outerInstance->log.Debug(std::to_string(subscriptionId) + " missing subscription, discarding subscribed event");
                    return;
                }
                outerInstance->log.Info(std::to_string(subscriptionId) + " successfully subscribed");
                subscription->onSubscribed(commandPosition, keyPosition, totalItems, totalFields);
            }

            void onSubscription(int subscriptionId, long reconfId) override {
                auto waitingId = outerInstance->pendingSubscriptionChanges.find(subscriptionId);
                if (waitingId == outerInstance->pendingSubscriptionChanges.end()) {
                    return;
                }

                if (reconfId == waitingId->second) {
                    outerInstance->pendingSubscriptionChanges.erase(subscriptionId);
                }
            }

            void onSubscriptionError(int subscriptionId, int errorCode, const std::string& errorMessage) override {
                auto subscription = extractSubscriptionOrUnsubscribe(subscriptionId);
                if (!subscription) {
                    outerInstance->log.Debug(std::to_string(subscriptionId) + " missing subscription, discarding error");
                    return;
                }
                outerInstance->log.Info(std::to_string(subscriptionId) + " subscription error");
                subscription->onSubscriptionError(errorCode, errorMessage);
            }
        };

        /**
        *
        */
        class SubscriptionsTutor : public requests::RequestTutor {
        protected:
            SubscriptionManager* outerInstance;

        public:
            SubscriptionsTutor(SubscriptionManager* outerInstance, long currentTimeout, session::SessionThread* thread, session::InternalConnectionOptions* connectionOptions)
                    : RequestTutor(currentTimeout, thread, connectionOptions, outerInstance->fixedTimeout > 0), outerInstance(outerInstance) {}

            virtual bool isTimeoutFixed() const {
                return outerInstance->fixedTimeout > 0;
            }
        };

        class UnsubscribeTutor : public SubscriptionsTutor {
            int subscriptionId;

        public:
            UnsubscribeTutor(SubscriptionManager* outerInstance, int subscriptionId, session::SessionThread* thread, long timeoutMs)
                    : SubscriptionsTutor(outerInstance, timeoutMs, thread, outerInstance->options), subscriptionId(subscriptionId) {}

            bool verifySuccess() override {
                return outerInstance->pendingDelete.find(subscriptionId) == outerInstance->pendingDelete.end();
            }

            void doRecovery() override {
                outerInstance->reunsubscribe(subscriptionId, timeoutMs);
            }

            void notifyAbort() override {
                // Remove pending operations
                outerInstance->pendingDelete.erase(subscriptionId);
                outerInstance->pendingUnsubscribe.erase(subscriptionId);
            }

            bool shouldBeSent() override {
                return outerInstance->pendingDelete.find(subscriptionId) != outerInstance->pendingDelete.end();
            }

            long fixedTimeout() const override {
                throw std::runtime_error("Not implemented exception");
            }
        };

        /**
         * @class SubscribeTutor
         *
         * @brief The SubscribeTutor class provides a tutor for managing subscription requests.
         */
        class SubscribeTutor : public SubscriptionsTutor {
            SubscriptionManager* outerInstance;

            int subscriptionId;
            int subscriptionPhase;

        public:
            SubscribeTutor(SubscriptionManager* outerInstance, int subscriptionId, int subscriptionPhase, session::SessionThread* thread, long timeoutMs)
                    : SubscriptionsTutor(outerInstance, timeoutMs, thread, outerInstance->options), outerInstance(outerInstance), subscriptionId(subscriptionId), subscriptionPhase(subscriptionPhase) {}

            long fixedTimeout() const override {
                throw std::runtime_error("Not implemented exception");
            }

            void notifySender(bool failed) override {
                auto it = outerInstance->subscriptions.find(subscriptionId);
                if (it == outerInstance->subscriptions.end()) {
                    outerInstance->log.Warn("Subscription not found [" + std::to_string(subscriptionId) + "/" + std::to_string(outerInstance->manager->getSessionId()) + "]");
                    return;
                }
                auto subscription = it->second;
                if (!subscription->checkPhase(subscriptionPhase)) {
                    // We don't care
                    return;
                }

                SubscriptionsTutor::notifySender(failed);
                if (!failed) {
                    subscription->onSubscriptionSent();
                    subscriptionPhase = subscription->getPhase();
                }
            }

            bool verifySuccess() override {
                auto it = outerInstance->subscriptions.find(subscriptionId);
                if (it == outerInstance->subscriptions.end()) {
                    // Subscription was removed, no need to keep going, let's say it's a success
                    return true;
                }
                auto subscription = it->second;
                if (!subscription->checkPhase(subscriptionPhase)) {
                    // Something else happened, consider it a success
                    return true;
                }
                return subscription->isSubscribed();
            }

            void doRecovery() override {
                auto it = outerInstance->subscriptions.find(subscriptionId);
                if (it == outerInstance->subscriptions.end()) {
                    // Subscription was removed, no need to keep going
                    return;
                }
                auto subscription = it->second;
                if (!subscription->checkPhase(subscriptionPhase)) {
                    // Something else happened
                    return;
                }
                outerInstance->resubscribe(subscription, timeoutMs);
            }

            void notifyAbort() override {
                // We don't have anything to do, it means that a
                // delete was queued before the add was sent
                // so the subscription should not exists anymore
                /*if (subscriptions.contains(this.subscriptionId)) {
                  // might actually happen if we stop a 2nd subscription effort
                  log.error("Was not expecting to find the subscription as it was supposedly removed");
                }*/
            }

            bool shouldBeSent() override {
                auto it = outerInstance->subscriptions.find(subscriptionId);
                if (it == outerInstance->subscriptions.end()) {
                    // Subscription was removed, no need to send the request
                    return false;
                }
                auto subscription = it->second;
                if (!subscription->checkPhase(subscriptionPhase)) {
                    return false;
                }
                return true;
            }
        };



        class ChangeSubscriptionTutor : public SubscriptionsTutor {
            SubscriptionManager* outerInstance;
            requests::ChangeSubscriptionRequest* request;

        public:
            ChangeSubscriptionTutor(SubscriptionManager* outerInstance, long currentTimeout, session::SessionThread* thread, session::InternalConnectionOptions* connectionOptions, requests::ChangeSubscriptionRequest* request)
                    : SubscriptionsTutor(outerInstance, currentTimeout, thread, connectionOptions), outerInstance(outerInstance), request(request) {}

            long fixedTimeout() const override {
                throw std::runtime_error("Not implemented exception");
            }

            bool verifySuccess() override {
                auto it = outerInstance->pendingSubscriptionChanges.find(request->getSubscriptionId());
                if (it == outerInstance->pendingSubscriptionChanges.end()) {
                    return true; // No pending changes means success
                }

                int waitingId = it->second;
                int reconfId = request->getReconfId();

                // if lower we don't care about this anymore
                // if equal we're still waiting
                // higher is not possible
                return reconfId < waitingId;
            }

            void doRecovery() override {
                auto it = outerInstance->subscriptions.find(request->getSubscriptionId());
                if (it == outerInstance->subscriptions.end()) {
                    // subscription was removed, no need to keep going
                    return;
                }

                auto subscription = it->second;
                outerInstance->changeFrequency(subscription, timeoutMs, request->getReconfId());
            }

            void notifyAbort() override {
                auto it = outerInstance->pendingSubscriptionChanges.find(request->getSubscriptionId());
                if (it == outerInstance->pendingSubscriptionChanges.end()) {
                    return;
                }

                int waitingId = it->second;
                int reconfId = request->getReconfId();
                if (waitingId == reconfId) {
                    outerInstance->pendingSubscriptionChanges.erase(request->getSubscriptionId());
                }
            }

            bool shouldBeSent() override {
                auto subIt = outerInstance->subscriptions.find(request->getSubscriptionId());
                if (subIt == outerInstance->subscriptions.end()) {
                    // subscription was removed, no need to send the request
                    return false;
                }

                auto it = outerInstance->pendingSubscriptionChanges.find(request->getSubscriptionId());
                if (it == outerInstance->pendingSubscriptionChanges.end()) {
                    return false;
                }

                int waitingId = it->second;
                int reconfId = request->getReconfId();

                // if lower we don't care about this anymore
                // if equal we're still waiting
                // higher is not possible
                return reconfId == waitingId;
            }
        };



    };


}


#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SUBSCRIPTIONMANAGER_HPP
