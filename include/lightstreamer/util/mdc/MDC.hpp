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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_MDC_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_MDC_HPP
#include "MDCProvider.hpp"
#include <memory>
#include <iostream>

namespace lightstreamer::util::mdc {

    /**
     * @class MDC
     * @brief The Mapped Diagnostic Context stores the context of the application and makes it available to the configured loggers.
     *
     * The MDC must be manually enabled by setting the system property "com.lightstreamer.logging.mdc".
     *
     * @note
     * - <b>NB 1:</b> The current implementation relies on Log4J facilities, so it is available only if the logger provider
     *   (set by `LogManager::setLoggerProvider(com::lightstreamer::log::LoggerProvider)`) is `Log4jWrapper`.
     * - <b>NB 2:</b> Since an MDC provider is not mandatory, it is better to guard each method call with the check `MDC::isEnabled()`.
     *
     * Example usage:
     * @code
     * if (MDC::isEnabled()) {
     *     MDC::put("key", "value");
     * }
     * @endcode
     */
    class MDC {
    private:
        static std::unique_ptr<MDCProvider> provider;

    public:
        // Static initializer for the MDC, which checks system properties or configuration to enable MDC
        MDC() {
            // Assuming a configuration or system property check needs to be implemented
            // For demonstration, we're always enabling MDC here
            //  implementation of MDCProvider
        }

        // Checks if the MDC is enabled
        static bool isEnabled() {
            return provider != nullptr;
        }

        // Puts a key-value pair into the MDC
        static void put(const std::string& key, const std::string& value) {
            if (isEnabled()) {
                provider->put(key, value);
            } else {
                std::cerr << "MDC is not enabled." << std::endl;
            }
        }

        // Retrieves a value by key from the MDC
        static std::string get(const std::string& key) {
            if (isEnabled()) {
                return provider->get(key);
            }
            return "";
        }

        // Removes a key-value pair from the MDC by key
        static void remove(const std::string& key) {
            if (isEnabled()) {
                provider->remove(key);
            }
        }

        // Clears all key-value pairs from the MDC
        static void clear() {
            if (isEnabled()) {
                provider->clear();
            }
        }
    };

    // Define the static provider
    std::unique_ptr<MDCProvider> MDC::provider = nullptr;
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_MDC_HPP
