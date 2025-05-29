from main import hello
import time

res = hello.delay()

print(f"Task id = {res.task_id}")


while True:
    if res.status == "SUCCESS":
        print(res.result)
        break
    time.sleep(2)
