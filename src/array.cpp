/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2023 SparseLinearAlgebra                                         */
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

#include <core/logger.hpp>
#include <core/tarray.hpp>

namespace spla {

    ref_ptr<Array> Array::make(uint n_values, const ref_ptr<Type>& type) {
        if (!type) {
            LOG_MSG(Status::InvalidArgument, "passed null type");
            return ref_ptr<Array>{};
        }

        Library::get();

        if (type == INT) {
            return ref_ptr<Array>(new TArray<std::int32_t>(n_values));
        }
        if (type == UINT) {
            return ref_ptr<Array>(new TArray<std::uint32_t>(n_values));
        }
        if (type == FLOAT) {
            return ref_ptr<Array>(new TArray<float>(n_values));
        }

        LOG_MSG(Status::NotImplemented, "not supported type " << type->get_name());
        return ref_ptr<Array>{};
    }

}// namespace spla