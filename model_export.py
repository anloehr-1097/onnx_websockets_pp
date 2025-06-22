from ultralytics import YOLO

MODEL_NAME = "model.pt"

model = YOLO(MODEL_NAME)
model.export(format="onnx", nms=True)
