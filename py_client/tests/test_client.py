import unittest 
from websockets.sync.client import connect
import subprocess
import re
from io import BytesIO
from PIL import Image

from pathlib import Path
class ServerIntegrationTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.root_path = Path(__file__).resolve().parent.parent.parent
        cls.server_path = cls.root_path / Path("build/src/main")
        assert cls.server_path.exists(), f"server path {cls.server_path} does not exist"
        cls.server_process = subprocess.Popen(
            [f"{cls.server_path}", "--test-mode", "9003"],
            stdout=subprocess.PIPE
        )

        while True:
            line = cls.server_process.stdout.readline()
            print(line)
            if b"Server ready." in line:
                break

    def test_request(self):
        img_path: Path = self.root_path / Path("images/test_img_broccoli.jpeg")
        img = Image.open(img_path).convert("RGB")
        with connect("ws://localhost:9003") as websocket:

            bytesio: BytesIO = BytesIO()
            img.save(bytesio, format="JPEG")

            websocket.send(bytesio.getvalue(), text=False)
            message = websocket.recv()
            print(f"Received: {message}")

            res_str: str = re.findall(pattern=r"result:\s+\d+", string=message)[0]
            assert res_str, "No pattern 'result: [NUMBER]' in message."
            res: str = re.findall(pattern=r"\d+", string=res_str)[0]
            assert res, "Could not extract number from string."
            self.assertEqual(int(res), 50, "Broccoli not detected")

    @classmethod
    def tearDownClass(cls):
        cls.server_process.terminate()


if __name__ == "__main__":
    unittest.main()
