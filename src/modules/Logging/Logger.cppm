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

export module ILogger;
import <string>;

export namespace Lightstreamer::Cpp::Logging::Log {
    class ILogger {
    public:
        virtual ~ILogger() = default;
        virtual void Error(const std::string& message) = 0;
        virtual void Warn(const std::string& message) = 0;
        virtual void Info(const std::string& message) = 0;
        virtual void Debug(const std::string& message) = 0;
        virtual void Fatal(const std::string& message) = 0;

        virtual bool IsDebugEnabled() const = 0;
        virtual bool IsInfoEnabled() const = 0;
        virtual bool IsWarnEnabled() const = 0;
        virtual bool IsErrorEnabled() const = 0;
        virtual bool IsFatalEnabled() const = 0;
    };
}
