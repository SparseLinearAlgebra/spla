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

    /**
     * @class Data
     * Generic data container to pass to/from CPU user host-data (with autorelease feature).
     */
    class SPLA_API Data: public Object {
    public:
        Data(Object::TypeName dataType, class Library& library);
        ~Data() override = default;

        /** Release proc used to release user data pointers */
        using ReleaseProc = std::function<void(void*)>;

        /**
         * Set release proc, used to deallocate data raw memory pointers, provided by user.
         * If no release proc set, the data won't be deallocated.
         *
         * @param releaseProc Function to deallocate data buffers memory
         */
        void SetReleaseProc(ReleaseProc releaseProc);

    protected:
        ReleaseProc mReleaseProc;
    };

    /**
     * @class DataMatrix
     * Matrix ram coo data.
     */
    class SPLA_API DataMatrix final: public Data {
    public:
        explicit DataMatrix(class Library& library);
        ~DataMatrix() override;

        /**
         * Set row indices buffer pointer.
         * @note Pass null, if this data must be ignored in `read` operations.
         * @param rows Data pointer
         */
        void SetRows(unsigned int* rows);

        /**
         * Set column indices buffer pointer.
         * @note Pass null, if this data must be ignored in `read` operations.
         * @param rows Data pointer
         */
        void SetCols(unsigned int* cols);

        /**
         * Set values buffer pointer.
         * @note Pass null, if this data must be ignored in `read` operations.
         * @param rows Data pointer
         */
        void SetValues(void* values);

        /**
         * Set values count in this data storage.
         * @param nvals Number of values (tuples of data).
         */
        void SetNvals(size_t nvals);

        /** @return Row indices buffer */
        unsigned int* GetRows() const;

        /** @return Column indices buffer */
        unsigned int* GetCols() const;

        /**@return Values buffer */
        void* GetValues() const;

        /** @return Number of entries in data */
        size_t GetNvals() const;

    private:
        unsigned int* mRows = nullptr;
        unsigned int* mCols = nullptr;
        void* mValues = nullptr;
        size_t mNvals = 0;
    };

    /**
     * @class DataVector
     * Vector ram coo data.
     */
    class SPLA_API DataVector final: public Data {
    public:
        explicit DataVector(class Library& library);
        ~DataVector() override;

        /**
         * Set row indices buffer pointer.
         * @note Pass null, if this data must be ignored in `read` operations.
         * @param rows Data pointer
         */
        void SetRows(unsigned int* rows);

        /**
         * Set values buffer pointer.
         * @note Pass null, if this data must be ignored in `read` operations.
         * @param rows Data pointer
         */
        void SetValues(void* values);

        /**
         * Set values count in this data storage.
         * @param nvals Number of values (tuples of data).
         */
        void SetNvals(size_t nvals);

        /** @return Row indices buffer */
        unsigned int* GetRows() const;

        /**@return Values buffer */
        void* GetValues() const;

        /** @return Number of entries in data */
        size_t GetNvals() const;

    private:
        unsigned int* mRows = nullptr;
        void* mValues = nullptr;
        size_t mNvals = 0;
    };

    /**
     * @class DataScalar
     * Scalar ram data
     */
    class SPLA_API DataScalar final: public Data {
    public:
        explicit DataScalar(class Library& library);
        ~DataScalar() override;

        /**
         * Set value buffer.
         * @param value Data pointer
         */
        void SetValue(void* value);

        /** @return Scalar values */
        void* GetValue() const;

    private:
        void* mValue = nullptr;
    };

}

#endif //SPLA_SPLADATA_HPP
