import subprocess
import time
from playwright.sync_api import sync_playwright
from os import environ, getcwd
# THIS IS A PLAN B FOR A21
RECORDING_NAME = "CID-140-recording-2020-03-18_144821-selection.rec"
DISPLAY = environ.get("DISPLAY", ":0")
PWD = getcwd()

def docker_image_exists(image_name):
    try:
        subprocess.run(["docker", "image", "inspect", image_name],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
        return True
    except subprocess.CalledProcessError:
        return False

def start_services():
    print("Starting opendlv-vehicle-view...")
    subprocess.run([
        "docker", "run", "--rm", "-d", "--init", "--net=host",
        "--name=opendlv-vehicle-view",
        "-v", f"{PWD}/res/video_feeds:/opt/vehicle-view/recordings",
        "-v", "/var/run/docker.sock:/var/run/docker.sock",
        "-p", "8081:8081",
        "chrberger/opendlv-vehicle-view:v0.0.64"
    ], check=True)

    if not docker_image_exists("h264decoder:v0.0.5"):
        print("Building h264-decoder...")
        subprocess.run([
            "docker", "build",
            "https://github.com/chalmers-revere/opendlv-video-h264-decoder.git#v0.0.5",
            "-f", "Dockerfile", "-t", "h264decoder:v0.0.5"
        ], check=True)

    print("Starting h264-decoder...")
    subprocess.run([
        "docker", "run", "--rm", "-d", "--net=host", "--ipc=host",
        "-e", f"DISPLAY={DISPLAY}", "-v", "/tmp:/tmp",
        "--name", "h264-decoder",
        "h264decoder:v0.0.5", "--cid=253", "--name=img"
    ], check=True)

    print("Waiting for services to stabilize...")
    time.sleep(5)

def build_nutmeg_if_needed():
    if not docker_image_exists("nutmeg:latest"):
        print("🔧 Building nutmeg image...")
        subprocess.run(["docker", "build", "-f", "Dockerfile", "-t", "nutmeg", "."], check=True)
    else:
        print("Nutmeg image already built.")

def start_nutmeg():
    print("Starting nutmeg...")
    return subprocess.Popen([
        "docker", "run", "--rm", "--net=host", "--ipc=host",
        "-e", f"DISPLAY={DISPLAY}", "-v", "/tmp:/tmp",
        "nutmeg:latest", "--cid=253", "--name=img",
        "--width=640", "--height=480", "--verbose"
    ])

# Start everything
start_services()
build_nutmeg_if_needed()

# Playwright to control UI
with sync_playwright() as p:
    browser = p.chromium.launch(headless=False)
    page = browser.new_page()

    page.goto("http://localhost:8081")
    page.click("#recordings")
    page.wait_for_selector("button.far.fa-play-circle")

    buttons = page.locator("button.far.fa-play-circle")
    count = buttons.count()
    found = False

    for i in range(count):
        btn = buttons.nth(i)
        row = btn.locator("xpath=ancestor::tr")
        if RECORDING_NAME in row.inner_text():
            print(f"Playing {RECORDING_NAME}")
            btn.click()
            found = True
            break

    if not found:
        print(f"Could not find play button for {RECORDING_NAME}")
        browser.close()
        exit(1)

    page.wait_for_selector("#playButton")
    page.click("#playButton")
    time.sleep(3)

    nutmeg_proc = start_nutmeg()

    page.click("#replayStartOver")
    #page.click("#playButton")

    print("Letting video play out...")
    time.sleep(40)

    print("Killing nutmeg...")
    nutmeg_proc.terminate()
    nutmeg_proc.wait()

    browser.close()
    print("✅ Done.")

