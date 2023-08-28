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

#ifndef SPLA_MEMVIEW_HPP
#define SPLA_MEMVIEW_HPP

#include "config.hpp"
#include "ref.hpp"

namespace spla {

    /**
     * @class MemView
     * @brief View to some memory resource without life-time control
     *
     * Memory view allows to inspect raw structure of a particular buffer without
     * explicit control over life time of without costly copy operation to
     * move data from one place into another. Use views to access data and then to
     * copy values or fill values from your own data source.
     *
     * @note Memory view does not control type and layout of data. Thus, be
     *       cautions when modifying data through the view.
     */
    class MemView final : public RefCnt {
    public:
        SPLA_API ~MemView() override = default;
        SPLA_API Status read(std::size_t offset, std::size_t size, void* dst);
        SPLA_API Status write(std::size_t offset, std::size_t size, const void* src);
        SPLA_API void*  get_buffer() const;
        SPLA_API std::size_t get_size() const;
        SPLA_API bool        is_mutable() const;

        SPLA_API static ref_ptr<MemView> make(void* buffer, std::size_t size, bool is_mutable = false);
        SPLA_API static ref_ptr<MemView> make();

    private:
        void*       m_buffer     = nullptr;
        std::size_t m_size       = 0;
        bool        m_is_mutable = false;
    };

}// namespace spla

#endif//SPLA_MEMVIEW_HPP
