import onnxruntime
from pathlib import Path
from PIL import Image
import onnx
import numpy


def load_model(model_name: str):
    session = onnxruntime.InferenceSession(model_name, None)

    # get the name of the first input of the model
    input_name = session.get_inputs()[0].name
    input_shape = session.get_inputs()[0].shape
    output_name = session.get_outputs()[0].name
    print('Input Name:', input_name)
    print('Output Name:', output_name)
    return session, input_name, input_shape, output_name


def load_image(im_path: Path, input_shape) -> numpy.ndarray:
    image: Image.Image = Image.open(im_path).convert("RGB")
    image = image.resize(input_shape[2:])
    return numpy.asarray(image)
        
def preprocess_image(img: numpy.ndarray) -> numpy.ndarray:
    img = img.astype(numpy.float32)
    img /= 255
    img = img.transpose((2,0,1))
    img = img[None, :]
    return img


def postprocess_onnx_out(out: list[numpy.ndarray]) -> numpy.ndarray:
    # assume only one output in list
    pred = out[0][0, :, :]

    # filter pred
    pred = pred[pred[:, 5] > 0.5]
    return pred


def main():
    session, iname, ishape, output_name = load_model("yolo11x_obb.onnx")
    img = load_image("dog.jpeg", ishape)
    img = preprocess_image(img)
    print(img.shape)
    img_tens = onnxruntime.OrtValue.ortvalue_from_numpy(img)
    out = session.run(output_names=[output_name], input_feed={iname: img_tens})
    pred = postprocess_onnx_out(out)
    print(pred)
    return

if __name__ == "__main__":
    main()
