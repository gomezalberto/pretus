# PRETUS - Plugins
**P**lug-in based, **Re**al-**t**ime **U**ltra**s**ound

---

# Overview

This folder contains the implementation of individual plug-ins. The plug-ins here are provided with PRETUS, however user plug-ins can be built from a different repository and linked in run time using the `PLUGIN_FOLDER` CMake field (which shoudl contain the folder with the plug-in compiled library). The plug-ins included in this release are:

Input plug-ins:

* **File Manager**: read image data from files and folders.
* **Video Manager**: read data from a video file.
* **Frame Frabber**: grab data from a video source using the Epiphane DVI2USB 3.0 video grabber.

Processing plug-ins:

* **Plane Detection**: implementation of the SonoNet algorithm to detect standard fetal planes.
* **Cpp Algortithm**: a sample plug-in implementing a simple image processing algorithm in C++.
* **Python Algorithm**: a sample plug-in implementing a simple image processing algorithm in Python.

Output plug-ins:

* **Image File Writer**: write one or more streams to file.
* **GUI**: visualize images and widgets from plugins


## Plugins using Python

### Advice: how to install and manage python dependencies

We recommend using [anaconda](https://www.anaconda.com) to install conda and python, and then use pip to manage packages. 
Note: If you will be using Tensorflow v1, then the highest version of python is 3.7

An error sometimes arises with HDF version missmatch. This is caused by having different versions of the library and of the headers. The error message will say something like:
"h5py is running against HDF5 1.10.4 when it was built against 1.12.1", and "Headers are 1.12.1, library is 1.10.4". The solution is to upgrade to a matching version by doing in this case:

```
conda install --force-reinstall anaconda hdf5==1.10.4
```

### Additional build notes
Plug-ins are implemented in C++, however the plug-in functionality can be implemented in python. An example plug-in implemented in python is `Plugin_PythonAlgorithm`. In order to build python-based plug-ins, the following CMake fields must be completed:

* `PYTHON_LIBRARY`, for example,  `<home folder>/anaconda3/lib/libpython3.7m.so`
* `PYTHON_INCLUDE_DIR`, for example, `<home folder>/anaconda3/include/python3.7m`
* `PYTHON_EXECUTABLE` , for example, `<home folder>/anaconda3/bin/python3.7m`

It is **crucial** that the same python version is used throughout. We recommend using a environment installation such as conda for the purpose. The python include and binary should be the same used for pybind11 too.

When configuring/generating in CMake, some warning or errors might appear. Do not ignore python related warnings since they may prevent proper execution. For example, you may get a zlib error saying: 

*runtime library [libz.so.1] in /usr/lib/x86_64-linux-gnu may be 
hidden by files in [folder]/miniconda3/lib*

In that case, find the CMake entry for libz and replace it by the miniconda version.




