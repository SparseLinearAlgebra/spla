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

#include <core/logger.hpp>
#include <core/tvector.hpp>

namespace spla {

    ref_ptr<Vector> make_vector(uint n_rows, const ref_ptr<Type>& type) {
        if (n_rows <= 0) {
            LOG_MSG(Status::InvalidArgument, "passed 0 dim");
            return ref_ptr<Vector>{};
        }
        if (!type) {
            LOG_MSG(Status::InvalidArgument, "passed null type");
            return ref_ptr<Vector>{};
        }

        if (type == BYTE) {
            return ref_ptr<Vector>(new TVector<std::int8_t>(n_rows));
        }
        if (type == INT) {
            return ref_ptr<Vector>(new TVector<std::int32_t>(n_rows));
        }
        if (type == UINT) {
            return ref_ptr<Vector>(new TVector<std::uint32_t>(n_rows));
        }
        if (type == FLOAT) {
            return ref_ptr<Vector>(new TVector<float>(n_rows));
        }

        LOG_MSG(Status::NotImplemented, "not supported type " << type->get_name());
        return ref_ptr<Vector>();
    }

}// namespace spla