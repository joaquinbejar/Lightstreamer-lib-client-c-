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

#ifndef LIGHTSTREAMERCLIENT_HPP
#define LIGHTSTREAMERCLIENT_HPP

#include <memory>
#include <vector>
#include <regex>
#include <string>
#include <iostream>
#include <mutex>
#include <thread>
#include <future>
#include <map>
#include <functional>




#include <Logger.hpp>

#include <lightstreamer/client/events/EventDispatcher.hpp>
#include <lightstreamer/client/events/ClientListenerStartEvent.hpp>
#include <lightstreamer/client/events/ClientListenerEndEvent.hpp>
#include <lightstreamer/client/events/EventsThread.hpp>
#include <lightstreamer/client/events/EventDispatcher.hpp>
#include <lightstreamer/client/events/Event.hpp>

#include <lightstreamer/client/session/InternalConnectionDetails.hpp>
#include <lightstreamer/client/session/InternalConnectionOptions.hpp>
#include <lightstreamer/client/session/SessionManager.hpp>

#include <lightstreamer/client/Constants.hpp>
#include <lightstreamer/client/LightstreamerEngine.hpp>
#include <lightstreamer/client/MessageManager.hpp>
#include <lightstreamer/client/SubscriptionManager.hpp>
#include <lightstreamer/client/ConnectionOptions.hpp>
#include <lightstreamer/client/Subscription.hpp>
#include <lightstreamer/client/ConnectionDetails.hpp>
#include <lightstreamer/client/ClientListener.hpp>


namespace lightstreamer::client {

/**
 * Facade class for the management of the communication to Lightstreamer Server. Used to provide configuration settings, event
 * handlers, operations for the control of the connection lifecycle, Subscription handling and to send messages.
 * An instance of LightstreamerClient handles the communication with Lightstreamer Server on a specified endpoint. Hence, it hosts one "Session";
 * or, more precisely, a sequence of Sessions, since any Session may fail and be recovered, or it can be interrupted on purpose.
 * So, normally, a single instance of LightstreamerClient is needed.
 * However, multiple instances of LightstreamerClient can be used, toward the same or multiple endpoints.
 */
    class LightstreamerClient {
    private:
        mutable std::mutex mutex;
        std::string lastStatus;
        std::vector<std::shared_ptr<Subscription>> subscriptionArray;
        std::unique_ptr<events::EventsThread> eventsThread;
        std::unique_ptr<SubscriptionManager> subscriptions;
        bool instanceFieldsInitialized = false;

        void initializeInstanceFields()
        {
            dispatcher = std::make_unique<EventDispatcher<ClientListener>>(eventsThread);
            internalListener = std::make_unique<InternalListener>(this);
            internalConnectionDetails = std::make_unique<InternalConnectionDetails>(*dispatcher);
            internalConnectionOptions = std::make_unique<InternalConnectionOptions>(*dispatcher, *internalListener);
            manager = std::make_unique<SessionManager>(*internalConnectionOptions, *internalConnectionDetails, sessionThread);
            engine = std::make_unique<LightstreamerEngine>(*internalConnectionOptions, sessionThread, eventsThread, *internalListener, *manager);
            messages = std::make_unique<MessageManager>(eventsThread, sessionThread, *manager, *internalConnectionOptions);
            subscriptions = std::make_unique<SubscriptionManager>(sessionThread, *manager, *internalConnectionOptions);
            // mpnManager = std::make_unique<MpnManager>(*manager, this, sessionThread);
            connectionOptions = std::make_unique<ConnectionOptions>(*internalConnectionOptions);
            connectionDetails = std::make_unique<ConnectionDetails>(*internalConnectionDetails);
        }

        /**
         * Internal listener class for handling client events within the LightstreamerClient context.
         * This class acts as a mediator between the LightstreamerClient and various system components,
         * responding to events such as server errors and status changes.
         */
        class InternalListener : public ClientListener {
        private:
            LightstreamerClient& outerInstance;

