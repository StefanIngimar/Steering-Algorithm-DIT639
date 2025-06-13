import os
import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path
import shutil

# --- CONFIGURATION ---
CSV_BASE = Path("comparison_csv")
PLOT_BASE = Path("comparison_plots")
MAX_COMMITS = 10

# double check whether this is passed. not sure if i did that
current_commit = os.environ.get("CI_COMMIT_SHA", "test_commit")

def get_previous_commit(dir_path: Path):
    """Returns the most recent commit before the current one"""
    commits = sorted([d.name for d in dir_path.iterdir() if d.is_dir() and d.name != current_commit])
    return commits[-1] if commits else None

def cleanup_old_commits(base_path: Path):
    """Keeps only the latest MAX_COMMITS folders per rec-file"""
    for rec_dir in base_path.iterdir():
        if not rec_dir.is_dir():
            continue
        commits = sorted([d for d in rec_dir.iterdir() if d.is_dir()], key=lambda x: x.stat().st_mtime, reverse=True)
        for old_commit in commits[MAX_COMMITS:]:
            shutil.rmtree(old_commit)

def plot_comparison(rec_name: str):
    rec_csv_dir = CSV_BASE / rec_name
    rec_plot_dir = PLOT_BASE / rec_name
    current_csv = rec_csv_dir / current_commit / "steeringAngles.csv"
    prev_commit = get_previous_commit(rec_csv_dir)
    prev_csv = rec_csv_dir / prev_commit / "steeringAngles.csv" if prev_commit else None
    output_plot = rec_plot_dir / f"comparison_{current_commit}.png"

    if not current_csv.exists():
        print(f"Missing current CSV for {rec_name}")
        return

    current_df = pd.read_csv(current_csv, sep=";")
    if current_df.empty:
        print(f"Current CSV is empty for {rec_name}")
        return

    plt.figure(figsize=(10, 5))
    plt.plot(current_df["Timestamp"], current_df["PredictedSteeringAngle"], label="Current Commit", alpha=0.7)
    plt.plot(current_df["Timestamp"], current_df["ActualSteeringAngle"], label="GroundTruth", alpha=0.7)

    if prev_csv and prev_csv.exists():
        prev_df = pd.read_csv(prev_csv, sep=";")
        if not prev_df.empty:
            plt.plot(prev_df["Timestamp"], prev_df["PredictedSteeringAngle"], label="Previous Commit", alpha=0.7)

    plt.title(f"Steering Comparison for {rec_name}")
    plt.xlabel("sampleTime in microseconds")
    plt.ylabel("Steering Angle")
    plt.legend()
    plt.grid(True)
    rec_plot_dir.mkdir(parents=True, exist_ok=True)
    plt.tight_layout()
    plt.savefig(output_plot)
    print(f"Saved plot to {output_plot}")
    plt.close()

# --- MAIN WORKFLOW ---
for rec_dir in CSV_BASE.iterdir():
    if not rec_dir.is_dir():
        continue
    plot_comparison(rec_dir.name)

# Cleanup old commits
cleanup_old_commits(CSV_BASE)
cleanup_old_commits(PLOT_BASE)

