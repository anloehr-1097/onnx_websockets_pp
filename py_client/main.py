from io import BytesIO
from pathlib import Path
from websockets.sync.client import connect
from PIL import Image

img_path: Path = Path("../images/cat.jpeg")
img = Image.open(img_path).convert("RGB")


def client():
    with connect("ws://localhost:9002") as websocket:
        # websocket.send("Hello world!")

        # need to save image in Bytesio buffer, cannot directly send the .tobytes() result
        bytesio: BytesIO = BytesIO()
        img.save(bytesio, format="JPEG")

        websocket.send(bytesio.getvalue(), text=False)
        message = websocket.recv()
        print(f"Received: {message}")


if __name__ == "__main__":
    client()