        public:
            /**
             * Constructs an InternalListener with a reference to its outer LightstreamerClient instance.
             * @param outerInstance Reference to the LightstreamerClient that this listener belongs to.
             */
            explicit InternalListener(LightstreamerClient& outerInstance)
                    : outerInstance(outerInstance) {}

            /**
             * Event handler for when listening ends. Currently not used.
             * @param client Reference to the LightstreamerClient.
             */
            virtual void onListenEnd(LightstreamerClient& client) override {
                // Currently not used
            }

            /**
             * Event handler for when listening starts. Currently not used.
             * @param client Reference to the LightstreamerClient.
             */
            virtual void onListenStart(LightstreamerClient& client) override {
                // Currently not used
            }

            /**
             * Event handler for server errors.
             * @param errorCode Error code provided by the server.
             * @param errorMessage Detailed error message.
             */
            virtual void onServerError(int errorCode, std::string errorMessage) override {
                outerInstance.dispatcher->dispatchEvent(std::make_shared<ClientListenerServerErrorEvent>(errorCode, errorMessage));
            }

            /**
             * Event handler for status changes. Dispatches a status change event if the status has actually changed.
             * @param status New status string to be set.
             */
            virtual void onStatusChange(std::string status) override {
                if (outerInstance.setStatus(status)) {
                    outerInstance.dispatcher->dispatchEvent(std::make_shared<ClientListenerStatusChangeEvent>(status));
                }
            }

            /**
             * Event handler for property changes. Depending on the property, different actions are triggered.
             * @param property Name of the property that has changed.
             */
            virtual void onPropertyChange(std::string property) override {
                if (property == "requestedMaxBandwidth") {
                    outerInstance.eventsThread->queue([this] {
                        outerInstance.engine->onRequestedMaxBandwidthChanged();
                    });
                } else if (property == "reverseHeartbeatInterval") {
                    outerInstance.eventsThread->queue([this] {
                        outerInstance.engine->onReverseHeartbeatIntervalChanged();
                    });
                } else if (property == "forcedTransport") {
                    outerInstance.eventsThread->queue([this] {
                        outerInstance.engine->onForcedTransportChanged();
                    });
                } else {
                    outerInstance.log->error("Unexpected call to internal onPropertyChange");
                }
            }
        };


    public:
        static constexpr char const* LIB_NAME = "Lightstreamer.DotNetStandard.Client";
        static constexpr char const* LIB_VERSION = "5.1.10";

        static const std::regex ext_alpha_numeric;

        /**
         * Static method that permits to configure the logging system used by the library. The logging system
         * must respect the LoggerProvider interface. A custom class can be used to wrap any third-party .NET logging tools.
         * If no logging system is specified, all the generated log is discarded.
         * The following categories are available to be consumed:
         * - lightstreamer.stream:
         *   logs socket activity on Lightstreamer Server connections;
         *   at INFO level, socket operations are logged;
         *   at DEBUG level, read/write data exchange is logged.
         * - lightstreamer.protocol:
         *   logs requests to Lightstreamer Server and Server answers;
         *   at INFO level, requests are logged;
         *   at DEBUG level, request details and events from the Server are logged.
         * - lightstreamer.session:
         *   logs Server Session lifecycle events;
         *   at INFO level, lifecycle events are logged;
         *   at DEBUG level, lifecycle event details are logged.
         * - lightstreamer.subscriptions:
         *   logs subscription requests received by the clients and the related updates;
         *   at WARN level, alert events from the Server are logged;
         *   at INFO level, subscriptions and unsubscriptions are logged;
         *   at DEBUG level, requests batching and update details are logged.
         * - lightstreamer.actions:
         *   logs settings / API calls.
         */
        static void setLoggerProvider(std::shared_ptr<ILoggerProvider> provider)
        {
            LogManager::setLoggerProvider(provider);
        }

