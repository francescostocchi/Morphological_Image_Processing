import sys
import os
import pandas as pd
import matplotlib.pyplot as plt

csv_file = sys.argv[1]

run_folder = os.path.dirname(csv_file)

images_folder = os.path.join(run_folder, "images")
os.makedirs(images_folder, exist_ok=True)

summary_folder = os.path.join(images_folder, "summary")
best_speedup_folder = os.path.join(images_folder, "best_speedup")
best_efficiency_folder = os.path.join(images_folder, "best_efficiency")
throughput_folder = os.path.join(images_folder, "throughput")
for folder in (summary_folder, best_speedup_folder, best_efficiency_folder, throughput_folder):
    os.makedirs(folder, exist_ok=True)

base_name = os.path.splitext(os.path.basename(csv_file))[0]
base_png = os.path.join(images_folder, base_name)
base_summary = os.path.join(summary_folder, base_name)
base_best_speedup = os.path.join(best_speedup_folder, base_name)
base_best_efficiency = os.path.join(best_efficiency_folder, base_name)
base_throughput = os.path.join(throughput_folder, base_name)

df = pd.read_csv(csv_file)

# Safety net: metrics.py should already produce a clean CSV, but just in case
# a raw CSV (with repeated headers) is passed directly to graphs.py.
df = df[df["dataset"] != "dataset"]

operazioni_ordine = ["Erosion", "Dilation", "Opening", "Closing"]
df = df[df["operazione"].isin(operazioni_ordine)]

# Columns produced by metrics.py. Only the ones actually present are used.
numeric_cols = [
    "tempo_medio_s", "cores", "num_images",
    "completion_time_s", "latency_s", "throughput_img_s",
    "bandwidth_Bps", "speedup", "efficiency", "scalability",
]
numeric_cols = [c for c in numeric_cols if c in df.columns]
for col in numeric_cols:
    df[col] = pd.to_numeric(df[col], errors="coerce")

agg_cols = [c for c in numeric_cols if c not in ("cores",)]
df = df.groupby(
    ["dataset", "elaborazione", "cores", "operazione"],
    as_index=False
)[agg_cols].mean()

has_bandwidth = "bandwidth_Bps" in df.columns and df["bandwidth_Bps"].notna().any()
has_scalability = "scalability" in df.columns and df["scalability"].notna().any()

datasets = df["dataset"].unique()

