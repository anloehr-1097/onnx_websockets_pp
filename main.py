#!/usr/bin/env python

from pathlib import Path
from websockets.sync.client import connect
from PIL import Image

img_path: Path = Path("test_img.png")
img = Image.open(img_path)


def client():
    with connect("ws://localhost:9002") as websocket:
        # websocket.send("Hello world!")
        websocket.send(img.tobytes(), text=False)
        message = websocket.recv()
        print(f"Received: {message}")


if __name__ == "__main__":
    client()
