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
import <iostream>;
import ConsoleLogLevel;

export namespace Lightstreamer::Cpp::Logger {

    std::string FormatMessageWithException(const std::string &message, const std::exception &e) {
        return std::format("{} With Exception: {}", message, e.what());
    }

    /**
	Interface to be implemented to consume log from the library.

    Instances of implemented classes are obtained by the library through the LoggerProvider instance set on \ref `.LightstreamerClient.setLoggerProvider`.
    **/
    class Logger {
    public:
        virtual ~Logger() = default;

        /**
		Receives log messages at Error level.

        @param line: The message to be logged.

        @param exception: An Exception instance related to the current log message.
        **/
        virtual void Error(const std::string &message) = 0;

        /**
		Receives log messages at Warn level and a related exception.

        @param line: The message to be logged.

        @param exception: An Exception instance related to the current log message.
        **/
        virtual void Warn(const std::string &message) = 0;

        /**
		Receives log messages at Trace level and a related exception.

        @param line: The message to be logged.

        @param exception: An Exception instance related to the current log message.
        **/
        virtual void Trace(const std::string &message) = 0;

        /**
		Receives log messages at Info level and a related exception.

        @param line: The message to be logged.

        @param exception: An Exception instance related to the current log message.
        **/
        virtual void Info(const std::string &message) = 0;

        /**
		Receives log messages at Debug level and a related exception.

        @param line: The message to be logged.

        @param exception: An Exception instance related to the current log message.
        **/
        virtual void Debug(const std::string &message) = 0;

        /**
		Receives log messages at Fatal level and a related exception.

        @param line: The message to be logged.

        @param exception: An Exception instance related to the current log message.
        **/
        virtual void Fatal(const std::string &message) = 0;

        /**
		Checks if this logger is enabled for the Debug level.

        The property should be true if this logger is enabled for Debug events, false otherwise.

        This property is intended to lessen the computational cost of disabled log Debug statements. Note
        that even if the property is false, Debug log lines may be received anyway by the Debug methods.
        **/
        virtual bool IsDebugEnabled() const = 0;

        /**
		Checks if this logger is enabled for the Info level.

        The property should be true if this logger is enabled for Info events, false otherwise.

        This property is intended to lessen the computational cost of disabled log Info statements. Note
                that even if the property is false, Info log lines may be received anyway by the Info methods.
        **/
        virtual bool IsInfoEnabled() const = 0;

        /**
		Checks if this logger is enabled for the Warn level.

        The property should be true if this logger is enabled for Warn events, false otherwise.

        This property is intended to lessen the computational cost of disabled log Warn statements. Note
                that even if the property is false, Warn log lines may be received anyway by the Warn methods.
        **/
        virtual bool IsWarnEnabled() const = 0;

        /**
		Checks if this logger is enabled for the Error level.

        The property should be true if this logger is enabled for Error events, false otherwise.

        This property is intended to lessen the computational cost of disabled log Error statements. Note
                that even if the property is false, Error log lines may be received anyway by the Error methods.
        **/
        virtual bool IsErrorEnabled() const = 0;

        /**
		Checks if this logger is enabled for the Fatal level.

        The property should be true if this logger is enabled for Fatal events, false otherwise.

        This property is intended to lessen the computational cost of disabled log Fatal statements. Note
                that even if the property is false, Fatal log lines may be received anyway by the Fatal methods.
        **/
        virtual bool IsFatalEnabled() const = 0;

        /**
		Checks if this logger is enabled for the Trace level.

        The property should be true if this logger is enabled for Trace events, false otherwise.

        This property is intended to lessen the computational cost of disabled log Trace statements. Note
                that even if the property is false, Trace log lines may be received anyway by the Trace methods.
        **/
        virtual bool IsTraceEnabled() const = 0;
    };

    class ILogEmpty : Logger {
    public:
        void Error(const std::string &message) override {}

        void Error(const std::string &message, const std::exception &e) {}

        void Warn(const std::string &message) override {}

        void Warn(const std::string &message, const std::exception &e) {}

        void Info(const std::string &message) override {}

        void Info(const std::string &message, const std::exception &e) {}

