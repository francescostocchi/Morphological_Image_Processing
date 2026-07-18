#include "functions.h"
#include "../sequential/morphology_seq.h"
#include "../omp/morphology_omp.h"
#include "../processing/loader.h"
#include <iostream>
#include <omp.h>
// Measures execution time for each operation and stores generated images for the 3 types of parallelization //

void process_dataset_serial(vector<Generated>& gen, const vector<Image>& db, vector<double>& time_seq, int k, vector<string>& info) {
	if (info.size() < 3) return;
	double tstart, t0, t1;
	int counter = 0;
	int lastPercent = -1;
	int dbsize = db.size();
	tstart = get_time();
	cout << "=== SERIAL EXECUTION ALGORITHM ===" << endl;
	for (const auto& img : db) {
		vector<unsigned char> outE(img.width * img.height);
		vector<unsigned char> outD(img.width * img.height);
		vector<unsigned char> outO(img.width * img.height);
		vector<unsigned char> outC(img.width * img.height);

		vector<unsigned char> temp(img.width * img.height);

		t0 = get_time();
		erosion_seq(img.data, outE, img.width, img.height, k);
		t1 = get_time();
		time_seq[0] += (t1 - t0);

		t0 = get_time();
		dilation_seq(img.data, outD, img.width, img.height, k);
		t1 = get_time();
		time_seq[1] += (t1 - t0);

		t0 = get_time();
		opening_seq(img.data, outO, temp, img.width, img.height, k);
		t1 = get_time();
		time_seq[2] += (t1 - t0);

		t0 = get_time();
		closing_seq(img.data, outC, temp, img.width, img.height, k);
		t1 = get_time();
		time_seq[3] += (t1 - t0);
		counter++;
		int percent = (counter * 100) / dbsize;
		if (percent != lastPercent) {
			progressBar(counter, dbsize, t1 - tstart);
			lastPercent = percent;
		}
		if (counter % 100 == 0 || dbsize < 10) {
			genera(gen, outE, img.width, img.height);
			genera(gen, outD, img.width, img.height);
			genera(gen, outO, img.width, img.height);
			genera(gen, outC, img.width, img.height);
		}
	}

	cout << endl;
	save(info[0], info[1], info[2], "serial", time_seq, 1, dbsize);
}

void process_dataset_parallel_type1(vector<Generated>& gen, const vector<Image>& db, int cores, vector<double>& time_seq, int k, vector<string>& info) {
	if (info.size() < 3) return;
	double time_par[4] = { 0,0,0,0 };
	int counter = 0;
	int lastPercent = -1;
	int dbsize = db.size();
	double time_start = get_time();
	cout << "=== PARALLEL EXECUTION V1 ALGORITHM ===" << endl;
	for (const auto& img : db) {
		vector<unsigned char> outE(img.width * img.height);
		vector<unsigned char> outD(img.width * img.height);
		vector<unsigned char> outO(img.width * img.height);
		vector<unsigned char> outC(img.width * img.height);

		vector<unsigned char> temp(img.width * img.height);

		double t0, t1;

		// EROSION
		t0 = get_time();
		erosion_omp(img.data, outE, img.width, img.height, k);
		t1 = get_time();
		time_par[0] += (t1 - t0);

		// DILATION
		t0 = get_time();
		dilation_omp(img.data, outD, img.width, img.height, k);
		t1 = get_time();
		time_par[1] += (t1 - t0);

		// OPENING
		t0 = get_time();
		opening_omp(img.data, outO, temp, img.width, img.height, k);
		t1 = get_time();
		time_par[2] += (t1 - t0);

		// CLOSING
		t0 = get_time();
		closing_omp(img.data, outC, temp, img.width, img.height, k);
		t1 = get_time();
		time_par[3] += (t1 - t0);
		counter++;
		if (counter % 100 == 0 || dbsize < 10) {
			genera(gen, outE, img.width, img.height);
			genera(gen, outD, img.width, img.height);
			genera(gen, outO, img.width, img.height);
			genera(gen, outC, img.width, img.height);
		}
		int percent = (counter * 100) / dbsize;
		if (percent != lastPercent) {
			progressBar(counter, dbsize, t1 - time_start);
			lastPercent = percent;
		}
	}
	cout << endl;
	vector<double> times(time_par, time_par + 4);
	save(info[0], info[1], info[2], "parallel", times, cores, dbsize);
}

