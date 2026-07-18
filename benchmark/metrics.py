import sys
import numpy as np
import pandas as pd


def compute_metrics(csv_path: str, output_path: str) -> str:
    df = pd.read_csv(csv_path, skipinitialspace=True)
    df.columns = [c.strip() for c in df.columns]

    required = {"dataset", "elaborazione", "operazione", "tempo_medio_s", "cores", "num_images"}
    missing = required - set(df.columns)
    if missing:
        raise ValueError(f"[METRICS] Colonne mancanti nel CSV: {missing}")


    for col in ["dataset", "elaborazione", "operazione"]:
        df[col] = df[col].astype(str).str.strip()

    numeric_cols = ["tempo_medio_s", "cores", "num_images"]
    if "width" in df.columns:
        numeric_cols.append("width")
    if "height" in df.columns:
        numeric_cols.append("height")
    if "channels" in df.columns:
        numeric_cols.append("channels")

    for col in numeric_cols:
        df[col] = pd.to_numeric(df[col].astype(str).str.strip(), errors="coerce")

    n_before = len(df)
    df = df.dropna(subset=numeric_cols).reset_index(drop=True)
    n_dropped = n_before - len(df)
    if n_dropped > 0:
        print(f"[METRICS] Warning: dropped {n_dropped} non-numeric rows "
              f"(likely repeated headers in the input CSV).")

    df["cores"] = df["cores"].astype(int)
    df["num_images"] = df["num_images"].astype(int)


    df["completion_time_s"] = df["tempo_medio_s"]
    df["latency_s"] = df["tempo_medio_s"] / df["num_images"]
    df["throughput_img_s"] = df["num_images"] / df["tempo_medio_s"]

    if {"width", "height"}.issubset(df.columns):
        channels = df["channels"] if "channels" in df.columns else 1
        df["bandwidth_Bps"] = (
            df["num_images"] * df["width"] * df["height"] * channels
        ) / df["tempo_medio_s"]
    else:
        df["bandwidth_Bps"] = np.nan
        print("[METRICS] Warning: 'width'/'height' columns not found, bandwidth not computed.")

    # --- Metriche che richiedono il baseline seriale ---
    df["speedup"] = np.nan
    df["efficiency"] = np.nan
    df["scalability"] = np.nan

    group_cols = ["dataset", "operazione"]
    for keys, group in df.groupby(group_cols):
        serial_rows = group[(group["elaborazione"] == "serial") & (group["cores"] == 1)]
        if serial_rows.empty:
            print(f"[METRICS] Warning: no serial baseline for {keys}, skipping speedup/efficiency.")
            continue

        t_serial = serial_rows["tempo_medio_s"].iloc[0]
        idx = group.index

        df.loc[idx, "speedup"] = t_serial / df.loc[idx, "tempo_medio_s"]
        df.loc[idx, "efficiency"] = df.loc[idx, "speedup"] / df.loc[idx, "cores"]

        for elab, subgroup in group.groupby("elaborazione"):
            if elab == "serial":
                continue
            unique_cores = subgroup["cores"].nunique()
            if unique_cores < 2:
                print(f"[METRICS] Info: {keys}, elaborazione='{elab}' tested at a single "
                      f"core count ({subgroup['cores'].iloc[0]}) -> scalability not defined "
                      f"(needs the same implementation run at multiple core counts).")
                continue
            sub_sorted = subgroup.sort_values("cores")
            prev_time = None
            prev_cores = None
            for i, row in sub_sorted.iterrows():
                if prev_time is not None and prev_cores is not None and row["cores"] != prev_cores:
                    ideal_ratio = row["cores"] / prev_cores
                    actual_ratio = prev_time / row["tempo_medio_s"]
                    df.loc[i, "scalability"] = actual_ratio / ideal_ratio
                prev_time = row["tempo_medio_s"]
                prev_cores = row["cores"]

    df.to_csv(output_path, index=False)
    print(f"[METRICS] Metrics written to {output_path}")
    return output_path


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python metrics.py <input_csv> [output_csv]")
        sys.exit(1)

    in_path = sys.argv[1]
    out_path = sys.argv[2] if len(sys.argv) > 2 else in_path.rsplit(".", 1)[0] + "_metrics.csv"
    compute_metrics(in_path, out_path)