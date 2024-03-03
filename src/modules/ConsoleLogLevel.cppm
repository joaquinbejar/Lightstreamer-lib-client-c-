/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 2/3/24
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

export module ConsoleLogLevel;
import <string>;
import <map>;
namespace Lightstreamer::Cpp::ConsoleLogLevel {
    export enum class ConsoleLogLevel {
        NONE = 0, // This level disables all logging.
        TRACE = 1, //  This level enables all logging.
        DEBUG = 2, // This level enables logging for debug, information, warnings, errors and fatal errors.
        INFO = 3, // This level enables logging for information, warnings, errors and fatal errors.
        WARN = 4, //  This level enables logging for warnings, errors and fatal errors.
        ERROR = 5, // This level enables logging for errors and fatal errors.
        FATAL = 6 // This level enables logging for fatal errors.
    };

    static const std::map<ConsoleLogLevel, std::string> ConsoleLogLevelStrings{
            {ConsoleLogLevel::NONE,     "none"},
            {ConsoleLogLevel::TRACE,    "trace"},
            {ConsoleLogLevel::DEBUG,    "debug"},
            {ConsoleLogLevel::INFO,     "info"},
            {ConsoleLogLevel::WARN,     "warn"},
            {ConsoleLogLevel::ERROR,    "error"},
            {ConsoleLogLevel::FATAL,    "fatal"}
    };

    std::string ConsoleLogLevel2String(ConsoleLogLevel level) {
        auto it = ConsoleLogLevelStrings.find(level);
        if (it != ConsoleLogLevelStrings.end()) {
            return it->second;
        } else {
            throw std::runtime_error("ConsoleLogLevel2String: Unknown log level");
        }
    }

}