void process_dataset_parallel_type2(vector<Generated>& gen, const vector<Image>& db, int cores, vector<double>& time_seq, int k, vector<string>& info) {
	if (info.size() < 3) return;
	double time_par[4] = { 0,0,0,0 };
	int counter = 0;
	int lastPercent = -1;
	int dbsize = db.size();
	double time_start = get_time();

	cout << "=== PARALLEL EXECUTION V2 ALGORITHM ===" << endl;
	for (const auto& img : db) {
		vector<unsigned char> outE(img.width * img.height);
		vector<unsigned char> outD(img.width * img.height);
		vector<unsigned char> outO(img.width * img.height);
		vector<unsigned char> outC(img.width * img.height);

		vector<unsigned char> temp(img.width * img.height);

		double t0, t1;

		// EROSION
		t0 = get_time();
		erosion_omp_1(img.data, outE, img.width, img.height, k);
		t1 = get_time();
		time_par[0] += (t1 - t0);

		// DILATION
		t0 = get_time();
		dilation_omp_1(img.data, outD, img.width, img.height, k);
		t1 = get_time();
		time_par[1] += (t1 - t0);

		// OPENING
		t0 = get_time();
		opening_omp_1(img.data, outO, temp, img.width, img.height, k);
		t1 = get_time();
		time_par[2] += (t1 - t0);

		// CLOSING
		t0 = get_time();
		closing_omp_1(img.data, outC, temp, img.width, img.height, k);
		t1 = get_time();
		time_par[3] += (t1 - t0);
		counter++;
		if (counter % 100 == 0 || dbsize < 10) {
			genera(gen, outE, img.width, img.height);
			genera(gen, outD, img.width, img.height);
			genera(gen, outO, img.width, img.height);
			genera(gen, outC, img.width, img.height);
		}
		int percent = (counter * 100) / dbsize;
		if (percent != lastPercent) {
			progressBar(counter, dbsize, t1 - time_start);
			lastPercent = percent;
		}
	}
	cout << endl;
	vector<double> times(time_par, time_par + 4);
	save(info[0], info[1], info[2], "parallel_v2", times, cores, dbsize);
}

void process_dataset_parallel_type3(vector<Generated>& gen, const vector<Image>& db, int cores, vector<double>& time_seq, int k, vector<string>& info) {
	if (info.size() < 3) return;
	double time_par[4] = { 0,0,0,0 };
	int counter = 0;
	int lastPercent = -1;
	int dbsize = db.size();
	double time_start = get_time();

	cout << "=== PARALLEL EXECUTION V3 ALGORITHM ===" << endl;
	for (const auto& img : db) {
		vector<unsigned char> outE(img.width * img.height);
		vector<unsigned char> outD(img.width * img.height);
		vector<unsigned char> outO(img.width * img.height);
		vector<unsigned char> outC(img.width * img.height);

		vector<unsigned char> temp(img.width * img.height);

		double t0, t1;

		// EROSION
		t0 = get_time();
		erosion_omp_2(img.data, outE, img.width, img.height, k);
		t1 = get_time();
		time_par[0] += (t1 - t0);

		// DILATION
		t0 = get_time();
		dilation_omp_2(img.data, outD, img.width, img.height, k);
		t1 = get_time();
		time_par[1] += (t1 - t0);

		// OPENING
		t0 = get_time();
		opening_omp_2(img.data, outO, temp, img.width, img.height, k);
		t1 = get_time();
		time_par[2] += (t1 - t0);

		// CLOSING
		t0 = get_time();
		closing_omp_2(img.data, outC, temp, img.width, img.height, k);
		t1 = get_time();
		time_par[3] += (t1 - t0);
		counter++;
		if (counter % 100 == 0 || dbsize < 10) {
			genera(gen, outE, img.width, img.height);
			genera(gen, outD, img.width, img.height);
			genera(gen, outO, img.width, img.height);
			genera(gen, outC, img.width, img.height);
		}
		int percent = (counter * 100) / dbsize;
		if (percent != lastPercent) {
			progressBar(counter, dbsize, t1 - time_start);
			lastPercent = percent;
		}
	}
	cout << endl;
	vector<double> times(time_par, time_par + 4);
	save(info[0], info[1], info[2], "parallel_v3", times, cores, dbsize);
}

double get_time() {
	return omp_get_wtime();
}

double speedUp(double t1, double t2) {
	return t1 / t2;
}

void progressBar(int progress, int total, int timer) {
	int barWidth = 50;

	float ratio = (float)progress / total;
	int filled = ratio * barWidth;

	cout << "\r[";

	for (int i = 0; i < barWidth; ++i) {
		if (i < filled) cout << "\033[32m" << "=" << "\033[0m";
		else cout << "\033[33m" << "-" << "\033[0m";
	}
	cout << "] " << int(ratio * 100) << "% Time: " << timer << "s";
	cout.flush();
}

void genera(vector<Generated>& gen, vector<unsigned char>& out, int width, int height) {
	int newId;
	if (gen.empty()) newId = 0;
	else newId = gen.back().id + 1;
	Generated g;
	g.id = newId;
	g.image.data = out;
	g.image.width = width;
	g.image.height = height;
	gen.push_back(g);
}

void save(string fileName, string dbName, string prefix, string etype, vector<double>& times, int T, int size) {
	vector<string> elaboration = { etype , etype, etype, etype };
	vector<string> ops = { "Erosion", "Dilation", "Opening", "Closing" };

	string f = dbName;
	if (f.rfind(prefix, 0) == 0) {
		f = f.substr(prefix.size());
	}
	if (!f.empty() && f.back() == '/') {
		f.pop_back();
	}
	save_results_csv(fileName, f, elaboration, ops, times, T, size);
}
