/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 28/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_GLOBALPROPERTIES_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_GLOBALPROPERTIES_HPP
#include <mutex>
#include <functional>

namespace lightstreamer::util {

    // Assuming RemoteCertificateValidationCallback is a std::function or similar callable object.
    // You may need to define it according to your SSL/TLS library's requirements.
    using RemoteCertificateValidationCallback = std::function<bool(/* Parameters as per your callback definition */)>;

    /**
     * Singleton class storing global properties affecting the behavior of the library.
     */
    class GlobalProperties {
    private:
        static GlobalProperties* instance;
        static std::mutex mutex;

        RemoteCertificateValidationCallback trustManagerFactory;

        // Private constructor for singleton pattern
        GlobalProperties() {}

        // Preventing copy and assignment
        GlobalProperties(const GlobalProperties&) = delete;
        GlobalProperties& operator=(const GlobalProperties&) = delete;

    public:
        /**
         * Returns the singleton instance of GlobalProperties.
         * @return A pointer to the singleton GlobalProperties instance.
         */
        static GlobalProperties* GetInstance() {
            std::lock_guard<std::mutex> lock(mutex);
            if (!instance) {
                instance = new GlobalProperties();
            }
            return instance;
        }

        /**
         * Gets the TrustManagerFactory.
         * @return The current RemoteCertificateValidationCallback instance.
         */
        RemoteCertificateValidationCallback GetTrustManagerFactory() {
            std::lock_guard<std::mutex> lock(mutex);
            return trustManagerFactory;
        }

        /**
         * Sets the TrustManagerFactory.
         * @param value The RemoteCertificateValidationCallback instance to set.
         */
        void SetTrustManagerFactory(RemoteCertificateValidationCallback value) {
            std::lock_guard<std::mutex> lock(mutex);
            trustManagerFactory = value;
        }
    };

    // Static members initialization
    GlobalProperties* GlobalProperties::instance = nullptr;
    std::mutex GlobalProperties::mutex;

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_GLOBALPROPERTIES_HPP
