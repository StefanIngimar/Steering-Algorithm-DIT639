import uuid
import subprocess
import os

RECORDINGS = [
    "CID-140-recording-2020-03-18_144821-selection.rec",
    # "CID-140-recording-2020-03-18_145043-selection.rec",
    # "CID-140-recording-2020-03-18_145233-selection.rec",
    # "CID-140-recording-2020-03-18_145641-selection.rec",
    # "CID-140-recording-2020-03-18_150001-selection.rec",
]

COMMIT_ID = subprocess.check_output(["git", "rev-parse", "--short", "HEAD"]).decode().strip()
#COMMIT_ID = uuid.uuid4()

for rec_file in RECORDINGS:
    print(f"Running with recording file '{rec_file}'")

    #env = os.environ.copy()
    #env["RECORDING_FILE"] = rec_file
    #env["GIT_COMMIT_ID"] = str(COMMIT_ID)
    with open(".env", "w") as f:
        f.write(f"RECORDING_FILE={rec_file}\n")
        f.write(f"GIT_COMMIT_ID={COMMIT_ID}\n")


    subprocess.run([
        "docker", "compose", "-f", "docker-compose.yml", "up", "--build", "--exit-code-from", "karen",
    ], check=True, env=env)

    subprocess.run([
    "docker", "run", "--rm",
    "-e", f"GIT_COMMIT_ID={COMMIT_ID}",
    "-e", f"RECORDING_FILE={rec_file}",
    "-v", f"{os.getcwd()}/data:/app/data",
    "2025-group-04-karen",
    "python", "src/plotting/commit_comparison.py",
    "--commit_id", str(COMMIT_ID),
    "--video_file", rec_file
])

    subprocess.run(["docker", "compose", "down", "--remove-orphans"], check=True)