        static std::shared_ptr<EventsThread> eventsThread = EventsThread::instance();

        std::unique_ptr<EventDispatcher<ClientListener>> dispatcher;
        std::shared_ptr<ILogger> log = LogManager::getLogger(Constants::ACTIONS_LOG);

        std::unique_ptr<InternalListener> internalListener;
        std::unique_ptr<InternalConnectionDetails> internalConnectionDetails;
        std::unique_ptr<InternalConnectionOptions> internalConnectionOptions;
        static std::shared_ptr<SessionThread> sessionThread = std::make_shared<SessionThread>();

        std::unique_ptr<SessionManager> manager;

        std::unique_ptr<LightstreamerEngine> engine;

        std::string lastStatus = Constants::DISCONNECTED;

        std::unique_ptr<MessageManager> messages;
        std::unique_ptr<SubscriptionManager> subscriptions;
        std::vector<std::shared_ptr<Subscription>> subscriptionArray;

        /**
         * Data object that contains options and policies for the connection to the server.
         * This instance is set up by the LightstreamerClient object at its own creation.
         * Properties of this object can be overwritten by values received from a Lightstreamer Server.
         */
        std::unique_ptr<ConnectionOptions> connectionOptions;

        /**
         * Data object that contains the details needed to open a connection to a Lightstreamer Server.
         * This instance is set up by the LightstreamerClient object at its own creation.
         * Properties of this object can be overwritten by values received from a Lightstreamer Server.
         */
        std::unique_ptr<ConnectionDetails> connectionDetails;

        /**
         * Creates an object to be configured to connect to a Lightstreamer server
         * and to handle all the communications with it.
         * Each LightstreamerClient is the entry point to connect to a Lightstreamer server,
         * subscribe to as many items as needed and to send messages.
         * @param serverAddress The address of the Lightstreamer Server to which this LightstreamerClient will connect.
         *                      It is possible to specify it later by using nullptr here. See ConnectionDetails::ServerAddress
         *                      for details.
         * @param adapterSet The name of the Adapter Set mounted on Lightstreamer Server to be used to handle all requests
         *                   in the Session associated with this LightstreamerClient. It is possible not to specify it at all
         *                   or to specify it later by using nullptr here. See ConnectionDetails::AdapterSet for details.
         */
        LightstreamerClient(std::string serverAddress, std::string adapterSet)
        {
            if (!instanceFieldsInitialized) {
                initializeInstanceFields();
                instanceFieldsInitialized = true;
            }

            log->info("New Lightstreamer Client instanced (library version: " + std::string(LIB_NAME) + " " + std::string(LIB_VERSION) + ")");

            // Environment::setEnvironmentVariable("io.netty.allocator.type", "unpooled");
            Environment::setEnvironmentVariable("io.netty.allocator.maxOrder", "5");

            // SessionThreadSet::sessionThreadSet.tryAdd(std::hash<std::thread::id>()(std::this_thread::get_id()), sessionThread);

            /* set circular dependencies */
            sessionThread->setSessionManager(manager.get());
            /* */
            if (!serverAddress.empty()) {
                connectionDetails->serverAddress = serverAddress;
            }
            if (!adapterSet.empty()) {
                connectionDetails->adapterSet = adapterSet;
            }

            if (TransportFactory<WebSocketProvider>::getDefaultWebSocketFactory() == nullptr) {
                log->info("WebSocket not available");
                connectionOptions->forcedTransport = "HTTP";
            } else {
                connectionOptions->forcedTransport = ""; // apply StreamSense
            }
        }

        /**
         * Adds a listener that will receive events from the LightstreamerClient instance.
         * The same listener can be added to several different LightstreamerClient instances.
         *
         * <b>Lifecycle:</b> A listener can be added at any time. A call to add a listener already
         * present will be ignored.
         * @param listener An object that will receive the events as documented in the ClientListener interface.
         */
        void addListener(std::shared_ptr<ClientListener> listener)
        {
            std::lock_guard<std::mutex> lock(mutex);
            dispatcher->addListener(listener, std::make_shared<ClientListenerStartEvent>(this));
        }

