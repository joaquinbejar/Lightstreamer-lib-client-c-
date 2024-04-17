/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 17/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SESSION_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SESSION_HPP

#include <iostream>
#include <functional>
#include <string>
#include <cassert>
#include <memory>
#include <atomic>
#include <mutex>
#include <Logger.hpp>
#include <lightstreamer/client/protocol/Protocol.hpp>
#include <lightstreamer/client/session/SessionListener.hpp>
#include <lightstreamer/client/session/SubscriptionsListener.hpp>
#include <lightstreamer/client/session/MessagesListener.hpp>
#include <lightstreamer/client/session/InternalConnectionDetails.hpp>
#include <lightstreamer/client/session/InternalConnectionOptions.hpp>
#include <lightstreamer/client/session/SessionThread.hpp>
#include <lightstreamer/client/session/SlowingHandler.hpp>
#include <lightstreamer/client/session/RecoveryBean.hpp>
#include <lightstreamer/client/session/OfflineCheck.hpp>
#include <lightstreamer/util/mdc/MDC.hpp>

namespace lightstreamer::client::session {

    /**
     * @brief Abstract class for managing sessions with the server.
     *
     * All calls to this class are performed through the Session Thread.
     */
    class Session {
    protected:
        enum class SessionState {
            OFF,
            CREATING,
            CREATED,
            FIRST_PAUSE,
            FIRST_BINDING,
            PAUSE,
            BINDING,
            RECEIVING,
            STALLING,
            STALLED,
            SLEEP,
            OTHER // for any state not covered above
        };

        static const bool GO_TO_SLEEP = true;
        static const bool GO_TO_OFF = false;
        static const bool CLOSED_ON_SERVER = true;
        static const bool OPEN_ON_SERVER = false;
        static const bool NO_RECOVERY_SCHEDULED = true;
        static const bool RECOVERY_SCHEDULED = false;

        static const int PERMISSION_TO_FAIL = 1;

        std::shared_ptr<Logger> log = LogManager::getLogger(Constants::SESSION_LOG);

        std::string sessionServerAddress = nullptr; ///< Address of the server for the current session.
        std::string serverAddressCache = nullptr;   ///< Cached server address.
        bool ignoreServerAddressCache = false;

        bool isPolling;
        bool isForced;
        std::string sessionId;
        int bindCount = 0;
        long long dataNotificationCount = 0;

        bool switchRequired = false;
        bool slowRequired = false;
        bool switchForced = false;
        std::string switchCause = "";
        bool switchToWebSocket = false; ///< WebSocket support enabled because client IP has changed.

        bool cachedRequiredBW = false;

        int workedBefore = 0;
        long long sentTime = 0;
        static std::shared_ptr<CancellationTokenSource> lastKATask;
        long long reconnectTimeout = 0;

        std::string phase = OFF;
        int phaseCount = 0;

        std::shared_ptr<SessionListener> handler;
        int handlerPhase;

        std::shared_ptr<InternalConnectionDetails> details;
        std::shared_ptr<InternalConnectionOptions> options;
        std::shared_ptr<SlowingHandler> slowing;
        std::shared_ptr<SubscriptionsListener> subscriptions;
        std::shared_ptr<MessagesListener> messages;

        std::shared_ptr<SessionThread> thread;
        std::shared_ptr<protocol::Protocol> protocol;
        bool retryAgainIfStreamFails;
        std::shared_ptr<OfflineCheck> offlineCheck;

        std::shared_ptr<RecoveryBean> recoveryBean;

        int objectId;

    public:
        Session(int objectId, bool isPolling, bool forced, std::shared_ptr<SessionListener> handler,
                std::shared_ptr<SubscriptionsListener> subscriptions, std::shared_ptr<MessagesListener> messages,
                std::shared_ptr<Session> originalSession, std::shared_ptr<SessionThread> thread,
                std::shared_ptr<protocol::Protocol> protocol, std::shared_ptr<InternalConnectionDetails> details,
                std::shared_ptr<InternalConnectionOptions> options, int callerPhase, bool retryAgainIfStreamFails,
                bool sessionRecovery)
                : objectId(objectId), isPolling(isPolling), isForced(forced), handler(handler),
                  handlerPhase(callerPhase), details(details), options(options),
                  thread(thread), protocol(protocol), retryAgainIfStreamFails(retryAgainIfStreamFails) {

            if (log->isDebugEnabled()) {
                log->debug("New session oid=" + std::to_string(this->objectId));
            }

            this->slowing = std::make_shared<SlowingHandler>(this->options);
            this->subscriptions = subscriptions;
            this->messages = messages;
            this->protocol->setListener(std::make_shared<TextProtocolListener>(this));

            this->offlineCheck = std::make_shared<OfflineCheck>(thread);

            if (originalSession) {
                sessionId = originalSession->sessionId;
                sessionServerAddress = originalSession->sessionServerAddress;
                bindCount = originalSession->bindCount;
                dataNotificationCount = originalSession->dataNotificationCount;

                assert(!originalSession->serverAddressCache.empty());
                serverAddressCache = originalSession->serverAddressCache;
                ignoreServerAddressCache = originalSession->ignoreServerAddressCache;

                slowing->setMeanElaborationDelay(originalSession->slowing->getMeanElaborationDelay());

                originalSession->protocol->copyPendingRequests(protocol);

                recoveryBean = std::make_shared<RecoveryBean>(sessionRecovery, originalSession->recoveryBean);
            } else {
                assert(!sessionRecovery);
                recoveryBean = std::make_shared<RecoveryBean>();
            }
        }

