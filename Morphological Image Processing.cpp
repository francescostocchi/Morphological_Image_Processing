#include <iostream>
#include <string>
#include "src/DataProcessing.cpp"
#include "data/DatasetBootstrapper.h"
#include "benchmark/BenchmarkRunner.h"

using namespace std;

static const char* red = "\033[31m";
static const char* green = "\033[32m";
static const char* reset = "\033[0m";

string pypath = "";

unsigned int starting_cores = 1;
static string results = "results.csv";
static string CONFIG_PATH = "data/dataset/config.json";
static int maximages = 50;
int main() {
	try {
		// Phase 0: database
		DatasetBootstrapper bootstrapper(CONFIG_PATH);
		pypath = bootstrapper.getpythonpath();
		if (!bootstrapper.ensureReady()) {
			cerr << red << "Impossibile procedere senza un database valido." << reset << endl;
			return 1;
		}
		// Phase 1: Computation
		DataProcessor dp(results, starting_cores, maximages);
		vector<Benchmark> all_benchmarks = dp.run();

		// Phase 2: Benchmarking
		string run_folder = dp.getRunFolder();
		cout << run_folder << endl;
		BenchmarkRunner runner;
		runner.run(all_benchmarks, dp.getRunFolder());

		string command = pypath + " benchmark/benchmark_setup_dependencies.py " + run_folder + "/" + results;
		system(command.c_str());
		cout << green << "Graph generated" << endl << reset;

	}
	catch (const std::exception& e) {
		cerr << red << "Errore di configurazione: " << e.what() << reset << endl;
		return 1;
	}
	return 0;
}