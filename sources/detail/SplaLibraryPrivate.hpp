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

#ifndef SPLA_SPLALIBRARYPRIVATE_HPP
#define SPLA_SPLALIBRARYPRIVATE_HPP

#include <spla-cpp/SplaLibrary.hpp>
#include <spla-cpp/SplaDescriptor.hpp>
#include <taskflow/taskflow.hpp>
#include <expression/SplaExpressionManager.hpp>

namespace spla {

    /**
     * @class LibraryPrivate
     * Private library state, accessible for all objects within library
     */
    class LibraryPrivate {
    public:
        explicit LibraryPrivate(Library& library);

        tf::Executor& GetTaskFlowExecutor() {
            return mExecutor;
        }

        const RefPtr<Descriptor> &GetDefaultDesc() {
            return mDefaultDesc;
        }

        const RefPtr<ExpressionManager> &GetExprManager() {
            return mExprManager;
        }

    private:
        tf::Executor mExecutor;
        RefPtr<Descriptor> mDefaultDesc;
        RefPtr<ExpressionManager> mExprManager;
    };

}

#endif //SPLA_SPLALIBRARYPRIVATE_HPP