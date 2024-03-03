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
import <format>;
import <mutex>;
import <memory>;
import <atomic>;
import <exception>;
import <iomanip>;
import <ctime>;
import <sstream>;
import <iostream>;
import <utility>;

import ConsoleLogLevel;

namespace Lightstreamer::Cpp::Logger {
    using ConsoleLogLevel::ConsoleLogLevel;

    std::string FormatMessageWithException(const std::string &message, const std::exception &e) {
         std::ostringstream oss;
         oss << message << " With Exception: " << e.what();
         return oss.str();
     }


    /**
	Interface to be implemented to consume log from the library.

    Instances of implemented classes are obtained by the library through the LoggerProvider instance set on \ref `.LightstreamerClient.setLoggerProvider`.
    **/
    class Logger {
        /**
         * @brief A mutex object to ensure exclusive access to shared data.
         *
         * This class provides a mechanism for controlling access to shared data by multiple threads.
         * It allows only one thread at a time to execute a certain block of code, ensuring that
         * the data accessed by that code remains consistent and avoids race conditions.
         */
        std::mutex m_mutex;
        /**
         * @brief Boolean flag indicating if the last log message was flushed.
         *
         * This flag is used in the ConsoleLogger class to determine whether to flush the output stream after writing a log message. It is an atomic flag that can be accessed from multiple threads concurrently.
         */
        std::atomic<bool> last_was_flush = false;
                /**
         * Prints a string to the specified output stream with thread-safe handling of flushing.
         *
         * @param s The string to be printed.
         * @param flush If true, indicates that the output should be immediately flushed. If false, indicates that the output should not be flushed.
         * @param out The output stream to which the string should be printed.
         *
         * If flush is true, the string will be printed with a carriage return ("\r") prepended to it.
         * If flush is false and the last call to this method was with flush as true, the string will be printed with a newline ("\n") prepended to it.
         * If flush is false and the last call to this method was with flush as false, the string will be printed as is.
         *
         * After printing the string, the method will flush the output stream and unlock the internal mutex used for thread-safety.
         *
         * Note: This method assumes that the caller has already acquired the necessary lock on the cout stream to avoid inconsistent output.
         */
        void m_safe_cout(const std::string &s, bool flush, std::ostream &out) {
            std::string ss = s;
            if (flush) {
                ss = "\r" + s;
            } else if (last_was_flush.load(std::memory_order_seq_cst)) {
                ss = "\n" + s;
            }

            if (!flush) {
                ss = ss + "\n";
            }

            std::unique_lock<std::mutex> lock(m_mutex);
            out << ss;
            last_was_flush.store(flush, std::memory_order_seq_cst);
            out.flush();
            lock.unlock();
        }

    protected:
        /**
         * Logs a message with a timestamp prefix. The message can be flushed immediately to the output stream if desired.
         *
         * @param s The message to log.
         * @param flush Determines whether to flush the output stream after logging the message.
         * @param out The output stream where the message should be logged.
         */
        void m_log(const std::string &s, bool flush, std::ostream &out) {
            std::time_t t = std::time(nullptr);
            std::tm tm = *std::localtime(&t);
            std::ostringstream time_prefix;
            time_prefix << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << " " << s;
            m_safe_cout(time_prefix.str(), flush, out);
        }
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


