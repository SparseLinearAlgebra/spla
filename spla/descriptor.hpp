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

#ifndef SPLA_DESCRIPTOR_HPP
#define SPLA_DESCRIPTOR_HPP

#include <sstream>
#include <string>
#include <unordered_map>

#include <spla/config.hpp>

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class Descriptor
     * @brief Descriptor object controls operations execution
     */
    class Descriptor {
    public:
        using FieldCustom = std::string;

        /**
         * @brief Set custom descriptor param from value
         * Value must provide `std::to_string()` to serialize its data.
         *
         * @tparam T Type of value to set
         *
         * @param field Name of the custom field
         * @param value Value to set
         */
        template<typename T>
        void set_custom(const FieldCustom &field, const T &value) {
            m_custom[field] = std::to_string(value);
        }

        /**
         * @brief Get custom value
         * Value must provide `std::stringstream operator >>` to deserialize its data.
         *
         * @tparam T Type of value to get
         *
         * @param field Name of custom field
         * @param value Value to read
         *
         * @return True if value in descriptor
         */
        template<typename T>
        bool get_custom(const FieldCustom &field, T &value) const {
            auto query = m_custom.find(field);

            if (query != m_custom.end()) {
                std::stringstream value_str(query->second);
                value_str >> value;
                return true;
            }

            return false;
        }

    private:
        std::unordered_map<FieldCustom, std::string> m_custom;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_DESCRIPTOR_HPP
