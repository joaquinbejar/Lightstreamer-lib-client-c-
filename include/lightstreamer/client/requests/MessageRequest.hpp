/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 1/4/24
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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_MESSAGEREQUEST_HP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_MESSAGEREQUEST_HP
#include <string>
#include <sstream>
#include <lightstreamer/client/requests/NumberedRequest.hpp>
#include <lightstreamer/client/Constants.hpp>

namespace lightstreamer::client::requests {

    /**
     * Represents a request to send a message to the Lightstreamer Server.
     */
    class MessageRequest : public NumberedRequest {
    private:
        std::string sequence;
        std::string message;
        int number;
        bool needsProg = false; // When false, we have a fire-and-forget request.
        long timeout;
        bool hasListener;

    public:
        // Copy constructor
        MessageRequest(const MessageRequest& mr)
                : MessageRequest(mr.message, mr.sequence, mr.number, mr.timeout, mr.hasListener) {}

        // Primary constructor
        MessageRequest(const std::string& message, const std::string& sequence, int number, long timeout, bool hasListener)
                : message(message), number(number), sequence(sequence), timeout(timeout), hasListener(hasListener) {
            this->addParameter("LS_message", message);

            if (hasListener) {
                // Default behavior is LS_outcome true, only specify when false
                needsProg = true;
            } else {
                this->addParameter("LS_outcome", "false");
            }

            if (sequence != Constants::UNORDERED_MESSAGES) {
                this->addParameter("LS_sequence", sequence);
                if (timeout >= 0) {
                    this->addParameter("LS_max_wait", std::to_string(timeout));
                }
                needsProg = true;
            }
        }

        std::string getRequestName() const override {
            return "msg";
        }

        int getMessageNumber() const {
            return number;
        }

        std::string getSequence() const {
            return sequence;
        }

        bool needsAck() const {
            return needsProg;
        }

        std::string getQueryString(const std::string& defaultSessionId = "", bool includeProg = true, bool ackIsForced = false) {
            std::ostringstream query;
            query << NumberedRequest::getQueryString(defaultSessionId);
            if (includeProg) {
                query << "&LS_msg_prog=" << number;
            } else {
                if (!ackIsForced) {
                    query << "&LS_ack=false";
                }
            }
            return query.str();
        }

        std::string getTransportUnawareQueryString() const {
            return getQueryString("", needsProg, false);
        }

        std::string getTransportAwareQueryString(const std::string& defaultSessionId, bool ackIsForced) const {
            return getQueryString(defaultSessionId, needsProg, ackIsForced);
        }
    };
}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_MESSAGEREQUEST_HP
