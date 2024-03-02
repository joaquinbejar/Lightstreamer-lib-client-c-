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

export module ILog;

import <string>;
import ILogger;
import ILoggerProvider;

export namespace Lightstreamer::Cpp::Logging::Log {

    class ILog : ILogger {
    public:


    };

    class ILogEmpty : ILogger {
    public:
        void Error(const std::string &message) override {}

        void Error(const std::string &message, const std::exception &e) override {}

        void Warn(const std::string &message) override {}

        void Warn(const std::string &message, const std::exception &e) override {}

        void Info(const std::string &message) override {}

        void Info(const std::string &message, const std::exception &e) override {}

        void Debug(const std::string &message) override {}

        void Debug(const std::string &message, const std::exception &e) override {}

        void Fatal(const std::string &message) override {}

        void Fatal(const std::string &message, const std::exception &e) override {}

        bool IsDebugEnabled() const override {
            return false;
        }

        bool IsInfoEnabled() const override {
            return false;
        }

        bool IsWarnEnabled() const override {
            return false;
        }

        bool IsErrorEnabled() const override {
            return false;
        }

        bool IsFatalEnabled() const override {
            return false;
        }
    };

}