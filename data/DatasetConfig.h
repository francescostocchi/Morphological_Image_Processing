#pragma once
// Loads data/dataset/config.json and exposes it as typed structs.
// This is the single source of truth for: total expected image count,
// dataset folders (names + target sizes), valid image extensions,
// and how to launch the Python pipeline.

#include <string>
#include <vector>
#include "JsonValue.h"

struct DatasetSubset {
	std::string name;
	int targetSize;
};

struct DatasetConfig {
	int totalImages;
	std::string baseFolder;              
	std::vector<std::string> extensions;
	bool verbose;
	std::vector<DatasetSubset> subsets;

	std::string pythonPath;
	std::string pythonScript;
	std::string sourceUrl;

	// Returns full paths for each dataset subset folder (baseFolder + "/" + name + "/")
	std::vector<std::string> subsetFolders() const {
		std::vector<std::string> out;
		for (const auto& s : subsets) {
			out.push_back(baseFolder + "/" + s.name + "/");
		}
		return out;
	}

	// Parses config.json and loads dataset + pipeline configuration
	static DatasetConfig loadFromFile(const std::string& path) {
		JsonValue root = JsonValue::parseFile(path);
		const JsonValue& ds = root["dataset"];
		const JsonValue& pl = root["pipeline"];

		DatasetConfig cfg;
		cfg.totalImages = ds["total_images"].asInt(0);
		cfg.baseFolder = ds["base_folder"].asString("data/Database");
		cfg.verbose = ds["verbose"].asBool(false);

		for (const auto& ext : ds["extensions"].asArray()) {
			cfg.extensions.push_back(ext.asString());
		}

		for (const auto& sub : ds["subsets"].asArray()) {
			DatasetSubset s;
			s.name = sub["name"].asString();
			s.targetSize = sub["target_size"].asInt(0);
			cfg.subsets.push_back(s);
		}

		// Load pipeline section
		cfg.pythonPath = pl["python_path"].asString("python");
		cfg.pythonScript = pl["script"].asString();
		cfg.sourceUrl = pl["source_url"].asString();

		// Validate required fields
		if (cfg.totalImages <= 0)
			throw std::runtime_error("config.json: 'dataset.total_images' mancante o non valido");
		if (cfg.subsets.empty())
			throw std::runtime_error("config.json: 'dataset.subsets' mancante o vuoto");
		if (cfg.pythonScript.empty())
			throw std::runtime_error("config.json: 'pipeline.script' mancante");

		return cfg;
	}
};
