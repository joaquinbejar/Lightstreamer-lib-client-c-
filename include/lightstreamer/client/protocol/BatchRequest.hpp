/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 16/3/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_BATCHREQUEST_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_BATCHREQUEST_HPP
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <iostream> // For simplified logging purposes; replace with your logging library if needed.
#include <lightstreamer/client/requests/RequestTutor.hpp>
#include <lightstreamer/client/protocol/RequestObjects.hpp>
#include <lightstreamer/client/transport/RequestListener.hpp>

namespace lightstreamer::client::protocol {

    class BatchRequest {
    public:
        static constexpr int MESSAGE = 1;
        static constexpr int HEARTBEAT = 2;
        // static constexpr int LOG = 3; // Uncomment if needed
        static constexpr int CONTROL = 4;

    private:
        static constexpr const char* CONSTRAINT_KEY = "C";
        static constexpr const char* FORCE_REBIND_KEY = "F";
        static constexpr const char* CHANGE_SUB_KEY = "X";
        static constexpr const char* MPN_KEY = "M";

        std::unordered_map<std::string, std::shared_ptr<RequestObjects>> keys;
        std::vector<std::string> queue;

        // Simplified logger; use your actual logging system
        static void logError(const std::string& message) {
            std::cerr << "ERROR: " << message << std::endl;
        }
        static void logDebug(const std::string& message) {
            std::cout << "DEBUG: " << message << std::endl;
        }

        int batchType;
        int messageNextKey = 0;

    public:
        explicit BatchRequest(int type) : batchType(type) {}

        int getLength() const {
            return queue.size();
        }

        std::string getRequestName() const {
            if (getLength() <= 0) {
                return "";
            }
            return keys.at(queue[0])->request.getRequestName();
        }

        long getNextRequestLength() const {
            if (getLength() <= 0) {
                return 0;
            }
            return keys.at(queue[0])->request.getTransportUnawareQueryString().length();
            // TODO we use the longest estimate, as we have no transport information here
        }

        std::shared_ptr<RequestObjects> shift() {
            if (getLength() <= 0) {
                return nullptr;
            }

            std::string key = queue.front();
            queue.erase(queue.begin());
            auto k = keys[key];
            keys.erase(key);
            return k;
        }

    private:
        void addRequestInternal(const std::string& key, std::shared_ptr<RequestObjects> request) {
            keys[key] = request;
            queue.push_back(key);
        }

        void substituteRequest(const std::string& key, std::shared_ptr<RequestObjects> newRequest) {
            keys[key] = newRequest;
        }

    public:
// Implementation of addRequestToBatch methods
        bool addRequestToBatch(std::shared_ptr<MessageRequest> request, std::shared_ptr<requests::RequestTutor> tutor, std::shared_ptr<transport::RequestListener> listener) {
            if (batchType != MESSAGE) {
                logError("Unexpected request type was given to batch");
                return false;
            }

            auto message = std::make_shared<RequestObjects>(request, tutor, listener);
            addRequestInternal(std::to_string(messageNextKey++), message);
            return true;
        }

        bool addRequestToBatch(std::shared_ptr<ReverseHeartbeatRequest> request, std::shared_ptr<requests::RequestTutor> tutor, std::shared_ptr<transport::RequestListener> listener) {
            if (batchType != HEARTBEAT) {
                logError("Unexpected request type was given to batch");
                return false;
            }

            auto hb = std::make_shared<RequestObjects>(request, tutor, listener);
            addRequestInternal(std::to_string(messageNextKey++), hb);
            return true;
        }

        bool addRequestToBatch(std::shared_ptr<ConstrainRequest> request, std::shared_ptr<requests::RequestTutor> tutor, std::shared_ptr<transport::RequestListener> listener) {
            if (batchType != CONTROL) {
                logError("Unexpected request type was given to batch");
                return false;
            }

            std::string key = CONSTRAINT_KEY;

            auto requestObj = std::make_shared<RequestObjects>(request, tutor, listener);
            auto it = keys.find(key);
            if (it != keys.end()) {
                it->second->tutor->notifyAbort();
                substituteRequest(key, requestObj);
            } else {
                addRequestInternal(key, requestObj);
            }

            return true;
        }

