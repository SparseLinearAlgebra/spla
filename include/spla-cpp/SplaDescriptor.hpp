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

#ifndef SPLA_SPLADESCRIPTOR_HPP
#define SPLA_SPLADESCRIPTOR_HPP

#include <sstream>
#include <string>
#include <unordered_map>

#include <spla-cpp/SplaObject.hpp>

namespace spla {

    /**
     * @addtogroup API
     * @{
     */

    /**
     * @class Descriptor
     *
     * Descriptor allows to tweak operation execution by specification
     * of the additional params.
     *
     * Params may be specified as is, or with additional string value,
     * which can be used to provide additional param options.
     */
    class SPLA_API Descriptor final : public Object {
    public:
        ~Descriptor() override = default;

        enum class Param {
            /** Provided matrix/vector values are sorted in row-column order */
            ValuesSorted,
            /** Provided matrix/vector values has no duplicated */
            NoDuplicates,
            /** Apply !mask (complementary mask) to result or input arguments */
            MaskComplement,
            /** Apply default or provided binary op accum to output and temporary operation result */
            AccumResult,
            /** Profiles time of the operation and outputs result to the log */
            ProfileTime,
            /** Factor used to transition to dense if possible */
            DenseFactor,
            /** Transpose operation arg 1 matrix before operation */
            TransposeArg1,
            /** Transpose operation arg 2 matrix before operation */
            TransposeArg2,
            /** Force `device-0` for expression node computation */
            DeviceId0,
            /** Force `device-1` for expression node computation */
            DeviceId1,
            /** Force `device-2` for expression node computation */
            DeviceId2,
            /** Force `device-3` for expression node computation */
            DeviceId3,
            /** Force `device-4` for expression node computation */
            DeviceId4,
            /** Force `device-5` for expression node computation */
            DeviceId5,
            /** Force `device-6` for expression node computation */
            DeviceId6,
            /** Force `device-7` for expression node computation */
            DeviceId7
        };

        /**
         * Set descriptor param value.
         * Pass empty string to set param, which does not expects any string value.
         *
         * @param param Param name to set
         * @param value String param value; may be empty
         */
        void SetParam(Param param, std::string value = std::string());

        /**
         * @brief Set descriptor no-value param.
         * Pass true to set of false to remove param.
         *
         * @param param Param name to set
         * @param flag True to set, false to remove
         */
        void SetParam(Param param, bool flag);

        /**
         * @brief Get descriptor param value.
         * If param was set without value, returned string value is empty.
         *
         * @param param Param name to get
         * @param[out] value Output string param value; may be empty
         *
         * @return True if this param was set in descriptor
         */
        bool GetParam(Param param, std::string &value) const;

        /**
         * @brief Remove param from desc
         *
         * @param param Param to remove
         *
         * @return True if params was in desc before removal
         */
        bool RemoveParam(Param param);

        /**
         * Check if specified param was set in descriptor.
         *
         * @param param Param name to check
         *
         * @return True if this param was set in descriptor
         */
        bool IsParamSet(Param param) const;

        /**
         * @brief Duplicate descriptor
         *
         * Creates exact copy of the descriptor as
         * new descriptor object. Mutation of this and
         * copied descriptor is safe and done independently.
         *
         * @return Duplicated descriptor
         */
        RefPtr<Descriptor> Dup() const;

        /**
         * Make new descriptor instance for specified library.
         *
         * @param library Library instance
         *
         * @return New descriptor instance
         */
        static RefPtr<Descriptor> Make(class Library &library);

        /**
         * @brief Get descriptor param value.
         * If param was set without value, returned string value is empty.
         *
         * @tparam T Type of value to get
         * @param param Param name to get
         * @param[out] value Output param value
         *
         * @return True if this param was set in descriptor
         */
        template<typename T>
        bool GetParamT(Param param, T &value) {
            std::string text;
            auto hasParam = GetParam(param, text);

            if (hasParam) {
                std::stringstream ss(text);
                ss >> value;
            }

            return hasParam;
        }

    private:
        explicit Descriptor(class Library &library);
        Descriptor(const Descriptor&);

        // Map of desc configurable params and its values
        std::unordered_map<Param, std::string> mParams;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLADESCRIPTOR_HPP
