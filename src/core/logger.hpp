/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2023 SparseLinearAlgebra                                         */
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

#ifndef SPLA_LOGGER_HPP
#define SPLA_LOGGER_HPP

#include <spla/config.hpp>

#include <functional>
#include <mutex>
#include <sstream>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class Logger
     * @brief Library logger
     */
    class Logger {
    public:
        void log_msg(Status status, const std::string& msg, const std::string& file, const std::string& function, int line);
        void set_msg_callback(MessageCallback callback);

    private:
        MessageCallback m_callback;

        mutable std::mutex m_mutex;
    };

    /**
     * @}
     */

}// namespace spla

#ifndef SPLA_RELEASE
    #define LOG_MSG(status, msg)                                                                                           \
        do {                                                                                                               \
            std::stringstream __ss;                                                                                        \
            __ss << msg;                                                                                                   \
            Library::get()->get_logger()->log_msg(status, __ss.str(), __FILE__, __FUNCTION__, static_cast<int>(__LINE__)); \
        } while (false);
#else
    #define LOG_MSG(status, msg)                                                                                               \
        do {                                                                                                                   \
            if ((status) != Status::Ok) {                                                                                      \
                std::stringstream __ss;                                                                                        \
                __ss << msg;                                                                                                   \
                Library::get()->get_logger()->log_msg(status, __ss.str(), __FILE__, __FUNCTION__, static_cast<int>(__LINE__)); \
            }                                                                                                                  \
        } while (false);
#endif

#endif//SPLA_LOGGER_HPP
