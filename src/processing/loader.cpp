#include "loader.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

bool readFile(const string& filename, vector<unsigned char>& image, int& width, int& height) {
    int channels;

    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 1);
    if (!data) {
        cout << "Errore: impossibile aprire o decodificare " << filename << endl;
        return false;
    }

    image.assign(data, data + width * height);

    stbi_image_free(data);

    return true;
}

bool writeFile(const string& filename, const vector<unsigned char>& image, int width, int height) {
    int quality = 90;
    
    int success = stbi_write_jpg(filename.c_str(), width, height, 1, image.data(), quality);

    if (!success) {
        cout << "Errore: impossibile scrivere JPG " << filename << endl;
        return false;
    }

    return true;
}

void save_results_csv(const string& filename, const string& dataset_name ,const vector<string>& etype, const vector<string>& operations, const vector<double>& times, int num_cores, int num_images) {    
    ofstream out(filename, ios::app);

    if (!out.is_open()) {
        cerr << "Errore apertura file: " << filename << endl;
        return;
    }

    out << "dataset,elaborazione,operazione,tempo_medio_s,cores,num_images\n";

    for (size_t i = 0; i < operations.size(); i++) {
        out << dataset_name<<"," 
            << etype[i] << ","
            << operations[i] << ","
            << times[i] << ","
            << num_cores << ","
            << num_images <<"\n";
    }


    out.close();
}

