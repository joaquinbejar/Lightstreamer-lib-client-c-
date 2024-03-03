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
namespace Lightstreamer::Cpp::ConsoleLogLevel {
    enum class ConsoleLogLevel {
        NONE = 0, // This level disables all logging.
        TRACE = 1, //  This level enables all logging.
        DEBUG = 2, // This level enables logging for debug, information, warnings, errors and fatal errors.
        INFO = 3, // This level enables logging for information, warnings, errors and fatal errors.
        WARN = 4, //  This level enables logging for warnings, errors and fatal errors.
        ERROR = 5, // This level enables logging for errors and fatal errors.
        FATAL = 6 // This level enables logging for fatal errors.
    };
}