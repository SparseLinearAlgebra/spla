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

#include <atomic>
#include <cassert>
#include <spla-cpp/SplaConfig.hpp>

namespace spla {

    /**
     * @addtogroup API
     * @{
     */

    /**
     * @class RefCnt
     *
     * Inherit from this class to have shared-ref logic for your class objects.
     * Use RefPtr to wrap and automate RefCnt objects references counting.
     *
     * @see RefPtr
     */
    class SPLA_API RefCnt {
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
            assert(GetRefs() >= 0);
            return mRefs.fetch_add(1);
        }

        int32_t RelRef() const {
            assert(GetRefs() > 0);
            auto refs = mRefs.fetch_sub(1);

            if (refs == 1) {
                // Was last reference
                // Destroy object and release memory
                delete this;
            }

            return refs;
        }

    private:
        // This type of object after creation always has no reference
        mutable std::atomic_int32_t mRefs{0};
    };

    template<typename T>
    static inline T *Ref(T *object) {
        assert(object);
        object->AddRef();
        return object;
    }

    template<typename T>
    static inline T *SafeRef(T *object) {
        if (object)
            object->AddRef();
        return object;
    }

    template<typename T>
    static inline void Unref(T *object) {
        if (object)
            object->RelRef();
    }

    /**
     * @class RefPtr
     * Automates reference counting and behaves as shared smar pointer.
     * @tparam T Type referenced object
     */
    template<typename T>
    class RefPtr {
    public:
        RefPtr() = default;
        RefPtr(T *object) {
            if (object)
                mObject = Ref(object);
        }
        RefPtr(const RefPtr &other) {
            if (other.mObject)
                mObject = Ref(other.mObject);
        }
        RefPtr(RefPtr &&other) noexcept {
            mObject = other.mObject;
            other.mObject = nullptr;
        }
        ~RefPtr() {
            Unref(mObject);
            mObject = nullptr;
        }

        RefPtr<T> &operator=(const RefPtr &other) {
            if (this != &other)
                this->Reset(SafeRef(other.Get()));
            return *this;
        }

        RefPtr<T> &operator=(RefPtr &&other) noexcept {
            if (this != &other)
                this->Reset(other.Release());
            return *this;
        }

        bool operator==(const RefPtr &other) const {
            return mObject == other.mObject;
        }

        [[nodiscard]] bool IsNull() const {
            return mObject == nullptr;
        }

        [[nodiscard]] bool IsNotNull() const {
            return mObject;
        }

        T *operator->() const {
            assert(mObject);
            return mObject;
        }

        T &operator*() const {
            assert(mObject);
            return *mObject;
        }

        explicit operator bool() const {
            return mObject != nullptr;
        }

        void Reset(T *ptr = nullptr) {
            T *old = mObject;
            mObject = ptr;
            Unref(old);
        }

        T *Release() {
            T *ptr = mObject;
            mObject = nullptr;
            return ptr;
        }

        T *Get() const {
            return mObject;
        }

        template<class G>
        RefPtr<G> As() const {
            return RefPtr<G>(mObject);
        }

        template<class G>
        RefPtr<G> Cast() const {
            return RefPtr<G>(dynamic_cast<G *>(mObject));
        }

    private:
        T *mObject = nullptr;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_SPLAREFCNT_HPP
