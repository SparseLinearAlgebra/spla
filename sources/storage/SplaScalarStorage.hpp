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

#ifndef SPLA_SPLASCALARSTORAGE_HPP
#define SPLA_SPLASCALARSTORAGE_HPP

#include <mutex>
#include <spla-cpp/SplaLibrary.hpp>

namespace spla {

    /**
     * @addtogroup Internal
     * @{
     */

    /**
     * @class ScalarStorage
     */
    class ScalarStorage final : public RefCnt {
    public:
        ~ScalarStorage() override;

        /** Set scalar value */
        void SetValue(const RefPtr<class ScalarValue> &value);

        /** Removes scalar value if present */
        void RemoveValue();

        /** @return True if stores value */
        bool HasValue() const;

        /** @return Returns scalar value; may be null if not presented */
        RefPtr<class ScalarValue> GetValue() const;

        /** Dump scalar content to provided stream */
        void Dump(std::ostream &stream) const;

        /** @return Cloned `cow` storage */
        RefPtr<ScalarStorage> Clone() const;

        static RefPtr<ScalarStorage> Make(Library &library);

    private:
        explicit ScalarStorage(Library &library);

        RefPtr<class ScalarValue> mValue;

        Library &mLibrary;
        mutable std::mutex mMutex;
    };

    /**
     * @}
     */

}// namespace spla


#endif//SPLA_SPLASCALARSTORAGE_HPP
