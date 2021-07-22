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

PLug-ins are implemented in C++, however the plug-in functionality can be implemented in python. An example plug-in implemented in python is `Plugin_PythonAlgorithm`. In order to build python-based plug-ins, the following CMake fields must be completed:

* `PYTHON_LIBRARY`, for example,  `<home folder>/anaconda3/lib/libpython3.7m.so`
* `PYTHON_INCLUDE_DIR`, for example, `<home folder>/anaconda3/include/python3.7m`
* `PYTHON_EXECUTABLE` , for example, `<home folder>/anaconda3/bin/python3.7m`

It is **crucial** that the same python version is used throughout. We recommend using a environment installation such as conda for the purpose. The python include and binary should be the same used for pybind11 too.
