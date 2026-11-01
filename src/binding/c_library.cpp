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
#include <core/configure.hpp>

#include <iostream>

void spla_Library_finalize() {
    spla::Library::get()->finalize();
}
spla_Status spla_Configure_and_init(int argc, char** argv) {
    auto* lib = spla::Library::get(argc, argv);
    if (!lib) return SPLA_STATUS_ERROR;
    // Report the real configuration status: on failure the library continues
    // with pure CPU computations, but the caller must be able to detect it.
    return to_c_status(lib->configure_status());
}

void spla_Config_help() {
    char  arg0[] = "spla";
    char  arg1[] = "--spla-help";
    char* argv[] = {arg0, arg1, nullptr};
    spla::make_config_final(2, argv);
}

void spla_Config_version() {
    std::cout << spla::get_spla_version() << std::endl;
}
spla_Status spla_Library_set_accelerator(spla_AcceleratorType accelerator) {
    return to_c_status(spla::Library::get()->set_accelerator(from_c_accelerator_type(accelerator)));
}

spla_Status spla_Library_set_platform(int index) {
    return to_c_status(spla::Library::get()->set_platform(index));
}

spla_Status spla_Library_set_device(int index) {
    return to_c_status(spla::Library::get()->set_device(index));
}

spla_Status spla_Library_set_queues_count(int count) {
    return to_c_status(spla::Library::get()->set_queues_count(count));
}

spla_Status spla_Library_set_message_callback(spla_MessageCallback callback, void* p_user_data) {
    auto wrapped_callback = [=](spla::Status       status,
                                const std::string& msg,
                                const std::string& file,
                                const std::string& function,
                                int                line) {
        callback(to_c_status(status),
                 msg.c_str(),
                 file.c_str(),
                 function.c_str(),
                 line,
                 p_user_data);
    };
    return to_c_status(spla::Library::get()->set_message_callback(wrapped_callback));
}

spla_Status spla_Library_set_default_callback() {
    return to_c_status(spla::Library::get()->set_default_callback());
}

spla_Status spla_Library_get_accelerator_info(char* buffer, int length) {
    std::string info;

    auto status = spla::Library::get()->get_accelerator_info(info);

    if (length > 0) {
        std::memcpy(buffer, info.c_str(), std::min(length, int(info.length())));
        buffer[length - 1] = '\0';
    }

    return to_c_status(status);
}
