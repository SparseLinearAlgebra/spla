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

#include "logger.hpp"

#include <filesystem>

namespace spla {

    static int status_to_level(Status status) {
        switch (status) {
            case Status::Error:
            case Status::PlatformNotFound:
            case Status::DeviceNotFound:
            case Status::InvalidArgument:
            case Status::CompilationError:
                return 1;

            case Status::NoAcceleration:
            case Status::InvalidState:
            case Status::NoValue:
            case Status::NotImplemented:
                return 2;

            case Status::Ok:
            default:
                return 3;
        }
    }

    void Logger::log_msg(Status status, const std::string& msg,
                         const std::string& file, const std::string& function, int line) {

        std::lock_guard guard(m_mutex);

        if (!m_verbosity.has_value()) return;

        int level = status_to_level(status);
        if (level > *m_verbosity) return;

        std::filesystem::path file_path(file);
        if (m_callback) m_callback(status, msg, file_path.filename().string(), function, line);
    }

    void Logger::set_msg_callback(MessageCallback callback) {
        std::lock_guard guard(m_mutex);
        m_callback = std::move(callback);
    }

    void Logger::set_verbosity(int verbosity) {
        std::lock_guard guard(m_mutex);
        m_verbosity = verbosity;
    }

}// namespace spla