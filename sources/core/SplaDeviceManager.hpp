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

#ifndef SPLA_SPLADEVICEMANAGER_HPP
#define SPLA_SPLADEVICEMANAGER_HPP

#include <boost/compute/device.hpp>
#include <cstddef>
#include <spla-cpp/SplaExpressionNode.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @class DeviceManager
     * @brief Computational devices management for expressions execution.
     *
     * Device manager allows to fetch single device id or a number of device ids
     * for processing sequential or parallel equally complex parts of expression nodes.
     */
    class DeviceManager {
    public:
        using DeviceId = std::size_t;
        using Device = boost::compute::device;

        /**
         * Fetch device id for execution for specified expression node.
         * @note Uses node descriptor and expression settings to select device.
         *
         * @param node Expression node to process by device.
         * @return Selected device id.
         */
        DeviceId FetchDevice(const RefPtr<ExpressionNode> &node);

        /**
         * Fetch devices' ids for execution for specified expression node.
         * Returns a list of devices to parallelize equally complex computations inside single node.
         * @note Uses node descriptor and expression settings to select device.
         *
         * @param required Required amount of devices for processing.
         * @param node Expression node to process by device.
         * @return Selected devices' ids (vector size matches provided `required` value).
         */
        std::vector<DeviceId> FetchDevices(std::size_t required, const RefPtr<ExpressionNode> &node);

        /**
         * Get boost device by device id.
         * @param id Device id returned by one of the `Fetch` functions.
         * @return Boost device.
         */
        const Device &GetDevice(DeviceId id) const;

        /**
         * Get boost devices.
         * @return Boost devices list.
         */
        const std::vector<Device> &GetDevices() const;

    private:
        friend class LibraryPrivate;
        explicit DeviceManager(std::vector<Device> devices);
        DeviceId NextDevice();

        std::vector<Device> mDevices;
        std::size_t mNextDevice = 0;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLADEVICEMANAGER_HPP