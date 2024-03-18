/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 16/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_HTTPREQUESTMANAGER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_HTTPREQUESTMANAGER_HPP

#include <lightstreamer/client/protocol/RequestManager.hpp>
#include <lightstreamer/client/protocol/BatchRequest.hpp>
#include <lightstreamer/client/requests/LightstreamerRequest.hpp>
#include <lightstreamer/client/session/SessionThread.hpp>
#include "Transport.h" // Placeholder for Transport class
#include "Protocol.h" // Placeholder for Protocol class
#include "InternalConnectionOptions.h" // Placeholder for InternalConnectionOptions class
#include "Logger.h" // Placeholder for Logger class
#include <array>
#include <list>
#include <string>
#include <memory>
#include <mutex>

namespace lightstreamer::client::protocol {

    /**
     * @file
     * Documentation extracted from the "Unified Client API" document.
     *
     * @section control_request_batching Control Request Batching
     * Control connections are automatically serialized and batched:
     * the first request is sent as soon as possible, subsequent requests are batched together while
     * the previous connection is open (the concept of "open" may vary depending on the
     * technology in use; the purpose is to always have at max 1 open socket dedicated to control
     * requests). Note that during websocket sessions, there is no need to batch, nor there is a need
     * to wait for a roundtrip before issuing a new control request, so that if a websocket is in use
     * control requests are all sent "as soon as possible" and only batched if the dequeing thread
     * finds more than one request ready when executing.
     *
     * Note that as the server specifies a maximum length for control requests body contents, a
     * batch may not contain all the available requests. Such limit must always be respected unless
     * one single request surpasses the limit: in that case the request is sent on its own even if we
     * already know that the server will refuse it.
     *
     * Note that each control request is always bound to a session. As a consequence, if the related
     * session ends while the request is on the wire, such request becomes completely useless:
     * when the related session is closed, any socket that is currently used to send control
     * request(s) MUST be closed (it obviously does not apply to sessions running over websocket
     * since such sockets are closed together with the session end).
     *
     * Some kind of Control Requests may not be compatible to be sent in the same batch. Due to
     * this, the client will keep different lists and will choose which one to dequeue from via
     * roundrobin. These are the different kinds of batches:
     *   1. control: subscription, unsubscription, and constraint (currently only bandwidth change
     *      is performed through constraint requests)
     *   2. msg: messages
     *   3. heartbeat: reverse heartbeats. These are never batched and only sent if there was
     *      silence on the control channel for a configurable time.
     *   4. send_log: remote client logging; it is not mandatory to implement these messages
     *   5. control: destroy requests are compatible with the first category but, while usually
     *      control requests are sent to the currently active server instance address (unless it is
     *      specified to ignore the server instance address), these requests must be sent to the
     *      server where the old session was open. For this reason, these requests are never
     *      batched.
     *
     * @section control_connection_timeout Control Connection Timeout Algorithm
     * In case no response, either synchronous or asynchronous, for a certain control connection,
     * is not received within 4 seconds, the missing control request will be sent again to the
     * batching algorithm (note that the 4 second timeout starts when the request is sent on the
     * net, not when the request is sent to the batching algorithm). The timeout is then doubled
     * each time a request is sent again. Also, the timeout is extended with the pollingInterval value
     * to prevent sending useless requests during "short polling" sessions.
     *
     * IMPLEMENTATION NOTE: the WebSocket case has no synchronous request/responses.
     * IMPLEMENTATION NOTE: if any control response, excluding destroy requests, returns with
     * a "sync error", the client will drop the current session and will open a new one.
     * IMPLEMENTATION NOTE: the Web/Node.js clients currently only handle the sync error
     * from synchronous responses (i.e., ignores ok or other kinds of errors, including network errors
     * and waits for such notifications on the stream connection).
     * IMPLEMENTATION NOTE: the HTML might not have the chance to read the synchronous
     * responses (control.html cases and JSONP cases).
     */
    class HttpRequestManager : public RequestManager {
        bool instanceFieldsInitialized = false;

        void initializeInstanceFields() {
            requestQueues = {&messageQueue, &controlQueue, &destroyQueue, &hbQueue};
        }

        static const std::string IDLE;
        static const std::string WAITING;
        static const std::string END;
        static const std::string ENDING;

        Logger &log = LogManager::getLogger(Constants::REQUESTS_LOG);

        BatchRequest messageQueue = BatchRequest(BatchRequest::MESSAGE);
        BatchRequest controlQueue = BatchRequest(BatchRequest::CONTROL);
        BatchRequest destroyQueue = BatchRequest(BatchRequest::CONTROL);
        BatchRequest hbQueue = BatchRequest(BatchRequest::HEARTBEAT);

        std::vector<BatchRequest *> requestQueues;