        /**
         * Removes a listener from the LightstreamerClient instance so that it will not receive events anymore.
         *
         * <b>Lifecycle:</b> A listener can be removed at any time.
         * @param listener The listener to be removed.
         */
        void removeListener(std::shared_ptr<ClientListener> listener)
        {
            std::lock_guard<std::mutex> lock(mutex);
            dispatcher->removeListener(listener, std::make_shared<ClientListenerEndEvent>(this));
        }

        /**
         * Returns a list containing the ClientListener instances that were added to this client.
         * @return A vector containing the listeners that were added to this client.
         */
        std::vector<std::shared_ptr<ClientListener>> listeners() const
        {
            std::lock_guard<std::mutex> lock(mutex);
            return dispatcher->getListeners();
        }

        /**
         * Operation method that requests to open a Session against the configured Lightstreamer Server.
         * When connect() is called, unless a single transport was forced through ConnectionOptions::ForcedTransport,
         * the so called "Stream-Sense" mechanism is started: if the client does not receive any answer for some seconds
         * from the streaming connection, then it will automatically open a polling connection.
         * A polling connection may also be opened if the environment is not suitable for a streaming connection.
         *
         * <b>Lifecycle:</b> Note that the request to connect is accomplished by the client appending the request to
         * the internal scheduler queue; this means that an invocation to status() right after connect() might not
         * reflect the change yet.
         * When the request to connect is finally being executed, if the current status of the client is
         * CONNECTING, CONNECTED:* or STALLED, then nothing will be done.
         */
        void connect()
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (connectionDetails->serverAddress.empty()) {
                throw std::invalid_argument("Configure the server address before trying to connect");
            }

            log->info("Connect requested");

            eventsThread->queue([this] {
                engine->connect();
            });
        }

        /**
         * Operation method that requests to close the Session opened against the configured Lightstreamer Server (if any).
         * When disconnect() is called, the "Stream-Sense" mechanism is stopped.
         *
         * <b>Lifecycle:</b> Note that the request to disconnect is accomplished by the client in a separate thread;
         * this means that an invocation to status() right after disconnect() might not reflect the change yet.
         * When the request to disconnect is finally being executed, if the status of the client is "DISCONNECTED",
         * then nothing will be done.
         */
        void disconnect()
        {
            std::lock_guard<std::mutex> lock(mutex);
            log->info("Disconnect requested - " + connectionDetails->adapterSet);

            eventsThread->queue([this] {
                engine->disconnect();
            });
        }

        /**
         * Works just like LightstreamerClient::disconnect(), but also returns a future which will be completed
         * when all involved threads started by all LightstreamerClient instances have been terminated, because no more
         * activities need to be managed and hence event dispatching is no longer necessary.
         * Such a method is especially useful in those environments which require appropriate resource management.
         * The method should be used in replacement of LightstreamerClient::disconnect() in all those circumstances
         * where it is indispensable to guarantee a complete shutdown of all user tasks, in order to avoid potential
         * memory leaks and waste resources.
         *
         * @return A future that will be completed when all the activities launched by all LightstreamerClient instances are terminated.
         */
        std::future<void> disconnectFuture()
        {
            disconnect();

            return std::async(std::launch::async, [this] {
                eventsThread->await();
                sessionThread->await();
                log->info("DisconnectFuture end.");
            });
        }

        /** Destructor for LightstreamerClient, ensures proper cleanup and logging. */
        ~LightstreamerClient()
        {
            log->info("I am disposing...");
        }

