# Goal
The objective of this tiny project is to serve an object detection model and on the path, learn about C++.
The model is served leveraging the onnxruntime and interacted with via the AMQP-CPP library.
Originally, the idea was to server the model via websockets as can be told by some remnants in the source.

# Next Steps
This serves as my own TODO list.

## Milestones
2. play back detection in a certain format from CPP --> Python & parse on python side
3. Write unittests and refactor code 
4. refactoring

## Websocket utility server (stashed)
- [x] send image via websocket connection to backend
- [ ] define ouput format, send results back

## OnnxInferLib
- [ ] interface __inference : image -> result__ accepting arbitrary images
- [ ] __class InputHandler__ capable of receiving and preprocessing image
- [ ] the RUN method of the onnx_sess should be of type __Run: images -> result__
- [ ] write postprocessor to handle outputs of the model

### Input images
- [ ] robustify
- [ ] maybe integrate preprocessor into main class to determine resize sz + norm constants dynamically


### postprocessor
- [ ] parse all outputs, not just the single highest one --> store in vector

### flexibility
- [ ] read input output sizes from model, same with input output names --> no hardcoding

## model
- [x] integrate YOLOv11 instead of random model used to date
- [ ] redesign constructor to allow for other inference providers
- [ ]


## AMQP
- reset internal buffer once message has been completely processed


## GTest 
- include and write tests

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




# Refactoring

- signal handler for graceful shutdown

## main.cpp
- read yolo_model file path from config file / parse as commandline arg
- read login information for AMQP connection from config file


- onData should check if the socket is writable, then send, else delay or abort

main components: 

connection
connectionHandler
MySocket
event loop
json parsing
onnx inference
image processing