    private:
        void reset() {
            sessionId = "";
            sessionServerAddress = "";
            bindCount = 0;
            dataNotificationCount = 0;

            serverAddressCache = "";
            ignoreServerAddressCache = false;

            switchRequired = false;
            switchForced = false;
            slowRequired = false;
            switchCause = "";

            cachedRequiredBW = false;
        }

        SessionState getStateFromString(const std::string &phase) const {
            if (phase == "OFF") return SessionState::OFF;
            if (phase == "CREATING") return SessionState::CREATING;
            if (phase == "CREATED") return SessionState::CREATED;
            if (phase == "FIRST_PAUSE") return SessionState::FIRST_PAUSE;
            if (phase == "FIRST_BINDING") return SessionState::FIRST_BINDING;
            if (phase == "PAUSE") return SessionState::PAUSE;
            if (phase == "BINDING") return SessionState::BINDING;
            if (phase == "RECEIVING") return SessionState::RECEIVING;
            if (phase == "STALLING") return SessionState::STALLING;
            if (phase == "STALLED") return SessionState::STALLED;
            if (phase == "SLEEP") return SessionState::SLEEP;
            return SessionState::OTHER;
        }

    protected:
        /**
         * @brief Checks if the current phase matches the specified phase.
         * @param phaseToCheck The phase to check against the current phase.
         * @return True if the current phase matches the specified phase, false otherwise.
         */
        virtual bool is(const std::string &phaseToCheck) const {
            return this->phase == phaseToCheck;
        }

        /**
         * @brief Checks if the current phase does not match the specified phase.
         * @param phaseToCheck The phase to check against the current phase.
         * @return True if the current phase does not match the specified phase, false otherwise.
         */
        virtual bool isNot(const std::string &phaseToCheck) const {
            return !is(phaseToCheck);
        }

        /**
         * @brief Changes the current phase to a new phase and optionally starts recovery.
         * @param newType The new phase to transition to.
         * @param startRecovery Indicates whether to start recovery.
         * @return True if the phase change was successful and not externally modified, false otherwise.
         */
        virtual bool changePhaseType(const std::string &newType, bool startRecovery = false) {
            std::string oldType = this->phase;
            int ph = this->phaseCount;

            if (isNot(newType)) {
                this->phase = newType;
                this->phaseCount++;
                ph = this->phaseCount;

                this->handler->sessionStatusChanged(this->handlerPhase, this->phase, startRecovery);

                if (log->isDebugEnabled()) {
                    log->debug(
                            "Session state change (" + std::to_string(objectId) + "): " + oldType + " -> " + newType);
                    log->debug(" phasing : " + std::to_string(ph) + " - " + std::to_string(this->phaseCount));
                }
            }

            return ph == this->phaseCount;
        }

        /**
         * @brief Retrieves the high-level status of the session based on current phase and recovery status.
         * @param startRecovery Indicates whether the recovery process is considered started.
         * @return The high-level status as a string.
         */
        std::string getHighLevelStatus(bool startRecovery) const {
            SessionState state = getStateFromString(this->phase);

            switch (state) {
                case SessionState::OFF:
                    return Constants::DISCONNECTED;
                case SessionState::SLEEP:
                    return startRecovery ? Constants::TRYING_RECOVERY : Constants::WILL_RETRY;
                case SessionState::CREATING:
                    return recoveryBean->isRecovery() ? Constants::TRYING_RECOVERY : Constants::CONNECTING;
                case SessionState::CREATED:
                case SessionState::FIRST_PAUSE:
                case SessionState::FIRST_BINDING:
                    return Constants::CONNECTED + this->firstConnectedStatus();
                case SessionState::STALLED:
                    return Constants::STALLED;
                default:
                    return Constants::CONNECTED + this->connectedHighLevelStatus();
            }
        }

        /**
 * @brief Pure virtual method to be overridden in derived classes to provide the first connected status.
 * @return A string representing the first connected status.
 */
        virtual std::string firstConnectedStatus() const = 0;

        /**
         * @brief Handles sending of a reverse heartbeat if necessary.
         * @param force If true, forcefully handle the reverse heartbeat.
         */
        virtual void handleReverseHeartbeat(bool force) {
            this->protocol->handleReverseHeartbeat();
        }

        /**
         * @brief Pure virtual method to determine if content length should be asked.
         * @return True if content length should be asked, false otherwise.
         */
        virtual bool shouldAskContentLength() const = 0;

        /**
         * @brief Determines if the session is considered open.
         * @return True if the session is not in OFF, CREATING, or SLEEP states.
         */
        bool isOpen() const {
            return isNot("OFF") && isNot("CREATING") && isNot("SLEEP");
        }

        /**
         * @brief Checks if the current session is a streaming session.
         * @return True if the session is not polling, false if it is polling.
         */
        bool isStreamingSession() const {
            return !this->isPolling;
        }

        /**
         * @brief Gets the server address to be used for push communication.
         * @return The control-link address if available; otherwise, the address configured at startup.
         */
        std::string pushServerAddress() const {
            if (sessionServerAddress.empty()) {
                return serverAddressCache;
            }
            return sessionServerAddress;
        }

        /**
         * @brief Executes the bind session process.
         * @param bindCause The reason for binding the session.
         * @return A future that will hold the result of the bind session request.
         */
        std::future<bool> bindSessionExecution(const std::string &bindCause) {
            auto request = std::make_shared<BindSessionRequest>(
                    pushServerAddress(),
                    getSessionId(),
                    isPolling,
                    bindCause,
                    options,
                    slowing->delay(),
                    shouldAskContentLength(),
                    protocol->maxReverseHeartbeatIntervalMs()
            );

            return protocol->sendBindRequest(request);
        }

