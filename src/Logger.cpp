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

/******************************************************************************
    Author: Joaquin Bejar Garcia 
    Email: jb@taunais.com 
    Date: 7/3/24
 ******************************************************************************/

#include <Logger.hpp>

namespace Logger {
    using ConsoleLogLevel::Level;
    typedef std::string Category;

    inline std::string formatMessageWithException(const std::string &message, const std::exception &e) {
        std::ostringstream oss;
        oss << message << " With Exception: " << e.what();
        return oss.str();
    }

    inline std::string getColoredLevel(const std::string &message, const Level level) {
        switch (level) {
            case Level::INFO:
                return give_color(simple_color::Colors::WHITE, "[INFO]: ") + message;
            case Level::DEBUG:
                return give_color(simple_color::Colors::SKYBLUE1, "[DEBUG]: ") + message;
            case Level::ERROR:
                return give_color(simple_color::Colors::RED, "[ERROR]: ") + message;
            case Level::FATAL:
                return give_color(simple_color::Colors::LIGHTCORAL, "[FATAL]: ", true) + message;
            case Level::WARN:
                return give_color(simple_color::Colors::YELLOW, "[WARNING]: ") + message;
            case Level::TRACE:
                return give_color(simple_color::Colors::LIGHTGREEN, "[TRACE]: ") + message;
            default:
                return give_color(simple_color::Colors::GREY54, "[UNKNOWN]: ") + message;
        }
    }


    void Logger::m_safe_cout(const std::string &s, const bool flush, std::ostream &out) {
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


    void Logger::m_log(const std::string &s, bool flush, std::ostream &out) {
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);
        std::ostringstream time_prefix;
        time_prefix << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << " " << s;
        m_safe_cout(time_prefix.str(), flush, out);
    }


    Logger::Logger(const Level level, const Category &category) : level(level), category(category) {
    }


    Level Logger::getLevel() const {
        return level;
    }


    ;


    ConsoleLogger::ConsoleLogger(const Level level, const Category &category) : Logger(level, category) {
        traceEnabled = level <= Level::TRACE;
        debugEnabled = level <= Level::DEBUG;
        infoEnabled = level <= Level::INFO;
        warnEnabled = level <= Level::WARN;
        errorEnabled = level <= Level::ERROR;
        fatalEnabled = level <= Level::FATAL;
    }


    std::shared_ptr<ConsoleLogger> ConsoleLogger::getInstance(const Level level, const Category &category) {
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
    void ConsoleLogger::Error(const std::string &message) override {
        if (errorEnabled)
            m_log(getColoredLevel(message, Level::ERROR), false, std::cerr);
    }

    /**
     * Logs an error message with an exception.
     *
     * @param message The error message to be logged.
     * @param e The exception associated with the error.
     */
    void ConsoleLogger::Error(const std::string &message, const std::exception &e) {
        if (errorEnabled)
            m_log(getColoredLevel(formatMessageWithException(message, e), Level::ERROR),
                  false, std::cerr);
    }

    /**
     * Logs a warning message.
     *
     * @param message The warning message to be logged.
     */
    void ConsoleLogger::Warn(const std::string &message) override {
        if (warnEnabled)
            m_log(getColoredLevel(message, Level::WARN), false, std::cerr);
    }

    /**
     * Logs a warning message with the given message and exception.
     *
     * @param message The warning message to be logged.
     * @param e The exception to be logged.
     */
    void ConsoleLogger::Warn(const std::string &message, const std::exception &e) {
        if (warnEnabled)
            m_log(getColoredLevel(formatMessageWithException(message, e), Level::WARN),
                  false, std::cerr);
    }

    /**
     * @brief Writes an informational message to the log.
     *
     * This method writes the given message to the log as an informational message.
     *
     * @param message The message to be written to the log.
     */
    void ConsoleLogger::Info(const std::string &message) override {
        if (infoEnabled)
            m_log(getColoredLevel(message, Level::INFO), false, std::cout);
    }

    /**
     * Logs an informational message.
     *
     * @param message The message to be logged.
     * @param e The exception to be logged along with the message.
     */
    void ConsoleLogger::Info(const std::string &message, const std::exception &e) {
        if (infoEnabled)
            m_log(getColoredLevel(formatMessageWithException(message, e), Level::INFO),
                  false, std::cout);
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
    void ConsoleLogger::Debug(const std::string &message) override {
        if (debugEnabled)
            m_log(getColoredLevel(message, Level::DEBUG), false, std::cout);
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
     **/
    void ConsoleLogger::Debug(const std::string &message, const std::exception &e) {
        if (debugEnabled)
            m_log(getColoredLevel(formatMessageWithException(message, e), Level::DEBUG),
                  false, std::cout);
    }

    /**
     * @brief Logs a fatal message with the given message.
     *
     * This method is called to log a fatal message. The message is sent to the logger specified in the `m_log` member variable.
     *
     * @param message The message to be logged.
     **/
    void ConsoleLogger::Fatal(const std::string &message) override {
        if (fatalEnabled)
            m_log(getColoredLevel(message, Level::FATAL), false, std::cerr);
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
     **/
    void ConsoleLogger::Fatal(const std::string &message, const std::exception &e) {
        if (fatalEnabled)
            m_log(getColoredLevel(formatMessageWithException(message, e), Level::FATAL),
                  false, std::cerr);
    }

    /**
     * Trace method sends the given message to the logging system.
     *
     * @param message The message to be logged.
     **/
    void ConsoleLogger::Trace(const std::string &message) override {
        if (traceEnabled)
            m_log(getColoredLevel(message, Level::TRACE), false, std::cout);
    }

    /**
     * Logs a trace message with an exception.
     *
     * @param message The message to be logged.
     * @param e The exception to be logged.
     **/
    void ConsoleLogger::Trace(const std::string &message, const std::exception &e) {
        if (traceEnabled)
            m_log(getColoredLevel(formatMessageWithException(message, e), Level::TRACE),
                  false, std::cout);
    }


    /**
     * Checks if debug mode is enabled.
     *
     * @return true if debug mode is enabled, false otherwise.
     */
    bool ConsoleLogger::IsDebugEnabled() const override {
        return debugEnabled;
    }

    /**
     * @brief Checks if the information level logging is enabled.
     *
     * This method returns a boolean value indicating whether the information level logging is enabled or not.
     *
     * @return True if the information level logging is enabled, false otherwise.
     */
    bool ConsoleLogger::IsInfoEnabled() const override {
        return infoEnabled;
    }

    /**
     * @brief Check if warning level log is enabled.
     *
     * @return True if warning level log is enabled, false otherwise.
     */
    bool ConsoleLogger::IsWarnEnabled() const override {
        return warnEnabled;
    }

    /**
     \brief Checks whether error logging is enabled.

     \return True if error logging is enabled, false otherwise.
     */
    bool ConsoleLogger::IsErrorEnabled() const override {
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
    bool ConsoleLogger::IsFatalEnabled() const override {
        return fatalEnabled;
    }

    /**
     * Check if trace logging is enabled.
     *
     * @return True if trace logging is enabled, false otherwise.
     */
    bool ConsoleLogger::IsTraceEnabled() const override {
        return traceEnabled;
    }
};
