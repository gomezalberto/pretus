"""
This file serves as backend for the C++ interface to the MICCAI2016 scanplane detection.

Loading the module initialises everything and compiles all the python functions.
This may take a while, espectially upon running the code for the first time.

Author: Tianru (Oct 2019),  Christian Baumgartner, Nicolas Toussaint (11. May 2016), Alberto Gomez (12 Aug 2019)
Edit (AG) this file used to be called api.py
"""

# Dependencies
import os, sys, numpy as np
import torch
from utils.util import json_file_to_pyobj
from skimage.transform import resize
from models import get_model
import time

imnumber = 0
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
#label_names = ['3VV', '4CH', 'Abdominal', 'Background', 'Brain (Cb.)', 'Brain (Tv.)', 'Femur', 'Kidneys', 'Lips', 'LVOT', 'Profile', 'RVOT', 'Spine (cor.)', 'Spine (sag.)']
#label_names = ['3VV', '4CH', 'ABDOMINAL', 'BACKGROUND', 'BRAIN-CB', 'BRAIN-TV', 'FEMUR', 'KIDNEYS', 'LIPS', 'LVOT','PROFILE', 'RVOT', 'SPINE-CORONAL', 'SPINE-SAGITTAL']
#label_names = ['3VV', '4CH', 'Abdominal', 'Brain (Cb.)', 'Brain (Tv.)', 'Femur', 'Kidneys', 'Lips', 'LVOT', 'Profile', 'RVOT',  'Spine (cor.)',  'Spine (sag.)', 'Background']
#label_names = ['3VV', '4CH', 'Abdominal', 'Background', 'Brain (Cb.)', 'Brain (Tv.)', 'Femur', 'Kidneys', 'Lips', 'LVOT', 'Profile', 'RVOT', 'Spine (cor.)', 'Spine (sag.)']
label_names = ['3VV', '4CH', 'Abdominal', 'Background', 'Brain (Cb.)', 'Brain (Tv.)', 'Femur', 'Kidneys', 'LVOT', 'Lips', 'Profile', 'RVOT', 'Spine (cor.)', 'Spine (sag.)']

class HiddenPrints:
    def __enter__(self):
        self._original_stdout = sys.stdout
        sys.stdout = None

    def __exit__(self, exc_type, exc_val, exc_tb):
        sys.stdout = self._original_stdout

def getlabels():
    return label_names

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
    return image_T_norm.cuda()


def initialize( python_path, modelname):
    print("the model is : "+modelname)
    global model
    """
    (Alberto Gomez) this is called once, at construction. can be used to load the model etc)
    """
    #mfunction = getattr(models, model_name)
    #global allmodels
    #allmodels = CreateModels(model_function=mfunction, model_filename=model_path, title=model_name)

    #checkpoint_file = python_path + "/checkpoints/ifind1_sononet_8/300_net_S.pth"
    #checkpoint_file = python_path + "/checkpoints/ifind1_sononet_8/iFind2_net_300.pth"
    #checkpoint_file = python_path + "/checkpoints/ifind1_sononet_8/ifind2all_net.pth"
    #checkpoint_file = python_path + "/checkpoints/ifind1_sononet_8/090_net_S_iFind2_newBg.pth"
    #checkpoint_file = python_path + "/checkpoints/ifind1_sononet_8/ifind2_net_Jan15.pth"
    checkpoint_file = python_path + "/checkpoints/ifind1_sononet_8/" + modelname

    json_filename = python_path + "/configs/config_sononet_8.json"
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


#def CreateModels(model_function=models.vgg16_bn, model_filename=vgg16_network, title='VGG16-BN'):
#    ret = []
#    vgg16 = Model(model_function, model_filename,title)
#    ret.append(vgg16)
#    return ret

def choose_predicton(in_list, labels):
    index = np.argmax(np.argmax(in_list, axis=1), axis=0)
    label = labels[np.argmax(in_list[index])]
    while (label == 'Background') and (len(in_list) > 1):
        in_list = np.delete(in_list, index, axis=0)
        index = np.argmax(np.argmax(in_list, axis=1), axis=0)
        label = labels[np.argmax(in_list[index])]
    return in_list[index]

def dowork(image_cpp, move_threshold=True, verbose=False):
    global imnumber
    try:
        start = time.time()

        labels = getlabels()
        if verbose:
            print("worker.py: planedetect - dowork() - getprediction: labels")
            print(labels)

        defaultret = np.array([0.0] * len(labels))

        # check image size
        size = image_cpp.shape
        if size[0] < 50 or size[1] < 50:
            print('worker.py ::WARNING:: image size {}x{} is invalid'.format(size[0], size[1]))
            return defaultret.astype(np.float32)

        # Now pad, crop, resize and normalize if needed
        image_size = [224, 288]
        im_resized = resize(image_cpp.astype(np.float32), (int(image_size[0]), int(image_size[1] )), preserve_range=True)
        #im_data = transform(image_cpp).unsqueeze(0).type(torch.FloatTensor).to(device)

        #---------------------------------------
        # AG: begin of problem: worstation freezes process when requesting to move to GPU
        a = torch.from_numpy(im_resized).type(torch.FloatTensor).unsqueeze(0).unsqueeze(0)
        if verbose:
            print("worker.py: planedetect - dowork() - getprediction: 9")
        #print(a.device)
        #print(device)
        # this is a problem in the workstation
        im_data = a.to(device)
        if verbose:
            print("worker.py: planedetect - dowork() - getprediction: 10")
        #im_data = torch.from_numpy(im_resized).type(torch.FloatTensor).unsqueeze(0).unsqueeze(0).to(device)
        #---------------------------------------


        im_data_norm = im_data.sub(im_data.mean()).div(im_data.std())
        #torch.save(im_data_norm,'/tmp/im{:d}.tch'.format(imnumber))
        #imnumber+=1
        model.set_input(im_data_norm)
        model.net.eval()
        with torch.no_grad():
            model.forward(split='test')

        #prediction = model.net.apply_argmax_softmax(model.prediction).squeeze().cpu().numpy()
        #print (model.prediction)
        prediction = model.prediction[0].cpu().numpy()
        ##model.pred = model.logits.data.max(1)
        ##pr_lbls = model.pred[1].item()
        output = tuple(i for i in prediction)
        return output

    except Exception as inst:
        print("worker.py ::WARNING:: planedetect detected an exception")
        print(type(inst))    # the exception instance
        print(inst.args)     # arguments stored in .args
        print(inst)          # __str__ allows args to be printed directly,
        return defaultret
