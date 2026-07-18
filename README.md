# Morphological Image Processing

A C++ implementation of grayscale morphological image processing, developed in both sequential and parallel (OpenMP) versions, with the goal of studying the speedup achieved through parallelization.

The software applies the four fundamental morphological operators to grayscale images using a square structuring element:
- **Erosion**: shrinks foreground regions, requiring the structuring element to fit entirely within them.
- **Dilation**: grows foreground regions, marking any pixel touched by the structuring element.
- **Opening**: erosion followed by dilation; removes small bright details and separates weakly connected objects.
- **Closing**: dilation followed by erosion; fills small dark gaps and holes.

Three progressively optimized OpenMP parallelization strategies are compared against the sequential baseline, differing in how image borders (which need special handling due to their incomplete neighborhood) are processed:
- **V1 (naive)**: `#pragma omp parallel for collapse(2)` directly on the sequential loop, with the per-pixel border check still in place.
- **V2 (border/interior split)**: borders are computed serially with clamping, while the interior is parallelized with no bound-checking and direct pointer arithmetic.
- **V3 (parallel border via tasks)**: same split as V2, but the four borders are additionally parallelized using `omp task`.

Benchmark datasets are generated automatically from MS COCO 2017 images at four resolutions (512, 1024, 2048, 4096 px), and results (timing, speedup, efficiency, scalability) are collected and validated for correctness (pixel-perfect comparison against the sequential baseline) before being exported to CSV and plotted.

## Prerequisites

Make sure you have the following installed:
- **Windows** (x64)
- **Visual Studio 2019+** (MSVC), C++17
- **OpenMP** (included with MSVC)
- **Python 3.x**, with `Pillow`, `pandas`, `matplotlib`, `numpy` (installed automatically on first run if missing)

### Configuration and Compilation

1. **Clone the repository** and navigate to the project directory:
   ```bash
   git clone https://github.com/francescostocchi/Morphological_Image_Processing.git
   cd morphological-image-processing
   ```
2. **Open the solution** in Visual Studio, select **Release | x64**, then build (`Ctrl+Shift+B`).

### Running the application

Run it **from within Visual Studio** (`Ctrl+F5`, or the Start button).

> All configuration (dataset sizes, source URL, image count) is centralized in `data/dataset/config.json`. On first run, if the image database is missing or incomplete, it is downloaded and generated automatically before benchmarking starts.
