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

#include <spla-cpp/SplaDecorated.hpp>

#include <core/SplaError.hpp>

spla::Decorated::Decorated() {
    mDecorations.resize(static_cast<unsigned int>(Decoration::Max));
}

void spla::Decorated::SetDecoration(spla::Decorated::Decoration decoration,
                                    const spla::RefPtr<spla::Object> &object) {
    std::lock_guard<std::mutex> lockGuard(mMutex);
    CHECK_RAISE_ERROR(static_cast<unsigned int>(decoration) < static_cast<unsigned int>(Decoration::Max),
                      InvalidArgument, "Invalid decoration type");
    mDecorations[static_cast<int>(decoration)] = object;
}

void spla::Decorated::RemoveDecoration(spla::Decorated::Decoration decoration) {
    std::lock_guard<std::mutex> lockGuard(mMutex);
    CHECK_RAISE_ERROR(static_cast<unsigned int>(decoration) < static_cast<unsigned int>(Decoration::Max),
                      InvalidArgument, "Invalid decoration type");
    mDecorations[static_cast<int>(decoration)].Reset();
}

bool spla::Decorated::HasDecoration(spla::Decorated::Decoration decoration) const {
    std::lock_guard<std::mutex> lockGuard(mMutex);
    CHECK_RAISE_ERROR(static_cast<unsigned int>(decoration) < static_cast<unsigned int>(Decoration::Max),
                      InvalidArgument, "Invalid decoration type");
    return mDecorations[static_cast<int>(decoration)].IsNotNull();
}

spla::RefPtr<spla::Object> spla::Decorated::GetDecoration(spla::Decorated::Decoration decoration) const {
    std::lock_guard<std::mutex> lockGuard(mMutex);
    CHECK_RAISE_ERROR(static_cast<unsigned int>(decoration) < static_cast<unsigned int>(Decoration::Max),
                      InvalidArgument, "Invalid decoration type");
    return mDecorations[static_cast<int>(decoration)];
}