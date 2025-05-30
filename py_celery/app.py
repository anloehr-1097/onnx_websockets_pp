from tasks import app, cpp_worker_task, cpp_worker_task_img
from io import BytesIO
from PIL import Image


img: Image.Image = Image.open("../images/bus.jpg")
bytes_img: BytesIO = BytesIO()
img.save(bytes_img, format=img.format)

if __name__ == "__main__":
    # This will load the Celery app but not start workers
    res = cpp_worker_task.delay("Hello cpp.")
    res_img = cpp_worker_task_img.delay(bytes_img.getvalue())
    print(f"Task id regular task = {res.task_id}")
    print(f"Task id image task = {res_img.task_id}")

    while True:
        if res.state == "SUCCESS":
            print(res.get())
