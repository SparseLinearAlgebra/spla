/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021-2022 JetBrains-Research                                     */
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

#ifndef SPLA_LIBRARY_HPP
#define SPLA_LIBRARY_HPP

#include "config.hpp"

#include <memory>
#include <string>

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class Library
     * @brief Library global state automatically instantiated on lib init
     */
    class Library final {
    public:
        SPLA_API Library();
        SPLA_API ~Library();

        Library(const Library&) = delete;
        Library(Library&&)      = delete;

        /**
         * @brief Finalize library execution
         *
         * Finalize method must be called at the end after application
         * to correctly shutdown global library state, release any
         * enabled acceleration device and release any pending device resources.
         *
         * @warning Must be called after application execution
         * @warning After this call no other library function call is allowed
         */
        SPLA_API void finalize();

        /**
         * @brief Set accelerator to be used in library computations
         *
         * Sets type of the accelerator to be used in library computations.
         * By default library attempts automatically init OpenCL accelerator
         * if OpenCL runtime present in the system. Set `None` to disable acceleration.
         *
         * @param accelerator Accelerate type
         *
         * @return Function call status
         */
        SPLA_API Status set_accelerator(AcceleratorType accelerator);

        /**
         * @brief Selects platform for computations for current accelerator
         *
         * @param index Platform index to select in current PC supported list.
         *
         * @return Function call status
         */
        SPLA_API Status set_platform(int index);

        /**
         * @brief Selects device for computations for current accelerator
         *
         * @param index Device index in current platform devices
         *
         * @return Function call status
         */
        SPLA_API Status set_device(int index);

        /**
         * @brief Set number of GPU queues for parallel ops execution
         *
         * @param count Number of queues to set
         *
         * @return Function call status
         */
        SPLA_API Status set_queues_count(int count);

        /**
         * @brief Set callback function called on library message event
         *
         * @param callback Function to be called
         *
         * @return Function call status
         */
        SPLA_API Status set_message_callback(MessageCallback callback);

        /**
         * @brief Sets default library callback to log messages to console
         *
         * @return Function call status
         */
        SPLA_API Status set_default_callback();

        /**
         * @warning Internal usage only!
         * @return Library computations accelerator if presented
         */
        class Accelerator* get_accelerator();

        /**
         * @warning Internal usage only!
         * @return Library algorithms registry
         */
        class Registry* get_registry();

        /**
         * @warning Internal usage only!
         * @return Library algorithms dispatcher
         */
        class Dispatcher* get_dispatcher();

        /**
         * @warning Internal usage only!
         * @return Library logger
         */
        class Logger* get_logger();

    private:
        std::unique_ptr<class Accelerator> m_accelerator;
        std::unique_ptr<class Registry>    m_registry;
        std::unique_ptr<class Dispatcher>  m_dispatcher;
        std::unique_ptr<class Logger>      m_logger;
    };

    /**
     * @brief Access global library instance
     *
     * Global library state instantiate once on first request to the library.
     * Call this function to access library and configure it first before any computations.

     * @note Only single global instance of the library allowed.
     *
     * @return Global library instance
     */
    SPLA_API Library* get_library();

    /**
     * @brief Global library computations accelerator if presented
     *
     * @warning Internal usage only!
     *
     * @return Library computations accelerator if presented
     */
    static class Accelerator* get_accelerator() { return get_library()->get_accelerator(); }

    /**
     * @brief Global library algorithms registry
     *
     * @warning Internal usage only!
     *
     * @return Library algorithms registry
     */
    static class Registry* get_registry() { return get_library()->get_registry(); }

    /**
     * @brief Global library algorithms dispatcher
     *
     * @warning Internal usage only!
     *
     * @return Library algorithms dispatcher
     */
    static class Dispatcher* get_dispatcher() { return get_library()->get_dispatcher(); }

    /**
     * @brief Global library logger
     *
     * @warning Internal usage only!
     *
     * @return Library logger
     */
    static class Logger* get_logger() { return get_library()->get_logger(); }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_LIBRARY_HPP
