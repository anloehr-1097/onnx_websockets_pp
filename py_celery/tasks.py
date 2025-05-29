from celery import Celery

app = Celery(
    "main",
    broker="amqp://guest@localhost//",
    backend="redis://localhost:6379/0",
)


@app.task(name="cpp_worker_task")
def cpp_worker_task(inp: str):
    raise NotImplementedError("Implemented in CPP")
