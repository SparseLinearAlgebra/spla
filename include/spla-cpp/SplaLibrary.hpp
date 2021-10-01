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

#ifndef SPLA_SPLALIBRARY_HPP
#define SPLA_SPLALIBRARY_HPP

#include <memory>
#include <optional>
#include <spla-cpp/SplaConfig.hpp>
#include <spla-cpp/SplaRefCnt.hpp>
#include <string>
#include <vector>

namespace spla {

    /**
     * @class Library
     *
     * Primary access point to the spla library operations.
     * This class encapsulates global library state, allows to create objects and
     * execute operations. Must be created as first spla object in the application.
     */
    class SPLA_API Library {
    public:
        /**
         * @class Config
         *
         * Represents system configuration used in all computations.
         * @p Config initializes OpenCL context, which includes all
         * specified devices.
         */
        class SPLA_API Config {
        public:
            /**
             * Default size (nrows and ncols) for matrix/vector block size.
             * Used to split matrix or vector into equal sized storage blocks.
             */
            static const size_t DEFAULT_BLOCK_SIZE = 1000000;

            /**
             * Type of OpenCL device.
             */
            enum DeviceType {
                GPU,
                CPU,
                Accelerator,
            };

            /**
             * Initializes @p Config.
             * If no other setter will be called, context with only one default device will be created.
             * Otherwise context will contain those and only those devices, which meet given
             * constraints.
             */
            explicit Config();

            /**
             * Sets platform name to @p platformName.
             * All context devices will belong to specified platform.
             * If this function was not called,
             * the one for which as many devices as possible meet the given parameters will be used.
             * 
             * @param platformName Full name of OpenCL platform.
             */
            Config &SetPlatform(std::string platformName);

            /**
             * Sets device type to @p deviceType.
             * All devices will be chosen, if device type was not set.
             * 
             * @param deviceType Type of device to be set.
             */
            Config &SetDeviceType(DeviceType deviceType);

            /**
             * Limits the quantity of devices in context.
             * @p 1 by default.
             * 
             * @param deviceAmount Required quantity of devices.
             */
            Config &LimitAmount(std::size_t deviceAmount);

            /**
             * Removes any limit on devices amount.
             * Hence, all available devices will be used.
             */
            Config &RemoveAmountLimit();

            /**
             * Set log file name, to log trace messages in debug library build.
             *
             * @note Use utf-16 encoded wstring on windows platform to specify log file name.
             * @note Use utf-8 encoded string on windows platform to specify log file name.
             *
             * @param filename Log file name
             * @return This config
             */
            Config &SetLogFilename(Filename filename);

            /**
             * Set matrix/vector block size param.
             *
             * This param used to split primitives data:
             * - matrix data in equally sized blocks of size `blockSize` x `blockSize`
             * - vector data in equally sized blocks of size `blockSize` x 1
             * Edge blocks has clamped size.
             *
             * This param defines granularity of the data processing, since
             * each block processing can be submitted to the separate gpu/queue
             * as a stand alone task.
             *
             * @param blockSize Size of the matrix/vector block; must be greater then zero
             * @return This config
             */
            Config &SetBlockSize(size_t blockSize);

            /** @return List of available devices for specified config settings */
            [[nodiscard]] std::vector<std::string> GetDevicesNames() const;

            /** @return Log filename */
            [[nodiscard]] const std::optional<Filename> &GetLogFilename() const;

            /** @return Block size */
            [[nodiscard]] size_t GetBlockSize() const;

        private:
            std::optional<std::string> mPlatformName;
            std::optional<DeviceType> mDeviceType;
            std::optional<std::size_t> mDeviceAmount = std::optional{1U};
            std::optional<Filename> mLogFilename;
            size_t mBlockSize = DEFAULT_BLOCK_SIZE;
        };

    public:
        Library(Config config);

        Library();

        ~Library();

        /**
         * Submit expression for the execution.
         * @param expression Expression for execution
         */
        void Submit(const RefPtr<class Expression> &expression) const;

        /** @return Private state (for internal usage only) */
        [[nodiscard]] class LibraryPrivate &GetPrivate() const noexcept;

        /** @return Private state (for internal usage only) */
        [[nodiscard]] const std::shared_ptr<class LibraryPrivate> &GetPrivatePtr() const noexcept;

        /**
         * Get description of computational devices.
         * 
         * @return String representation of config
         */
        [[nodiscard]] std::string PrintContextConfig() const noexcept;

    private:
        // Private state
        std::shared_ptr<class LibraryPrivate> mPrivate;
    };

}// namespace spla

#endif//SPLA_SPLALIBRARY_HPP
