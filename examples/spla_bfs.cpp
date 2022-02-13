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

#include <cxxopts.hpp>

#include <spla-algo/SplaAlgo.hpp>
#include <spla-cpp/Spla.hpp>

int main(int argc, const char *const *argv) {
    cxxopts::Options options("spla-bfs", "BFS (breadth first search) algorithm with spla library");
    options.add_option("", cxxopts::Option("h,help", "display help info", cxxopts::value<bool>()->default_value("false")));
    options.add_option("", cxxopts::Option("mtxpath", "path to matrix file", cxxopts::value<std::string>()));
    options.add_option("", cxxopts::Option("niters", "number of iterations to run", cxxopts::value<int>()->default_value("4")));
    options.add_option("", cxxopts::Option("source", "source vertex to run bfs", cxxopts::value<int>()->default_value("0")));
    options.add_option("", cxxopts::Option("bsize", "size of block to store matrix/vector", cxxopts::value<int>()->default_value("10000000")));
    options.add_option("", cxxopts::Option("undirected", "force graph to be undirected", cxxopts::value<bool>()->default_value("false")));
    options.add_option("", cxxopts::Option("devices-count", "amount of devices for execution", cxxopts::value<int>()->default_value("1")));
    options.add_option("", cxxopts::Option("verbose", "verbose std output", cxxopts::value<bool>()->default_value("true")));
    options.add_option("", cxxopts::Option("platform", "opencl platform to select", cxxopts::value<std::string>()->default_value("")));
    options.add_option("", cxxopts::Option("dense-factor", "factor used to dense transition", cxxopts::value<float>()->default_value("0.1f")));
    options.add_option("", cxxopts::Option("debug-timing", "timing for each iteration of algorithm", cxxopts::value<bool>()->default_value("false")));
    cxxopts::ParseResult args;

    try {
        args = options.parse(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << "Failed parse input arguments: " << e.what();
        return 1;
    }

    if (args["help"].as<bool>()) {
        std::cout << options.help();
        return 0;
    }

    std::string mtxpath;
    int niters;
    int source;
    int bsize;
    int devicesCount;
    bool undirected;
    bool removeLoops = true;
    bool ignoreValues = true;
    bool verbose;
    bool debugTiming;
    float denseFactor;
    std::string platform;

    try {
        mtxpath = args["mtxpath"].as<std::string>();
        niters = args["niters"].as<int>();
        source = args["source"].as<int>();
        bsize = args["bsize"].as<int>();
        devicesCount = args["devices-count"].as<int>();
        undirected = args["undirected"].as<bool>();
        verbose = args["verbose"].as<bool>();
        debugTiming = args["debug-timing"].as<bool>();
        denseFactor = args["dense-factor"].as<float>();
        platform = args["platform"].as<std::string>();
    } catch (const std::exception &e) {
        std::cerr << "Invalid input arguments: " << e.what();
        return 1;
    }

    assert(niters > 0);
    assert(source >= 0);
    assert(bsize > 1);
    assert(devicesCount >= 1);

    // Load data
    spla::MatrixLoader<std::int32_t> loader;

    try {
        loader.Load(mtxpath, undirected, removeLoops, ignoreValues, verbose);
    } catch (const std::exception &e) {
        std::cerr << "Failed load matrix: " << e.what();
        return 1;
    }

    assert(loader.GetNrows() == loader.GetNcols());

    // Configure library
    spla::Library::Config config;
    config.SetDeviceType(spla::Library::Config::DeviceType::GPU);
    config.SetBlockSize(bsize);
    config.SetWorkersCount(devicesCount * devicesCount);
    config.LimitAmount(devicesCount);
    config.SetPlatform(platform);
    spla::Library library(config);

    // Output info about devices for evaluation
    if (verbose)
        std::cout << library.PrintContextConfig() << std::endl;

    // v and M bfs args
    spla::RefPtr<spla::Vector> v;
    spla::RefPtr<spla::Matrix> M = spla::Matrix::Make(loader.GetNrows(), loader.GetNcols(), spla::Types::Void(library), library);

    // Data is sorted without duplicated values
    spla::RefPtr<spla::Descriptor> dataDesc = spla::Descriptor::Make(library);
    dataDesc->SetParam(spla::Descriptor::Param::ValuesSorted);
    dataDesc->SetParam(spla::Descriptor::Param::NoDuplicates);

    // Prepare data and fill M
    spla::RefPtr<spla::Expression> prepareData = spla::Expression::Make(library);
    prepareData->MakeDataWrite(M, spla::DataMatrix::Make(loader.GetRowIndices().data(), loader.GetColIndices().data(), nullptr, loader.GetNvals(), library), dataDesc);
    prepareData->SubmitWait();
    SPLA_ALGO_CHECK(prepareData);

    spla::RefPtr<spla::Descriptor> desc = spla::Descriptor::Make(library);
    desc->SetParam(spla::Descriptor::Param::DeviceFixedStrategy, true);// Force fixed device allocation for all blocks
    desc->SetParam(spla::Descriptor::Param::ProfileTime, debugTiming); // Show timing per iteration and tight measurements
    desc->SetParamT(spla::Descriptor::Param::DenseFactor, denseFactor);// When transition sparse to dense v depth

    // Warm up phase
    spla::CpuTimer tWarmUp;
    tWarmUp.Start();
    spla::Bfs(v, M, source, desc);
    tWarmUp.Stop();

    // Main phase, measure iterations
    std::vector<spla::CpuTimer> tIters(niters);
    for (int i = 0; i < niters; i++) {
        tIters[i].Start();
        spla::Bfs(v, M, source, desc);
        tIters[i].Stop();
    }

    spla::OutputMeasurements(tWarmUp, tIters);
    return 0;
}