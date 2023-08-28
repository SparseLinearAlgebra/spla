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

#include "c_config.hpp"

spla_Status spla_MemView_make(spla_MemView* view, void* buffer, spla_size_t size, spla_bool is_mutable) {
    auto memory_view = spla::MemView::make(buffer, size, bool(is_mutable));
    *view            = as_ptr<spla_MemView_t>(memory_view.release());
    return SPLA_STATUS_OK;
}
spla_Status spla_MemView_read(spla_MemView view, spla_size_t offset, spla_size_t size, void* dst) {
    return to_c_status(as_ptr<spla::MemView>(view)->read(offset, size, dst));
}
spla_Status spla_MemView_write(spla_MemView view, spla_size_t offset, spla_size_t size, const void* src) {
    return to_c_status(as_ptr<spla::MemView>(view)->write(offset, size, src));
}
spla_Status spla_MemView_get_buffer(spla_MemView view, void** buffer) {
    *buffer = as_ptr<spla::MemView>(view)->get_buffer();
    return SPLA_STATUS_OK;
}
spla_Status spla_MemView_get_size(spla_MemView view, spla_size_t* size) {
    *size = as_ptr<spla::MemView>(view)->get_size();
    return SPLA_STATUS_OK;
}
spla_Status spla_MemView_is_mutable(spla_MemView view, spla_bool* is_mutable) {
    *is_mutable = as_ptr<spla::MemView>(view)->is_mutable();
    return SPLA_STATUS_OK;
}