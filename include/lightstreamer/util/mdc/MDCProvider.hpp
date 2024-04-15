/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 15/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_MDCPROVIDER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_MDCPROVIDER_HPP
#include <string>

namespace lightstreamer::util::mdc {

    /**
     * Abstract class representing a provider for Mapped Diagnostic Context (MDC).
     * It is essentially a key-value store used to keep context information useful for logging purposes.
     */
    class MDCProvider {
    public:
        virtual ~MDCProvider() = default;

        /**
         * Inserts or updates a key-value pair in the MDC.
         *
         * @param key The key associated with the context information.
         * @param value The value associated with the key.
         */
        virtual void put(const std::string& key, const std::string& value) = 0;

        /**
         * Retrieves a value from the MDC based on its key.
         *
         * @param key The key whose associated value is to be returned.
         * @return The value associated with the key, or an empty string if the key does not exist.
         */
        virtual std::string get(const std::string& key) = 0;

        /**
         * Removes a key-value pair from the MDC based on its key.
         *
         * @param key The key whose associated value is to be removed.
         */
        virtual void remove(const std::string& key) = 0;

        /**
         * Clears all key-value pairs stored in the MDC.
         */
        virtual void clear() = 0;
    };

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_MDCPROVIDER_HPP
