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

#include <spla-cpp/SplaObject.hpp>
#include <unordered_map>
#include <string>

namespace spla {

    /**
     * @class Descriptor
     *
     * Descriptor allows to tweak operation execution by specification
     * of the additional params.
     *
     * Params may be specified as is, or with additional string value,
     * which can be used to provide additional param options.
     */
    class SPLA_API Descriptor final: public Object {
    public:
        ~Descriptor() override = default;

        enum class Param {
            /** Provided matrix/vector values are sorted in row-column order */
            ValuesSorted,
            /** Provided matrix/vector values has no duplicated */
            NoDuplicates,
            /** Profiles time of the operation and outputs result to the log */
            ProfileTime,
            /** Transpose operation arg 1 matrix before operation */
            TransposeArg1,
            /** Transpose operation arg 2 matrix before operation */
            TransposeArg2,
        };

        /**
         * Set descriptor param value.
         * Pass empty string to set param, which does not expects any string value.
         *
         * @param param Param name to set
         * @param value String param value; may be empty
         */
        void SetParam(Param param, std::wstring value = std::wstring());

        /**
         * Get descriptor param value.
         * If param was set without value, returned string value is empty.
         *
         * @param param Param name to get
         * @param[out] value Output string param value; may be empty
         *
         * @return True if this param was set in descriptor
         */
        bool GetParam(Param param, std::wstring& value) const;

        /**
         * Check if specified param was set in descriptor.
         *
         * @param param Param name to check
         *
         * @return True if this param was set in descriptor
         */
        bool IsParamSet(Param param) const;

        /**
         * Make new descriptor instance for specified library.
         *
         * @param library Library instance
         *
         * @return New descriptor instance
         */
        static RefPtr<Descriptor> Make(class Library& library);

    private:
        explicit Descriptor(class Library& library);

        // Map of desc configurable params and its values
        std::unordered_map<Param, std::wstring> mParams;
    };

}

#endif //SPLA_SPLADESCRIPTOR_HPP
