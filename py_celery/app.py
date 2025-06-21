from tasks import app, cpp_worker_task, cpp_worker_task_img
from io import BytesIO
from pathlib import Path
from PIL import Image
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import time
import pydantic
import json


class PredResult(pydantic.BaseModel):
    result: list


def draw(img: Image.Image, res: PredResult):

    # Create figure and axes
    fig, ax = plt.subplots()

    # Display the image
    ax.imshow(img)

    for elem in res.result:
        rect = patches.Rectangle(
            elem[:2], elem[2], elem[3], linewidth=1, edgecolor="r", facecolor="none"
        )

        # Add the patch to the Axes
        ax.add_patch(rect)
        ax.annotate(f"{elem[-2]}", elem[:2], color="blue", weight="bold", fontsize=8)

    plt.show()
    return


im_path: Path = Path(__file__).parent.parent / Path("images/bus.jpg")
# img: Image.Image = Image.open("images/bus.jpg")
img: Image.Image = Image.open(str(im_path))
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
        print(res_img.status)
        print("Waiting ... ")
        time.sleep(1)
    res = res_img.get()
    js_res = json.loads(res)
    pred = PredResult(result=js_res)
    print(pred)
    draw(img.resize((640, 640)), pred)
