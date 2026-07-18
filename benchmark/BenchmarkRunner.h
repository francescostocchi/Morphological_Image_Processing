#pragma once
#include <vector>
#include <string>
#include "../src/DataProcessing.cpp"

class BenchmarkRunner {
public:
    bool run(const std::vector<Benchmark>& all_benchmarks, const std::string& run_folder);

private:
    bool compare_images(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b, int w, int h);

    bool test_outputs(const std::vector<Generated>& serial, const ParallelResult& pr, const std::string& logs_dir);

    void save_mismatch(const std::string& logs_dir, const std::string& comparison, int cores, size_t idx, const Generated& a, const Generated& b);
};
