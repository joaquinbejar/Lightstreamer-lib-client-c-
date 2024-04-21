/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 19/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_INTERNALCONNECTIONOPTIONS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_INTERNALCONNECTIONOPTIONS_HPP

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <format>
#include <lightstreamer/client/ClientListener.hpp>
#include <lightstreamer/client/session/RetryDelayCounter.hpp>
#include <lightstreamer/client/Constants.hpp>
#include "Logger.hpp"
#include <lightstreamer/client/events/EventDispatcher.hpp>
#include <lightstreamer/client/events/ClientListenerPropertyChangeEvent.hpp>
#include <lightstreamer/client/Proxy.hpp>
#include <lightstreamer/util/Number.hpp>
#include <lightstreamer/client/transport/providers/HttpProvider.hpp>
#include <lightstreamer/client/transport/providers/TransportFactory.hpp>

namespace lightstreamer::client::session {

    class InternalConnectionOptions {
    private:
        mutable std::mutex mutex;
        long long contentLength = 50'000'000;
        bool earlyWSOpenEnabled = false;
        long long firstRetryMaxDelay = 100;
        long long forceBindTimeout = 2000; // Not exposed
        std::string forcedTransport;
        std::map<std::string, std::string> httpExtraHeaders;

        long long idleTimeout = 19000;
        long long keepaliveInterval = 0;
        double requestedMaxBandwidth = 0;
        std::string realMaxBandwidth;
        bool unmanagedBandwidth = false;
        long long pollingInterval = 0;
        long long reconnectTimeout = 3000;
        std::unique_ptr<RetryDelayCounter> currentRetryDelay = std::make_unique<RetryDelayCounter>(4000);
        long long reverseHeartbeatInterval = 0;
        bool serverInstanceAddressIgnored = false;
        bool slowingEnabled = true;
        long long stalledTimeout = 2000;
        long long sessionRecoveryTimeout = 15000;
        long long switchCheckTimeout = 4000; // Not exposed
        std::unique_ptr<Proxy> proxy;

        std::shared_ptr<ILogger> log = LogManager::GetLogger(Constants::ACTIONS_LOG);
        std::shared_ptr<events::EventDispatcher<ClientListener>> eventDispatcher;
        std::shared_ptr<ClientListener> internalListener;

    public:
        bool httpExtraHeadersOnSessionCreationOnly = false;

        InternalConnectionOptions(std::shared_ptr<events::EventDispatcher<ClientListener>> eventDispatcher,
                                  std::shared_ptr<ClientListener> internalListener)
                : eventDispatcher(std::move(eventDispatcher)), internalListener(std::move(internalListener)) {
            if (transport::providers::TransportFactory<transport::providers::HttpProvider>::getDefaultHttpFactory()->ResponseBuffered()) {
                this->contentLength = 4'000'000;
            }
        }

        long long getCurrentConnectTimeout() {
            std::lock_guard<std::mutex> guard(this->mutex); // Asegurándonos de que 'mutex' es un miembro de la clase
            return currentRetryDelay->getCurrentRetryDelay();
        }

        long long getContentLength() const {
            return contentLength;
        }

        void setContentLength(long long newContentLength) {
            util::Number::verifyPositive(newContentLength, util::Number::DONT_ACCEPT_ZERO);
            contentLength = newContentLength;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("contentLength"));
            log->Info(std::format("Content Length value changed to {}", newContentLength));
        }

        long long getFirstRetryMaxDelay() {
            std::lock_guard<std::mutex> guard(
                    this->mutex); // Asumiendo que existe un std::mutex mutex como miembro de la clase
            return firstRetryMaxDelay;
        }

