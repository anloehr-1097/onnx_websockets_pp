from tasks import app, cpp_worker_task, cpp_worker_task_img
from io import BytesIO
from PIL import Image
import time


img: Image.Image = Image.open("images/guac.jpeg")
bytes_img: BytesIO = BytesIO()
img.save(bytes_img, format=img.format)

if __name__ == "__main__":
    # This will load the Celery app but not start workers
    res = cpp_worker_task.apply_async(("Hello cpp.",), queue="celery")
    res_img = cpp_worker_task_img.apply_async(
        (bytes_img.getvalue(),),
        serializer="json",
        queue="yolo prediction",
        routing_key="yolo_inf",
    )
    print(f"Task id regular task = {res.task_id}")
    print(f"Task id image task = {res_img.task_id}")

    while res_img.status != "SUCCESS":
        print("Waiting ... ")
        time.sleep(1)
    print(res_img.get())
