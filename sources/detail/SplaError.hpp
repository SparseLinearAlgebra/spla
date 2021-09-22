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

#ifndef SPLA_SPLAERROR_HPP
#define SPLA_SPLAERROR_HPP

#include <exception>
#include <string>
#include <sstream>

namespace spla {

    enum class Status {
        Error,
        DeviceError,
        DeviceNotPresent,
        MemOpFailed,
        InvalidArgument,
        InvalidState,
        NotImplemented
    };

    /**
     * Generic library exception.
     */
    class Exception: public std::exception {
    public:

        Exception(std::wstring message, std::string function, std::string file, size_t line, Status status, bool critical)
                : std::exception(),
                  mMessage(std::move(message)),
                  mFunction(std::move(function)),
                  mFile(std::move(file)),
                  mLine(line),
                  mStatus(status),
                  mCritical(critical) {

        }

        Exception(const Exception& e) noexcept = default;
        Exception(Exception&& e) noexcept = default;
        ~Exception() noexcept override = default;

        const char* what() const noexcept override {
            if (!mCached) {
                mCached = true;

                std::stringstream s;
                s << "Exception in" << GetFile()
                  << ": line: " << GetLine()
                  << " function: " << GetFunction();

                mWhatCached = s.str();
            }

            return mWhatCached.c_str();
        }

        const std::wstring& GetMessage() const noexcept {
            return mMessage;
        }

        const std::string& GetFunction() const noexcept {
            return mFunction;
        }

        const std::string& GetFile() const noexcept {
            return mFile;
        }

        size_t GetLine() const {
            return mLine;
        }

        Status GetStatus() const noexcept {
            return mStatus;
        }

        bool IsCritical() const noexcept {
            return mCritical;
        }

    private:
        std::wstring mMessage;
        std::string mFunction;
        std::string mFile;
        size_t mLine;
        Status mStatus;
        bool mCritical;

        mutable std::string mWhatCached;
        mutable bool mCached = false;
    };

    /**
     * Exceptions with Status error code parametrisation.
     * @tparam Type Exception error code (type)
     */
    template<Status status>
    class TException: public Exception {
    public:
        TException(std::wstring message, std::string&& function, std::string&& file, size_t line, bool critical)
                : Exception(std::move(message), std::move(function), std::move(file), line, status, critical)  {

        }

        TException(const TException& other) noexcept = default;
        TException(TException&& other) noexcept = default;
        ~TException() noexcept override = default;
    };

    // Errors exposed to the C API
    using Error = TException<Status::Error>;
    using DeviceError = TException<Status::DeviceError>;
    using DeviceNotPresent = TException<Status::DeviceNotPresent>;
    using MemOpFailed = TException<Status::MemOpFailed>;
    using InvalidArgument = TException<Status::InvalidArgument>;
    using InvalidState = TException<Status::InvalidState>;
    using NotImplemented = TException<Status::NotImplemented>;

}

// An error, in theory, can recover after this
#define RAISE_ERROR(type, message)                                                      \
    do {                                                                                \
        ::std::wstringstream __ws;                                                      \
        __ws << message;                                                                \
        throw ::spla::type(__ws.str(), __FUNCTION__, __FILE__, __LINE__, false);        \
    } while (0);

#define CHECK_RAISE_ERROR(condition, type, message)                                     \
    if (!(condition)) { RAISE_ERROR(type, message); } else { }

// Critical errors, cause library shutdown
#define RAISE_CRITICAL_ERROR(type, message)                                             \
    do {                                                                                \
        ::std::wstringstream __ws;                                                      \
        __ws << message;                                                                \
        throw ::spla::type(__ws.str(), __FUNCTION__, __FILE__, __LINE__, true);         \
    } while (0);

#define CHECK_RAISE_CRITICAL_ERROR(condition, type, message)                            \
    if (!(condition)) { RAISE_CRITICAL_ERROR(type, message); } else { }

#endif //SPLA_SPLAERROR_HPP
