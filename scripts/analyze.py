import uuid
import subprocess
import os

RECORDINGS = [
    "CID-140-recording-2020-03-18_144821-selection.rec",
    "CID-140-recording-2020-03-18_145043-selection.rec",
    "CID-140-recording-2020-03-18_145233-selection.rec",
    "CID-140-recording-2020-03-18_145641-selection.rec",
    "CID-140-recording-2020-03-18_150001-selection.rec",
]

# TODO: random for now, change later!
COMMIT_ID = subprocess.check_output(["git", "rev-parse", "--short", "HEAD"]).decode().strip()

for rec_file in RECORDINGS:
    print(f"Running with recording file '{rec_file}'")

    with open(".env", "w") as f:
        f.write(f"RECORDING_FILE={rec_file}\n")
        f.write(f"GIT_COMMIT_ID={COMMIT_ID}\n")

    try:
        subprocess.run([
        "docker", "compose", "up", "--build", "--abort-on-container-exit"
        ], check=True)
        
        subprocess.run([
            "docker", "compose", "down", "--remove-orphans"
        ], check=True)

    except subprocess.CalledProcessError as e:
        print(f"Error running docker compose for {rec_file}: {e}")
        break

    #env = os.environ.copy()
    #env["RECORDING_FILE"] = rec_file
    #env["GIT_COMMIT_ID"] = str(COMMIT_ID)

    #subprocess.run([
    #    "docker", "compose", "-f", "docker-compose.yml", "up", "--build", "--abort-on-container-exit"
    #], check=True, env=env)

    #subprocess.run(["docker", "compose", "down", "--remove-orphans"], check=True)

