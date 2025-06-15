from typing import Optional
from fastapi import FastAPI, UploadFile
from pydantic import BaseModel
from io import BytesIO
from PIL import Image
import yaml
from pathlib import Path
from tasks import cpp_worker_task_img, app
from celery.result import AsyncResult


cls_path: Path = Path(__file__).parent / Path("coco.yaml")
assert cls_path.exists(), f"Make sure class path {cls_path} exists."
with open(cls_path, "r") as fp:
    CLASSES = yaml.safe_load(fp)["names"]

fast_app = FastAPI()


@fast_app.post("/predict")
def predict(img_file: UploadFile) -> str:
    img: Image.Image = Image.open(img_file.file)
    bytes_img: BytesIO = BytesIO()
    img.save(bytes_img, format=img.format)

    res_img = cpp_worker_task_img.apply_async(
        (bytes_img.getvalue(),), serializer="json", queue="yolo prediction"
    )
    return res_img.task_id


@fast_app.get("/predict")
def get_prediction(task_id: str) -> Optional[str]:
    res: AsyncResult = AsyncResult(task_id, app=app)
    if res.status == "SUCCESS":
        return CLASSES[int(res.get())]
    else:
        return None

    return res


@fast_app.get("/")
def root():
    return {"Hello": "World"}
