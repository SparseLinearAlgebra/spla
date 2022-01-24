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

#include <core/SplaError.hpp>
#include <core/SplaLibraryPrivate.hpp>
#include <core/SplaTaskBuilder.hpp>
#include <spdlog/spdlog.h>

#ifdef SPLA_PROFILING
    #include <spla-cpp/SplaUtils.hpp>
#endif

tf::Task spla::TaskBuilder::Emplace(std::function<void()> work) {
    return Emplace("no-name", std::move(work));
}

tf::Task spla::TaskBuilder::Emplace(const std::string &workName, std::function<void()> work) {
    auto taskId = mTaskID++;
    auto task = [expression = mExpression, work = std::move(work), taskId, workName]() {
        // If some error occurred earlier, no sense to continue
        if (expression->GetState() == Expression::State::Aborted)
            return;

        try {
#ifdef SPLA_PROFILING
            CpuTimer timer;
            timer.Start();
#endif
            work();
#ifdef SPLA_PROFILING
            timer.Stop();
            std::cout << "[SplaTaskBuilder.cpp:53] task-" << taskId << " (" << workName << ") " << timer.GetElapsedMs() << " ms" << std::endl;
#endif
        } catch (std::exception &ex) {
            expression->SetState(Expression::State::Aborted);
            auto logger = expression->GetLibrary().GetPrivate().GetLogger();
            SPDLOG_LOGGER_ERROR(logger, "Error inside expression node task-{} ({}). {}", taskId, workName, ex.what());
        }
    };

    return mSubflow.emplace(std::move(task));
}

spla::TaskBuilder::TaskBuilder(spla::Expression *expression, tf::Subflow &subflow)
    : mExpression(expression), mSubflow(subflow) {
}