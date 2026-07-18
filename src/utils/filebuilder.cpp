#include "filebuilder.h"
#include <ctime>
#include <sstream>
#include <filesystem>

using namespace std;

namespace fs = filesystem;

string build_output_folder() {
    time_t now = time(NULL);
    tm* t = localtime(&now);

    char date_buf[11];  
    char time_buf[9];

    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", t);
    strftime(time_buf, sizeof(time_buf), "%H-%M-%S", t);

    fs::path folder = fs::path("output") / date_buf / time_buf;

    fs::create_directories(folder);

    create_subfolders(folder.string());

    return folder.string();
}

string build_results_filename(const string& base) {
    /*
    time_t now = time(NULL);
    tm* t = localtime(&now);

    char date_buf[11];  
    strftime(date_buf, sizeof(date_buf), "%m%d%y", t);

    */
    return base;
}

void create_subfolders(const std::string& root) {
    fs::create_directories(fs::path(root) / "images");
    fs::create_directories(fs::path(root) / "logs");
}

void ensure_folder(const string& path) {
    fs::create_directories(path);
}
