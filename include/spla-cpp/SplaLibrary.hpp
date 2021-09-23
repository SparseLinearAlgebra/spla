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

#include <spla-cpp/SplaConfig.hpp>
#include <spla-cpp/SplaRefCnt.hpp>
#include <memory>
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
             * Type of OpenCL device.
             */
            enum DeviceType {
                GPU,
                CPU,
                Accelerator,
            };

            /**
             * Amount of used OpenCL devices.
             */
            enum DeviceAmount {
                One,
                All,
            };

            /**
             * Creates @p Config with all specified devices.
             *
             * @param devices Vector of device names which will be used in context.
             *
             * @note No checks about devices are performed, so
             * first make sure that all @p devices can be used in
             * the same context.
             */
            explicit Config(const std::vector<std::string>& devices);

            /**
             * Creates @p Config with only one default device.
             */
            explicit Config();

            /**
             * Creates @p Config with all devices from @p platform,
             * specified type @p type and @p amount.
             * 
             * @param platform Name of OpenCL platform.
             * @param type Type of required device type.
             * @param amount Quantity of devices of @p type which will be used in context.
             * In this case all devices in context will be of same type.
             */
            explicit Config(
                const std::string& platform,
                DeviceType type,
                DeviceAmount amount
            );

            /**
             * Creates @p Config which context will contain devices of @p type
             * in quantity @p amount.
             * 
             * @param type Type of required device.
             * @param amount Quantity of required device type.
             * 
             * @note No platform is specified, so the one containing more
             * devices of type @p type will be used.
             */
            explicit Config(
                DeviceType type,
                DeviceAmount amount
            );

            /**
             * Creates @p Config with context will contain devices in
             * quantiity @p amount.
             * 
             * @param amount Quantity of required devices.
             * 
             * @note No platform is specified, so the one containing more
             * devices will be used.
             */
            explicit Config(
                DeviceAmount amount
            );

            const std::vector<std::string>& GetDevicesNames() const noexcept;

        private:
            std::vector<std::string> mDevicesNames;
        };

    public:
        Library(const Config& config);

        Library();

        ~Library();

        /**
         * Submit expression for the execution.
         * @param expression Expression for execution
         */
        void Submit(const RefPtr<class Expression>& expression);

        /** @return Private state (for internal usage only) */
        class LibraryPrivate& GetPrivate();

    private:
        // Private state
        std::unique_ptr<class LibraryPrivate> mPrivate;
    };

}

#endif //SPLA_SPLALIBRARY_HPP
