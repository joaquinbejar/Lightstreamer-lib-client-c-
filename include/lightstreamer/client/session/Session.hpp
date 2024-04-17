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

namespace lightstreamer::client::session {

    /**
     * @brief Abstract class for managing sessions with the server.
     *
     * All calls to this class are performed through the Session Thread.
     */
    class Session {
    protected:
        static const std::string OFF;
        static const std::string CREATING;
        static const std::string CREATED;
        static const std::string FIRST_PAUSE;
        static const std::string FIRST_BINDING;
        static const std::string PAUSE;
        static const std::string BINDING;
        static const std::string RECEIVING;
        static const std::string STALLING;
        static const std::string STALLED;
        static const std::string SLEEP;

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
        std::shared_ptr<Protocol> protocol;
        bool retryAgainIfStreamFails;
        std::shared_ptr<OfflineCheck> offlineCheck;

        std::shared_ptr<RecoveryBean> recoveryBean;

        int objectId;

    public:
        Session(int objectId, bool isPolling, bool forced, std::shared_ptr<SessionListener> handler,
                std::shared_ptr<SubscriptionsListener> subscriptions, std::shared_ptr<MessagesListener> messages,
                std::shared_ptr<Session> originalSession, std::shared_ptr<SessionThread> thread,
                std::shared_ptr<Protocol> protocol, std::shared_ptr<InternalConnectionDetails> details,
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
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SESSION_HPP
