#ifndef LOADER_H
#define LOADER_H

#include <string>
#include <vector>
using namespace std;

bool readFile(const string& filename, vector<unsigned char>& image, int& width, int& height);

bool writeFile(const string& filename, const vector<unsigned char>& image, int width, int height);

void save_results_csv(const string& filename, const string& dataset_name, const vector<string>& etype, const vector<string>& operations, const vector<double>& times, int num_cores, int num_images);
#endif
