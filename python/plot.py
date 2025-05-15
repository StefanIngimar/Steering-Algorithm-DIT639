import os
import pandas as pd
import matplotlib.pyplot as plt

INPUT_PATH = "/app/data/steeringAngles.csv"
OUTPUT_PATH = "/app/data/steering_plot.png"

assert os.path.exists(INPUT_PATH), f"Input file not found: {INPUT_PATH}"

df = pd.read_csv(INPUT_PATH, sep=";")

print("Data preview:")
print(df.head())

if df.empty:
    print("Dataset is empty")
else:
    plt.figure(figsize=(10, 5))
    plt.plot(df["Timestamp"], df["PredictedSteeringAngle"], label="Predicted")
    plt.plot(df["Timestamp"], df["ActualSteeringAngle"], label="Actual")
    plt.xlabel("Elapsed time in microseconds")
    plt.ylabel("Steering Angle")
    plt.legend()
    plt.title("Steering Angle over time")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(OUTPUT_PATH)
    print(f"Saved plot to {OUTPUT_PATH}")
