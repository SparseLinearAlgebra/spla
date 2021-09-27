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

#ifndef SPLA_SPLASVM_HPP
#define SPLA_SPLASVM_HPP

#include <boost/compute/svm.hpp>
#include <detail/SplaLibraryPrivate.hpp>
#include <spla-cpp/SplaLibrary.hpp>
#include <spla-cpp/SplaRefCnt.hpp>

namespace spla {

    /**
     * @class Svm
     *
     * Reference counted shared pointer wrapper around boost::compute svm pointer.
     * Uses Library global context for memory allocation.
     * Automatically releases svm memory when no references to this object.
     *
     * @tparam T Type of the elements, pointed by the svm pointer.
     */
    template<typename T>
    class Svm final : public RefCnt {
    public:
        using Ptr = boost::compute::svm_ptr<T>;

        ~Svm() override {
            assert(mPtr);
            auto &ctx = mPtr.get_context();
            boost::compute::svm_free(ctx, mPtr);
        }

        [[nodiscard]] Ptr Get() const {
            return mPtr;
        }

        [[nodiscard]] static RefPtr<Svm<T>> Make(size_t size, Library &library, cl_svm_mem_flags flags = CL_MEM_READ_WRITE) {
            assert(size);
            auto &state = library.GetPrivate();
            auto &ctx = state.GetContext();
            Ptr ptr = boost::compute::svm_alloc<T>(ctx, size, flags);
            return RefPtr<Svm<T>>(new Svm<T>(ptr));
        }

    private:
        explicit Svm(Ptr ptr) : mPtr(ptr) {}

        Ptr mPtr;
    };

}// namespace spla

#endif//SPLA_SPLASVM_HPP
