from tasks import app, cpp_worker_task


if __name__ == "__main__":
    # This will load the Celery app but not start workers
    res = cpp_worker_task.delay("Hello cpp.")

    print(f"Task id = {res.task_id}")

    while True:
        if res.state == "SUCCESS":
            print(res.get())