        /**
         * @brief Attempts to recover the session after a network error or similar issue.
         */
        void recoverSession() {
            auto request = std::make_shared<RecoverSessionRequest>(
                    pushServerAddress(),
                    getSessionId(),
                    "network.error",
                    options,
                    slowing->delay(),
                    dataNotificationCount
            );

            protocol->sendRecoveryRequest(request);
            createSent(); // Assume createSent() manages the reconnection logic.
        }

        /**
         * @brief Requests a switch in the session state to handle new phase changes or recoveries.
         */
        void requestSwitch(int newHPhase, const std::string &switchCause, bool forced, bool startRecovery) {
            handlerPhase = newHPhase;

            if (switchRequired) {
                return; // Switch already requested!
            }

            if (log->isDebugEnabled()) {
                log->debug("Switch requested phase=" + phase + " cause=" + switchCause);
            }

            slowRequired = false; // Overriding any pending slow-switch command.

            if (is("CREATING") || is("SLEEP") || is("OFF")) {
                handler->streamSense(handlerPhase, switchCause, forced);
            } else if (is("PAUSE") || is("FIRST_PAUSE")) {
                handler->switchReady(handlerPhase, switchCause, forced, startRecovery);
            } else {
                switchRequired = true;
                switchForced = forced;
                this->switchCause = switchCause;
                sendForceRebind(switchCause);
            }
        }

        /**
         * @brief Requests a slow operation mode to accommodate slower network conditions or client performance.
         */
        void requestSlow(int newHPhase) {
            handlerPhase = newHPhase;

            if (slowRequired) {
                return; // Slow mode already requested.
            }

            log->debug("Slow requested");

            if (is("CREATING") || is("SLEEP") || is("OFF")) {
                log->error("Unexpected phase during slow handling");
                shutdown(GO_TO_OFF);
                return;
            }

            if (is("PAUSE") || is("FIRST_PAUSE")) {
                handler->slowReady(handlerPhase);
            } else {
                slowRequired = true;
                sendForceRebind("slow");
            }
        }

        /**
         * @brief Closes the session with the specified reason.
         * @param closeReason The reason for closing the session.
         * @param alreadyClosedOnServer Whether the session is already closed on the server.
         * @param noRecoveryScheduled Whether recovery is scheduled or not.
         */
        virtual void
        closeSession(const std::string &closeReason, bool alreadyClosedOnServer, bool noRecoveryScheduled) {
            closeSession(closeReason, alreadyClosedOnServer, noRecoveryScheduled, false);
        }

        /**
         * @brief Closes the session with additional control over the connection state.
         * @param closeReason The reason for closing the session.
         * @param alreadyClosedOnServer Whether the session is already closed on the server.
         * @param noRecoveryScheduled Whether recovery is scheduled or not.
         * @param forceConnectionClose Forcefully close the connection if true.
         */
        void closeSession(const std::string &closeReason, bool alreadyClosedOnServer, bool noRecoveryScheduled,
                          bool forceConnectionClose) {
            log.info("Closing session: " + closeReason);

            if (isOpen()) {
                if (!alreadyClosedOnServer) {
                    sendDestroySession(closeReason);
                }

                subscriptions->onSessionClose();
                messages->onSessionClose();
                handlerPhase = handler->onSessionClose(handlerPhase, noRecoveryScheduled);

                details->clear();
                options->resetInternalMaxBandwidth();
            } else {
                subscriptions->onSessionClose();
                messages->onSessionClose();
                handlerPhase = handler->onSessionClose(handlerPhase, noRecoveryScheduled);
            }

            shutdown(!noRecoveryScheduled, forceConnectionClose);
        }

        /**
         * @brief Shutdown the session optionally going to sleep or closing off completely.
         * @param goToSleep Indicates whether to go to sleep (temporary shutdown) or not.
         */
        virtual void shutdown(bool goToSleep) {
            shutdown(goToSleep, false);
        }

        /**
         * @brief Forcefully or gracefully shuts down the session.
         * @param goToSleep Indicates whether to put the session in sleep mode.
         * @param forceConnectionClose Forcefully closes the connection if true.
         */
        virtual void shutdown(bool goToSleep, bool forceConnectionClose) {
            reset();
            changePhaseType(goToSleep ? SLEEP : OFF);
            protocol->stop(goToSleep, forceConnectionClose);
        }

    public:
        /**
         * @brief Gets the session ID.
         * @return The session ID if set, or an empty string if not.
         */
        std::string getSessionId() const {
            if (sessionId.empty()) {
                return "";
            }
            return sessionId;
        }

        /**
         * @brief Sets the session ID.
         * @param value The new session ID.
         */
        void setSessionId(const std::string &value) {
            sessionId = value;
            // Assuming MDC is some sort of logging context manager:
            if (MDC::isEnabled()) {
                MDC::put("sessionId", sessionId);
            }
        }


    private:
        // Helper methods for deriving specific status strings
        std::string firstConnectedStatus() const {
            // Placeholder: implement based on specific logic that determines the "first connected" status.
            return ""; // Modify as needed
        }

        std::string connectedHighLevelStatus() const {
            // Placeholder: implement based on specific logic that determines the "connected high level" status.
            return ""; // Modify as needed
        }

        class IRunnable {
        public:
            virtual void run() = 0;

            virtual ~IRunnable() = default;
        };

        class MyRunnableA : public IRunnable {
        private:
            Session *session; // Using raw pointer for simplicity in this context.

        public:
            // Constructor to initialize the session pointer.
            explicit MyRunnableA(Session *session) : session(session) {}

            // Implementation of the run method.
            void run() override {
                // Debug assertion to ensure this is being called on the correct thread could be added here.
                session->bindSent();
            }

            // Ensure proper destruction.
            ~MyRunnableA() override = default;
        };

        bool createNewOnFirstBindTimeout() {
            return isPolling;
        }