        /**
         * Inquiry method that gets the current client status and transport (when applicable).
         * @return The current client status. It can be one of the following values:
         *  - "CONNECTING": the client is waiting for a Server's response in order to establish a connection;
         *  - "CONNECTED:STREAM-SENSING": the client has received a preliminary response from the server and
         *    is currently verifying if a streaming connection is possible;
         *  - "CONNECTED:WS-STREAMING": a streaming connection over WebSocket is active;
         *  - "CONNECTED:HTTP-STREAMING": a streaming connection over HTTP is active;
         *  - "CONNECTED:WS-POLLING": a polling connection over WebSocket is in progress;
         *  - "CONNECTED:HTTP-POLLING": a polling connection over HTTP is in progress;
         *  - "STALLED": the Server has not been sending data on an active streaming connection for longer
         *    than a configured time;
         *  - "DISCONNECTED:WILL-RETRY": no connection is currently active but one will be opened after a timeout;
         *  - "DISCONNECTED:TRYING-RECOVERY": no connection is currently active, but one will be opened as soon as possible,
         *    as an attempt to recover the current session after a connection issue;
         *  - "DISCONNECTED": no connection is currently active.
         */
        std::string status() const
        {
            std::lock_guard<std::mutex> lock(mutex);
            return lastStatus;
        }

        /**
         * Operation method that adds a Subscription to the list of "active" Subscriptions. The Subscription cannot already
         * be in the "active" state.
         * Active subscriptions are subscribed to through the server as soon as possible (i.e. as soon as there is a
         * session available). Active Subscriptions are automatically persisted across different sessions as long as a
         * related unsubscribe call is not issued.
         *
         * <b>Lifecycle:</b> Subscriptions can be given to the LightstreamerClient at any time. Once done the Subscription
         * immediately enters the "active" state. Once "active", a Subscription instance cannot be provided again to a
         * LightstreamerClient unless it is first removed from the "active" state through a call to unsubscribe().
         * Forwarding of the subscription to the server is made appending the request to the internal scheduler.
         * A successful subscription to the server will be notified through a SubscriptionListener::onSubscription() event.
         * @param subscription A Subscription object, carrying all the information needed to process real-time values.
         */
        void subscribe(std::shared_ptr<Subscription> subscription)
        {
            std::lock_guard<std::mutex> lock(mutex);
            subscription->setActive();
            subscriptionArray.push_back(subscription);
            eventsThread->queue([this, subscription] {
                subscriptions->add(subscription);
            });
        }

        /**
         * Operation method that removes a Subscription that is currently in the "active" state.
         * By bringing back a Subscription to the "inactive" state, the unsubscription from all its items is
         * requested to Lightstreamer Server.
         *
         * <b>Lifecycle:</b> Subscription can be unsubscribed from at any time. Once done the Subscription immediately
         * exits the "active" state.
         * Forwarding of the unsubscription to the server is made appending the request to the internal scheduler.
         * The unsubscription will be notified through a SubscriptionListener::onUnsubscription() event.
         * @param subscription An "active" Subscription object that was activated by this LightstreamerClient instance.
         */
        void unsubscribe(std::shared_ptr<Subscription> subscription)
        {
            std::lock_guard<std::mutex> lock(mutex);
            subscription->setInactive();
            subscriptionArray.erase(std::remove(subscriptionArray.begin(), subscriptionArray.end(), subscription), subscriptionArray.end());
            eventsThread->queue([this, subscription] {
                subscriptions->remove(subscription);
            });
        }

        /**
         * Inquiry method that returns a list containing all the Subscription instances that are
         * currently "active" on this LightstreamerClient. Internal second-level Subscription are not included.
         * @return A vector, containing all the Subscription currently "active" on this LightstreamerClient.
         *         The vector can be empty.
         */
        std::vector<std::shared_ptr<Subscription>> getSubscriptions()
        {
            std::lock_guard<std::mutex> lock(mutex);
            return std::vector<std::shared_ptr<Subscription>>(subscriptionArray);  // Copy the vector to avoid external modifications.
        }

