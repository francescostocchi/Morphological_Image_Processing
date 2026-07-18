#include "DatasetBootstrapper.h"
#include <iostream>
#include <filesystem>
#include <cctype>
#include <cstdlib>
#include <vector>
#include <string>

using namespace std;

namespace fs = filesystem;

static const char* green = "\033[32m";
static const char* yellow = "\033[33m";
static const char* red = "\033[31m";
static const char* reset = "\033[0m";

// Searches common Windows directories for python.exe installations
static vector<string> findPythonExecutable() {
	vector<string> found;

	vector<fs::path> searchPaths = {
		fs::path(getenv("LOCALAPPDATA")) / "Programs" / "Python",
		fs::path(getenv("PROGRAMFILES")) / "Python",
		fs::path(getenv("PROGRAMFILES(X86)")) / "Python"
	};

	for (const auto& base : searchPaths) {
		if (!fs::exists(base)) continue;

		for (auto& entry : fs::recursive_directory_iterator(base)) {
			if (entry.path().filename() == "python.exe") {

				string p = entry.path().string();

				if (p.find("WindowsApps") != string::npos)
					continue;

				if (p.find("venv") != string::npos)
					continue;

				if (p.find("embed") != string::npos)
					continue;

				fs::path parent = entry.path().parent_path();
				if (parent.filename().string().rfind("Python", 0) == 0) {
					found.push_back(p);
				}
			}
		}
	}

	return found;
}


DatasetBootstrapper::DatasetBootstrapper(const string& configPath)
	: config_(DatasetConfig::loadFromFile(configPath)) {
}

// Checks whether a file extension matches one of the allowed image extensions
bool DatasetBootstrapper::isImageFile(const string& filename) const {
	fs::path p(filename);
	string e = p.extension().string();
	for (auto& c : e) c = (char)tolower((unsigned char)c);
	for (const auto& x : config_.extensions) {
		if (e == x) return true;
	}
	return false;
}

// Counts all valid image files inside the configured dataset folders
int DatasetBootstrapper::countDatasetImages() const {
	int count = 0;
	for (const auto& folder : config_.subsetFolders()) {
		if (!fs::exists(folder)) continue;
		for (const auto& entry : fs::directory_iterator(folder)) {
			if (entry.is_regular_file() && isImageFile(entry.path().string())) {
				count++;
			}
		}
	}
	return count;
}

bool DatasetBootstrapper::runGenerationScript() const {
	string py = getpythonpath();
	string command = py + " setup_dependencies.py";
	cout << "[DEBUG] Executed: " << command << endl;
	int ret = system(command.c_str());
	return ret == 0;
}

// Ensures the dataset contains the expected number of images
bool DatasetBootstrapper::ensureReady() {
	int current = countDatasetImages();

	if (current == config_.totalImages) {
		cout << green << "[FASE 0] Database already ready: " << current << " images." << reset << endl;
		return true;
	}

	cout << yellow << "[FASE 0] Database not ready (" << current << "/" << config_.totalImages << " iamges). Generating..." << reset << endl;

	if (!runGenerationScript()) {
		cerr << red << "[FASE 0] Database initialization error." << reset << endl;
		return false;
	}

	int after = countDatasetImages();
	if (after != config_.totalImages) {
		cerr << red << "[FASE 0] Warning: (" << after
				   << ") expected: (" << config_.totalImages << ")." << reset << endl;
		return false;
	}

	cout << green << "[FASE 0] Database correctly generated: " << after << " images." << reset << endl;
	return true;
}

string DatasetBootstrapper::getpythonpath() const {
	if (pypath == "")	{
		auto pythonList = findPythonExecutable();

		if (pythonList.empty()) {
			cerr << "[FASE 0] No valid installation of python found." << endl;
			return "python";
		}
		pypath = pythonList[0];
	}
	return pypath;
}