        std::future<void>
        launchTimeout(const std::string &timeoutType, long pauseToUse, const std::string &cause, bool startRecovery) {
            int pc = phaseCount;
            log.debug("Status timeout in " + std::to_string(pauseToUse) + " [" + timeoutType + "] due to " + cause);

            return std::async(std::launch::async, [this, pc, pauseToUse, timeoutType, cause, startRecovery] {
                std::this_thread::sleep_for(std::chrono::milliseconds(pauseToUse + 50));

                if (pc != phaseCount) {
                    return;
                }

                onTimeout(timeoutType, pc, pauseToUse, cause, startRecovery);
            });
        }

        void timeoutForStalling() {
            if (options.keepaliveInterval > 0) {
                if (lastKATask != nullptr && !lastKATask->isCanceled()) {
                    lastKATask->cancel();
                }

                lastKATask = launchTimeout("keepaliveInterval", options.keepaliveInterval, "", false);
            }
        }

        void timeoutForStalled() {
            if (!changePhaseType("STALLING")) {
                return;
            }

            launchTimeout("stalledTimeout", options.stalledTimeout + 500, "", false);
        }

        void timeoutForReconnect() {
            if (!changePhaseType("STALLED")) {
                return;
            }

            long timeLeftMs = recoveryBean.timeLeftMs(options.sessionRecoveryTimeout);
            bool startRecovery = timeLeftMs > 0;
            launchTimeout("reconnectTimeout", options.reconnectTimeout, "", startRecovery);
        }

        void timeoutForExecution() {
            try {
                launchTimeout("executionTimeout", options.stalledTimeout, "", false);
            } catch (const std::exception &e) {
                log.warn("Something went wrong: " + std::string(e.what()));
            }
            log.debug("Check Point 1a120ak.");
        }

        long getBindTimeout() const {
            if (isPolling) {
                return options.currentConnectTimeout + options.idleTimeout;
            } else {
                return (workedBefore > 0 && reconnectTimeout > 0) ? reconnectTimeout : options.currentConnectTimeout;
            }
        }

        long getRealPollingInterval() const {
            if (is("FIRST_PAUSE")) {
                return options.pollingInterval;
            } else {
                auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()
                );
                long spent = (now - sentTime).count();
                return spent > options.pollingInterval ? 0 : options.pollingInterval - spent;
            }
        }

