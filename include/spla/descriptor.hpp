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
        ~Descriptor() override = default;

        void set_push_only(bool value) { push_only = value; }
        void set_pull_only(bool value) { pull_only = value; }
        void set_push_pull(bool value) { push_pull = value; }
        void set_front_factor(float value) { front_factor = value; }
        void set_discovered_factor(float value) { discovered_factor = value; }
        void set_early_exit(bool value) { early_exit = value; }

        bool  get_push_only() const { return push_only; }
        bool  get_pull_only() const { return pull_only; }
        bool  get_push_pull() const { return push_pull; }
        float get_front_factor() const { return front_factor; }
        float get_discovered_factor() const { return discovered_factor; }
        bool  get_early_exit() const { return early_exit; }

        void               set_label(std::string label) override;
        const std::string& get_label() const override;

    private:
        std::string m_label;

        bool  push_only         = false;
        bool  pull_only         = false;
        bool  push_pull         = true;
        float front_factor      = 0.1f;
        float discovered_factor = 0.7f;
        bool  early_exit        = false;
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
