import subprocess
import sys
import importlib

REQUIRED = [
    "Pillow",
    "matplotlib",
    "pandas"
]

def install(package):
    print(f"[SETUP] Installation of {package}...")
    subprocess.check_call([sys.executable, "-m", "pip", "install", package])

def ensure_dependencies():
    for pkg in REQUIRED:
        try:
            importlib.import_module(pkg.replace("-", "_"))
            print(f"[SETUP] {pkg} already installed.")
        except ImportError:
            install(pkg)

if __name__ == "__main__":
    ensure_dependencies()
    csv_path = sys.argv[1]

    print("[SETUP] Computing derived metrics (speedup, efficiency, scalability, ...)...")
    metrics_csv_path = csv_path.rsplit(".", 1)[0] + "_metrics.csv"
    subprocess.check_call([sys.executable, "benchmark/metrics.py", csv_path, metrics_csv_path])

    print("[SETUP] Launching graphs.py...")
    subprocess.check_call([sys.executable, "benchmark/graphs.py", metrics_csv_path])
