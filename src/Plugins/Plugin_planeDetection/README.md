# Scan plane detection Plug-in

Author: Christian Baumgartner, Nicolas Toussaint, Tianrui Liu
Integrated by Alberto Gomez (alberto.gomez@kcl.ac.uk)

# Summary

This plug-in classifies the views into a number of standard planes or backgorund. The implementation in this plug-in is based in the work:

*Baumgartner, Christian F., et al. "SonoNet: real-time detection and localisation of fetal standard scan planes in freehand ultrasound." IEEE transactions on medical imaging 36.11 (2017): 2204-2215.*


# Build instructions

Dependencies for this plug-in:

* Python 3 (tested on Python 3.7)
* Python 3 library (tested on Python 3.7)
* Scipy
* [PyBind11](https://pybind11.readthedocs.io/en/stable/advanced/cast/overview.html) (for the python interface if required), with python 3.
* [Pytorch]()
* [Scikit]()


## Build and install


Launch CMake configure and generate. Then make and install
``` bash
$ make && make install
$ export  PYTHONPATH=<your selected install folder\>:"$PYTHONPATH"
```
And launch.

The `make install` step is crucial so that PRETUS finds the plug-in.

## Troubleshooting

* You will need to set `CMAKE_INSTALL_PREFIX` to a path where your python scripts will go, e.g. <your selected install folder\> .
* Make sure that `PYTHON_INCLUDE_DIR` and `PYTHON_LIBRARY` are correctly set to Python 3.