        long calculateRetryDelay() const {
            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
            );
            long spent = (now - sentTime).count();
            long currentRetryDelay = options.currentRetryDelay;
            return spent > currentRetryDelay ? 0 : currentRetryDelay - spent;
        }

        void sendForceRebind(const std::string &rebindCause) {
            log.info("Sending request to the server to force a rebind on the current connection during " + phase);

            ForceRebindRequest request(pushServerAddress(), sessionId, rebindCause, slowing.delay());
            ForceRebindTutor tutor(*this, phaseCount, rebindCause);

            protocol.sendForceRebind(request, tutor);
        }

        void sendDestroySession(const std::string &closeReason) {
            log.info("Sending request to the server to destroy the current session during " + phase);

            DestroyRequest request(pushServerAddress(), sessionId, closeReason);
            protocol.sendDestroy(request);

            // we do not retry destroy requests: just fire and forget
        }

        void sendMessage(MessageRequest request, RequestTutor tutor) {
            request.setServer(pushServerAddress());
            request.setSession(sessionId);

            if (log.isDebugEnabled()) {
                log.debug("Sending client message request to the server. " + std::to_string(request.requestId()));
            }

            protocol.sendMessageRequest(request, tutor);
        }

        /// Sends a bandwidth request to the transport layer.
        void sendConstrain(long timeoutMs, ConstrainRequest *clientRequest) {
            if (is("OFF") || is("SLEEP")) {
                return;
            } else if (bandwidthUnmanaged) {
                // If the bandwidth is unmanaged, it is useless to try to change it.
                return;
            } else if (is("CREATING")) {
                // Too late to send it via create_session, too early to send it via control.
                cachedRequiredBW = true;
                return;
            }

            ConstrainRequest request(options.internalMaxBandwidth, clientRequest);
            request.setSession(sessionId);
            ConstrainTutor tutor(timeoutMs, request, *this, options);
            request.setServer(pushServerAddress());

            if (bwRetransmissionMonitor.canSend(request)) {
                protocol.sendConstrainRequest(request, tutor);
            }
        }

        class BandwidthRetransmissionMonitor {
        public:
            BandwidthRetransmissionMonitor() : lastReceivedRequestId(-1), lastPendingRequestId(-1) {}

            // Must be checked before sending a request to ensure it does not override newer requests
            bool canSend(const ConstrainRequest &request) {
                std::lock_guard<std::mutex> lock(mutex);
                long clientId = request.getClientRequestId();
                bool isForbidden = (clientId < lastPendingRequestId || clientId <= lastReceivedRequestId);
                if (!isForbidden) {
                    lastPendingRequestId = clientId;
                }
                return !isForbidden;
            }

            // Must be checked after receiving a response to update the state correctly
            void onReceivedResponse(const ConstrainRequest &request) {
                std::lock_guard<std::mutex> lock(mutex);
                long clientId = request.getClientRequestId();
                if (clientId > lastReceivedRequestId) {
                    lastReceivedRequestId = clientId;
                }
            }

        private:
            std::mutex mutex;
            long lastReceivedRequestId;
            long lastPendingRequestId;
        };

    protected:

        /**
         * @brief Initiates creation of a new session, potentially replacing an old one.
         * @param oldSessionId The session ID of the previous session.
         * @param reconnectionCause The cause of the reconnection.
         */
        virtual void createSession(const std::string &oldSessionId, const std::string &reconnectionCause) {
            bool openOnServer = isNot("OFF") && isNot("SLEEP") ? OPEN_ON_SERVER : CLOSED_ON_SERVER;

            std::string cause = reconnectionCause.empty() ? "" : reconnectionCause;
            if (openOnServer == OPEN_ON_SERVER) {
                closeSession("new." + cause, OPEN_ON_SERVER, RECOVERY_SCHEDULED);
            }

            reset();

            details->sessionId = "";
            details->serverSocketName = "";
            details->clientIp = "";
            details->serverInstanceAddress = "";

            serverAddressCache = details->serverAddress;

            ignoreServerAddressCache = options->serverInstanceAddressIgnored;

            options->internalRealMaxBandwidth.reset();

            log->info("Opening new session ... ");

            if (createSessionExecution(phaseCount, oldSessionId, cause)) {
                createSent();
            }
        }

        /**
         * @brief Executes the session creation process.
         * @param ph The phase count.
         * @param oldSessionId The previous session's ID.
         * @param cause The cause for session creation.
         * @return True if the session was successfully created, false otherwise.
         */
        virtual bool createSessionExecution(int ph, const std::string &oldSessionId, const std::string &cause) {
            if (ph != phaseCount) {
                return false;
            }

            std::string server = pushServerAddress();

            if (offlineCheck->shouldDelay(server)) {
                log->info("Client is offline, delaying connection to server");
                thread->schedule([=] {
                    createSessionExecution(ph, oldSessionId, "offline");
                }, offlineCheck->delay());

                return false;
            }

            auto request = std::make_shared<CreateSessionRequest>(server, isPolling, cause, options, details,
                                                                  slowing->delay(), details->password, oldSessionId);
            protocol->sendCreateRequest(request);

            return true;
        }

        /**
         * @brief Binds the session with the server.
         * @param bindCause The reason for binding.
         */
        virtual void bindSession(const std::string &bindCause) {
            bindCount++;

            if (isNot("PAUSE") && isNot("FIRST_PAUSE") && isNot("OFF")) {
                log->error("Unexpected phase during binding of session");
                shutdown(GO_TO_OFF);
                return;
            }

            if (is("OFF")) {
                if (!changePhaseType("FIRST_PAUSE")) {
                    return;
                }
            }

            if (!isPolling) {
                log->info("Binding session");
            } else {
                log->debug("Binding session");
            }

            auto bindFuture = bindSessionExecution(bindCause);
            bindFuture.then([this]() {
                // Placeholder for fulfilled action
            });
        }

        void onTimeout(const std::string &timeoutType, int phaseCount, long usedTimeout, const std::string &coreCause,
                       bool startRecovery) {
            if (phaseCount != this->phaseCount) {
                return;
            }

            log.debug("Timeout event [" + timeoutType + "] while " + phase + " cause=" + coreCause);

            std::string tCause = "timeout." + phase + "." + std::to_string(bindCount);
            if (is("SLEEP") && !coreCause.empty()) {
                tCause = coreCause;
            }

            if (is("CREATING")) {
                long timeLeftMs = recoveryBean.timeLeftMs(options.sessionRecoveryTimeout);
                if (recoveryBean.recovery && timeLeftMs > 0) {
                    log.debug("Start session recovery. Cause: no response timeLeft=" + std::to_string(timeLeftMs));
                    options.increaseConnectTimeout();
                    handler.recoverSession(handlerPhase, tCause, isForced, workedBefore > 0);
                } else {
                    log.debug("Start new session. Cause: no response");
                    closeSession("create.timeout", CLOSED_ON_SERVER, RECOVERY_SCHEDULED, true);
                    options.increaseConnectTimeout();
                    launchTimeout("zeroDelay", 0, "create.timeout", false);
                }
            } else if (is("CREATED") || is("BINDING") || is("STALLED") || is("SLEEP")) {
                if (slowRequired || switchRequired) {
                    log.debug("Timeout: switch transport");
                    handler.streamSense(handlerPhase, tCause + ".switch", switchForced);
                } else if (!isPolling || isForced) {
                    if (startRecovery) {
                        handler.recoverSession(handlerPhase, tCause, isForced, workedBefore > 0);
                    } else {
                        log.debug("Timeout: new session");
                        handler.retry(handlerPhase, tCause, isForced, workedBefore > 0);
                    }
                } else {
                    log.debug(startRecovery ? "Timeout: switch transport from polling (ignore recovery)"
                                            : "Timeout: switch transport from polling");
                    handler.streamSense(handlerPhase, tCause, false);
                }
            } else if (is("FIRST_BINDING")) {
                if (slowRequired || switchRequired) {
                    handler.streamSense(handlerPhase, tCause + ".switch", switchForced);
                } else if (workedBefore > 0 || isForced || retryAgainIfStreamFails) {
                    handler.retry(handlerPhase, tCause, isForced, workedBefore > 0);
                } else if (createNewOnFirstBindTimeout()) {
                    handler.streamSense(handlerPhase, tCause + ".switch", switchForced);
                } else {
                    handler.streamSenseSwitch(handlerPhase, tCause, phase, recoveryBean.recovery);
                }
            } else if (is("PAUSE")) {
                if (isPolling) {
                    slowing.testPollSync(usedTimeout, std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch()).count());
                }
                bindSession("loop");
            } else if (is("FIRST_PAUSE")) {
                if (switchToWebSocket) {
                    handler.switchToWebSocket(recoveryBean.recovery);
                    switchToWebSocket = false;
                } else {
                    bindSession("loop1");
                }
            } else if (is("RECEIVING")) {
                timeoutForStalled();
            } else if (is("STALLING")) {
                timeoutForReconnect();
            } else { // _OFF
                log.error("Unexpected timeout event while session is OFF");
                shutdown(GO_TO_OFF);
            }
        }

        void createSent() {
            sentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
            if (isNot("OFF") && isNot("SLEEP")) {
                log.error("Unexpected phase after create request sent: " + phase);
                shutdown(GO_TO_OFF);
                return;
            }
            if (!changePhaseType("CREATING")) {
                return;
            }
            launchTimeout("currentConnectTimeout", options.currentConnectTimeout, "", false);
        }

        void bindSent() {
            sentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
            if (isNot("PAUSE") && isNot("FIRST_PAUSE")) {
                log.error("Unexpected phase after bind request sent: " + phase);
                shutdown(GO_TO_OFF);
                return;
            }
            if (!changePhaseType(is("PAUSE") ? "BINDING" : "FIRST_BINDING")) {
                return;
            }
            launchTimeout("bindTimeout", bindTimeout, "", false);
        }

        virtual void
        doOnErrorEvent(const std::string &reason, bool closedOnServer, bool unableToOpen, bool startRecovery,
                       long timeLeftMs, bool wsError) {
            log.Debug("Evento de error por " + reason + " mientras " + phase);

            if (is(RECEIVING) || is(STALLED) || is(STALLING) || is(BINDING) || is(PAUSE)) {
                if (startRecovery) {
                    log.Debug("Iniciar recuperación de la sesión. Causa: fallo de socket mientras se recibían datos.");
                    changePhaseType(SLEEP, startRecovery);
                } else {
                    closeSession(reason, closedOnServer, RECOVERY_SCHEDULED);
                    assert(is(SLEEP));
                    long pause = static_cast<long>(std::round(GlobalRandom::NextDouble() * options.FirstRetryMaxDelay));
                    launchTimeout("firstRetryMaxDelay", pause, reason, startRecovery);
                }
            } else if (is(CREATING) || is(CREATED) || is(FIRST_BINDING)) {
                if (recoveryBean.Recovery && timeLeftMs > 0 && !closedOnServer) {
                    log.Debug("Iniciar recuperación de la sesión. Causa: fallo de socket durante la recuperación.");
                    changePhaseType(SLEEP, true);
                    launchTimeout("currentRetryDelay", calculateRetryDelay(), reason, startRecovery);
                    options.increaseRetryDelay();
                } else if (switchRequired && !isForced) {
                    handler.streamSense(handlerPhase, switchCause + ".error", switchForced);
                } else {
                    std::string cause = (closedOnServer ? "cerrado por el servidor" : "error de socket");
                    long crd = calculateRetryDelay();
                    log.Debug("Iniciar nueva sesión. Causa: " + cause + " en " + std::to_string(crd));
                    closeSession(reason, closedOnServer, RECOVERY_SCHEDULED);
                    launchTimeout("currentRetryDelay", crd, reason, false);
                    options.increaseRetryDelay();
                }
            } else {
                log.Error("Evento de error inesperado mientras la sesión está en un estado no activo: " + phase);
            }
        }

        virtual void changeControlLink(const std::string &controlLink) {}

    public:
        void sendSubscription(SubscribeRequest &request, RequestTutor &tutor) {
            request.setServer(pushServerAddress());
            request.setSession(sessionId);
            protocol.sendSubscriptionRequest(request, tutor);
        }

        void sendUnsubscription(UnsubscribeRequest &request, RequestTutor &tutor) {
            request.setServer(pushServerAddress());
            request.setSession(sessionId);
            protocol.sendUnsubscriptionRequest(request, tutor);
        }

        void sendSubscriptionChange(ChangeSubscriptionRequest &request, RequestTutor &tutor) {
            request.setServer(pushServerAddress());
            request.setSession(sessionId);
            protocol.sendConfigurationRequest(request, tutor);
        }

        void sendReverseHeartbeat(ReverseHeartbeatRequest &request, RequestTutor &tutor) {
            request.setServer(pushServerAddress());
            request.setSession(sessionId);
            protocol.sendReverseHeartbeat(request, tutor);
        }

        /// Closes the session and notifies the error to ClientListener.
        void notifyServerError(int errorCode, const std::string &errorMessage) {
            closeSession("end", true, true);
            handler.onServerError(errorCode, errorMessage);
        }

        class TextProtocolListener : public protocol::ProtocolListener {
            Session &outerInstance;

        public:
            TextProtocolListener(Session &outerInstance) : outerInstance(outerInstance) {}

            void onInterrupted(bool wsError, bool unableToOpen) override {
                // An interruption triggers an attempt to recover the session.
                onErrorEvent("network.error", false, unableToOpen, true, wsError);
            }

            void onConstrainResponse(const ConstrainTutor &tutor) override {
                outerInstance.bwRetransmissionMonitor.onReceivedResponse(tutor.getRequest());
            }

            void onServerSentBandwidth(const std::string &maxBandwidth) override {
                if (maxBandwidth == "unmanaged") {
                    outerInstance.options.BandwidthUnmanaged = true;
                }
                outerInstance.options.InternalRealMaxBandwidth = (maxBandwidth == "unmanaged") ? "unlimited"
                                                                                               : maxBandwidth;
            }

            void onTakeover(int specificCode) override {
                onErrorEvent("error" + std::to_string(specificCode), CLOSED_ON_SERVER, false, false, false);
            }

            void onKeepalive() override {
                onEvent();
            }

            void onOKReceived(const std::string &newSession, const std::string &controlLink, long requestLimitLength,
                              long keepaliveIntervalDefault) override {
                outerInstance.logDebug("OK event while " + outerInstance.phase);
                if (!outerInstance.is(CREATING) && !outerInstance.is(FIRST_BINDING) && !outerInstance.is(BINDING)) {
                    outerInstance.logError("Unexpected OK event while session is in status: " + outerInstance.phase);
                    outerInstance.shutdown(GO_TO_OFF);
                    return;
                }

                std::string lastUsedAddress = outerInstance.PushServerAddress();
                std::string addressToUse = (!controlLink.empty() && !outerInstance.ignoreServerAddressCache)
                                           ? controlLink : lastUsedAddress;
                outerInstance.sessionServerAddress = addressToUse;

                outerInstance.logDebug("Address to use after create: " + outerInstance.sessionServerAddress);

                if (lastUsedAddress != outerInstance.sessionServerAddress && outerInstance.is(CREATING)) {
                    outerInstance.changeControlLink(outerInstance.sessionServerAddress);
                }

                if (keepaliveIntervalDefault > 0) {
                    if (outerInstance.isPolling) {
                        outerInstance.options.IdleTimeout = keepaliveIntervalDefault;
                    } else {
                        outerInstance.options.KeepaliveInterval = keepaliveIntervalDefault;
                    }
                }

                if (outerInstance.is(CREATING)) {
                    outerInstance.SessionId = newSession;
                } else if (outerInstance.SessionId != newSession) {
                    outerInstance.logError("Bound unexpected session: " + newSession);
                    outerInstance.shutdown(GO_TO_OFF);
                } else {
                    long spentTime = getCurrentMilliseconds() - outerInstance.sentTime;
                    outerInstance.reconnectTimeout = std::max(spentTime, outerInstance.options.CurrentConnectTimeout);
                    outerInstance.logDebug("CurrentConnectTimeout: " + std::to_string(outerInstance.reconnectTimeout));
                }

                outerInstance.slowing.startSync(outerInstance.isPolling, outerInstance.isForced);
                onEvent();
            }

            void onLoopReceived(long serverSentPause) override {
                if (outerInstance.is(RECEIVING) || outerInstance.is(STALLING) || outerInstance.is(STALLED) ||
                    outerInstance.is(CREATED)) {
                    if (outerInstance.switchRequired) {
                        outerInstance.handler.switchReady(outerInstance.handlerPhase, outerInstance.switchCause,
                                                          outerInstance.switchForced, false);
                    } else if (outerInstance.slowRequired) {
                        outerInstance.handler.slowReady(outerInstance.handlerPhase);
                    } else {
                        doPause(serverSentPause);
                    }
                } else {
                    outerInstance.logError(
                            "Unexpected loop event while session is in non-active status: " + outerInstance.phase);
                    outerInstance.shutdown(GO_TO_OFF);
                }
            }

            void onSyncError(bool async) override {
                std::string cause = async ? "syncerror" : "control.syncerror";
                onErrorEvent(cause, true, false, false, false);
            }

            void onRecoveryError() override {
                onErrorEvent("recovery.error", true, false, false, false);
            }

            void onExpiry() override {
                onErrorEvent("expired", true, false, false, false);
            }

            void onUpdateReceived(int subscriptionId, int item, const std::vector<std::string> &args) override {
                onEvent();
                outerInstance.subscriptions.onUpdateReceived(subscriptionId, item, args);
            }

            void onEndOfSnapshotEvent(int subscriptionId, int item) override {
                onEvent();
                outerInstance.subscriptions.onEndOfSnapshotEvent(subscriptionId, item);
            }

            void onClearSnapshotEvent(int subscriptionId, int item) override {
                onEvent();
                outerInstance.subscriptions.onClearSnapshotEvent(subscriptionId, item);
            }

            void onLostUpdatesEvent(int subscriptionId, int item, int lost) override {
                onEvent();
                outerInstance.subscriptions.onLostUpdatesEvent(subscriptionId, item, lost);
            }

            void onConfigurationEvent(int subscriptionId, const std::string &frequency) override {
                onEvent();
                outerInstance.subscriptions.onConfigurationEvent(subscriptionId, frequency);
            }

            void onMessageAck(const std::string &sequence, int number, bool async) override {
                if (async) {
                    onEvent();
                }
                outerInstance.messages.onMessageAck(sequence, number);
            }

            void onMessageOk(const std::string &sequence, int number) override {
                onEvent();
                outerInstance.messages.onMessageOk(sequence, number);
            }

            void onMessageDeny(const std::string &sequence, int denyCode, const std::string &denyMessage, int number,
                               bool async) override {
                if (async) {
                    onEvent();
                }
                outerInstance.messages.onMessageDeny(sequence, denyCode, denyMessage, number);
            }

            void onMessageDiscarded(const std::string &sequence, int number, bool async) override {
                if (async) {
                    onEvent();
                }
                outerInstance.messages.onMessageDiscarded(sequence, number);
            }

            void onMessageError(const std::string &sequence, int errorCode, const std::string &errorMessage, int number,
                                bool async) override {
                if (async) {
                    onEvent();
                }
                outerInstance.messages.onMessageError(sequence, errorCode, errorMessage, number);
            }

            void onSubscriptionError(int subscriptionId, int errorCode, const std::string &errorMessage,
                                     bool async) override {
                if (async) {
                    onEvent();
                }
                outerInstance.subscriptions.onSubscriptionError(subscriptionId, errorCode, errorMessage);
            }

            void onServerError(int errorCode, const std::string &errorMessage) override {
                outerInstance.notifyServerError(errorCode, errorMessage);
            }

            void onUnsubscriptionAck(int subscriptionId) override {
                onEvent();
                outerInstance.subscriptions.onUnsubscriptionAck(subscriptionId);
            }

            void onUnsubscription(int subscriptionId) override {
                onEvent();
                outerInstance.subscriptions.onUnsubscription(subscriptionId);
            }

            void onSubscriptionAck(int subscriptionId) override {
                outerInstance.subscriptions.onSubscriptionAck(subscriptionId);
            }

            void onSubscription(int subscriptionId, int totalItems, int totalFields, int keyPosition,
                                int commandPosition) override {
                onEvent();
                outerInstance.subscriptions.onSubscription(subscriptionId, totalItems, totalFields, keyPosition,
                                                           commandPosition);
            }

            void onSubscriptionReconf(int subscriptionId, long reconfId, bool async) override {
                if (async) {
                    onEvent();
                }
                outerInstance.subscriptions.onSubscriptionReconf(subscriptionId, reconfId);
            }

            void onSyncMessage(long seconds) override {
                onEvent();
                bool syncOk = outerInstance.slowing.syncCheck(seconds, !outerInstance.isPolling);
                if (!syncOk) {
                    if (!outerInstance.switchRequired && !outerInstance.slowRequired) {
                        outerInstance.handler.onSlowRequired(outerInstance.handlerPhase, outerInstance.slowing.Delay);
                    }
                }
            }

            void onServerName(const std::string &serverName) override {
                outerInstance.details.ServerSocketName = serverName;
            }

            void onClientIp(const std::string &clientIp) override {
                outerInstance.details.ClientIp = clientIp;
                outerInstance.handler.onIPReceived(clientIp);
            }

        private:
            void onEvent() {
                if (outerInstance.is(CREATING) && !outerInstance.changePhaseType(CREATED)) {
                    return;
                }
                outerInstance.timeoutForExecution();
            }

            void onErrorEvent(const std::string &reason, bool closedOnServer, bool unableToOpen, bool tryRecovery,
                              bool wsError) {
                long timeLeftMs = outerInstance.recoveryBean.timeLeftMs(outerInstance.options.SessionRecoveryTimeout);
                if (outerInstance.is(OFF)) {
                    return;
                }
                bool startRecovery = tryRecovery && timeLeftMs > 0;
                outerInstance.doOnErrorEvent(reason, closedOnServer, unableToOpen, startRecovery, timeLeftMs, wsError);
            }

            void doPause(long serverSentPause) {
                long pauseToUse = serverSentPause;
                if (outerInstance.isPolling && outerInstance.isNot(FIRST_PAUSE)) {
                    if (serverSentPause >= outerInstance.options.PollingInterval) {
                        // Server response pause time is acceptable; no adjustment needed.
                    } else {
                        // Adapt to the server-suggested pause time.
                        outerInstance.options.PollingInterval = serverSentPause;
                    }
                    pauseToUse = outerInstance.realPollingInterval();
                }
                if (outerInstance.isNot(FIRST_PAUSE) && pauseToUse > 0) {
                    outerInstance.launchTimeout("pause", pauseToUse, nullptr, false);
                } else {
                    outerInstance.onTimeout("noPause", outerInstance.phaseCount, 0, nullptr, false);
                }
            }
        };

        class ForceRebindTutor : public requests::RequestTutor {
        private:
            Session &outerInstance;
            int currentPhase;
            std::string cause;

        public:
            ForceRebindTutor(Session &outerInstance, int currentPhase, const std::string &cause)
                    : RequestTutor(outerInstance.thread, outerInstance.options),
                      outerInstance(outerInstance),
                      currentPhase(currentPhase),
                      cause(cause) {}

            bool verifySuccess() override {
                return this->currentPhase != outerInstance.getPhaseCount();
            }

            void doRecovery() override {
                outerInstance.sendForceRebind(this->cause);
            }

            void notifyAbort() override {
                // No action needed
            }

            bool timeoutFixed() const override {
                return true;
            }

            long fixedTimeout() const override {
                return this->connectionOptions.forceBindTimeout();
            }

            bool shouldBeSent() override {
                return this->currentPhase == outerInstance.getPhaseCount();
            }
        };

        class ConstrainTutor : public requests::RequestTutor {
        private:
            ConstrainRequest &request;

        public:
            ConstrainTutor(long timeoutMs, ConstrainRequest &request, SessionThread &sessionThread,
                           InternalConnectionOptions &options)
                    : RequestTutor(timeoutMs, sessionThread, options, false),
                      request(request) {}

            bool verifySuccess() override {
                // NB: la verificación real se realiza dentro del método Session::changeBandwidth
                return false;
            }

            void doRecovery() override {
                Session *session = sessionThread.getSessionManager().getSession();
                if (session) {
                    session->sendConstrain(this->timeoutMs, request);
                }
            }

            void notifyAbort() override {
                // No se requiere ninguna acción
            }

            bool timeoutFixed() const override {
                return false;
            }

            long fixedTimeout() const override {
                return 0;
            }

            bool shouldBeSent() override {
                return true;
            }

            ConstrainRequest &getRequest() const {
                return request;
            }
        };

        /**
          * This method is called by SessionManager to notify the session that WebSocket support has been enabled again
          * because the client IP has changed. So next bind_session must try WebSocket as transport
          * (except in the case of forced transport).
          */
        virtual void restoreWebSocket() {
            if (options.ForcedTransport.empty()) {
                switchToWebSocket = true;
            } else {
                // If the transport is forced, it is either HTTP or WebSocket.
                // If it is HTTP, we must not switch to WebSocket. So the flag must remain false.
                // If it is WebSocket, the switch is useless. So the flag must remain false.
                // In either case, we don't need to change it.
            }
        }

        /**
         * Notifies about a fatal error and closes the session.
         */
        virtual void onFatalError(const std::exception& e) {
            log.Error("A fatal error has occurred. The session will be closed. Cause: " + std::string(e.what()));
            protocol.onFatalError(e);
        }

    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SESSION_HPP
