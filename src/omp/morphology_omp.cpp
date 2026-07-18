#include "morphology_omp.h"
#include <iostream>
#include <algorithm>
#include <omp.h>

// --- PARALLEL v1 ---
// Basic OMP implementation: brute-force neighborhood scan with bounds checking
void erosion_omp(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k) {
	int r = k / 2;
#pragma omp parallel for collapse(2) schedule(static) //omp parallel for collapse(2) 
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

void dilation_omp(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k) {
	int r = k / 2;
#pragma omp parallel for collapse(2) schedule(static) //omp parallel for collapse(2) 
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

void opening_omp(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k) {
	erosion_omp(input, temp, width, height, k);
	dilation_omp(temp, output, width, height, k);
}

void closing_omp(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k) {
	dilation_omp(input, temp, width, height, k);
	erosion_omp(temp, output, width, height, k);
}

// --- Parallel v2 ---
// Optimized border handling: manually compute borders sequentially, then parallelize only the central region.
void erosion_omp_1(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k) {
	int r = k / 2;

	for (int y = 0; y < r; y++) {
		for (int x = 0; x < width; x++) {

			unsigned char minVal = 255;

			int y0 = max(0, y - r);
			int y1 = min(height - 1, y + r);
			int x0 = max(0, x - r);
			int x1 = min(width - 1, x + r);

			for (int yy = y0; yy <= y1; yy++) {
				const unsigned char* row = &input[yy * width + x0];
				for (int xx = x0; xx <= x1; xx++)
					minVal = min(minVal, *row++);
			}

			output[y * width + x] = minVal;
		}
	}
	for (int y = height - r; y < height; y++) {
		for (int x = 0; x < width; x++) {

			unsigned char minVal = 255;

			int y0 = max(0, y - r);
			int y1 = min(height - 1, y + r);
			int x0 = max(0, x - r);
			int x1 = min(width - 1, x + r);

			for (int yy = y0; yy <= y1; yy++) {
				const unsigned char* row = &input[yy * width + x0];
				for (int xx = x0; xx <= x1; xx++)
					minVal = min(minVal, *row++);
			}

			output[y * width + x] = minVal;
		}
	}
	for (int y = r; y < height - r; y++) {
		for (int x = 0; x < r; x++) {

			unsigned char minVal = 255;

			int y0 = y - r;
			int y1 = y + r;
			int x0 = 0;
			int x1 = x + r;

			for (int yy = y0; yy <= y1; yy++) {
				const unsigned char* row = &input[yy * width + x0];
				for (int xx = x0; xx <= x1; xx++)
					minVal = min(minVal, *row++);
			}

			output[y * width + x] = minVal;
		}
	}
	for (int y = r; y < height - r; y++) {
		for (int x = width - r; x < width; x++) {

			unsigned char minVal = 255;

			int y0 = y - r;
			int y1 = y + r;
			int x0 = x - r;
			int x1 = width - 1;

			for (int yy = y0; yy <= y1; yy++) {
				const unsigned char* row = &input[yy * width + x0];
				for (int xx = x0; xx <= x1; xx++)
					minVal = min(minVal, *row++);
			}

			output[y * width + x] = minVal;
		}
	}
#pragma omp parallel for collapse(2) schedule(static)
	for (int y = r; y < height - r; y++) {
		for (int x = r; x < width - r; x++) {

			unsigned char minVal = 255;

			for (int dy = -r; dy <= r; dy++) {
				const unsigned char* row = &input[(y + dy) * width + (x - r)];
				for (int dx = 0; dx < k; dx++) {
					unsigned char v = row[dx];
					if (v < minVal) minVal = v;
				}
			}

			output[y * width + x] = minVal;
		}
	}
}

void dilation_omp_1(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k) {
	int r = k / 2;
	for (int y = 0; y < r; y++) {
		for (int x = 0; x < width; x++) {

			unsigned char maxVal = 0;

			int y0 = 0;
			int y1 = y + r;
			int x0 = max(0, x - r);
			int x1 = min(width - 1, x + r);

			for (int yy = y0; yy <= y1; yy++) {
				const unsigned char* row = &input[yy * width + x0];
				for (int xx = x0; xx <= x1; xx++) {
					unsigned char v = *row++;
					if (v > maxVal) maxVal = v;
				}
			}

			output[y * width + x] = maxVal;
		}
	}

	for (int y = height - r; y < height; y++) {
		for (int x = 0; x < width; x++) {

			unsigned char maxVal = 0;

			int y0 = y - r;
			int y1 = height - 1;
			int x0 = max(0, x - r);
			int x1 = min(width - 1, x + r);

			for (int yy = y0; yy <= y1; yy++) {
				const unsigned char* row = &input[yy * width + x0];
				for (int xx = x0; xx <= x1; xx++) {
					unsigned char v = *row++;
					if (v > maxVal) maxVal = v;
				}
			}

			output[y * width + x] = maxVal;
		}
	}

	for (int y = r; y < height - r; y++) {
		for (int x = 0; x < r; x++) {

			unsigned char maxVal = 0;

			int y0 = y - r;
			int y1 = y + r;
			int x0 = 0;
			int x1 = x + r;

			for (int yy = y0; yy <= y1; yy++) {
				const unsigned char* row = &input[yy * width + x0];
				for (int xx = x0; xx <= x1; xx++) {
					unsigned char v = *row++;
					if (v > maxVal) maxVal = v;
				}
			}

			output[y * width + x] = maxVal;
		}
	}

	for (int y = r; y < height - r; y++) {
		for (int x = width - r; x < width; x++) {

			unsigned char maxVal = 0;

			int y0 = y - r;
			int y1 = y + r;
			int x0 = x - r;
			int x1 = width - 1;

			for (int yy = y0; yy <= y1; yy++) {
				const unsigned char* row = &input[yy * width + x0];
				for (int xx = x0; xx <= x1; xx++) {
					unsigned char v = *row++;
					if (v > maxVal) maxVal = v;
				}
			}

			output[y * width + x] = maxVal;
		}
	}

#pragma omp parallel for collapse(2) schedule(static)
	for (int y = r; y < height - r; y++) {
		for (int x = r; x < width - r; x++) {

			unsigned char maxVal = 0;

			for (int dy = -r; dy <= r; dy++) {
				const unsigned char* row = &input[(y + dy) * width + (x - r)];
				for (int dx = 0; dx < k; dx++) {
					unsigned char v = row[dx];
					if (v > maxVal) maxVal = v;
				}
			}

			output[y * width + x] = maxVal;
		}
	}
}


void opening_omp_1(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k) {
	erosion_omp_1(input, temp, width, height, k);
	dilation_omp_1(temp, output, width, height, k);
}

void closing_omp_1(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k) {
	dilation_omp_1(input, temp, width, height, k);
	erosion_omp_1(temp, output, width, height, k);
}

// --- Parallele v3 ---
// Uses OpenMP tasks to compute border regions concurrently, then parallel-for for the central region
void erosion_omp_2(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k) {
	output.resize(width * height);
	int r = k / 2;
#pragma omp parallel
	{
#pragma omp single
		{
#pragma omp task
			{
				for (int y = 0; y < r; y++) {
					for (int x = 0; x < width; x++) {

						unsigned char minVal = 255;

						int y0 = max(0, y - r);
						int y1 = min(height - 1, y + r);
						int x0 = max(0, x - r);
						int x1 = min(width - 1, x + r);

						for (int yy = y0; yy <= y1; yy++) {
							const unsigned char* row = &input[yy * width + x0];
							for (int xx = x0; xx <= x1; xx++)
								minVal = min(minVal, *row++);
						}

						output[y * width + x] = minVal;
					}
				}
			}
#pragma omp task
			{
				for (int y = height - r; y < height; y++) {
					for (int x = 0; x < width; x++) {

						unsigned char minVal = 255;

						int y0 = max(0, y - r);
						int y1 = min(height - 1, y + r);
						int x0 = max(0, x - r);
						int x1 = min(width - 1, x + r);

						for (int yy = y0; yy <= y1; yy++) {
							const unsigned char* row = &input[yy * width + x0];
							for (int xx = x0; xx <= x1; xx++)
								minVal = min(minVal, *row++);
						}

						output[y * width + x] = minVal;
					}
				}
			}
#pragma omp task
			{
				for (int y = r; y < height - r; y++) {
					for (int x = 0; x < r; x++) {

						unsigned char minVal = 255;

						int y0 = y - r;
						int y1 = y + r;
						int x0 = 0;
						int x1 = x + r;

						for (int yy = y0; yy <= y1; yy++) {
							const unsigned char* row = &input[yy * width + x0];
							for (int xx = x0; xx <= x1; xx++)
								minVal = min(minVal, *row++);
						}

						output[y * width + x] = minVal;
					}
				}
			}
#pragma omp task
			{
				for (int y = r; y < height - r; y++) {
					for (int x = width - r; x < width; x++) {

						unsigned char minVal = 255;

						int y0 = y - r;
						int y1 = y + r;
						int x0 = x - r;
						int x1 = width - 1;

						for (int yy = y0; yy <= y1; yy++) {
							const unsigned char* row = &input[yy * width + x0];
							for (int xx = x0; xx <= x1; xx++)
								minVal = min(minVal, *row++);
						}

						output[y * width + x] = minVal;
					}
				}
			}
#pragma omp taskwait

#pragma omp parallel for collapse(2) schedule(static)
			for (int y = r; y < height - r; y++) {
				for (int x = r; x < width - r; x++) {

					unsigned char minVal = 255;

					for (int dy = -r; dy <= r; dy++) {
						const unsigned char* row = &input[(y + dy) * width + (x - r)];
						for (int dx = 0; dx < k; dx++) {
							unsigned char v = row[dx];
							if (v < minVal) minVal = v;
						}
					}

					output[y * width + x] = minVal;
				}
			}
		}
	}
}

void dilation_omp_2(const vector<unsigned char>& input, vector<unsigned char>& output, int width, int height, int k) {
	output.resize(width * height);
	int r = k / 2;
#pragma omp parallel
	{
#pragma omp single
		{
#pragma omp task
			{
				for (int y = 0; y < r; y++) {
					for (int x = 0; x < width; x++) {

						unsigned char maxVal = 0;

						int y0 = 0;
						int y1 = y + r;
						int x0 = max(0, x - r);
						int x1 = min(width - 1, x + r);

						for (int yy = y0; yy <= y1; yy++) {
							const unsigned char* row = &input[yy * width + x0];
							for (int xx = x0; xx <= x1; xx++) {
								unsigned char v = *row++;
								if (v > maxVal) maxVal = v;
							}
						}

						output[y * width + x] = maxVal;
					}
				}
			}
#pragma omp task
			{
				for (int y = height - r; y < height; y++) {
					for (int x = 0; x < width; x++) {

						unsigned char maxVal = 0;

						int y0 = y - r;
						int y1 = height - 1;
						int x0 = max(0, x - r);
						int x1 = min(width - 1, x + r);

						for (int yy = y0; yy <= y1; yy++) {
							const unsigned char* row = &input[yy * width + x0];
							for (int xx = x0; xx <= x1; xx++) {
								unsigned char v = *row++;
								if (v > maxVal) maxVal = v;
							}
						}

						output[y * width + x] = maxVal;
					}
				}
			}
#pragma omp task
			{
				for (int y = r; y < height - r; y++) {
					for (int x = 0; x < r; x++) {

						unsigned char maxVal = 0;

						int y0 = y - r;
						int y1 = y + r;
						int x0 = 0;
						int x1 = x + r;

						for (int yy = y0; yy <= y1; yy++) {
							const unsigned char* row = &input[yy * width + x0];
							for (int xx = x0; xx <= x1; xx++) {
								unsigned char v = *row++;
								if (v > maxVal) maxVal = v;
							}
						}

						output[y * width + x] = maxVal;
					}
				}
			}
#pragma omp task
			{
				for (int y = r; y < height - r; y++) {
					for (int x = width - r; x < width; x++) {

						unsigned char maxVal = 0;

						int y0 = y - r;
						int y1 = y + r;
						int x0 = x - r;
						int x1 = width - 1;

						for (int yy = y0; yy <= y1; yy++) {
							const unsigned char* row = &input[yy * width + x0];
							for (int xx = x0; xx <= x1; xx++) {
								unsigned char v = *row++;
								if (v > maxVal) maxVal = v;
							}
						}

						output[y * width + x] = maxVal;
					}
				}
			}
#pragma omp taskwait

#pragma omp parallel for collapse(2) schedule(static)
			for (int y = r; y < height - r; y++) {
				for (int x = r; x < width - r; x++) {

					unsigned char maxVal = 0;

					for (int dy = -r; dy <= r; dy++) {
						const unsigned char* row = &input[(y + dy) * width + (x - r)];
						for (int dx = 0; dx < k; dx++) {
							unsigned char v = row[dx];
							if (v > maxVal) maxVal = v;
						}
					}

					output[y * width + x] = maxVal;
				}
			}
		}
	}
}

void opening_omp_2(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k) {
	erosion_omp_2(input, temp, width, height, k);
	dilation_omp_2(temp, output, width, height, k);
}

void closing_omp_2(const vector<unsigned char>& input, vector<unsigned char>& output, vector<unsigned char>& temp, int width, int height, int k) {
	dilation_omp_2(input, temp, width, height, k);
	erosion_omp_2(temp, output, width, height, k);
}