        void Debug(const std::string &message) override {}

        void Debug(const std::string &message, const std::exception &e) {}

        void Fatal(const std::string &message) override {}

        void Fatal(const std::string &message, const std::exception &e) {}

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

    export class ConsoleLogger : public Logger {
    private:
        std::mutex m_mutex;
        std::atomic<bool> last_was_flush = false;

        ConsoleLogLevel level = ConsoleLogLevel::NONE;
        std::string category;
        bool traceEnabled;
        bool debugEnabled;
        bool infoEnabled;
        bool warnEnabled;
        bool errorEnabled;
        bool fatalEnabled;

        void m_safe_cout(const std::string &s, bool flush, std::iostream &out) {
            std::string ss = s;
            if (flush) {
                ss = "\r" + s;
            } else if (last_was_flush.load(std::memory_order_seq_cst)) {
                ss = "\n" + s;
            }

            if (!flush) {
                ss = ss + "\n";
            }

            std::unique_lock <std::mutex> lock(m_mutex);
            out << ss;
            last_was_flush.store(flush, std::memory_order_seq_cst);
            out.flush();
            lock.unlock();

        }

        void m_log(const std::string &s, bool flush, std::iostream &out) {
            std::time_t t = std::time(nullptr);
            std::tm tm = *std::localtime(&t);
            std::ostringstream time_prefix;
            time_prefix << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << " " << s;
            m_safe_cout(time_prefix.str(), flush, out);
        }


    public:
        ConsoleLogger(ConsoleLogLevel level, const std::string &category) : level(level), category(category) {
            traceEnabled = level <= ConsoleLogLevel::TRACE;
            debugEnabled = level <= ConsoleLogLevel::DEBUG;
            infoEnabled = level <= ConsoleLogLevel::INFO;
            warnEnabled = level <= ConsoleLogLevel::WARN;
            errorEnabled = level <= ConsoleLogLevel::ERROR;
            fatalEnabled = level <= ConsoleLogLevel::FATAL;
        }


        void Error(const std::string &message) override {
            m_log(message, false, std::cerr);
        }

        void Error(const std::string &message, const std::exception &e) override {
            m_log(FormatMessageWithException(message, e), false, std::cerr);
        }

        void Warn(const std::string &message) override {
            m_log(message, false, std::cerr);
        }

        void Warn(const std::string &message, const std::exception &e) override {
            m_log(FormatMessageWithException(message, e), false, std::cerr);
        }

        void Info(const std::string &message) override {
            m_log(message, false, std::cout);
        }

        void Info(const std::string &message, const std::exception &e) override {
            m_log(FormatMessageWithException(message, e), false, std::cout);
        }

        void Debug(const std::string &message) override {
            m_log(message, false, std::cout);
        }

        void Debug(const std::string &message, const std::exception &e) override {
            m_log(FormatMessageWithException(message, e), false, std::cout);
        }

        void Fatal(const std::string &message) override {
            m_log(message, false, std::cerr);
        }

        void Fatal(const std::string &message, const std::exception &e) override {
            m_log(FormatMessageWithException(message, e), false, std::cerr);
        }

        void Trace(const std::string &message) override {
            m_log(message, false, std::cout);
        }

        void Trace(const std::string &message, const std::exception &e) override {
            m_log(FormatMessageWithException(message, e), false, std::cout);
        }


        bool IsDebugEnabled() const override {
            return debugEnabled;
        }

        bool IsInfoEnabled() const override {
            return infoEnabled;
        }

        bool IsWarnEnabled() const override {
            return warnEnabled;
        }

        bool IsErrorEnabled() const override {
            return errorEnabled;
        }

        bool IsFatalEnabled() const override {
            return fatalEnabled;
        }

        bool IsTraceEnabled() const override {
            return traceEnabled;
        }


    };


    class LoggerProvider {
    public:
        virtual ~LoggerProvider() = default;

        /**
        Request for a Logger instance that will be used for logging occurring on the given category.
        It is suggested, but not mandatory, that subsequent calls to this method related to the same category
        @return the same Logger instance.
         **/
        virtual Logger *GetLogger(const std::string &category) = 0;
    };


}
