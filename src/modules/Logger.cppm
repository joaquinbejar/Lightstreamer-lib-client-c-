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

export module Logger;
import <string>;

export namespace Lightstreamer::Cpp::Logger {
    class Logger {
    public:
        virtual ~Logger() = default;
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

    class ILogEmpty : Logger {
    public:
        void Error(const std::string &message) override {}

        void Error(const std::string &message, const std::exception &e)  {}

        void Warn(const std::string &message) override {}

        void Warn(const std::string &message, const std::exception &e)  {}

        void Info(const std::string &message) override {}

        void Info(const std::string &message, const std::exception &e)  {}

        void Debug(const std::string &message) override {}

        void Debug(const std::string &message, const std::exception &e)  {}

        void Fatal(const std::string &message) override {}

        void Fatal(const std::string &message, const std::exception &e)  {}

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

    class ILog : Logger {

    public:

        ILog() = default;

        void Error(const std::string &message) override {
        }

        void Warn(const std::string &message) override {
        }

        void Info(const std::string &message) override {
        }

        void Debug(const std::string &message) override {
        }

        void Fatal(const std::string &message) override {
        }
    };

    class LoggerProvider {
    public:
        virtual ~LoggerProvider() = default;

        // Request for a Logger instance that will be used for logging occurring on the given category.
        // It is suggested, but not mandatory, that subsequent calls to this method related to the same category return the same Logger instance.
        virtual Logger *GetLogger(const std::string &category) = 0;
    };



}
