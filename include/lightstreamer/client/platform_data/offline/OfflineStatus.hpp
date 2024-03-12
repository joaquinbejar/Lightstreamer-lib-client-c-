/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 12/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_OFFLINESTATUS_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_OFFLINESTATUS_HPP

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <lightstreamer/client/platform_data/offline/CSOfflineStatus.hpp>
#include <lightstreamer/client/platform_data/offline/OfflineStatusInterface.hpp>

namespace lightstreamer::client::platform_data::offline {

    class OfflineStatus {

    private:
        static std::unique_ptr<OfflineStatusInterface> implementation;

    public:
        static void setDefault(std::unique_ptr<OfflineStatusInterface> impl) {
            if (!impl) {
                throw std::invalid_argument("Specify an implementation");
            }
            implementation = std::move(impl);
        }

        static bool isOffline(const std::string &server) {
            if (!implementation) {
                // Thread-safety should be ensured by the caller or using std::call_once
                implementation = std::make_unique<CSOfflineStatus>(); // Default to CSOfflineStatus or use a factory
                // In a real application, consider logging or handling the lack of implementation more gracefully
            }
            return implementation->isOffline(server);
        }
    };

// Static member definition
    std::unique_ptr<OfflineStatusInterface> OfflineStatus::implementation = nullptr;

} // namespace lightstreamer::client::platform_data::offline

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_OFFLINESTATUS_HPP
