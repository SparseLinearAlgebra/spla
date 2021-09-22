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

#include <spla-cpp/SplaDescriptor.hpp>

spla::Descriptor::Descriptor(class spla::Library &library) : Object(Object::TypeName::Descriptor, library) {

}

void spla::Descriptor::SetParam(spla::Descriptor::Param param, std::wstring value) {
    mParams.emplace(param, std::move(value));
}

bool spla::Descriptor::GetParam(spla::Descriptor::Param param, std::wstring &value) const {
    auto query = mParams.find(param);

    if (query != mParams.end()) {
        value = query->second;
        return true;
    }

    return false;
}

bool spla::Descriptor::IsParamSet(spla::Descriptor::Param param) const {
    return mParams.find(param) != mParams.end();
}

spla::RefPtr<spla::Descriptor> spla::Descriptor::Make(class spla::Library &library) {
    return spla::RefPtr<spla::Descriptor>(new Descriptor(library));
}
