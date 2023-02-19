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

#include "c_config.hpp"

void spla_Library_finalize() {
    spla::Library::get()->finalize();
}

spla_Status spla_Library_set_accelerator(spla_AcceleratorType accelerator) {
    return spla::to_c_status(spla::Library::get()->set_accelerator(spla::from_c_accelerator_type(accelerator)));
}

spla_Status spla_Library_set_platform(int index) {
    return spla::to_c_status(spla::Library::get()->set_platform(index));
}

spla_Status spla_Library_set_device(int index) {
    return spla::to_c_status(spla::Library::get()->set_device(index));
}

spla_Status spla_Library_set_queues_count(int count) {
    return spla::to_c_status(spla::Library::get()->set_queues_count(count));
}

spla_Status spla_Library_set_message_callback(spla_MessageCallback callback, void* p_user_data) {
    auto wrapped_callback = [=](spla::Status       status,
                                const std::string& msg,
                                const std::string& file,
                                const std::string& function,
                                int                line) {
        callback(spla::to_c_status(status),
                 msg.c_str(),
                 file.c_str(),
                 function.c_str(),
                 line,
                 p_user_data);
    };
    return spla::to_c_status(spla::Library::get()->set_message_callback(wrapped_callback));
}

spla_Status spla_Library_set_default_callback() {
    return spla::to_c_status(spla::Library::get()->set_default_callback());
}