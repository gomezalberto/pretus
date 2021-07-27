# Pytorch-SonoNet <br /> (Ultrasound Standard Plane Detection)

Pytorch implementation of SonoNet.


### References:
The code is a modified version of https://github.com/ozan-oktay/Attention-Gated-Networks

1) "Attention-Gated Networks for Improving Ultrasound Scan Plane Detection", MIDL'18, Amsterdam 
[Conference Paper](https://openreview.net/pdf?id=BJtn7-3sM) <br />
[Conference Poster](https://www.doc.ic.ac.uk/~oo2113/posters/MIDL2018_poster_Jo.pdf)

2) Baumgartner, Christian F., et al. "SonoNet: real-time detection and localisation of fetal standard scan planes in freehand ultrasound." <br />
IEEE transactions on medical imaging 36.11 (2017): 2204-2215.
[Paper] (https://ieeexplore.ieee.org/abstract/document/7974824)

### Installation
pip install --process-dependency-links -e .

      install_requires=[
        "numpy",
        "torch>=1.0.1",
        "matplotlib",
        "scipy",
        "torchvision",
        "tqdm",
        "visdom",
        "nibabel",
        "h5py",
        "pandas",
        "dominate",
        'torchsample==0.1.3',
      ], 

Run the demo.py would make a prediction based on an input image in ./demo_example 
as well as probabilities for each class