for dataset in datasets:
    df_ds = df[df["dataset"] == dataset]
    dataset_safe = dataset.replace("/", "_").replace("\\", "_")

    # --- Serial baseline table ---
    df_serial = df_ds[df_ds["elaborazione"] == "serial"].copy()
    df_serial = df_serial.set_index("operazione").reindex(operazioni_ordine)

    df_nonserial = df_ds[df_ds["elaborazione"] != "serial"].copy()
    df_nonserial = df_nonserial.sort_values(["elaborazione", "cores", "operazione"])

    gruppi = df_nonserial.groupby(["elaborazione", "cores"])

    # --- Plot 1: tempi medi ---
    fig, ax = plt.subplots(figsize=(10, 6))

    for (elab, cores), gruppo in gruppi:
        gruppo = gruppo.set_index("operazione").reindex(operazioni_ordine)
        ax.plot(
            operazioni_ordine,
            gruppo["tempo_medio_s"],
            marker="o",
            label=f"{elab} ({cores} cores)"
        )

    ax.set_title(f"Tempi medi - {dataset}")
    ax.set_xlabel("Operazione", labelpad=-10)
    ax.set_ylabel("Tempo medio (s)")
    ax.grid(True, linestyle="--", alpha=0.4)
    ax.legend(bbox_to_anchor=(1.05, 1), loc="upper left")

    serial_table = df_serial[["tempo_medio_s"]]
    serial_table.index.name = "Operazione"

    plt.table(
        cellText=serial_table.values,
        rowLabels=serial_table.index,
        colLabels=["Tempo serial (s)"],
        cellLoc="center",
        rowLoc="center",
        loc="bottom",
        bbox=[0.0, -0.45, 1, 0.35]
    )

    plt.subplots_adjust(left=0.1, bottom=0.35)

    png_name = f"{base_png}_{dataset_safe}.png"
    plt.savefig(png_name, dpi=300, bbox_inches="tight")
    plt.close()
    print(f"Generated: {png_name}")

    # --- Plot 2: summary table ---
    df_summary = df_ds.sort_values(["elaborazione", "cores", "operazione"])

    fig, ax = plt.subplots(figsize=(14, 6))
    ax.axis("off")

    table = plt.table(
        cellText=df_summary.round(4).values,
        colLabels=df_summary.columns,
        cellLoc="center",
        loc="center"
    )

    table.auto_set_font_size(False)
    table.set_fontsize(8)
    table.scale(1, 1.5)

    png_summary = f"{base_summary}_{dataset_safe}_summary.png"
    plt.savefig(png_summary, dpi=300, bbox_inches="tight")
    plt.close()
    print(f"Generated: {png_summary}")

    # --- Plot 3: migliore speedup per operazione ---
    df_best_speedup = df_nonserial.loc[
        df_nonserial.groupby("operazione")["speedup"].idxmax()
    ].set_index("operazione").reindex(operazioni_ordine)

    fig, ax = plt.subplots(figsize=(10, 6))
    ax.plot(operazioni_ordine, df_best_speedup["speedup"], marker="o", linewidth=2)

    labels = [
        f"{row['elaborazione']} ({int(row['cores'])} cores)" if pd.notna(row['cores']) else "N/A"
        for _, row in df_best_speedup.iterrows()
    ]
    for x, y, label in zip(operazioni_ordine, df_best_speedup["speedup"], labels):
        ax.text(x, y, label, fontsize=9, ha="center", va="bottom")

    ax.set_title(f"Migliori speedup - {dataset}")
    ax.set_xlabel("Operazione")
    ax.set_ylabel("Speedup massimo")
    ax.grid(True, linestyle="--", alpha=0.4)

    png_best = f"{base_best_speedup}_{dataset_safe}_best_speedup.png"
    plt.savefig(png_best, dpi=300, bbox_inches="tight")
    plt.close()
    print(f"Generated: {png_best}")

    # --- Plot 4: migliore efficienza per operazione ---
    df_best_eff = df_nonserial.loc[
        df_nonserial.groupby("operazione")["efficiency"].idxmax()
    ].set_index("operazione").reindex(operazioni_ordine)

    fig, ax = plt.subplots(figsize=(10, 6))
    ax.plot(operazioni_ordine, df_best_eff["efficiency"], marker="o", linewidth=2, color="darkorange")

    labels = [
        f"{row['elaborazione']} ({int(row['cores'])} cores)" if pd.notna(row['cores']) else "N/A"
        for _, row in df_best_eff.iterrows()
    ]
    for x, y, label in zip(operazioni_ordine, df_best_eff["efficiency"], labels):
        ax.text(x, y, label, fontsize=9, ha="center", va="bottom")

    ax.set_title(f"Migliore efficienza - {dataset}")
    ax.set_xlabel("Operazione")
    ax.set_ylabel("Efficienza massima (speedup / cores)")
    ax.grid(True, linestyle="--", alpha=0.4)

    png_best_eff = f"{base_best_efficiency}_{dataset_safe}_best_efficiency.png"
    plt.savefig(png_best_eff, dpi=300, bbox_inches="tight")
    plt.close()
    print(f"Generated: {png_best_eff}")

    # --- Plot 5: throughput (img/s) per operazione ---
    fig, ax = plt.subplots(figsize=(10, 6))

    for (elab, cores), gruppo in df_nonserial.groupby(["elaborazione", "cores"]):
        gruppo = gruppo.set_index("operazione").reindex(operazioni_ordine)
        ax.plot(
            operazioni_ordine,
            gruppo["throughput_img_s"],
            marker="o",
            label=f"{elab} ({cores} cores)"
        )
    serial_throughput = df_serial["throughput_img_s"]
    ax.plot(operazioni_ordine, serial_throughput, marker="s", linestyle="--",
            color="black", label="serial")

    ax.set_title(f"Throughput - {dataset}")
    ax.set_xlabel("Operazione")
    ax.set_ylabel("Immagini / secondo")
    ax.grid(True, linestyle="--", alpha=0.4)
    ax.legend(bbox_to_anchor=(1.05, 1), loc="upper left")

    png_throughput = f"{base_throughput}_{dataset_safe}_throughput.png"
    plt.savefig(png_throughput, dpi=300, bbox_inches="tight")
    plt.close()
    print(f"Generated: {png_throughput}")

    # --- Plot 6 (opzionale): bandwidth ---
    if has_bandwidth:
        fig, ax = plt.subplots(figsize=(10, 6))
        for (elab, cores), gruppo in df_nonserial.groupby(["elaborazione", "cores"]):
            gruppo = gruppo.set_index("operazione").reindex(operazioni_ordine)
            ax.plot(
                operazioni_ordine,
                gruppo["bandwidth_Bps"],
                marker="o",
                label=f"{elab} ({cores} cores)"
            )
        ax.plot(operazioni_ordine, df_serial["bandwidth_Bps"], marker="s",
                linestyle="--", color="black", label="serial")

        ax.set_title(f"Bandwidth - {dataset}")
        ax.set_xlabel("Operazione")
        ax.set_ylabel("Byte / secondo")
        ax.grid(True, linestyle="--", alpha=0.4)
        ax.legend(bbox_to_anchor=(1.05, 1), loc="upper left")

        png_bandwidth = f"{base_png}_{dataset_safe}_bandwidth.png"
        plt.savefig(png_bandwidth, dpi=300, bbox_inches="tight")
        plt.close()
        print(f"Generated: {png_bandwidth}")

    # --- Plot 7 (opzionale): scalability vs cores ---
    if has_scalability:
        fig, ax = plt.subplots(figsize=(10, 6))
        for (elab, operazione), gruppo in df_nonserial.groupby(["elaborazione", "operazione"]):
            gruppo = gruppo.dropna(subset=["scalability"]).sort_values("cores")
            if gruppo.empty:
                continue
            ax.plot(
                gruppo["cores"], gruppo["scalability"],
                marker="o", label=f"{elab} - {operazione}"
            )

        ax.axhline(1.0, color="gray", linestyle="--", linewidth=1, label="ideale")
        ax.set_title(f"Scalability - {dataset}")
        ax.set_xlabel("Cores")
        ax.set_ylabel("Scalability (speedup reale / speedup ideale)")
        ax.grid(True, linestyle="--", alpha=0.4)
        ax.legend(bbox_to_anchor=(1.05, 1), loc="upper left")

        png_scalability = f"{base_png}_{dataset_safe}_scalability.png"
        plt.savefig(png_scalability, dpi=300, bbox_inches="tight")
        plt.close()
        print(f"Generated: {png_scalability}")