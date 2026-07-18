#include "BenchmarkRunner.h"
#include <iostream>
#include <filesystem>
#include "../src/utils/functions.h" 

using namespace std;

namespace fs = filesystem;

static const char* red = "\033[31m";
static const char* green = "\033[32m";
static const char* yellow = "\033[33m";
static const char* cyan = "\033[36m";
static const char* reset = "\033[0m";

// Compares two grayscale images pixel-by-pixel.
bool BenchmarkRunner::compare_images(const vector<unsigned char>& a, const vector<unsigned char>& b, int w, int h) {
    if (a.size() < static_cast<size_t>(w) * h || b.size() < static_cast<size_t>(w) * h) {
        return false;
    }
    for (int i = 0; i < w * h; i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}
// Saves two mismatching images to disk for debugging.
void BenchmarkRunner::save_mismatch(const string& logs_dir, const string& comparison, int cores, size_t idx, const Generated& a, const Generated& b) {
    fs::create_directories(logs_dir);

    string base = logs_dir + "/cores" + to_string(cores) + "_" + comparison + "_idx" + to_string(idx);

    writeFile(base + "_a.jpg", a.image.data, a.image.width, a.image.height);
    writeFile(base + "_b.jpg", b.image.data, b.image.width, b.image.height);
}
// Compares serial output with parallel outputs (type1, type2, type3)
bool BenchmarkRunner::test_outputs(const vector<Generated>& serial, const ParallelResult& pr, const string& logs_dir) {
    size_t size = serial.size();
    bool ok = true;

    if (pr.type1.size() != size || pr.type2.size() != size || pr.type3.size() != size) {
        cout << red << "[FASE 2] Dimension mismatch (serial=" << size
                   << ", type1=" << pr.type1.size()
                   << ", type2=" << pr.type2.size()
                   << ", type3=" << pr.type3.size() << ")" << reset << "\n";
        return false;
    }

    for (size_t i = 0; i < size; i++) {

        // serial vs type1
        if (!compare_images(serial[i].image.data, pr.type1[i].image.data,
                             serial[i].image.width, serial[i].image.height))
        {
            cout << yellow << "[FASE 2] Mismatch serial vs type1 on index " << i << reset << "\n";
            save_mismatch(logs_dir, "serial_vs_type1", pr.cores, i, serial[i], pr.type1[i]);
            ok = false;
        }

        // type1 vs type2
        if (!compare_images(pr.type1[i].image.data, pr.type2[i].image.data,
                             pr.type1[i].image.width, pr.type1[i].image.height))
        {
            cout << yellow << "[FASE 2] Mismatch type1 vs type2 on index " << i << reset << "\n";
            save_mismatch(logs_dir, "type1_vs_type2", pr.cores, i, pr.type1[i], pr.type2[i]);
            ok = false;
        }

        // type1 vs type3
        if (!compare_images(pr.type1[i].image.data, pr.type3[i].image.data,
                             pr.type1[i].image.width, pr.type1[i].image.height))
        {
            cout << yellow << "[FASE 2] Mismatch type1 vs type3 on index " << i << reset << "\n";
            save_mismatch(logs_dir, "type1_vs_type3", pr.cores, i, pr.type1[i], pr.type3[i]);
            ok = false;
        }
    }

    return ok;
}

bool BenchmarkRunner::run(const vector<Benchmark>& all_benchmarks, const string& run_folder)
{
    bool all_passed = true;
    int testnum = 0;

    for (const auto& bm : all_benchmarks) {
        testnum++;
        cout << "\n" << cyan << "[FASE 2] --- Benchmark test set " << testnum << " ---" << reset << "\n";

        string logs_dir = run_folder + "/logs/imgs" + to_string(testnum);

        for (const auto& pr : bm.BenchmarkParallel) {
            cout << "[FASE 2] Verifying with " << pr.cores << " cores... ";

            if (test_outputs(bm.BenchmarkSerial, pr, logs_dir)) {
                cout << green << "[FASE 2] PASS" << reset << "\n";
            } else {
                cout << red << "[FASE 2] FAIL (images saved in " << logs_dir << ")" << reset << "\n";
                all_passed = false;
            }
        }
    }

    if (all_passed) {
        cout << "\n" << green << "[FASE 2] All benchmarks successfully passed!" << reset << "\n";
    } else {
        cout << "\n" << red << "[FASE 2] At least one benchmark failed... check log directory" << reset << "\n";
    }

    return all_passed;
}
