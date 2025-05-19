import os
import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

CSV_DIR = Path("/data/csv")
PLOT_DIR = Path("/data/plots")
PLOT_DIR.mkdir(parents=True, exist_ok=True)

csv_files = sorted(CSV_DIR.glob("steeringAngles_*.csv"))

if not csv_files:
    raise FileNotFoundError(f"No CSV files found in {CSV_DIR}")
# generate plot for each csv file available. if a csv already has a corresponding plot, skip it
for csv_path in csv_files:
    timestamp = csv_path.stem.replace("steeringAngles_", "")
    output_path = PLOT_DIR / f"steering_plot_{timestamp}.png"

    if output_path.exists():
        print(f"plot already exists for {csv_path.name}, skipping.")
        continue

    df = pd.read_csv(csv_path, sep=";")
    if df.empty:
        print(f"{csv_path} is empty")
        continue

    plt.figure(figsize=(10, 5))
    plt.text(0.5, 0.5, "Group 4", fontsize=50, ha="center", va="center", transform=plt.gca().transAxes)
    plt.plot(df["Timestamp"], df["PredictedSteeringAngle"], label="Predicted")
    plt.plot(df["Timestamp"], df["ActualSteeringAngle"], label="Actual")
    plt.xlabel("sampleTime in microseconds")
    plt.ylabel("Steering Angle")
    plt.legend()
    plt.title("Steering Angle over time")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(output_path)
    print(f"Saved plot to {output_path}")
