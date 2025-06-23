# Goal
The objective of this tiny project is to serve an object detection model and on the path, learn about C++.
The model is served leveraging the onnxruntime and interacted with via the AMQP-CPP library.
Originally, the idea was to server the model via websockets as can be told by some remnants in the source.

# Next Steps
This serves as my own TODO list.

## Websocket utility server (stashed)
- [x] send image via websocket connection to backend
- [ ] define ouput format, send results back

## OnnxInferLib
- [x] interface __inference : image -> result__ accepting arbitrary images

### Input images
- [ ] maybe integrate preprocessor into main class to determine resize sz + norm constants dynamically


### flexibility
- [ ] read input output sizes from model, same with input output names --> no hardcoding

## model
- [x] integrate YOLOv11 instead of random model used to date



# Dependencies
- OpenCV
- OnnxRuntime
- WebSocketpp
- GTest
- Base64 (https://github.com/ReneNyffenegger/cpp-base64)

# Build

## Build & Run Commands
Use 'make' to build and run the project / services.
### Building the project
```bash
make all
```

### Running the AMQP-CPP client
```bash
make run
```

### Running the py celery app
```bash
make py-celery
```

### Running the Rabbit MQ Broker Docker Container
```bash
make run-rabbit
```

### Running the Redis Backend Docker Container
```bash
make run-redis
```

It is essential that the redis and the rabbit mq containers are up before starting the python celery app or the C++ AMQP client.

# Remarks
- export the yolo model with nms=True to onnx format.


# YOLOv11
classes 80
img size: 640 x 640
input name: images
output name: output0


# Disclaimer
As this is part of my (early) Cpp journey and I continue to be a bloody beginner, the implementation does not claim to be {memory safe, optimized, <you name it>}.

For response format to be inserted into Backend, see
"/opt/homebrew/Caskroom/miniconda/base/lib/python3.12/site-packages/celery/backends/base.py"



# Main Components
connection
connectionHandler
MySocket
event loop
json parsing
onnx inference
image processing


# 06/23/2025 Tasks sorted acc to priority

onData should check if the socket is writable, then send, else delay or abort

Define config & model for value stream model onnx inferenece 

Define docker compose file to ensure all the required services are up for testing and running the amqp consumer application

Define CI/CD scripts

Fork repo --> new repo clean

Define the task interface

signal handler for graceful shutdown

*Bug*: Sometimes, the prediction task for images is not called even when triggered from python client. After a restart of the cpp consumer, it consumes the prediction tasks infinitely. The most effective method to solve this issue is to restart the rabbit-mq message broker and the redis backend

Complete unit testing suite for AMQP module: 
	tasks
	conn_handler
	callbacks
	amqp_socket
	amqp_utils


Resolve all compiler warning


Complete unit testing suite for utils module: 
base64
debug_utils
json_utils

support GPU execution or any other execution provider

read login information for AMQP connection from config file

Check for resource leaks using sanitizers
