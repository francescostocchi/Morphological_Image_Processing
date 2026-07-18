#pragma once
#include <iostream>
#include <omp.h>
#include <filesystem>
#include <vector>
#include <thread>
#include "utils/filebuilder.h"
#include "utils/functions.h"
#include "processing/loader.h"
#include "omp/morphology_omp.h"

using namespace std;

namespace fs = filesystem;

struct ParallelResult {
    int cores;
    vector<Generated> type1;
    vector<Generated> type2;
    vector<Generated> type3;
};

struct Benchmark {
    vector<Generated> BenchmarkSerial;
    vector<ParallelResult> BenchmarkParallel;
};

class DataProcessor {
private:
    int MAX_IMAGES = 1000;
    // Costanti
    static const int k = 5;
    // Colori ANSI
    static constexpr const char* red = "\033[31m";
    static constexpr const char* green = "\033[32m";
    static constexpr const char* cyan = "\033[36m";
    static constexpr const char* reset = "\033[0m";

    // Variabili interne
    string results;
    string run_folder;
    string prefix = "data/";
    string currentFolder = " ";
    int start_cores = 1;

    vector<string> folders = {
        "data/Database/dataset1_small/",
        "data/Database/dataset2_medium/",
        "data/Database/dataset3_large/",
        "data/Database/dataset4_xlarge/",
    };

    vector<string> info;

    vector<Generated> genS;
    vector<Generated> gen1;
    vector<Generated> gen2;
    vector<Generated> gen3;
public:
    const string& getRunFolder() const { return run_folder; }
    DataProcessor(const string& results_filename = "", unsigned int initial_cores = 1, int maximages = 1000) : results(results_filename)
    {
        MAX_IMAGES = maximages;
        start_cores = initial_cores;
        run_folder = build_output_folder();
        ensure_folder(run_folder);

        results = run_folder + "/" + build_results_filename(results);
    }

    vector<Benchmark> run() {
        vector<Benchmark> all_bm;
        int testnum = 0;
        unsigned int max_cores = thread::hardware_concurrency();

        warmup_thread_pool(max_cores);

        for (auto& folder : folders) {

            Benchmark bm;
            info = { results, folder, prefix };
            currentFolder = folder;
            testnum++;

            vector<double> time_seq = { 0,0,0,0 };

            cout << green <<"\n[FASE 1] Loading test set " << testnum << reset <<endl;
            auto files = list_jpg_files(folder);

            auto db = load_dataset(files);
            cout << "[FASE 1] Test set loaded" << endl;

            int dbsize = db.size();
            cout << green << "[FASE 1] Loaded " << dbsize << " images from: " << folder << reset << endl;

            process_dataset_serial(genS, db, time_seq, k, info);
            bm.BenchmarkSerial = genS;
            genS.clear();

            for (int i = start_cores; i < max_cores; i++) {
                ParallelResult pr;
                pr.cores = i;

                omp_set_num_threads(i);
                warmup_kernels(i, k);
                cout << cyan << "\n[FASE 1] Starting test with: " << i << " cores" << reset << endl;

                process_dataset_parallel_type1(gen1, db, i, time_seq, k, info);
                process_dataset_parallel_type2(gen2, db, i, time_seq, k, info);
                process_dataset_parallel_type3(gen3, db, i, time_seq, k, info);

                pr.type1 = gen1;
                pr.type2 = gen2;
                pr.type3 = gen3;

                bm.BenchmarkParallel.push_back(move(pr));

                gen1.clear();
                gen2.clear();
                gen3.clear();
            }
            all_bm.push_back(std::move(bm));
        }

        cout << green << "[FASE 1] Images Processed\n" << reset;
        return all_bm;
    }

private:
    vector<string> list_jpg_files(const string& folder) {
        vector<string> files;

        for (const auto& entry : fs::directory_iterator(folder)) {
            string path = entry.path().string();
            if (path.ends_with(".jpg") || path.ends_with(".jpeg") || path.ends_with(".png")) {
                files.push_back(path);
            }
        }
        return files;
    }

    vector<Image> load_dataset(const vector<string>& paths) {
        vector<Image> db;
        int i = 0;

        for (const auto& p : paths) {
            if (i == MAX_IMAGES) return db;
            i++;

            Image img;
            if (!readFile(p, img.data, img.width, img.height)) {
                cerr << "Errore caricando " << p << endl;
                continue;
            }
            db.push_back(img);
        }
        return db;
    }
    // Warms up OpenMP thread pool to avoid initialization overhead during benchmarks
    void warmup_thread_pool(unsigned int max_cores) {
        omp_set_num_threads(max_cores);
#pragma omp parallel
        {
            volatile int x = omp_get_thread_num();
            (void)x;
        }
    }

    // Warms up morphology kernels to avoid first-call overhead
    void warmup_kernels(int cores, int k) {
        const int W = 512, H = 512;
        vector<unsigned char> in(W * H, 128);
        vector<unsigned char> outE(W * H), outD(W * H);
        vector<unsigned char> outO(W * H), outC(W * H), temp(W * H);

        omp_set_num_threads(cores);

        erosion_omp(in, outE, W, H, k);
        dilation_omp(in, outD, W, H, k);
        opening_omp(in, outO, temp, W, H, k);
        closing_omp(in, outC, temp, W, H, k);
    }
};
