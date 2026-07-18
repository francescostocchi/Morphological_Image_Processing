import subprocess
import sys
import importlib

REQUIRED = [
    "Pillow"
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

    print("[SETUP] Launching Automation.py...")
    subprocess.check_call([sys.executable, "data/dataset/Automation.py"])
