import onnxruntime
from pathlib import Path
from PIL import Image
from pydantic import BaseModel
from matplotlib import pyplot as plt
import matplotlib.patches as patches
import numpy


def load_model(model_name: str):
    session = onnxruntime.InferenceSession(model_name, None)

    # get the name of the first input of the model
    input_name = session.get_inputs()[0].name
    input_shape = session.get_inputs()[0].shape
    output_name = session.get_outputs()[0].name
    print("Input Name:", input_name)
    print("Output Name:", output_name)
    return session, input_name, input_shape, output_name


def load_image(im_path: Path, input_shape) -> numpy.ndarray:
    image: Image.Image = Image.open(im_path).convert("RGB")
    image = image.resize(input_shape[2:])
    return numpy.asarray(image)


def preprocess_image(img: numpy.ndarray) -> numpy.ndarray:
    img = img.astype(numpy.float32)
    img /= 255
    img = img.transpose((2, 0, 1))
    img = img[None, :]
    return img


def postprocess_onnx_out(out: list[numpy.ndarray]) -> numpy.ndarray:
    # assume only one output in list
    pred = out[0][0, :, :]

    # filter pred
    pred = pred[pred[:, 5] > 0.5]
    return pred


class PredResult(BaseModel):
    result: list


def main():
    session, iname, ishape, output_name = load_model(
        "trained_on_sieben_and_old_ptw.onnx"
    )
    img = load_image("images/vs.jpg", ishape)
    img = preprocess_image(img)
    print(img.shape)
    img_tens = onnxruntime.OrtValue.ortvalue_from_numpy(img)
    out = session.run(output_names=[output_name], input_feed={iname: img_tens})
    pred = postprocess_onnx_out(out)
    print(pred)
    pred_results = PredResult(result=pred.tolist())
    draw_img = Image.open("images/vs.jpg").resize((3008, 3008))
    draw(draw_img, pred_results)

    return


def draw(img: Image.Image, res: PredResult):

    # Create figure and axes
    fig, ax = plt.subplots()

    # Display the image
    ax.imshow(img)

    for elem in res.result:
        x, y = elem[:2]
        width = elem[2] - elem[0]
        height = elem[3] - elem[1]
        label = elem[-1]
        rect = patches.Rectangle(
            (x, y), width, height, linewidth=1, edgecolor="r", facecolor="none"
        )

        # Add the patch to the Axes
        ax.add_patch(rect)
        ax.annotate(f"{label}", elem[:2], color="blue", weight="bold", fontsize=8)

    plt.show()
    return


if __name__ == "__main__":
    main()