        long requestLimit = 0;
        int nextQueue = 0; // handles turns (control-sendMessage-sendLog)

        std::string status = IDLE;
        int statusPhase = 1;
        session::SessionThread *sessionThread;
        Transport *transport;
        Protocol *protocol;
        InternalConnectionOptions *options;

        RequestHandle *activeConnection;

        FatalErrorListener *errorListener;
        std::list<RequestObjects> ongoingRequests; // List of requests that the manager has sent but no response has still arrived

      static constexpr long SYNC_DEQUEUE = -1;
        static constexpr long ASYNC_DEQUEUE = 0;

        void changeStatus(const std::string& newStatus) {
            this->statusPhase++; // Used to verify dequeue and sendHeartbeats calls
            this->status = newStatus;
        }

    public:
        HttpRequestManager(session::SessionThread *thread, Transport *transport, InternalConnectionOptions *options)
                : HttpRequestManager(thread, nullptr, transport, options, nullptr) {
            if (!instanceFieldsInitialized) {
                initializeInstanceFields();
                instanceFieldsInitialized = true;
            }
        }

        HttpRequestManager(session::SessionThread *thread, Protocol *protocol, Transport *transport,
                           InternalConnectionOptions *options, FatalErrorListener *errListener) {
            if (!instanceFieldsInitialized) {
                initializeInstanceFields();
                instanceFieldsInitialized = true;
            }
            this->sessionThread = thread;
            this->transport = transport;
            this->protocol = protocol;
            this->options = options;
            this->errorListener = errListener;
        }

        bool is(const std::string &status) const {
            return this->status == status;
        }

        bool isNot(const std::string &status) const {
            return !this->is(status);
        }

        void close(bool waitPending) override {
            if (!waitPending || this->activeConnection == nullptr) {
                if (this->activeConnection != nullptr) {
                    if (requestQueues[this->nextQueue] != &destroyQueue) {
                        this->activeConnection->close(false);
                    } // else do not bother destroy requests
                }
                this->changeStatus(END);
            } else {
                this->changeStatus(ENDING);
            }
        }

        // Setter for requestLimit, replacing the C# property
        void setRequestLimit(long limit) override {
            requestLimit = limit;
        }

        bool addToProperBatch(std::unique_ptr<LightstreamerRequest> request, std::unique_ptr<RequestTutor> tutor,
                              std::unique_ptr<RequestListener> listener) {
            // Example for MessageRequest, similar for others
            if (auto *msgRequest = dynamic_cast<MessageRequest *>(request.get())) {
                if (log.isDebugEnabled()) {
                    log.debug("New Message request: " + request->getRequestName());
                }
                return messageQueue.addRequestToBatch(std::move(request), std::move(tutor), std::move(listener));
            } else if (auto *hbRequest = dynamic_cast<ReverseHeartbeatRequest *>(request.get())) {
                return hbQueue.addRequestToBatch(std::move(request), std::move(tutor), std::move(listener));
            }
                // Similar checks and casts for other request types
            else {
                return false;
            }
        }

        void copyTo(ControlRequestHandler &newHandler) override {
            // Skip destroy requests logic here

            if (!ongoingRequests.empty()) {
                for (auto &req: ongoingRequests) {
                    newHandler.addRequest(std::move(req.request), std::move(req.tutor), std::move(req.listener));
                }
                ongoingRequests.clear();
            }

            for (auto *queue: requestQueues) {
                RequestObjects *migrating;
                while ((migrating = queue->shift()) != nullptr) {
                    newHandler.addRequest(std::move(migrating->request), std::move(migrating->tutor),
                                          std::move(migrating->listener));
                }
            }

            newHandler.setRequestLimit(requestLimit);
        }

        void addRequest(std::shared_ptr<LightstreamerRequest> request, std::shared_ptr<RequestTutor> tutor, std::shared_ptr<RequestListener> listener) override {
            assert(dynamic_cast<ControlRequest*>(request.get()) || dynamic_cast<MessageRequest*>(request.get()) || dynamic_cast<ReverseHeartbeatRequest*>(request.get()));

            if (is("END") || is("ENDING")) {
                log.error("Unexpected call on dismissed batch manager: " + request->getTransportUnawareQueryString());
                throw std::invalid_argument("Unexpected call on dismissed batch manager");
            }

            addToProperBatch(std::move(request), std::move(tutor), std::move(listener));

            if (is("IDLE")) {
                dequeue(SYNC_DEQUEUE, "add");
            } else {
                log.debug("Request manager busy: the request will be sent later " + request->getTransportUnawareQueryString());
            }
        }

        std::unique_ptr<RequestHandle> createSession(std::unique_ptr<CreateSessionRequest> request, std::unique_ptr<StreamListener> reqListener, long tcpConnectTimeout, long tcpReadTimeout) override {
            return transport->sendRequest(protocol, std::move(request), std::move(reqListener), options->getHttpExtraHeaders(), options->getProxy(), tcpConnectTimeout, tcpReadTimeout);
        }

