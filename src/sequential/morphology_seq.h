#ifndef MORPHOLOGY_SEQ_H
#define MORPHOLOGY_SEQ_H

#include <vector>

using namespace std;

// --- Versioni sequenziali ---
void erosion_seq(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k);

void dilation_seq(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k);

void opening_seq(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k);

void closing_seq(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k);

#endif