    export
    /**
     * @class ConsoleLogger
     * @brief Implementation of the Logger interface for logging to the console.
     *
     * The ConsoleLogger class provides a way to log messages to the console. It
     * implements the Logger interface and can be used by the library to consume
     * log messages.
     */
    class ConsoleLogger : public Logger {
    private:
        /**
         \brief Shared pointer to an instance of ConsoleLogger.

         This static variable is used to store a shared pointer to an instance of the ConsoleLogger class.
         The shared pointer allows the instance to be safely shared and managed by multiple objects.
         The ConsoleLogger is a class that implements the Logger interface, which is used to consume log from the library.
         Instances of the ConsoleLogger class can be obtained through the LoggerProvider instance set on the LightstreamerClient object.

         \see ConsoleLogger, Logger, LightstreamerClient, LoggerProvider
         **/
        static std::shared_ptr<ConsoleLogger> instance;
        /**
         * @brief A mutex variable used for implementing a singleton pattern.
         *
         * This static variable is used to provide thread-safe access to a singleton instance.
         * It ensures that only one thread can execute the critical section of the code at a time,
         * preventing multiple instances of the singleton class from being created.
         *
         * @details The std::mutex class provides a non-recursive mutex that can be locked and
         * unlocked by different threads.
         *
         * @note This mutex variable should be used along with a singleton design pattern to provide
         * thread-safe access to a single instance of a class.
         *
         * @see http://en.cppreference.com/w/cpp/thread/mutex
         * @see https://en.wikipedia.org/wiki/Singleton_pattern
         */
        static std::mutex singleton_mutex;

        /**
         * The `ConsoleLogLevel` enum class represents the different levels of logging for the console logger.
         *
         * The possible values are:
         * - `ConsoleLogLevel::NONE`: This level disables all logging.
         * - `ConsoleLogLevel::TRACE`: This level enables all logging.
         * - `ConsoleLogLevel::DEBUG`: This level enables logging for debug, information, warnings, errors, and fatal errors.
         * - `ConsoleLogLevel::INFO`: This level enables logging for information, warnings, errors, and fatal errors.
         * - `ConsoleLogLevel::WARN`: This level enables logging for warnings, errors, and fatal errors.
         * - `ConsoleLogLevel::ERROR`: This level enables logging for errors and fatal errors.
         * - `ConsoleLogLevel::FATAL`: This level enables logging for fatal errors.
         */
        ConsoleLogLevel level = ConsoleLogLevel::NONE;
        /**
         This variable represents the category for logging purposes.
         It is of type `std::string` and is used by the `ConsoleLogger` class for categorizing log messages.
         **/
        std::string category;
        /**
         * \brief Flag indicating whether trace logging is enabled.
         */
        bool traceEnabled;
        /**
         * @brief Indicates whether debug logs are enabled.
         *
         * If this variable is set to true, it means that debug logs are enabled.
         * Debug logs provide detailed information that helps in identifying and debugging issues.
         *
         * You can use the value of this variable to conditionally execute code blocks that generate debug logs,
         * by checking its value before logging.
         *
         * Note that the value of this variable may be modified dynamically at runtime, depending on the
         * configuration or other factors.
         */
        bool debugEnabled;
        /**
         * @brief Boolean flag indicating whether the logging of informational messages is enabled.
         *
         * This flag is used to determine whether informational log messages should be printed or not.
         * Set this flag to true to enable logging of informational messages, and false to disable it.
         *
         * @see Logger
         * @see LoggerProvider
         * @see LightstreamerClient.setLoggerProvider
         */
        bool infoEnabled;
        /**
         @brief A boolean variable indicating whether warning messages are enabled or not.

         This variable can be used to enable or disable warning messages in the code.

         @note Enabling warning messages can provide additional debug information, but may also impact performance.
         */
        bool warnEnabled;
        /**
         * @brief Flag indicating whether error logging is enabled.
         * @details If this flag is set to true, error messages will be logged. If it is set to false, error logging will be disabled.
         *
         * @see Logger
         * @see LoggerProvider
         * @see `.LightstreamerClient.setLoggerProvider`
         */
        bool errorEnabled;
        /**
         * Indicates whether fatal logging is enabled.
         *
         * This variable is a boolean flag that indicates whether the logging of fatal-level messages is enabled or disabled.
         *
         * Logging is enabled if the value of this variable is true, and disabled if the value is false.
         *
         * Fatal-level messages typically represent critical errors or unexpected conditions that cause the program to terminate.
         *
         * Developers can use this flag to control the logging behavior at the fatal level.
         *
         * @see Logger
         * @see LoggerProvider
         * @see `.LightstreamerClient.setLoggerProvider`
         */
        bool fatalEnabled;

