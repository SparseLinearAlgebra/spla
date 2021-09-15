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

#ifndef SPLA_SPLAREFCNT_HPP
#define SPLA_SPLAREFCNT_HPP

#include <cassert>
#include <atomic>

namespace spla {

    /**
     * @brief Reference counted object.
     *
     * Inherit from this class to have shared-ref logic for your class objects.
     * Use RefPtr to wrap and automate RefCnt objects references counting.
     *
     * @see RefPtr
     */
    class RefCnt {
    public:

        virtual ~RefCnt() {
#ifdef SPLA_DEBUG
            assert(mRefs.load() == 0);
            mRefs.store(0);
#endif
        }

        bool IsUnique() const {
            return GetRefs() == 1;
        }

        int32_t GetRefs() const {
            return mRefs.load(std::memory_order_relaxed);
        }

        int32_t AddRef() const {
            assert(GetRefs() > 0);
            return mRefs.fetch_add(1, std::memory_order_acq_rel);
        }

        int32_t RelRef() const {
            assert(GetRefs() > 0);
            auto refs = mRefs.fetch_sub(1, std::memory_order_acq_rel);

            if (refs == 1) {
                // Was last reference
                // Destroy object and release memory
                delete this;
            }

            return refs;
        }

    private:
        // This type of object after creation always has at least 1 reference
        mutable std::atomic_int32_t mRefs{1};
    };

    template<typename T>
    static inline T* Ref(T* object) {
        static_assert(std::is_base_of<RefCnt, T>::value, "T must be derived from RefCnt");
        assert(object);
        object->AddRef();
        return object;
    }

    template<typename T>
    static inline T* SafeRef(T* object) {
        static_assert(std::is_base_of<RefCnt, T>::value, "T must be derived from RefCnt");
        if (object)
            object->AddRef();
        return object;
    }

    template<typename T>
    static inline void Unref(T* object) {
        static_assert(std::is_base_of<RefCnt, T>::value, "T must be derived from RefCnt");
        if (object)
            Unref(object);
    }

    template<typename T>
    class RefPtr {
    public:
        static_assert(std::is_base_of<RefCnt, T>::value, "T must be derived from RefCnt");

        RefPtr() = default;
        explicit RefPtr(T* object) {
            if (object)
                mObject = Ref(object);
        }
        RefPtr(const RefPtr& other) {
            if (other.mObject)
                mObject = Ref(other.mObject);
        }
        RefPtr(RefPtr&& other) noexcept {
            mObject = other.mObject;
            other.mObject = nullptr;
        }
        ~RefPtr() {
            Unref(mObject);
            mObject = nullptr;
        }

        RefPtr<T> &operator=(const RefPtr& other) {
            if (this != &other)
                this->Reset(SafeRef(other.Get()));
            return *this;
        }

        RefPtr<T> &operator=(RefPtr &&other) noexcept {
            if (this != &other)
                this->Reset(other.Release());
            return *this;
        }

        explicit operator bool() {
            return mObject != nullptr;
        }

        void Reset(T* ptr = nullptr) {
            T* old = mObject;
            mObject = ptr;
            Unref(old);
        }

        T* Release() {
            T* ptr = mObject;
            mObject = nullptr;
            return ptr;
        }

        T* Get() const {
            return mObject;
        }

    private:
        T* mObject = nullptr;
    };

}

#endif //SPLA_SPLAREFCNT_HPP
