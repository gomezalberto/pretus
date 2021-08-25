import os, sys, numpy as np
import cv2
from utils.util import json_file_to_pyobj
from skimage.transform import resize
import SimpleITK as sitk
import standardplanedetection_worker

def image_loader_sitk(image_name, image_size):
    """load image, returns cuda tensor"""
    image = sitk.ReadImage(image_name)
    image_np = sitk.GetArrayFromImage(image)
    if len(image_np.shape) > 2:
        image_np = image_np[0, ...]

    # preprocess the image
    im_array = resize(image_np.astype(np.float32), (int(image_size[0]), int(image_size[1])), preserve_range=True)
    return im_array

if __name__ == '__main__':

    if len(sys.argv) < 2:
        print("Missing one argument-input image file")
        exit(-1)
    im_file = sys.argv[1]
    print('Input image: {}'.format(im_file))

    standardplanedetection_worker.initialize('./','ifind2_net_Jan15.pth')

    # Load Testing image
    image = image_loader_sitk(im_file, [224, 288])

    output = standardplanedetection_worker.dowork(image, verbose=False)

    label_names = standardplanedetection_worker.getlabels()
    id = np.argmax(output)

    print('Predicted {}'.format(label_names[id]))


