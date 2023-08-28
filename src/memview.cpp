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

#include "spla/memview.hpp"

#include <cstring>

namespace spla {

    Status MemView::read(std::size_t offset, std::size_t size, void* dst) {
        if (offset + size > m_size) {
            return Status::InvalidArgument;
        }
        std::memcpy(dst, (char*) m_buffer + offset, size);
        return Status::Ok;
    }
    Status MemView::write(std::size_t offset, std::size_t size, const void* src) {
        if (!m_is_mutable) {
            return Status::InvalidState;
        }
        if (offset + size > m_size) {
            return Status::InvalidArgument;
        }
        std::memcpy((char*) m_buffer + offset, src, size);
        return Status::Ok;
    }
    void* MemView::get_buffer() const {
        return m_buffer;
    }
    std::size_t MemView::get_size() const {
        return m_size;
    }
    bool MemView::is_mutable() const {
        return m_is_mutable;
    }
    ref_ptr<MemView> MemView::make(void* buffer, std::size_t size, bool is_mutable) {
        auto view          = make_ref<MemView>();
        view->m_buffer     = buffer;
        view->m_size       = size;
        view->m_is_mutable = is_mutable;
        return view;
    }
    ref_ptr<MemView> MemView::make() {
        return make_ref<MemView>();
    }

}// namespace spla