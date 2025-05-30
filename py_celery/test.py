from tasks import cpp_worker_task, cpp_worker_task_img
import time
from io import BytesIO
from PIL import Image


img: Image.Image = Image.open("../images/bus.jpg").convert("RGB")
bytes_img: BytesIO = BytesIO()
img.save(bytes_img)

res = cpp_worker_task.delay("Hello cpp.")
res_img = cpp_worker_task_img.delay(bytes_img)

print(f"Task id = {res.task_id}")


while True:
    if res.state == "SUCCESS":
        print(res.get())
        break
    time.sleep(2)
