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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_INTERNALCONNECTIONDETAILS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_INTERNALCONNECTIONDETAILS_HPP
#include <string>
#include <memory>
#include <stdexcept>
#include <lightstreamer/client/events/EventDispatcher.hpp>
#include "Logger.hpp" // Assuming Logger is implemented
#include <lightstreamer/client/ClientListener.hpp>
#include <lightstreamer/client/events/ClientListenerPropertyChangeEvent.hpp>

namespace lightstreamer::client::session {

    /**
     * Manages details about the connection to the Lightstreamer server.
     */
    class InternalConnectionDetails {
    private:
        std::shared_ptr<EventDispatcher<ClientListener>> eventDispatcher;
        Logger log = Logger::getLogger("ActionsLog");

        std::string serverInstanceAddress;
        std::string serverSocketName;
        std::string clientIp;
        std::string password;
        std::string adapterSet;
        std::string serverAddress;
        std::string user;
        std::string sessionId;
        std::mutex mutex;

        static void verifyServerAddress(const std::string& serverAddress) {
            // C++ does not have a direct equivalent to C#'s Uri class for validating URLs,
            // so you might use a regular expression, a custom parser, or a third-party library.
            // This is a placeholder for URL validation logic.
            if (serverAddress.find("http://") != 0 && serverAddress.find("https://") != 0) {
                throw std::invalid_argument("The given server address has not a valid scheme");
            }
            // Further validation logic here...
        }

    public:
        InternalConnectionDetails(std::shared_ptr<EventDispatcher<ClientListener>> eventDispatcher)
        : eventDispatcher(std::move(eventDispatcher)) {}

        std::string getAdapterSet() const {
            return adapterSet;
        }

        void setAdapterSet(const std::string& value) {
            adapterSet = value;
            eventDispatcher->dispatchEvent(ClientListenerPropertyChangeEvent("adapterSet"));
            log.info("Adapter Set value changed to " + value);
        }

        std::string getServerAddress() const {
            return serverAddress;
        }

        void setServerAddress(const std::string& value) {
            std::string modifiedValue = value;
            if (!value.ends_with("/")) {
                modifiedValue += "/";
            }
            verifyServerAddress(modifiedValue); // Throws std::invalid_argument if invalid

            serverAddress = modifiedValue;
            eventDispatcher->dispatchEvent(ClientListenerPropertyChangeEvent("serverAddress"));
            log.info("Server Address value changed to " + modifiedValue);
        }

        std::string getUser() const {
            return user;
        }

        void setUser(const std::string& value) {
            user = value;
            eventDispatcher->dispatchEvent(ClientListenerPropertyChangeEvent("user"));
            log.info("User value changed to " + value);
        }

        std::string getServerInstanceAddress()  {
            std::scoped_lock lock(mutex); // TODO: Using std::scoped_lock for thread safety
            return serverInstanceAddress;
        }

        void setServerInstanceAddress(const std::string& value) {
            std::scoped_lock lock(mutex);
            serverInstanceAddress = value;
            eventDispatcher->dispatchEvent(ClientListenerPropertyChangeEvent("serverInstanceAddress"));
            log.info("Server Instance Address value changed to " + value);
        }

        std::string getServerSocketName()  {
            std::scoped_lock lock(mutex);
            return serverSocketName;
        }

        void setServerSocketName(const std::string& value) {
            std::scoped_lock lock(mutex);
            serverSocketName = value;
            eventDispatcher->dispatchEvent(ClientListenerPropertyChangeEvent("serverSocketName"));
            log.info("Server Socket Name value changed to " + value);
        }

        std::string getClientIp()  {
            std::scoped_lock lock(mutex);
            return clientIp;
        }

        void setClientIp(const std::string& value) {
            std::scoped_lock lock(mutex);
            clientIp = value;
            eventDispatcher->dispatchEvent(ClientListenerPropertyChangeEvent("clientIp"));
            log.info("Client IP value changed to " + value);
        }

        std::string getSessionId()  {
            std::scoped_lock lock(mutex);
            return sessionId;
        }

        void setSessionId(const std::string& value) {
            std::scoped_lock lock(mutex);
            sessionId = value;
            eventDispatcher->dispatchEvent(ClientListenerPropertyChangeEvent("sessionId"));
            log.info("Session ID value changed to " + value);
        }

        std::string getPassword()  {
            std::scoped_lock lock(mutex);
            return password;
        }

        void setPassword(const std::string& value) {
            std::scoped_lock lock(mutex);
            password = value;
            eventDispatcher->dispatchEvent(ClientListenerPropertyChangeEvent("password"));
            log.info("Password value changed");
        }



    };

} // namespace lightstreamer::client::session

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_INTERNALCONNECTIONDETAILS_HPP