        bool addRequestToBatch(std::shared_ptr<ForceRebindRequest> request, std::shared_ptr<requests::RequestTutor> tutor, std::shared_ptr<transport::RequestListener> listener) {
            if (batchType != CONTROL) {
                logError("Unexpected request type was given to batch");
                return false;
            }

            std::string key = FORCE_REBIND_KEY;

            auto requestObj = std::make_shared<RequestObjects>(request, tutor, listener);
            auto it = keys.find(key);
            if (it != keys.end()) {
                it->second->tutor->notifyAbort();
                substituteRequest(key, requestObj);
            } else {
                addRequestInternal(key, requestObj);
            }

            return true;
        }

        bool addRequestToBatch(std::shared_ptr<UnsubscribeRequest> request, std::shared_ptr<requests::RequestTutor> tutor, std::shared_ptr<transport::RequestListener> listener) {
            if (batchType != CONTROL) {
                logError("Unexpected request type was given to batch");
                return false;
            }

            std::string key = std::to_string(request->getSubscriptionId());

            auto requestObj = std::make_shared<RequestObjects>(request, tutor, listener);
            auto it = keys.find(key);
            if (it != keys.end()) {
                // Assuming SubscribeRequest can be checked through dynamic_pointer_cast or similar mechanism
                logDebug("Substituting SUBSCRIBE request with UNSUBSCRIBE");
                it->second->tutor->notifyAbort();
                substituteRequest(key, requestObj);
            } else {
                addRequestInternal(key, requestObj);
            }

            return true;
        }

        bool addRequestToBatch(std::shared_ptr<SubscribeRequest> request, std::shared_ptr<requests::RequestTutor> tutor, std::shared_ptr<transport::RequestListener> listener) {
            if (batchType != CONTROL) {
                logError("Unexpected request type was given to batch");
                return false;
            }

            std::string key = std::to_string(request->getSubscriptionId());

            auto requestObj = std::make_shared<RequestObjects>(request, tutor, listener);
            auto it = keys.find(key);
            if (it != keys.end()) {
                logDebug("Handling unexpected case for SUBSCRIBE request");
                it->second->tutor->notifyAbort();
                substituteRequest(key, requestObj);
            } else {
                addRequestInternal(key, requestObj);
            }

            return true;
        }

        bool addRequestToBatch(std::shared_ptr<ChangeSubscriptionRequest> request, std::shared_ptr<requests::RequestTutor> tutor, std::shared_ptr<transport::RequestListener> listener) {
            if (batchType != CONTROL) {
                logError("Unexpected request type was given to batch");
                return false;
            }

            std::string key = CHANGE_SUB_KEY + std::to_string(request->getSubscriptionId());

            auto requestObj = std::make_shared<RequestObjects>(request, tutor, listener);
            auto it = keys.find(key);
            if (it != keys.end()) {
                logDebug("Replacing old CHANGE SUBSCRIPTION request");
                it->second->tutor->notifyAbort();
                substituteRequest(key, requestObj);
            } else {
                addRequestInternal(key, requestObj);
            }

            return true;
        }

        bool addRequestToBatch(std::shared_ptr<DestroyRequest> request, std::shared_ptr<requests::RequestTutor> tutor, std::shared_ptr<transport::RequestListener> listener) {
            if (batchType != CONTROL) {
                logError("Unexpected request type was given to batch");
                return false;
            }

            std::string key = request->getSession();

            auto requestObj = std::make_shared<RequestObjects>(request, tutor, listener);
            auto it = keys.find(key);
            if (it != keys.end()) {
                logDebug("Substituting DESTROY request");
                it->second->tutor->notifyAbort();
                substituteRequest(key, requestObj);
            } else {
                addRequestInternal(key, requestObj);
            }

            return true;
        }



    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_BATCHREQUEST_HPP
