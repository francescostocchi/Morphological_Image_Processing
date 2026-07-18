#pragma once

#include<vector>
#include<string>

using namespace std;

struct Image {
	vector<unsigned char> data;
	int width;
	int height;
};

struct Generated {
	Image image;
	int id;
};

void process_dataset_serial(vector<Generated>& gen, const vector<Image>& db, vector<double>& time_seq, int k, vector<string>& info);

void process_dataset_parallel_type1(vector<Generated>& gen, const vector<Image>& db, int cores, vector<double>& time_seq, int k, vector<string>& info);

void process_dataset_parallel_type2(vector<Generated>& gen, const vector<Image>& db, int cores, vector<double>& time_seq, int k, vector<string>& info);

void process_dataset_parallel_type3(vector<Generated>& gen, const vector<Image>& db, int cores, vector<double>& time_seq, int k, vector<string>& info);

double get_time();

double speedUp(double t1, double t2);

void progressBar(int progress, int total, int timer);

void genera(vector<Generated>& gen, vector<unsigned char>& out, int width, int height);

void save(string fileName, string dbName, string prefix, string etype, vector<double>& times, int T, int size);
