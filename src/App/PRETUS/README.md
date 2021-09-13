# PRETUS executable

Author: Alberto Gomez (alberto.gomez@kcl.ac.uk)

# Summary

This program does nearly nothing on its own. Basically, it will load all plug-ins found in the plug-in folder (which can be defined through CMake with the variable ```PLUGIN_FOLDER```, or with the config file that will be created under ```$HOME/.iFIND/PRETUS.conf```). Then, it will do the following operations in order:

1. Read from the command line the user-defined pipeline and re-order the plug-ins that will be used accordingly.
2. Connect each plug-in to the next. For each pair of connected plug-ins, two connections are made: first the configuration connection ( a plug-in can pass configuration parameters to the next); second, the real-time imaging connection (a plug-in will pass a processed image to the next, as they are processed).
3. All plug-ins are initialized (this normally triggers object creation, loading default values, models, etc).
4. All plug-ins are activated (this triggers the actual processing pipeline)


# Build instructions

## Dependencies

The minimum requirements are:

* VTK
* ITK (for the webcam plug-in, built with the `ITKVideoBridgeOpencv` option `ON`)
* Boost
* Qt 5 (tested with 5.12)
* c++11

Additionally, some plug-ins will have the following requirements:

* For the python plug-in:
    * Python 3 (tested on Python 3.6)
    * Python 3 library
    * [PyBind11](https://pybind11.readthedocs.io/en/stable/advanced/cast/overview.html) (for the python interface if required), with python 3.
    * [Pytorch](https://pytorch.org/get-started/locally/) >= 1.0
* For the webcam plug-in:
    * OpenCV 3
* For the Robot plug-in:
    * iFIND - Controller server client (branch b9), based on OpenIGTLink


## Build and install

Launch CMake configure and generate. Then make and install
``` bash
$ make && make install
$ export  PYTHONPATH=<your selected install folder\>:"$PYTHONPATH"
```
And launch.

## Troubleshooting

* You will need to set `CMAKE_INSTALL_PREFIX` to a path where your python scripts will go, e.g. <your selected install folder\> .
* Make sure that `PYTHON_INCLUDE_DIR` and `PYTHON_LIBRARY` are correctly set to Python 3.

