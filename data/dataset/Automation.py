import os
import json
import random
import urllib.request
import shutil
import zipfile
from PIL import Image

BASE_DIR = os.path.dirname(os.path.abspath(__file__))


def p(*parts):
    return os.path.normpath(os.path.join(BASE_DIR, *parts))

CONFIG_PATH = p("config.json")

with open(CONFIG_PATH, "r", encoding="utf-8") as f:
    _config = json.load(f)

_ds_cfg = _config["dataset"]
_pipeline_cfg = _config["pipeline"]

PROJECT_ROOT = p("..", "..")

URL = _pipeline_cfg["source_url"]
DOWNLOAD_DIR = p("dataset_raw")
ZIP_PATH = os.path.join(DOWNLOAD_DIR, os.path.basename(URL))
INPUT_DIR = p("extracted")
TEMP_DIR = p("dataset_raw_temp")
FINAL_ROOT = os.path.normpath(os.path.join(PROJECT_ROOT, _ds_cfg["base_folder"]))

TARGET_SIZE = {s["name"]: s["target_size"] for s in _ds_cfg["subsets"]}
DATASET_DIRS = [os.path.join(FINAL_ROOT, name) for name in TARGET_SIZE]

for d in DATASET_DIRS:
    os.makedirs(d, exist_ok=True)

EXT = set(_ds_cfg["extensions"])

# visualizzazione progress bar(F)/log(T)
VERBOSE = _ds_cfg.get("verbose", False)

MAX_SOURCE_IMAGES = _ds_cfg.get("total_images", False)

os.makedirs(DOWNLOAD_DIR, exist_ok=True)
os.makedirs(INPUT_DIR, exist_ok=True)
os.makedirs(TEMP_DIR, exist_ok=True)

def print_progress(current, total, prefix=""):
    if total == 0:
        return
    width = 40
    filled = int(width * current / total)
    bar = "#" * filled + "-" * (width - filled)
    percent = int(100 * current / total)
    end = "\n" if current == total else ""
    print(f"\r{prefix}[{bar}] {percent}% ({current}/{total})", end=end, flush=True)


def extract(zip_path, extract_dir):
    print(f"[EXTRACTOR] Extracting main ZIP: {zip_path}")
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        zip_ref.extractall(extract_dir)
    print("[EXTRACTOR] Main extraction completed.")

    for root, dirs, files in os.walk(extract_dir):
        for file in files:
            if file.lower().endswith(".zip"):
                inner_zip_path = os.path.join(root, file)
                inner_extract_dir = os.path.splitext(inner_zip_path)[0]

                print(f"\n[EXTRACTOR] Found inner ZIP: {inner_zip_path}")
                os.makedirs(inner_extract_dir, exist_ok=True)

                with zipfile.ZipFile(inner_zip_path, 'r') as inner_zip:
                    inner_zip.extractall(inner_extract_dir)

                print("[EXTRACTOR] Inner ZIP extraction completed.")


def run_downloader():
    print("\n[DOWNLOADER] Starting download...")
    if os.path.exists(ZIP_PATH):
        print(f"[DOWNLOADER] File already present, skipping download: {ZIP_PATH}")
    else:
        print(f"[DOWNLOADER] Downloading from Archive.org:\n{URL}")
        urllib.request.urlretrieve(URL, ZIP_PATH)
        print(f"[DOWNLOADER] Downloaded to {ZIP_PATH}")
    extract(ZIP_PATH, INPUT_DIR)
    print("[DOWNLOADER] Done")


