/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 18/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONFACTORY_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONFACTORY_HPP

#include <atomic>
#include <memory>
#include <lightstreamer/client/session/Session.hpp>
#include <Logger.hpp>
#include <lightstreamer/client/transport/providers/HttpProvider.hpp>
#include  <lightstreamer/client/protocol/Protocol.hpp>
#include <lightstreamer/client/session/SessionListener.hpp>
#include <lightstreamer/client/protocol/TextProtocolHttp.hpp>
#include <lightstreamer/client/protocol/TextProtocolWS.hpp>
#include <lightstreamer/client/session/MessagesListener.hpp>

namespace lightstreamer::client::session {

    /**
     * Factory for creating new session objects.
     */
    class SessionFactory {
    private:
        static inline std::atomic<int> objectIdGenerator{0};
        Logger log = Logger::getLogger("SessionLog");

    public:
        /**
         * Creates a new session instance based on the specified parameters.
         */
        std::unique_ptr<Session> createNewSession(bool isPolling, bool isComboForced, bool isHTTP, Session *prevSession,
                                                  std::shared_ptr<SessionListener> listener,
                                                  std::shared_ptr<SubscriptionsListener> subscriptions,
                                                  std::shared_ptr<MessagesListener> messages,
                                                  std::shared_ptr<SessionThread> sessionThread,
                                                  std::shared_ptr<InternalConnectionDetails> details,
                                                  std::shared_ptr<InternalConnectionOptions> options, int handlerPhase,
                                                  bool retryAgainIfStreamFails, bool sessionRecovery) {
            int objectId = ++objectIdGenerator;

            auto httpProvider = HttpProviderFactory::getDefaultInstance(sessionThread);
            auto httpTransport = std::make_shared<Http>(sessionThread, httpProvider);

            if (isHTTP) {
                auto txt = std::make_shared<protocol::TextProtocolHttp>(objectId, sessionThread, options,
                                                                        httpTransport);
                return std::make_unique<SessionHTTP>(objectId, isPolling, isComboForced, listener, subscriptions,
                                                     messages, prevSession, sessionThread, txt, details, options,
                                                     handlerPhase, retryAgainIfStreamFails, sessionRecovery);
            } else {
                std::shared_ptr<Protocol> ws;

                try {
                    ws = std::make_shared<protocol::TextProtocolWS>(objectId, sessionThread, options, details,
                                                                    httpTransport);
                } catch (const std::exception &e) {
                    log.error("Error creating TextProtocolWS: " + std::string(e.what()));
                    ws = nullptr;
                }

                return std::make_unique<SessionWS>(objectId, isPolling, isComboForced, listener, subscriptions,
                                                   messages, prevSession, sessionThread, ws, details, options,
                                                   handlerPhase, retryAgainIfStreamFails, sessionRecovery);
            }
        }
    };

} // namespace lightstreamer::client::session

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_SESSIONFACTORY_HPP
