/**********************************************************************************/
/* This file is part of spla project                                              */
/* https://github.com/SparseLinearAlgebra/spla                                    */
/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2023 SparseLinearAlgebra                                         */
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

#ifndef SPLA_DESCRIPTOR_HPP
#define SPLA_DESCRIPTOR_HPP

#include "object.hpp"

namespace spla {

    /**
     * @addtogroup spla
     * @{
     */

    /**
     * @class Descriptor
     * @brief Descriptor object used to parametrize execution of particular scheduled tasks
     */
    class Descriptor final : public Object {
    public:
        enum class TraversalMode {
            Push,
            Pull,
            PushPull
        };

        ~Descriptor() override = default;

        void set_traversal_mode(TraversalMode value) { mode = value; }
        void set_front_factor(float value) { front_factor = value; }
        void set_early_exit(bool value) { early_exit = value; }
        void set_struct_only(bool value) { struct_only = value; }

        bool  get_push_only() const { return mode == TraversalMode::Push; }
        bool  get_pull_only() const { return mode == TraversalMode::Pull; }
        bool  get_push_pull() const { return mode == TraversalMode::PushPull; }
        float get_front_factor() const { return front_factor; }
        bool  get_early_exit() const { return early_exit; }
        bool  get_struct_only() const { return struct_only; }

        void               set_label(std::string label) override;
        const std::string& get_label() const override;

    private:
        std::string m_label;

        TraversalMode mode         = TraversalMode::PushPull;
        float         front_factor = 0.1f;
        bool          early_exit   = false;
        bool          struct_only  = false;
    };

    /**
     * @brief Makes new empty descriptor object
     *
     * @return New descriptor
     */
    SPLA_API ref_ptr<Descriptor> make_desc();

    /**
     * @}
     */

}// namespace spla

#endif//SPLA_DESCRIPTOR_HPP
