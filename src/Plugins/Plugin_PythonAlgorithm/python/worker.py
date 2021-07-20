
# Dependencies
import os, sys, numpy as np
import time

import SimpleITK as sitk

def initialize( python_path):
    print("worker.py: Initialize: the python folder is : "+python_path)


def dowork(image_cpp, fsigma=1.0, delay_sec=0, verbose=False):
    """
    image_cpp is theinput image (a numpy array)
    fkernel is the kernel size for the Gaussian
    fsigma is the Gaussian width
    verbose allows to print extra info
    """
    try:

        time.sleep(delay_sec)
        im = sitk.GetImageFromArray(image_cpp)
        pixelID = im.GetPixelID()
        gaussian = sitk.SmoothingRecursiveGaussianImageFilter()
        gaussian.SetSigma(fsigma)
        output = gaussian.Execute(im)

        caster = sitk.CastImageFilter()
        caster.SetOutputPixelType(pixelID)
        output = caster.Execute(output)

        output_np = sitk.GetArrayFromImage(output)
        return output_np

    except Exception as inst:
        print("::WARNING:: pythonalgorithm detected an exception")
        print(type(inst))    # the exception instance
        print(inst.args)     # arguments stored in .args
        print(inst)          # __str__ allows args to be printed directly,
        return defaultret
