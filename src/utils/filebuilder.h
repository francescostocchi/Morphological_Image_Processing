#ifndef FILEBUILDER_H
#define FILEBUILDER_H

#include <string>

using namespace std;

string build_output_folder();
string build_results_filename(const string& base);
void create_subfolders(const string& root);
void ensure_folder(const string& path);

#endif