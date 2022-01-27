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

#ifndef SPLA_SPLAVECTORDENSE_H
#define SPLA_SPLAVECTORDENSE_H

#include <boost/compute.hpp>
#include <storage/SplaVectorBlock.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    class VectorDense final : public VectorBlock {
    public:
        using Mask = boost::compute::vector<Index>;
        using Values = boost::compute::vector<unsigned char>;

        ~VectorDense() override = default;

        [[nodiscard]] const Mask &GetMask() const noexcept;

        [[nodiscard]] const Values &GetVals() const noexcept;

        std::size_t GetMemoryUsage() const override;

        void Dump(std::ostream &stream, unsigned int baseI) const override;

        static RefPtr<VectorDense> Make(std::size_t nrows, std::size_t nvals, Mask mask, Values vals);

    private:
        VectorDense(std::size_t nrows, std::size_t nvals, Mask mask, Values vals);

        Mask mMask;
        Values mVals;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAVECTORDENSE_H