        /**
         * @brief ConsoleLogger constructor.
         *
         * Initializes a new instance of the ConsoleLogger class with the specified log level and category.
         *
         * @param level The log level for this logger instance.
         * @param category The category for this logger instance.
         */
        ConsoleLogger(ConsoleLogLevel level, const std::string &category) : level(level), category(category) {
            traceEnabled = level <= ConsoleLogLevel::TRACE;
            debugEnabled = level <= ConsoleLogLevel::DEBUG;
            infoEnabled = level <= ConsoleLogLevel::INFO;
            warnEnabled = level <= ConsoleLogLevel::WARN;
            errorEnabled = level <= ConsoleLogLevel::ERROR;
            fatalEnabled = level <= ConsoleLogLevel::FATAL;
        }

    public:
        /**
         @class ConsoleLogger
         @brief This class provides logging functionality to the console.

         Instances of this class are used to log debug, information, warning, and error messages to the console.

         @sa LoggerProvider, LightstreamerClient

         @param other The instance of ConsoleLogger to be copied. This parameter is not used and should be ignored.
         @return This method does not return a value.
         */
        ConsoleLogger(const ConsoleLogger &) = delete;

        /**
         * @brief Assignment operator.
         *
         * This assignment operator is explicitly deleted for the `ConsoleLogger` class, preventing copying of `ConsoleLogger` objects.
         *
         * @param other The `ConsoleLogger` object to be assigned.
         * @return Returns a reference to the assigned `ConsoleLogger` object.
         */
        ConsoleLogger &operator=(const ConsoleLogger &) = delete;

        /**
         * \brief ConsoleLogger class
         *
         * The ConsoleLogger class provides logging functionality to the console.
         */
        ConsoleLogger(ConsoleLogger &&) = delete;

        /**
         * @brief Assignment operator.
         *
         * This assignment operator is disabled by using the `delete` keyword. This means that it is not possible to assign a `ConsoleLogger` object to another `ConsoleLogger` object using the move assignment operator.
         *
         * @param other The rvalue reference to another `ConsoleLogger` object.
         *
         * @return The reference to the modified `ConsoleLogger` object.
         */
        ConsoleLogger &operator=(ConsoleLogger &&) = delete;


        /**
         * Returns an instance of ConsoleLogger with the specified log level and category.
         *
         * @param level The log level to be set for the ConsoleLogger instance.
         * @param category The log category to be set for the ConsoleLogger instance.
         *
         * @return A shared pointer to the ConsoleLogger instance.
         */
        static std::shared_ptr<ConsoleLogger> getInstance(ConsoleLogLevel level, const std::string &category) {
            std::lock_guard<std::mutex> lock(singleton_mutex);
            if (!instance) {
                instance = std::shared_ptr<ConsoleLogger>(new ConsoleLogger(level, category));
            }
            return instance;
        }


        /**
         * @brief Handles and logs an error message.
         *
         * This method is called when an error occurs and needs to be handled and logged.
         *
         * @param message The error message to be logged.
         */
        void Error(const std::string &message) override {
            m_log(message, false, std::cerr);
        }

        /**
         * Logs an error message with an exception.
         *
         * @param message The error message to be logged.
         * @param e The exception associated with the error.
         */
        void Error(const std::string &message, const std::exception &e) {
            m_log(FormatMessageWithException(message, e), false, std::cerr);
        }

        /**
         * Logs a warning message.
         *
         * @param message The warning message to be logged.
         */
        void Warn(const std::string &message) override {
            m_log(message, false, std::cerr);
        }

        /**
         * Logs a warning message with the given message and exception.
         *
         * @param message The warning message to be logged.
         * @param e The exception to be logged.
         */
        void Warn(const std::string &message, const std::exception &e) {
            m_log(FormatMessageWithException(message, e), false, std::cerr);
        }

        /**
         * @brief Writes an informational message to the log.
         *
         * This method writes the given message to the log as an informational message.
         *
         * @param message The message to be written to the log.
         */
        void Info(const std::string &message) override {
            m_log(message, false, std::cout);
        }

