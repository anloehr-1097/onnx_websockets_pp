from celery import Celery
from kombu import Exchange, Queue
from io import BytesIO

app = Celery(
    "main",
    broker="amqp://guest@localhost//",
    backend="redis://localhost:6379/0",
)

task_routes = {
    "tasks.cpp_worker_task": {"queue": "celery"},
    "tasks.cpp_worker_task_with_img": {"queue": "yolo prediction"},
}
app.conf.task_queues = (
    Queue("celery", Exchange("celery"), routing_key="celery", durable=False),
    Queue(
        "yolo prediction", Exchange("yolo_pred"), routing_key="yolo_inf", durable=False
    ),
)
app.conf.task_routes = task_routes


@app.task(name="cpp_worker_task")
def cpp_worker_task(inp: str):
    raise NotImplementedError("Implemented in CPP")


@app.task(name="cpp_worker_task_with_img")
def cpp_worker_task_img(image: BytesIO) -> str:
    raise NotImplementedError("Also implemented in CPP")
