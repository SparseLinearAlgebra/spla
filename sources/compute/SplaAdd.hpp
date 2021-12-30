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
#ifndef SPLA_SPLAADD_HPP
#define SPLA_SPLAADD_HPP

#include <boost/compute.hpp>
#include <boost/compute/detail/meta_kernel.hpp>

#include <compute/metautil/SplaMetaUtil.hpp>


namespace spla {

    inline boost::compute::vector<unsigned char> Add(const boost::compute::vector<unsigned char> &a,
                                                     const boost::compute::vector<unsigned char> &b,
                                                     std::size_t resultByteSize,
                                                     const std::string &addBody,
                                                     boost::compute::command_queue &queue) {
        using namespace boost;
        using namespace detail::meta;

        compute::detail::meta_kernel k("spla_scalar_add");

        FunctionApplication add(k, "spla_scalar_add_func", addBody,
                                Visibility::Global,
                                Visibility::Global,
                                Visibility::Global);
        compute::vector<unsigned char> result(resultByteSize, queue.get_context());
        compute::fill(result.begin(), result.end(), 0, queue);// TODO: Remove it maybe?
        k << add.Apply(ValArrItem(a, "0", a.size(), k), ValArrItem(b, "0", b.size(), k), ValArrItem(result, "0", resultByteSize, k));
        k.exec(queue);
        return result;
    }

}// namespace spla

#endif//SPLA_SPLAADD_HPP
