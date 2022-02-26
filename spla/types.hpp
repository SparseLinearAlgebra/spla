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

#ifndef SPLA_TYPES_HPP
#define SPLA_TYPES_HPP

#include <spla/config.hpp>

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @brief Check if type has actual values
     *
     * @tparam T Actual type param
     * @return True if has values
     */
    template<typename T>
    inline bool type_has_values() {
        return true;
    }

    /**
     * @brief Get type actual size in bytes
     *
     * @tparam T Actual type param
     * @return Size
     */
    template<typename T>
    inline std::size_t type_size() {
        return sizeof(T);
    }

    /**
     * @class Unit
     * @brief Built-in void type
     *
     * Unit type has no actual stored values.
     * Can be used only as a structural graph information.
     */
    using Unit = unsigned char;

    template<>
    inline bool type_has_values<Unit>() {
        return false;
    }

    template<>
    inline std::size_t type_size<Unit>() {
        return 0;
    }

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_TYPES_HPP