        std::unique_ptr<RequestHandle> bindSession(std::unique_ptr<BindSessionRequest> request, std::unique_ptr<StreamListener> reqListener, long tcpConnectTimeout, long tcpReadTimeout, std::promise<void>& requestFuture) {
            auto handle = transport->sendRequest(protocol.get(), std::move(request), std::move(reqListener),
                                                 options->getHttpExtraHeadersOnSessionCreationOnly() ? nullptr : options->getHttpExtraHeaders(),
                                                 options->getProxy(), tcpConnectTimeout, tcpReadTimeout);
            requestFuture.set_value();
            return handle;
        }

        std::unique_ptr<RequestHandle> recoverSession(std::unique_ptr<RecoverSessionRequest> request, std::unique_ptr<StreamListener> reqListener, long tcpConnectTimeout, long tcpReadTimeout) {
            return transport->sendRequest(protocol.get(), std::move(request), std::move(reqListener),
                                          options->getHttpExtraHeadersOnSessionCreationOnly() ? nullptr : options->getHttpExtraHeaders(),
                                          options->getProxy(), tcpConnectTimeout, tcpReadTimeout);
        }

        class FatalErrorListener {
        public:
            virtual ~FatalErrorListener() = default;

            virtual void onError(int errorCode, const std::string& errorMessage) = 0;
        };

    private:
        void dequeue(long delay, const std::string& who) {
            if (delay == SYNC_DEQUEUE) {
                if (log.isDebugEnabled()) {
                    log.debug("Ready to dequeue control requests to be sent to server");
                }
                dequeueControlRequests(statusPhase, who);
            } else {
                int sc = statusPhase;
                // Lambda to be executed either immediately or scheduled for later.
                auto task = [this, sc, who]() { this->dequeueControlRequests(sc, "async." + who); };

                if (delay == ASYNC_DEQUEUE) {
                    // Assuming SessionThread has a method to queue a task
                    sessionThread->queue(task);
                } else {
                    // Schedule the task for execution after a delay
                    std::thread([task, delay]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                        task();
                    }).detach(); // Detach the thread to let it run independently
                }
            }
        }

        void dequeueControlRequests(int statusPhase, const std::string& who) {
            if (statusPhase != this->statusPhase) {
                return;
            }

            if (this->isNot("IDLE")) {
                if (this->is("WAITING")) {
                    // Might happen if an async dequeue is surpassed by a sync one
                    return;
                } else if (this->is("END")) {
                    // Game over
                    return;
                } else if (this->is("ENDING")) {
                    log.error("Dequeue call on unexpected status");
                    this->changeStatus("END");
                    return;
                }
            }

            int c = 0;
            while (c < this->requestQueues.size()) {
                // Switch the flag to change turn
                nextQueue = (nextQueue < requestQueues.size() - 1) ? nextQueue + 1 : 0;

                if (!requestQueues[nextQueue]->isEmpty()) {
                    bool sent = sendBatch(*requestQueues[nextQueue]);
                    if (sent) {
                        changeStatus("WAITING");
                        return;
                    }
                }
                ++c;
            }

            // Nothing to send, we're still IDLE
        }

        bool sendBatch(BatchRequest& batch) {
            if (batch.length() <= 0) {
                log.error("Unexpected call");
                return false; // Early exit if batch is empty
            }

            BatchedListener combinedRequestListener(*this);
            BatchedRequest combinedRequest(*this);

            RequestObjects* first = nullptr;
            while (first == nullptr && !batch.isEmpty()) {
                first = batch.shift();
                if (first && first->tutor->shouldBeSent()) {
                    combinedRequest.setServer(first->request->getTargetServer());
                    combinedRequest.setRequestName(first->request->getRequestName());

                    combinedRequest.add(*first->request);
                    combinedRequestListener.add(*first->listener);
                    ongoingRequests.push_back(std::move(*first)); // Assuming move semantics are properly handled
                } else if (first) {
                    first->tutor->notifyAbort();
                    first = nullptr; // Continue looking for a valid first request
                }
            }

            if (combinedRequest.length() == 0) {
                return false; // Nothing to send
            }

            while ((requestLimit == 0 || (combinedRequest.length() + batch.nextRequestLength()) < requestLimit) && !batch.isEmpty()) {
                auto next = batch.shift();
                if (next && next->tutor->shouldBeSent()) {
                    combinedRequest.add(*next->request);
                    combinedRequestListener.add(*next->listener);
                    ongoingRequests.push_back(std::move(*next));
                } else if (next) {
                    next->tutor->notifyAbort();
                }
            }

            activeConnection = transport->sendRequest(*protocol, combinedRequest, combinedRequestListener, options->httpExtraHeadersOnSessionCreationOnly() ? nullptr : options->httpExtraHeaders(), options->proxy(), options->tcpConnectTimeout(), options->tcpReadTimeout());

            return true;
        }

