from tasks import cpp_worker_task
import time

res = cpp_worker_task.delay("Hello cpp.")

print(f"Task id = {res.task_id}")


while True:
    if res.state == "SUCCESS":
        print(res.get())
        break
    time.sleep(2)
