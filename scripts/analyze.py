import subprocess
import os
import time

RECORDINGS = [
    "CID-140-recording-2020-03-18_144821-selection.rec",
    "CID-140-recording-2020-03-18_145043-selection.rec",
    "CID-140-recording-2020-03-18_145233-selection.rec",
    "CID-140-recording-2020-03-18_145641-selection.rec",
    "CID-140-recording-2020-03-18_150001-selection.rec",
]

COMMIT_ID = subprocess.check_output(["git", "rev-parse", "--short", "HEAD"]).decode().strip()

for rec_file in RECORDINGS:
    print(f"Running with recording file '{rec_file}'")

    with open(".env", "w") as f:
        f.write(f"RECORDING_FILE={rec_file}\n")
        f.write(f"GIT_COMMIT_ID={COMMIT_ID}\n")

    compose_proc = subprocess.Popen([
        "docker", "compose", "-f", "docker-compose.yml", "up", "--build"
    ])

    print("[INFO] Launched containers, monitoring nutmeg...")

    while True:
        result = subprocess.run([
            "docker", "compose", "-f", "docker-compose.yml", "ps", "--status=exited"
        ], capture_output=True, text=True)

        if "nutmeg" in result.stdout:
            print("[INFO] Nutmeg exited. Stopping h264-decoder...")
            subprocess.run([
                "docker", "compose", "-f", "docker-compose.yml", "stop", "h264-decoder"
            ], check=True)
            break
        time.sleep(2)

    compose_proc.wait()

    print("[INFO] Cleaning up containers...")
    subprocess.run([
        "docker", "compose", "-f", "docker-compose.yml", "down", "--remove-orphans"
    ], check=True) 


os.remove(".env")

