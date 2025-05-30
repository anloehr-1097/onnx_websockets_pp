from celery import Celery
from io import BytesIO

app = Celery(
    "main",
    broker="amqp://guest@localhost//",
    backend="redis://localhost:6379/0",
)


@app.task(name="cpp_worker_task")
def cpp_worker_task(inp: str):
    raise NotImplementedError("Implemented in CPP")


@app.task(name="cpp_worker_task_with_img")
def cpp_worker_task_img(image: BytesIO):
    raise NotImplementedError("Also implemented in CPP")
