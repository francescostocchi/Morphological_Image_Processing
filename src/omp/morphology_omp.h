#pragma once

#include <vector>

using namespace std;

// --- parallel version 1 ---
void erosion_omp(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k);

void dilation_omp(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k);

void opening_omp(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k);

void closing_omp(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k);

// --- parallel version 2 ---
void erosion_omp_1(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k);

void dilation_omp_1(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k);

void opening_omp_1(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k);

void closing_omp_1(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k);

// --- parallel version 3 ---
void erosion_omp_2(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k);

void dilation_omp_2(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k);

void opening_omp_2(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k);

void closing_omp_2(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k);

