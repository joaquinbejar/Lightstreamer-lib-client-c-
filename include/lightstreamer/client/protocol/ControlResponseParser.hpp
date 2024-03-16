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

#ifndef LIGHTSTREAMER_LIB_CLIENT_CPP_CONTROLRESPONSEPARSER_HPP
#define LIGHTSTREAMER_LIB_CLIENT_CPP_CONTROLRESPONSEPARSER_HPP

#include <string>
#include <stdexcept>
#include <sstream> // For string parsing
#include <charconv> // For std::from_chars

namespace lightstreamer::client::protocol {

    class ParsingException : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error; // Inherit constructors
    };

    long myParseLong(const std::string &field, const std::string &description, const std::string &orig) {
        long value;
        auto result = std::from_chars(field.data(), field.data() + field.size(), value);
        if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
            throw ParsingException("Malformed " + description + " in message: " + orig);
        }
        return value;
    }

    int myParseInt(const std::string &field, const std::string &description, const std::string &orig) {
        int value;
        auto result = std::from_chars(field.data(), field.data() + field.size(), value);
        if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
            throw ParsingException("Malformed " + description + " in message: " + orig);
        }
        return value;
    }


    class ControlResponseParser {
    public:
        // Factory method to parse control response
        static ControlResponseParser *parseControlResponse(const std::string &message);

        virtual ~ControlResponseParser() = default; // Ensure proper destruction of derived classes

    protected:
        // Utility functions for parsing, assume these are defined elsewhere
        // Implemented at the end of this file for completeness
    };


    class REQOKParser : public ControlResponseParser {
        long requestId;

    public:
        REQOKParser(const std::string &message) {
            size_t reqIndex = message.find(',') + 1;
            if (reqIndex <= 0) {
                requestId = -1; // Heartbeat REQOKs have no requestId
            } else {
                requestId = myParseLong(message.substr(reqIndex), "request field", message);
            }
        }

        long getRequestId() const {
            if (requestId == -1) {
                throw std::invalid_argument("Invalid request identifier");
            }
            return requestId;
        }
    };

    class REQERRParser : public ControlResponseParser {
        long requestId;
        int errorCode;
        std::string errorMsg;

    public:
        REQERRParser(const std::string &message) {
            auto pieces = split(message, ','); // Assume split is implemented
            if (pieces.size() != 4) {
                throw std::runtime_error("Unexpected response to control request: " + message);
            }
            requestId = myParseLong(pieces[1], "request identifier", message);
            errorCode = myParseInt(pieces[2], "error code", message);
            errorMsg = unquote(pieces[3]); // Assume unquote is implemented
        }
    };

    class ERRORParser : public ControlResponseParser {


    public:
        ERRORParser(const std::string &message) {
            auto pieces = split(message, ','); // Assume split is implemented
            if (pieces.size() != 3) {
                throw std::runtime_error("Unexpected response to control request: " + message);
            }
            errorCode = myParseInt(pieces[1], "error code", message);
            errorMsg = unquote(pieces[2]); // Assume unquote is implemented
        }

        int errorCode;
        std::string errorMsg;
    };

// Implementation of the factory method
    ControlResponseParser *ControlResponseParser::parseControlResponse(const std::string &message) {
        if (message.starts_with("REQOK")) {
            return new REQOKParser(message);
        } else if (message.starts_with("REQERR")) {
            return new REQERRParser(message);
        } else if (message.starts_with("ERROR")) {
            return new ERRORParser(message);
        } else {
            throw ParsingException("Unexpected response to control request: " + message);
        }
    }

}

#endif //LIGHTSTREAMER_LIB_CLIENT_CPP_CONTROLRESPONSEPARSER_HPP
