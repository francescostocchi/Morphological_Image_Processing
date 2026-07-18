#pragma once
// DatasetBootstrapper: si occupa della Fase 0 (verifica che il database di
// immagini sia pronto, e se non lo e' richiama la pipeline Python per
// generarlo).

#include <string>
#include "DatasetConfig.h"

class DatasetBootstrapper {
public:
	// configPath: percorso del config.json condiviso con lo script Python.
	explicit DatasetBootstrapper(const std::string& configPath);

	// Verifica che il dataset abbia il numero di immagini atteso.
	// Se non lo ha, lancia lo script Python per generarlo.
	// Ritorna true se al termine il database e' pronto e utilizzabile.
	bool ensureReady();
	std::string getpythonpath() const;
	const DatasetConfig& config() const { return config_; }

private:
	DatasetConfig config_;
	mutable std::string pypath = "";
	bool isImageFile(const std::string& filename) const;
	int countDatasetImages() const;
	bool runGenerationScript() const;
};