        bool onComplete(const std::string& why) {
            if (is("END")) {
                return false; // Ignore if already ended
            } else if (is("ENDING")) {
                changeStatus("END");
            } else {
                if (is("IDLE")) {
                    log.error("Unexpected batch manager status at connection end");
                }

                log.info("Batch completed");
                changeStatus("IDLE");
                dequeue(ASYNC_DEQUEUE, "closed"); // Prepare for future operations
            }
            activeConnection = nullptr; // Clear the active connection
            return true;
        }

        class ProtocolErrorException : public std::exception {
            long serialVersionUID = 1L; // This is typically not needed in C++
            int errorCode;

        public:
            // Constructor initializes the errorCode after converting the string to an integer
            ProtocolErrorException(const std::string& errorCode, const std::string& errorMessage)
                    : std::exception(errorMessage.c_str()), errorCode(std::stoi(errorCode)) {}

            // Getter for errorCode to replace the ErrorCode property in C#
            int getErrorCode() const {
                return errorCode;
            }

            // Override what() to include both error code and message
            const char* what() const noexcept override {
                static std::string fullMessage = "Error " + std::to_string(errorCode) + ": " + std::exception::what();
                return fullMessage.c_str();
            }
        };

        class BatchedRequest : public LightstreamerRequest {
            private:
            HttpRequestManager& outerInstance;

            std::stringstream fullRequest;
            std::string requestName;

            public:
            BatchedRequest(HttpRequestManager& outerInstance) : outerInstance(outerInstance) {}

            void setRequestName(const std::string& name) override {
                    requestName = name;
            }

            std::string getRequestName() const override {
                    return requestName;
            }

            void add(const LightstreamerRequest& request) {
                if (fullRequest.tellp() > 0) {
                    fullRequest << "\r\n";
                }
                fullRequest << request.getTransportAwareQueryString("", true);
            }

            long length() const {
                return static_cast<long>(fullRequest.tellp());
            }

            std::string getTransportUnawareQueryString() const override {
                    // The caller isn't aware of the transport, but we are
                    return fullRequest.str();
            }

            std::string getTransportAwareQueryString(const std::string& defaultSessionId = "", bool ackIsForced = false) const override {
                    // The caller must be aligned with the transport assumed here
                    return fullRequest.str();
            }
        };

        class BatchedListener : public RequestListener {
            HttpRequestManager& outerInstance;
            bool completed = false;
            std::vector<std::string> messages;
            std::vector<std::shared_ptr<RequestListener>> listeners;

        public:
            BatchedListener(HttpRequestManager& outer) : outerInstance(outer) {}

            size_t size() const {
                return listeners.size();
            }

            void onMessage(const std::string& message) override {
                messages.push_back(message);
            }

            void add(std::shared_ptr<RequestListener> listener) {
                listeners.push_back(std::move(listener));
            }

            void onOpen() override {
                if (outerInstance.is("END")) {
                    return; // Don't care
                }
                for (auto& listener : listeners) {
                    listener->onOpen();
                }
            }

            void dispatchMessages() {
                // Abnormal conditions handling omitted for brevity

                for (size_t i = 0; i < messages.size(); ++i) {
                    listeners[i]->onMessage(messages[i]);
                }
            }

            void onClosed() override {
                outerInstance.clearOngoingRequests();
                if (outerInstance.is("END")) {
                    return; // Don't care
                }
                if (!completed) {
                    if (outerInstance.onComplete("closed")) {
                        if (!messages.empty()) {
                            dispatchMessages();
                        }
                    }
                    completed = true;
                }

                for (auto& listener : listeners) {
                    listener->onClosed();
                }
            }

            void onBroken() override {
                outerInstance.ongoingRequests.Clear();
                if (outerInstance.is("END")) {
                    return; // Don't care
                }
                if (!completed) {
                    if (outerInstance.onComplete("broken")) {
                        // Potentially salvage messages if not empty
                        if (!messages.empty()) {
                            dispatchMessages();
                        }
                    }
                    completed = true;
                }

                for (auto& listener : listeners) {
                    listener->onBroken();
                }
            }
        };

    };

// Static constant definitions
    const std::string HttpRequestManager::IDLE = "IDLE";
    const std::string HttpRequestManager::WAITING = "WAITING";
    const std::string HttpRequestManager::END = "END";
    const std::string HttpRequestManager::ENDING = "ENDING";

} // namespace com::lightstreamer::client::protocol

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_HTTPREQUESTMANAGER_HPP
