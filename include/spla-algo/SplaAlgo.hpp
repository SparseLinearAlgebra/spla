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

#ifndef SPLA_SPLAALGO_HPP
#define SPLA_SPLAALGO_HPP

/**
 * @defgroup Algorithm
 *
 * @brief Public library algorithms interface
 *
 * @details Algorithm module provides interface to comment graph
 * algorithms, such as breadth-first search (bfs), single source
 * shortest paths (ssps), triangles counting (tc), page rank,
 * connected components and etc. implemented both using
 * spla library API (matrix, vectors, expressions) for multi-GPU evaluation
 * and using standard C++ primitives (for reference and conformance checks only).
 *
 * This algorithms can be safely used in user applications.
 *
 * Implementation details are hidden in private Internal sources module.
 * Header files has no other dependencies, except standard c++ library files and core SPLA API interface.
 *
 * File SplaAlgo.hpp provides access to all algorithm module components.
 */

#include <spla-algo/SplaAlgoBfs.hpp>
#include <spla-algo/SplaAlgoCommon.hpp>

#endif//SPLA_SPLAALGO_HPP