        /**
         * Logs an informational message.
         *
         * @param message The message to be logged.
         * @param e The exception to be logged along with the message.
         */
        void Info(const std::string &message, const std::exception &e) {
            m_log(FormatMessageWithException(message, e), false, std::cout);
        }

        /**
         * \brief Outputs the debug message to the console.
         *
         * \param message The debug message to be outputted.
         *
         * This function is called to output the debug message to the console. The debug message is received as a parameter
         * and is then passed to the `m_log` function along with `false` and `std::cout` as arguments to indicate that
         * it is a debug message and the output stream to use is `std::cout`.
         */
        void Debug(const std::string &message) override {
            m_log(message, false, std::cout);
        }

        /**
         @brief Logs a debug message with an exception.

         This method logs a debug message along with an exception. The debug message should provide additional information about the exception.

         @param message The debug message to log.
         @param e The exception to log.

         @see ConsoleLogger
         @see FormatMessageWithException
         @see m_log

         @note This method is a member of the `ConsoleLogger` class.
         */
        void Debug(const std::string &message, const std::exception &e) {
            m_log(FormatMessageWithException(message, e), false, std::cout);
        }

        /**
         * @brief Logs a fatal message with the given message.
         *
         * This method is called to log a fatal message. The message is sent to the logger specified in the `m_log` member variable.
         *
         * @param message The message to be logged.
         */
        void Fatal(const std::string &message) override {
            m_log(message, false, std::cerr);
        }

        /**
         * @brief Logs a fatal error message with an exception.
         *
         * This method logs a fatal error message with an exception to the configured logger.
         *
         * @param message The error message to log.
         * @param e The exception associated with the error.
         *
         * @see setLoggerProvider()
         */
        void Fatal(const std::string &message, const std::exception &e) {
            m_log(FormatMessageWithException(message, e), false, std::cerr);
        }

        /**
         * Trace method sends the given message to the logging system.
         *
         * @param message The message to be logged.
         */
        void Trace(const std::string &message) override {
            m_log(message, false, std::cout);
        }

        /**
         * Logs a trace message with an exception.
         *
         * @param message The message to be logged.
         * @param e The exception to be logged.
         */
        void Trace(const std::string &message, const std::exception &e) {
            m_log(FormatMessageWithException(message, e), false, std::cout);
        }


        /**
         * Checks if debug mode is enabled.
         *
         * @return true if debug mode is enabled, false otherwise.
         */
        bool IsDebugEnabled() const override {
            return debugEnabled;
        }

        /**
         * @brief Checks if the information level logging is enabled.
         *
         * This method returns a boolean value indicating whether the information level logging is enabled or not.
         *
         * @return True if the information level logging is enabled, false otherwise.
         */
        bool IsInfoEnabled() const override {
            return infoEnabled;
        }

        /**
         * @brief Check if warning level log is enabled.
         *
         * @return True if warning level log is enabled, false otherwise.
         */
        bool IsWarnEnabled() const override {
            return warnEnabled;
        }

        /**
         \brief Checks whether error logging is enabled.

         \return True if error logging is enabled, false otherwise.
         */
        bool IsErrorEnabled() const override {
            return errorEnabled;
        }

        /**
         * @brief Checks if fatal logging level is enabled.
         *
         * This method is used to check if the fatal logging level is enabled or not.
         * If the fatal logging level is enabled, it means that logging of fatal events
         * is enabled and will be outputted to the log.
         *
         * @return bool - Returns true if the fatal logging level is enabled, false otherwise.
         */
        bool IsFatalEnabled() const override {
            return fatalEnabled;
        }

        /**
         * Check if trace logging is enabled.
         *
         * @return True if trace logging is enabled, false otherwise.
         */
        bool IsTraceEnabled() const override {
            return traceEnabled;
        }
    };

    std::mutex ConsoleLogger::singleton_mutex;
    std::shared_ptr<ConsoleLogger> ConsoleLogger::instance = nullptr;


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