def builder():
    print(f"\n[BUILDER] Organizing images in {INPUT_DIR}/...")

    os.makedirs(TEMP_DIR, exist_ok=True)

    # FIX: os.listdir guardava solo il primo livello di INPUT_DIR, ma lo zip
    # si estrae in sottocartelle annidate (annotations/panoptic_train2017/,
    # panoptic_val2017/...), quindi non trovava mai nessuna immagine.
    all_files = []
    for root, dirs, files in os.walk(INPUT_DIR):
        for filename in files:
            if any(filename.lower().endswith(e) for e in EXT):
                all_files.append((root, filename))
                if MAX_SOURCE_IMAGES is not None and len(all_files) >= MAX_SOURCE_IMAGES:
                    break
        if MAX_SOURCE_IMAGES is not None and len(all_files) >= MAX_SOURCE_IMAGES:
            break

    total = len(all_files)
    done = 0
    for root, filename in all_files:
        path = os.path.join(root, filename)

        try:
            with Image.open(path) as img:
                w, h = img.size
        except Exception as e:
            done += 1
            if VERBOSE:
                print(f"Error with {filename}: {e}")
            else:
                print_progress(done, total, prefix="[BUILDER] ")
            continue

        dim = tuple(sorted((w, h)))  # ignora orientamento
        folder_name = f"db{dim[0]}x{dim[1]}"

        dest_folder = os.path.join(TEMP_DIR, folder_name)
        os.makedirs(dest_folder, exist_ok=True)

        shutil.copy2(path, os.path.join(dest_folder, filename))
        done += 1
        if VERBOSE:
            print(f"{filename} → {folder_name}")
        else:
            print_progress(done, total, prefix="[BUILDER] ")


def sorter():
    print("\n[SORTER] Allocating images to datasets (balanced by post-resize weight)...")

    images = []
    for root, dirs, files in os.walk(TEMP_DIR):
        for filename in files:
            if any(filename.lower().endswith(e) for e in EXT):
                images.append(os.path.join(root, filename))

    if MAX_SOURCE_IMAGES is not None:
        images = images[:MAX_SOURCE_IMAGES]

    random.shuffle(images)
    total = len(images)

    names = list(TARGET_SIZE.keys())
    inv_area = {name: 1.0 / (TARGET_SIZE[name] ** 2) for name in names}
    inv_sum = sum(inv_area.values())

    counts = {name: int(round(total * inv_area[name] / inv_sum)) for name in names}
    #per ottenere esattamente total
    diff = total - sum(counts.values())
    if diff != 0:
        counts[names[0]] += diff

    print(f"Available images: {total} -> " + ", ".join(f"{n}: {counts[n]}" for n in names))

    cursor = 0
    copied = 0
    for name in names:
        dest = os.path.join(FINAL_ROOT, name)
        os.makedirs(dest, exist_ok=True)
        chunk = images[cursor: cursor + counts[name]]
        cursor += counts[name]
        for path in chunk:
            try:
                shutil.copy2(path, os.path.join(dest, os.path.basename(path)))
                copied += 1
                if VERBOSE:
                    print(f"COPIED: {path} -> {dest}")
                else:
                    print_progress(copied, total, prefix="[SORTER] ")
            except Exception as e:
                copied += 1
                if VERBOSE:
                    print(f"ERROR copying {path} -> {dest}: {e}")
                else:
                    print_progress(copied, total, prefix="[SORTER] ")

    print("\n[SORTER] Done")


def converter():
    print("\n[CONVERTER] Resizing to fixed size + converting to grayscale...")

    all_files = []
    for name in TARGET_SIZE:
        folder = os.path.join(FINAL_ROOT, name)
        if not os.path.isdir(folder):
            continue
        for filename in os.listdir(folder):
            if any(filename.lower().endswith(e) for e in EXT):
                all_files.append((name, folder, filename))

    total = len(all_files)
    done = 0
    for name, folder, filename in all_files:
        target = TARGET_SIZE[name]
        path = os.path.join(folder, filename)

        try:
            img = Image.open(path)
            img = img.resize((target, target), Image.BICUBIC)
            gray = img.convert("L")
            gray.save(path)
            done += 1
            if VERBOSE:
                print(f"Converted: {path} -> {target}x{target}")
            else:
                print_progress(done, total, prefix="[CONVERTER] ")

        except Exception as e:
            done += 1
            if VERBOSE:
                print(f"Error with {path}: {e}")
            else:
                print_progress(done, total, prefix="[CONVERTER] ")


def run_pipeline():
    print("\n=== INITIAL AUTOMATION ===")
    run_downloader()
    builder()
    sorter()
    converter()
    print("\n=== DONE ===")
    print("=== Removing intermediate folders ===")
    shutil.rmtree(TEMP_DIR, ignore_errors=True)
    shutil.rmtree(TEMP_DIR, ignore_errors=True)
    shutil.rmtree(INPUT_DIR, ignore_errors=True)
    print("=== Intermediate folders removed ===")


if __name__ == "__main__":
    run_pipeline()