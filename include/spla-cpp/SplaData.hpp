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

#ifndef SPLA_SPLADATA_HPP
#define SPLA_SPLADATA_HPP

#include <spla-cpp/SplaObject.hpp>
#include <functional>

namespace spla {

    /** Generic data container to pass to/from CPU user host-data (with autorelease feature) */
    class SPLA_API Data: public Object {
    public:
        Data(ObjectType dataType, class Library& library);
        ~Data() override = default;

        using ReleaseProc = std::function<void(void*)>;
        void SetReleaseProc(ReleaseProc releaseProc);

    protected:
        ReleaseProc mReleaseProc;
    };

    /** Matrix ram coo data */
    class SPLA_API DataMatrix final: public Data {
    public:
        explicit DataMatrix(class Library& library);
        ~DataMatrix() override;

        void SetRows(unsigned int* rows);
        void SetCols(unsigned int* cols);
        void SetValues(void* values);

        unsigned int* GetRows() const;
        unsigned int* GetCols() const;
        void* GetValues() const;

    private:
        ReleaseProc mReleaseProc;
        unsigned int* mRows = nullptr;
        unsigned int* mCols = nullptr;
        void* mValues = nullptr;
    };

    /** Vector ram coo data */
    class SPLA_API DataVector final: public Data {
    public:
        explicit DataVector(class Library& library);
        ~DataVector() override;

        void SetRows(unsigned int* rows);
        void SetValues(void* values);

        unsigned int* GetRows() const;
        void* GetValues() const;

    private:
        ReleaseProc mReleaseProc;
        unsigned int* mRows = nullptr;
        void* mValues = nullptr;
    };

    /** Scalar ram data */
    class SPLA_API DataScalar final: public Data {
    public:
        explicit DataScalar(class Library& library);
        ~DataScalar() override;

        void SetValue(void* value);
        void* GetValue() const;

    private:
        ReleaseProc mReleaseProc;
        void* mValue = nullptr;
    };

}

#endif //SPLA_SPLADATA_HPP
