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

#include <spla-cpp/SplaMatrix.hpp>
#include <storage/SplaMatrixStorage.hpp>

std::pair<std::size_t, std::size_t> spla::Matrix::GetDim() const {
    return {GetNrows(), GetNcols()};
}

std::size_t spla::Matrix::GetNrows() const {
    return mStorage->GetNrows();
}

std::size_t spla::Matrix::GetNcols() const {
    return mStorage->GetNcols();
}

std::size_t spla::Matrix::GetNvals() const {
    return mStorage->GetNvals();
}

const spla::RefPtr<spla::MatrixStorage> &spla::Matrix::GetStorage() const {
    return mStorage;
}

void spla::Matrix::Dump(std::ostream &stream) const {
    mStorage->Dump(stream);
}

spla::RefPtr<spla::Object> spla::Matrix::Clone() const {
    return RefPtr<Matrix>(new Matrix(GetNrows(), GetNcols(), GetType(), GetLibrary(), GetStorage()->Clone())).As<Object>();
}

spla::RefPtr<spla::Matrix> spla::Matrix::Make(std::size_t nrows, std::size_t ncols,
                                              const RefPtr<Type> &type,
                                              spla::Library &library) {
    return spla::RefPtr<spla::Matrix>(new Matrix(nrows, ncols, type, library));
}

spla::Matrix::Matrix(std::size_t nrows, std::size_t ncols,
                     const RefPtr<Type> &type,
                     spla::Library &library,
                     RefPtr<class MatrixStorage> storage) : TypedObject(type, Object::TypeName::Matrix, library) {
    mStorage = storage.IsNotNull() ? std::move(storage) : MatrixStorage::Make(nrows, ncols, GetLibrary());
}

spla::RefPtr<spla::Object> spla::Matrix::CloneEmpty() {
    return Make(GetNrows(), GetNcols(), GetType(), GetLibrary()).As<Object>();
}

void spla::Matrix::CopyData(const spla::RefPtr<spla::Object> &object) {
    auto matrix = dynamic_cast<const Matrix *>(object.Get());
    assert(matrix);

    if (matrix) {
        assert(GetNrows() == matrix->GetNrows());
        assert(GetNcols() == matrix->GetNcols());
        assert(GetType() == matrix->GetType());
        mStorage = matrix->GetStorage();
    }
}

spla::Matrix::~Matrix() = default;