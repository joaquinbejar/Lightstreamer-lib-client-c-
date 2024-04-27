/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 26/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKETPOOLMANAGER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKETPOOLMANAGER_HPP
#include <memory>
#include <utility>
#include "HttpClient.h"
#include "ExtendedNettyFullAddress.h"

namespace lightstreamer::client::transport::providers::cpp::pool {
    class WebSocketPoolManager {
    protected:
        static ILogger *log;
        AbstractChannelPoolMap <ExtendedNettyFullAddress, WebSocketChannelPool> *poolMap;

    public:
        WebSocketPoolManager(HttpPoolManager *httpPoolMap);

        IChannelPool *get(ExtendedNettyFullAddress addr);

        void Dispose();

        virtual IChannelPoolHandler *decorateChannelPoolHandler(IChannelPoolHandler *handler);

        class WebSocketChannelPoolHandler : public BaseChannelPoolHandler {
        public:
            void ChannelReleased(IChannel *ch) override;

            void ChannelAcquired(IChannel *ch) override;

            void ChannelCreated(IChannel *ch) override;
        };
    };

// WebSocketPoolManager.cpp

    ILogger *WebSocketPoolManager::log = LogManager::GetLogger(Constants::NETTY_POOL_LOG);

    WebSocketPoolManager::WebSocketPoolManager(HttpPoolManager *httpPoolMap) {
        // Initialize poolMap...
    }

    IChannelPool *WebSocketPoolManager::get(ExtendedNettyFullAddress addr) {
        return poolMap->Get(addr);
    }

    void WebSocketPoolManager::Dispose() {
        poolMap->Dispose();
    }

    IChannelPoolHandler *WebSocketPoolManager::decorateChannelPoolHandler(IChannelPoolHandler *handler) {
        return handler;
    }

    void WebSocketPoolManager::WebSocketChannelPoolHandler::ChannelReleased(IChannel *ch) {
        // ChannelReleased implementation...
    }

    void WebSocketPoolManager::WebSocketChannelPoolHandler::ChannelAcquired(IChannel *ch) {
        // ChannelAcquired implementation...
    }

    void WebSocketPoolManager::WebSocketChannelPoolHandler::ChannelCreated(IChannel *ch) {
        // ChannelCreated implementation...
    }
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_WEBSOCKETPOOLMANAGER_HPP
