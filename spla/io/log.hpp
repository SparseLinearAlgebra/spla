/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021 JetBrains-Research                                          */
/*                                                                                */
/* Permission is hereby granted, free of charge, to any person obtaining a copy   */
/* of this software and associated documentation files (the "Software"), to deal  */
/* in the Software without restriction, including without limitation the rights   */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      */
/* copies of the Software, and to permit persons to whom the Software is          */
/* furnished to do so, subject to the following conditions:                       */
/*                                                                                */
/* The above copyright notice and this permission notice shall be included in all */
/* copies or substantial portions of the Software.                                */
/*                                                                                */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  */
/* SOFTWARE.                                                                      */
/**********************************************************************************/

#ifndef SPLA_LOG_HPP
#define SPLA_LOG_HPP

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <vector>

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class Log
     * @brief Library log class
     *
     * Log is used for debug and release mode messaging,
     * for critical errors and timing info display.
     *
     * @note Available by the `get_log()` function.
     */
    class Log {
    public:
        /**
         * @brief Level of logged message
         */
        enum class Level {
            Info = 0,
            Warning = 1,
            Error = 2
        };

        /**
         * @brief Log entry
         */
        struct Entry {
            std::string message;
            std::string file;
            std::string function;
            std::size_t line;
            Level level;
        };

        static const std::size_t DEFAULT_SIZE = 32;
        using Listener = std::function<void(const Entry &)>;

        /**
         * @brief Log message to the log
         *
         * @param level Message level of importance
         * @param message Message string
         * @param file File of invocation
         * @param function Function of invocation
         * @param line Line of code
         */
        void log_message(Level level, std::string message, std::string file, std::string function, std::size_t line) {
            Entry entry{std::move(message), std::move(file), std::move(function), line, level};
            std::lock_guard<std::mutex> lockGuard(m_mutex);
            for (auto &listener : m_listeners) listener(entry);
            add_entry(std::move(entry));
        }

        /**
         * @brief Add new listener to observe log messages
         * Use this function to your custom listener.
         *
         * @param listener Listener to add
         */
        void add_listener(Listener listener) {
            std::lock_guard<std::mutex> lockGuard(m_mutex);
            m_listeners.push_back(std::move(listener));
        }

    private:
        void shrink() {
            if (m_entries.size() >= m_size)
                m_entries.pop();
        }

        void add_entry(Entry &&entry) {
            shrink();
            m_entries.push(std::move(entry));
        }

    private:
        std::queue<Entry> m_entries;
        std::vector<Listener> m_listeners;
        std::size_t m_size = DEFAULT_SIZE;

        mutable std::mutex m_mutex;
    };

    /** @return Log level to string */
    inline std::string to_string(Log::Level level) {
        switch (level) {
            case Log::Level::Info:
                return "info";
            case Log::Level::Warning:
                return "warning";
            case Log::Level::Error:
                return "error";
            default:
                return "none";
        }
    }

    /**
     * @brief Get library log ptr
     * @note For internal usage
     *
     * @return Log ptr
     */
    inline std::unique_ptr<Log> &get_log_ptr() {
        static std::unique_ptr<Log> log;
        return log;
    }

    /**
     * @brief Get library log for logging messages.
     * Use this function to access global library logger.
     *
     * @return Library log
     */
    inline Log &get_log() {
        auto &log = get_log_ptr();

        if (!log)
            log = std::make_unique<Log>();

        return *log;
    }

    /**
     * @}
     */

#define SPLA_LOG(level, message)                                                                                              \
    do {                                                                                                                      \
        std::stringstream __log_stream;                                                                                       \
        __log_stream << message;                                                                                              \
        ::spla::get_log().log_message(level, __log_stream.str(), __FILE__, __FUNCTION__, static_cast<std::size_t>(__LINE__)); \
    } while (false);

#ifdef SPLA_DEBUG
    #define SPLA_LOG_INFO(message) SPLA_LOG(::spla::Log::Level::Info, message)
    #define SPLA_LOG_WARNING(message) SPLA_LOG(::spla::Log::Level::Warning, message)
#else
    #define SPLA_LOG_INFO(message)
    #define SPLA_LOG_WARNING(message)
#endif

#define SPLA_LOG_ERROR(message) SPLA_LOG(::spla::Log::Level::Error, message)

}// namespace spla

#endif//SPLA_LOG_HPP
