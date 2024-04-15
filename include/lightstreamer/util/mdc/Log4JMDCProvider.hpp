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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_LOG4JMDCPROVIDER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_LOG4JMDCPROVIDER_HPP
#include <lightstreamer/util/mdc/MDCProvider.hpp>
#include <lightstreamer/util/mdc/MDC.hpp>
#include <string>

namespace lightstreamer::util::mdc {
    
    /**
     * @class Log4JMDCProvider
     * @brief A MDC provider relying on a logging framework similar to Log4J's MDC mechanism.
     *
     * This class is an adapter over a hypothetical C++ logging framework's MDC functionality.
     */
    class Log4JMDCProvider : public MDCProvider {
    public:
        void put(const std::string& key, const std::string& value) override {
            // Hypothetical usage of a global MDC-like storage in the logging framework
            MCD::put(key, value);
        }

        std::string get(const std::string& key) override {
            // Hypothetical usage of a global MDC-like storage in the logging framework
            return MCD::get(key);
        }

        void remove(const std::string& key) override {
            // Hypothetical usage of a global MDC-like storage in the logging framework
            MCD::remove(key);
        }

        void clear() override {
            // Hypothetical usage of a global MDC-like storage in the logging framework
            MCD::clear();
        }
    };
    
}


#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_LOG4JMDCPROVIDER_HPP
