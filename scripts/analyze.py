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

    env = os.environ.copy()
    env["RECORDING_FILE"] = rec_file
    env["GIT_COMMIT_ID"] = str(COMMIT_ID)


    #subprocess.run([
    #    "docker", "compose", "-f", "docker-compose.yml", "up", "--build", "--exit-code-from", "karen"
    #], check=True, env=env)

    subprocess.run([
    "docker", "compose", "-f", "docker-compose.yml", "up", "--build", "-d"
    ], check=True, env=env)

    subprocess.run([
    "docker", "compose", "-f", "docker-compose.yml", "wait", "karen"
    ], check=True)

    for service in ["producer", "bridge", "nutmeg", "h264-decoder", "karen"]:
    subprocess.run([
        "docker", "compose", "-f", "docker-compose.yml", "logs", service
    ], check=True)
 
    subprocess.run([
    "docker", "compose", "-f", "docker-compose.yml", "down", "--remove-orphans"
    ], check=True)

    #subprocess.run(["docker", "compose", "down", "--remove-orphans"], check=True)

