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

module;

export module ILoggerProvider;
import <string>;

import ILogger;

export namespace Lightstreamer::Cpp::Logging::Log {
    class ILoggerProvider {
    public:
        virtual ~ILoggerProvider() = default;

        // Request for an ILogger instance that will be used for logging occurring on the given category.
        // It is suggested, but not mandatory, that subsequent calls to this method related to the same category return the same ILogger instance.
        virtual ILogger *GetLogger(const std::string &category) = 0;
    };

}
