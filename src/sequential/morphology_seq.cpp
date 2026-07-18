#include "morphology_seq.h"
#include <iostream>
#include <algorithm>
#include <omp.h>

using namespace std;

// --- SEQUENTIALS ---
// Basic morphological operations without parallelization
// These functions serve as the reference baseline for correctness
// Performs grayscale erosion using a square structuring element of size k
// For each pixel, selects the minimum value in the k×k neighborhood
void erosion_seq(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k) {
	int r = k / 2;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {

			unsigned char minVal = 255;

			for (int dy = -r; dy <= r; dy++) {
				for (int dx = -r; dx <= r; dx++) {

					int yy = y + dy;
					int xx = x + dx;

					if (xx >= 0 && xx < width && yy >= 0 && yy < height) {
						unsigned char v = input[yy * width + xx];
						if (v < minVal) minVal = v;
					}
				}
			}

			output[y * width + x] = minVal;
		}
	}
}

// Performs grayscale dilation using a square structuring element of size k
// For each pixel, selects the maximum value in the k×k neighborhood
void dilation_seq(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k) {
	int r = k / 2;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {

			unsigned char maxVal = 0;

			for (int dy = -r; dy <= r; dy++) {
				for (int dx = -r; dx <= r; dx++) {

					int yy = y + dy;
					int xx = x + dx;

					if (xx >= 0 && xx < width && yy >= 0 && yy < height) {
						unsigned char v = input[yy * width + xx];
						if (v > maxVal) maxVal = v;
					}
				}
			}

			output[y * width + x] = maxVal;
		}
	}
}

// Opening = erosion followed by dilation
// Removes small bright noise while preserving overall shape
void opening_seq(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k) {
	erosion_seq(input, temp, width, height, k);
	dilation_seq(temp, output, width, height, k);
}

// Closing = dilation followed by erosion
// Fills small dark holes while preserving overall shape
void closing_seq(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k) {
	dilation_seq(input, temp, width, height, k);
	erosion_seq(temp, output, width, height, k);
}