        /**
         * A simplified version of sendMessage that uses default parameters for simplicity and an optimized
         * fire-and-forget behavior, not involving sequence or listener.
         * @param message A text message, whose interpretation is entirely demanded to the Metadata Adapter
         *                associated to the current connection.
         */
        void sendMessage(std::string message)
        {
            std::lock_guard<std::mutex> lock(mutex);
            sendMessage(message, "", -1, nullptr, false);
        }

        /**
         * Operation method that sends a message to the Server, handled by the Metadata Adapter associated to the current Session.
         * This method supports in-order guaranteed message delivery with automatic batching.
         * @param message A text message, whose interpretation is entirely demanded to the Metadata Adapter
         *                associated to the current connection.
         * @param sequence An alphanumeric identifier for the message sequence; if empty, "UNORDERED_MESSAGES" is assumed.
         * @param delayTimeout A timeout in milliseconds, with server default used if negative.
         * @param listener An object for receiving notifications about the processing outcome.
         * @param enqueueWhileDisconnected If true, the message is queued during a disconnected state, waiting for a new session.
         */
        void sendMessage(std::string message, std::string sequence, int delayTimeout, std::shared_ptr<ClientMessageListener> listener, bool enqueueWhileDisconnected)
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (sequence.empty()) {
                sequence = Constants::UNORDERED_MESSAGES;
            } else if (!std::regex_match(sequence, std::regex("^[a-zA-Z0-9_]*$"))) {
                throw std::invalid_argument("The given sequence name is not valid, use only alphanumeric characters plus underscore, or empty for default");
            }

            eventsThread->queue([this, message, sequence, delayTimeout, listener, enqueueWhileDisconnected]() {
                messages->send(message, sequence, delayTimeout, listener, enqueueWhileDisconnected);
            });
        }

        /**
         * Static method that allows sharing cookies between connections to the Server and other sites by the application.
         * @param uri The URI from which the supplied cookies were received.
         * @param cookies A vector of cookies, represented in the HttpCookie type.
         */
        static void addCookies(std::string uri, std::vector<HttpCookie> cookies)
        {
            CookieHelper::addCookies(uri, cookies);
        }

        /**
         * Static inquiry method that can be used to retrieve cookies suitable for sending to a specified URI.
         * This method allows cookies received from the Server to be extracted for sending through other connections,
         * according to the URI to be accessed.
         * @param uri The URI to which the cookies should be sent, or empty to retrieve all available non-expired cookies.
         * @return A vector with the various cookies that can be sent in a HTTP request for the specified URI.
         *         If an empty URI is supplied, all available non-expired cookies will be returned.
         */
        static std::vector<HttpCookie> getCookies(std::string uri)
        {
            return CookieHelper::getCookies(uri);
        }

        /**
         * Provides a means to control the way TLS certificates are evaluated, with the possibility to accept untrusted ones.
         * @param validator A function that acts as a callback to validate server certificates.
         *
         * <b>Lifecycle:</b> May be called only once before creating any LightstreamerClient instance.
         */
        static void setTrustManagerFactory(std::function<bool(const std::string& certData, bool preverified, int depth, std::string& error)> validator)
        {
            // Equivalent to attaching a validation callback, in environments like C++ where you control TLS directly, you'd set this on your SSL context setup.
            SSLManager::setCertificateValidationCallback(validator);
        }

        /**
         * Internal method to update the status of the client. If the status is different from the current one, it updates and returns true.
         * @param status New status to be set.
         * @return true if the status was updated, false otherwise.
         */
        virtual bool setStatus(std::string status)
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (lastStatus != status)
            {
                lastStatus = status;
                return true;
            }
            return false;
        }

    };

    const std::regex LightstreamerClient::ext_alpha_numeric("^[a-zA-Z0-9_]*$");

}


#endif //LIGHTSTREAMERCLIENT_HPP