        void setFirstRetryMaxDelay(long long value) {
            std::lock_guard<std::mutex> guard(this->mutex);
            util::Number::verifyPositive(value, util::Number::DONT_ACCEPT_ZERO);
            firstRetryMaxDelay = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("firstRetryMaxDelay"));
            log->Info(std::format("First Retry Max Delay value changed to {}", value));
        }

        long long getForceBindTimeout() {
            std::lock_guard<std::mutex> guard(this->mutex);
            return forceBindTimeout;
        }

        void setForceBindTimeout(long long value) {
            std::lock_guard<std::mutex> guard(this->mutex);
            forceBindTimeout = value;
        }

        std::string getForcedTransport() {
            std::lock_guard<std::mutex> guard(this->mutex);
            return forcedTransport;
        }

        void setForcedTransport(const std::string &value) {
            std::lock_guard<std::mutex> guard(this->mutex);
            if (Constants::FORCED_TRANSPORTS.find(value) == Constants::FORCED_TRANSPORTS.end()) {
                throw std::invalid_argument(
                        "The given value is not valid. Use one of: \"HTTP-STREAMING\", \"HTTP-POLLING\", \"WS-STREAMING\", \"WS-POLLING\", \"WS\", \"HTTP\", or null");
            }
            forcedTransport = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("forcedTransport"));
            internalListener->onPropertyChange("forcedTransport");
            log->Info(std::format("Forced Transport value changed to {}", value));
        }

        std::map<std::string, std::string> getHttpExtraHeaders() {
            std::lock_guard<std::mutex> guard(mutex);
            return httpExtraHeaders;
        }

        void setHttpExtraHeaders(const std::map<std::string, std::string> &value) {
            std::lock_guard<std::mutex> guard(mutex);
            httpExtraHeaders = value;
            // Suponiendo que EventDispatcher y ClientListenerPropertyChangeEvent están definidos
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("httpExtraHeaders"));
            log->Info("Extra headers Map changed");
        }

        long long getIdleTimeout() {
            std::lock_guard<std::mutex> guard(mutex);
            return idleTimeout;
        }

        void setIdleTimeout(long long value) {
            std::lock_guard<std::mutex> guard(mutex);
            if (value < 0) throw std::invalid_argument("Value must be positive or zero.");
            idleTimeout = value;
            eventDispatcher->dispatchEvent(std::make_shared<events::ClientListenerPropertyChangeEvent>("idleTimeout"));
            log->Info(std::format("Idle Timeout value changed to {}", value));
        }

        long long getKeepaliveInterval() {
            std::lock_guard<std::mutex> guard(mutex);
            return keepaliveInterval;
        }

        void setKeepaliveInterval(long long value) {
            std::lock_guard<std::mutex> guard(mutex);
            if (value < 0) throw std::invalid_argument("Value must be positive or zero.");
            keepaliveInterval = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("keepaliveInterval"));
            log->Info(std::format("Keepalive Interval value changed to {}", value));
        }

        std::string getRequestedMaxBandwidth() {
            std::lock_guard<std::mutex> guard(mutex);
            if (requestedMaxBandwidth == 0) {
                return "unlimited";
            } else {
                return std::to_string(requestedMaxBandwidth);
            }
        }

        void setRequestedMaxBandwidth(const std::string &value) {
            std::lock_guard<std::mutex> guard(mutex);
            setMaxBandwidthInternal(value, false);
        }

        double getInternalMaxBandwidth() {
            std::lock_guard<std::mutex> guard(mutex);
            return requestedMaxBandwidth;
        }

        std::string getRealMaxBandwidth() {
            std::lock_guard<std::mutex> guard(mutex);
            return realMaxBandwidth;
        }

        void setInternalRealMaxBandwidth(const std::string &value) {
            std::lock_guard<std::mutex> guard(mutex);
            realMaxBandwidth = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("realMaxBandwidth"));
        }

        bool getBandwidthUnmanaged() {
            std::lock_guard<std::mutex> guard(mutex);
            return unmanagedBandwidth;
        }

        void setBandwidthUnmanaged(bool value) {
            std::lock_guard<std::mutex> guard(mutex);
            unmanagedBandwidth = value;
        }

        long long getPollingInterval() {
            std::lock_guard<std::mutex> guard(mutex);
            return pollingInterval;
        }

        void setPollingInterval(long long value) {
            std::lock_guard<std::mutex> guard(mutex);
            if (value < 0) throw std::invalid_argument("Value must be positive or zero.");
            pollingInterval = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("pollingInterval"));
            log->Info(std::format("Polling Interval value changed to {}", pollingInterval));
        }

        long long getReconnectTimeout() {
            std::lock_guard<std::mutex> guard(mutex);
            return reconnectTimeout;
        }

        void setReconnectTimeout(long long value) {
            std::lock_guard<std::mutex> guard(mutex);
            if (value <= 0) throw std::invalid_argument("Value must be positive and non-zero.");
            reconnectTimeout = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("reconnectTimeout"));
            log->Info(std::format("Reconnect Timeout value changed to {}", reconnectTimeout));
        }

        long long getRetryDelay() {
            std::lock_guard<std::mutex> guard(mutex);
            return currentRetryDelay->getRetryDelay();
        }

        void setRetryDelay(long long value) {
            std::lock_guard<std::mutex> guard(mutex);
            if (value <= 0) throw std::invalid_argument("Value must be positive and non-zero.");
            currentRetryDelay->reset(value);
            eventDispatcher->dispatchEvent(std::make_shared<events::ClientListenerPropertyChangeEvent>("retryDelay"));
            log->Info(std::format("Retry Delay value changed to {}", value));
        }

        long long getCurrentRetryDelay() {
            std::lock_guard<std::mutex> guard(mutex);
            return currentRetryDelay->getCurrentRetryDelay();
        }

        long long getReverseHeartbeatInterval() {
            std::lock_guard<std::mutex> guard(mutex);
            return reverseHeartbeatInterval;
        }

        void setReverseHeartbeatInterval(long long value) {
            std::lock_guard<std::mutex> guard(mutex);
            if (value < 0) throw std::invalid_argument("Value must be positive or zero.");
            reverseHeartbeatInterval = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("reverseHeartbeatInterval"));
            internalListener->onPropertyChange("reverseHeartbeatInterval");
            log->Info(std::format("Reverse Heartbeat Interval value changed to {}", reverseHeartbeatInterval));
        }

        long long getStalledTimeout() {
            std::lock_guard<std::mutex> guard(mutex);
            return stalledTimeout;
        }

        void setStalledTimeout(long long value) {
            std::lock_guard<std::mutex> guard(mutex);
            if (value <= 0) throw std::invalid_argument("Value must be positive and non-zero.");
            stalledTimeout = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("stalledTimeout"));
            log->Info(std::format("Stalled Timeout value changed to {}", stalledTimeout));
        }

        long long getSessionRecoveryTimeout() {
            std::lock_guard<std::mutex> guard(mutex);
            return sessionRecoveryTimeout;
        }

        void setSessionRecoveryTimeout(long long value) {
            std::lock_guard<std::mutex> guard(mutex);
            if (value < 0) throw std::invalid_argument("Value must be positive or zero.");
            sessionRecoveryTimeout = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("sessionRecoveryTimeout"));
            log->Info(std::format("Session Recovery Timeout value changed to {}", sessionRecoveryTimeout));
        }

        std::unique_ptr<Proxy> getProxy() {
            std::lock_guard<std::mutex> guard(mutex);
            return std::make_unique<Proxy>(*proxy);
        }

        void setProxy(std::unique_ptr<Proxy> newProxy) {
            std::lock_guard<std::mutex> guard(mutex);
            proxy = std::move(newProxy);
            eventDispatcher->dispatchEvent(std::make_shared<events::ClientListenerPropertyChangeEvent>("proxy"));
            log->Info("Proxy configuration changed.");
        }

        long long getSwitchCheckTimeout() {
            std::lock_guard<std::mutex> guard(mutex);
            return switchCheckTimeout;
        }

        void setSwitchCheckTimeout(long long value) {
            std::lock_guard<std::mutex> guard(mutex);
            switchCheckTimeout = value;
        }

        long long getTCPConnectTimeout() {
            std::lock_guard<std::mutex> guard(mutex);
            return currentRetryDelay->getCurrentRetryDelay() + 1000;
        }

        long long getTCPReadTimeout() {
            std::lock_guard<std::mutex> guard(mutex);
            return keepaliveInterval + stalledTimeout + 1000;
        }

        bool getEarlyWSOpenEnabled() {
            std::lock_guard<std::mutex> guard(mutex);
            return earlyWSOpenEnabled;
        }

        void setEarlyWSOpenEnabled(bool value) {
            std::lock_guard<std::mutex> guard(mutex);
            earlyWSOpenEnabled = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("earlyWSOpenEnabled"));
            log->Info(std::format("Early WS Open Enabled value changed to {}", value));
        }

        bool getHttpExtraHeadersOnSessionCreationOnly() {
            std::lock_guard<std::mutex> guard(mutex);
            return httpExtraHeadersOnSessionCreationOnly;
        }

        void setHttpExtraHeadersOnSessionCreationOnly(bool value) {
            std::lock_guard<std::mutex> guard(mutex);
            httpExtraHeadersOnSessionCreationOnly = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>(
                            "httpExtraHeadersOnSessionCreationOnly"));
            log->Info(std::format("Extra Headers On Session Creation Only flag changed to {}", value));
        }

        bool getServerInstanceAddressIgnored() {
            std::lock_guard<std::mutex> guard(mutex);
            return serverInstanceAddressIgnored;
        }

        void setServerInstanceAddressIgnored(bool value) {
            std::lock_guard<std::mutex> guard(mutex);
            serverInstanceAddressIgnored = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("serverInstanceAddressIgnored"));
            log->Info(std::format("Server Instance Address Ignored flag changed to {}", value));
        }

        bool getSlowingEnabled() {
            std::lock_guard<std::mutex> guard(mutex);
            return slowingEnabled;
        }

        void setSlowingEnabled(bool value) {
            std::lock_guard<std::mutex> guard(mutex);
            slowingEnabled = value;
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("slowingEnabled"));
            log->Info(std::format("Slowing Enabled flag changed to {}", value));
        }

        void increaseConnectTimeout() {
            std::lock_guard<std::mutex> guard(mutex);
            currentRetryDelay->increase();
        }

        void increaseRetryDelay() {
            std::lock_guard<std::mutex> guard(mutex);
            currentRetryDelay->increase();
        }

        void resetConnectTimeout() {
            std::lock_guard<std::mutex> guard(mutex);
            currentRetryDelay->reset(getRetryDelay());
        }

        void setMaxBandwidthInternal(const std::string &maxBandwidth, bool serverCall) {
            std::lock_guard<std::mutex> guard(mutex);
            if (maxBandwidth == Constants::UNLIMITED) {
                requestedMaxBandwidth = 0;
                log->Info("Max Bandwidth value changed to unlimited");
            } else {
                try {
                    double tmp = std::stod(maxBandwidth);
                    if (tmp < 0 || (!serverCall && tmp == 0))
                        throw std::invalid_argument("Value must be positive or zero.");
                    requestedMaxBandwidth = tmp;
                } catch (const std::invalid_argument &ia) {
                    // Aquí se maneja la conversión fallida de string a double
                    throw std::invalid_argument(
                            "The given value is not a valid value for setRequestedMaxBandwidth. Use a positive number or the string \"unlimited\"");
                }
                log->Info(std::format("Max Bandwidth value changed to {}", requestedMaxBandwidth));
            }
            eventDispatcher->dispatchEvent(
                    std::make_shared<events::ClientListenerPropertyChangeEvent>("requestedMaxBandwidth"));
        }

        long long getRetryDelay() const {
            // Implementación que retorna el valor actual de retry delay
            return currentRetryDelay->getRetryDelay();
        }

        std::shared_ptr<events::EventDispatcher<ClientListener>> getEventDispatcher() const {
            return eventDispatcher;
        }

    };
}


#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_INTERNALCONNECTIONOPTIONS_HPP
