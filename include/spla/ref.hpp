/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/JetBrains-Research/spla                                     */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2021-2022 JetBrains-Research                                     */
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

#ifndef SPLA_REF_HPP
#define SPLA_REF_HPP

#include <atomic>
#include <cassert>

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class RefCnt
     * @brief Base class for object with built-in reference counting mechanism
     *
     * Inherit from this class to have shared-ref logic for your class objects.
     * Use RefPtr to wrap and automate RefCnt objects references counting.
     *
     * @note Used primary to marshal object handles in wrapping packages
     *
     * @see ref_ptr
     */
    class RefCnt {
    public:
        virtual ~RefCnt() {
#ifdef SPLA_DEBUG
            assert(m_refs.load() == 0);
            m_refs.store(0);
#endif
        }

        bool is_unique() const {
            return get_refs() == 1;
        }

        std::int32_t get_refs() const {
            return m_refs.load(std::memory_order_relaxed);
        }

        std::int32_t add_ref() const {
            assert(get_refs() >= 0);
            return m_refs.fetch_add(1);
        }

        std::int32_t rel_ref() const {
            assert(get_refs() > 0);
            auto refs = m_refs.fetch_sub(1);

            if (refs == 1) {
                // Was last reference
                // Destroy object and release memory
                delete this;
            }

            return refs;
        }

    private:
        // This type of object after creation always has no reference
        mutable std::atomic_int32_t m_refs{0};
    };

    template<typename T>
    static inline T *safe_ref(T *object) {
        if (object)
            object->add_ref();
        return object;
    }

    template<typename T>
    static inline void unref(T *object) {
        if (object)
            object->rel_ref();
    }

    /**
     * @class ref_ptr
     * @brief Automates reference counting and behaves as shared smart pointer.
     *
     * @note Naming is similar to std shader pointer class
     *
     * @tparam T Type referenced object
     */
    template<typename T>
    class ref_ptr {
    public:
        ref_ptr() = default;

        explicit ref_ptr(T *object) {
            m_object = safe_ref(object);
        }

        ref_ptr(const ref_ptr &other) {
            m_object = safe_ref(other.m_object);
        }

        ref_ptr(ref_ptr &&other) noexcept {
            m_object       = other.m_object;
            other.m_object = nullptr;
        }
        ~ref_ptr() {
            unref(m_object);
            m_object = nullptr;
        }

        ref_ptr<T> &operator=(const ref_ptr &other) {
            if (this != &other)
                this->reset(safe_ref(other.get()));
            return *this;
        }

        template<typename G>
        ref_ptr<T> &operator=(const ref_ptr<G> &other) {
            if (get() != other.get())
                this->reset(safe_ref(other.get()));
            return *this;
        }

        ref_ptr<T> &operator=(ref_ptr &&other) noexcept {
            if (this != &other)
                this->reset(other.release());
            return *this;
        }

        bool operator==(const ref_ptr &other) const {
            return m_object == other.m_object;
        }

        [[nodiscard]] bool is_null() const {
            return m_object == nullptr;
        }

        [[nodiscard]] bool is_not_null() const {
            return m_object;
        }

        T *operator->() const {
            assert(m_object);
            return m_object;
        }

        T &operator*() const {
            assert(m_object);
            return *m_object;
        }

        explicit operator bool() const {
            return m_object != nullptr;
        }

        void acquire(T *safe_ref_ptr = nullptr) {
            reset(safe_ref(safe_ref_ptr));
        }

        void reset(T *ptr = nullptr) {
            T *old   = m_object;
            m_object = ptr;
            unref(old);
        }

        T *release() {
            T *ptr   = m_object;
            m_object = nullptr;
            return ptr;
        }

        T *get() const {
            return m_object;
        }

        template<typename G>
        [[nodiscard]] bool is() const {
            return !m_object || dynamic_cast<G *>(m_object);
        }

        template<class G>
        ref_ptr<G> as() const {
            return ref_ptr<G>(m_object);
        }

        template<class G>
        ref_ptr<G> cast() const {
            return ref_ptr<G>(dynamic_cast<G *>(m_object));
        }

    private:
        T *m_object = nullptr;
    };

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_REF_HPP
