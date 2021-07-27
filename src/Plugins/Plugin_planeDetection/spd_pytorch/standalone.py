import os, sys, numpy as np
import cv2
import torch
from utils.util import json_file_to_pyobj
from skimage.transform import resize
from models import get_model
import SimpleITK as sitk


class HiddenPrints:
    def __enter__(self):
        self._original_stdout = sys.stdout
        sys.stdout = None

    def __exit__(self, exc_type, exc_val, exc_tb):
        sys.stdout = self._original_stdout


def image_loader(image_name, image_size):
    """load image, returns cuda tensor"""
    image = cv2.imread(image_name)
    image= cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
    im_array = resize(image, (int(image_size[0]), int(image_size[1] )), preserve_range=True)
    image_T = torch.from_numpy(im_array)
    image_T = image_T.type(torch.FloatTensor)
    image_T = image_T.unsqueeze(0)
    image_T = image_T.unsqueeze(0)
    image_T_norm = image_T.sub(image_T.mean()).div(image_T.std())
    torch.save(image_T_norm,'/tmp/tmp.tch')
    return image_T_norm.cuda()


def image_loader_sitk(image_name, image_size):
    """load image, returns cuda tensor"""
    image = sitk.ReadImage(image_name)
    image_np = sitk.GetArrayFromImage(image)
    if len(image_np.shape) > 2:
        image_np = image_np[0, ...]

    # preprocess the image
    im_array = resize(image_np.astype(np.float32), (int(image_size[0]), int(image_size[1])), preserve_range=True)
    image_T = torch.from_numpy(im_array).type(torch.FloatTensor).unsqueeze(0).unsqueeze(0)
    return image_T

if __name__ == '__main__':

    if len(sys.argv) < 2:
        print("Missing one argument-input image file")
        exit(-1)
    im_file = sys.argv[1]
    print('Input image: {}'.format(im_file))

    #checkpoint_file = "./checkpoints/ifind1_sononet_8/300_net_S.pth"
    #checkpoint_file = "./checkpoints/ifind1_sononet_8/090_net_S_iFind2_newBg.pth"
    checkpoint_file = "./checkpoints/ifind1_sononet_8/ifind2_net_Jan15.pth"

    json_filename = "./configs/config_sononet_8.json"
    json_opts = json_file_to_pyobj(json_filename)

    with HiddenPrints():
        model = get_model(json_opts.model)

    if hasattr(model.net, 'deep_supervised'):
        model.net.deep_supervised = False

    # Load checkpoint
    if os.path.isfile(checkpoint_file):
        checkpoint = torch.load(checkpoint_file)
        model.net.load_state_dict(checkpoint)
        print("=> Loaded checkpoint '{}'".format(checkpoint_file))
    else:
        print("=> No checkpoint found at '{}'!!!!!!".format(checkpoint_file))

    # Load Testing image
    image = image_loader_sitk(im_file, [224, 288])
    device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
    image.to(device)
    im_data_norm = image.sub(image.mean()).div(image.std())
    #torch.save(image,'/tmp/im1.tch')

    model.set_input(im_data_norm)
    model.net.eval()
    with torch.no_grad():
        model.forward(split='test')
    print (model.prediction)
    model.logits = model.net.apply_argmax_softmax(model.prediction)
    model.pred = model.logits.data.max(1)
    pr_lbls = model.pred[1].item()

    label_names = ['3VV', '4CH', 'Abdominal', 'Background', 'Brain (Cb.)', 'Brain (Tv.)', 'Femur', 'Kidneys', 'LVOT',
                   'Lips', 'Profile', 'RVOT', 'Spine (cor.)', 'Spine (sag.)']

    print("The testing image is predicated as %s" % (label_names[pr_lbls]))





