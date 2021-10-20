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

#include <algo/vector/SplaVectorAssignCOO.hpp>
#include <storage/block/SplaVectorCOO.hpp>

bool spla::VectorAssignCOO::Select(const spla::AlgorithmParams &params) const {
    auto p = dynamic_cast<const ParamsVectorAssign *>(&params);

    return p &&
           p->output.Is<VectorCOO>() &&
           p->input.Is<VectorCOO>() &&
           (p->mask.IsNull() || p->mask.Is<VectorCOO>());
}

void spla::VectorAssignCOO::Process(spla::AlgorithmParams &params) {
    auto p = dynamic_cast<const ParamsVectorAssign *>(&params);
    assert(p);


}

spla::Algorithm::Type spla::VectorAssignCOO::GetType() const {
    return Type::VectorAssign;
}

std::string spla::VectorAssignCOO::GetName() const {
    return "VectorAssignCOO";
}
