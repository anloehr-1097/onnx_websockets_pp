# Goal
I am convinced that learning or gaining a better understanding of a programming language is best done by implementing projects 
in this language. 

The objective of this tiny project is to serve an object detection model
leveraging the onnxruntime and provide an inference endpoint via a websocket connection.
This is part of my Cpp journey. As I continue to be a bloody beginner, the implementation does not claim to be {memory safe, optimized, <you name it>}.

# Next Steps
This serves as my own TODO list.

## Websocket utility server
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
- extend handlers to parse json, reset internal buffer once message has been completely processed
- send integrate image processing on bytes frame

# Status
First baby steps have been taken toward the goal, but a lot of work remains to be done.


## GTest include and write tests

# Dependencies
- OpenCV
- OnnxRuntime
- WebSocketpp
- GTest

# Build

## Modifications to CMakeLists.txt
- [ ] TODO
## Build Commands
After adapting the CMakeLists.txt, the following commands build an executable called 'main' in the build folder.
```
mkdir build && cd build && cmake ..
cmake --build .

```

# Remarks
- export the yolo model with nms=True to onnx format.


# YOLOv11
classes 80
img size: 640 x 640
input name: images
output name: output0
