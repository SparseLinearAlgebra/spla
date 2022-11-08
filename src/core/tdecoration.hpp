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

#ifndef SPLA_T_DECORATION_HPP
#define SPLA_T_DECORATION_HPP

#include <spla/ref.hpp>

namespace spla {

    /**
     * @addtogroup internal
     * @{
     */

    /**
     * @class TDecoration
     * @brief Base class for typed decoration for storage object
     *
     * @tparam T Type of stored values
     */
    template<typename T>
    class TDecoration : public RefCnt {
    public:
        ~TDecoration() override = default;

        /** @return Number of value in decoration */
        [[nodiscard]] virtual uint get_values() const { return values; }

        /** @return Version of decoration required to sync data between storages */
        [[nodiscard]] virtual uint get_version() const { return version; };

        /**
         * @brief Updates version of the decorator stored content
         *
         * @param new_version New version to set
         */
        virtual void update_version(uint new_version) { version = new_version; }

        /**
         * @brief Checks if version of this decorator is valid
         *
         * @param current_version Version to check
         *
         * @return True if valid
         */
        virtual bool is_valid_version(uint current_version) { return version == current_version; }

    public:
        uint values  = 0;
        uint version = 0;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_T_DECORATION_HPP