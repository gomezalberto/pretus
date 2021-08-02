# Scan plane detection Plug-in

Author: Christian Baumgartner, Nicolas Toussaint, Tianrui Liu
Integrated by Alberto Gomez (alberto.gomez@kcl.ac.uk)

# Summary

This plug-in classifies the views into a number of standard planes or backgorund. The implementation in this plug-in is based in the work:

*Baumgartner, Christian F., et al. "SonoNet: real-time detection and localisation of fetal standard scan planes in freehand ultrasound." IEEE transactions on medical imaging 36.11 (2017): 2204-2215.*


# Usage

```bash
# PLUGIN Standard plane detection
   Standard plane detection using SonoNet by Baumgartner et al.
	--standardplanedetection_stream <val> [ type: STRING]	Name of the stream(s) that this plug-in takes as input. (Default: ) 
	--standardplanedetection_layer <val> [ type: INT]	Number of the input layer to pass to the processing task. If negative, starts 
                                                  		from te end. (Default: 0) 
	--standardplanedetection_framerate <val> [ type: FLOAT]	Frame rate at which the plugin does the work. (Default: 20) 
	--standardplanedetection_verbose <val> [ type: BOOL]	Whether to print debug information (1) or not (0). (Default: 0) 
	--standardplanedetection_time <val> [ type: BOOL]	Whether to measure execution time (1) or not (0). (Default: 0) 
	--standardplanedetection_showimage <val> [ type: INT]	Whether to display realtime image outputs in the central window (1) or not (0). 
                                                      		(Default: <1 for input plugins, 0 for the rest>) 
	--standardplanedetection_showwidget <val> [ type: INT]	Whether to display widget with plugin information (1-4) or not (0). Location is 
                                                       		1- top left, 2- top right, 3-bottom left, 4-bottom right. (Default: visible, 
                                                       		default location depends on widget.) 
   Plugin-specific arguments:
	--standardplanedetection_taverage <val> [ type: INT]	Number of frames used for a temporal average of the detection. (Default: 0) 
	--standardplanedetection_modelname <*.pth> [ type: STRING]	Model file name (without folder). (Default: ifind2_net_Jan15.pth) 
	--standardplanedetection_bckth <val> [ type: FLOAT]	Min value for background to be considered; below this value, background will be 
                                                    		ignored and second best picked. Range is [0.0, 1.0] If -1, ths flag is not used. 
                                                    		(Default: -1) 
	--standardplanedetection_savebck <0/1> [ type: INT]	Whether to save background images to file (1, in this stream) or not (0). 
                                                    		(Default: 0) 
```

# Build instructions

Dependencies for this plug-in:

* Python 3 (tested on Python 3.7)
* Python 3 library (tested on Python 3.7)
* Scipy
* [PyBind11](https://pybind11.readthedocs.io/en/stable/advanced/cast/overview.html) (for the python interface if required), with python 3.
* [Pytorch]()
* [Scikit]()

To build, first configure the plug-in dependencies using CMake. You will need to define the following CMake variables:

* `PYTHON_LIBRARY` for example, `<HOME>/anaconda3/lib/libpython3.7m.so`
* `PYTHON_INCLUDE_DIR` for example `<HOME>/anaconda3/include/python3.7m`
* `PYTHON_EXECUTABLE` for example `<HOME>/anaconda3/bin/python3.7`
* `PYBIND11_DIR` for example `<HOME>/local/pybind11/share/cmake/pybind11`



## Build and install


Make and install
``` bash
$ make && make install
$ export  PYTHONPATH=<your selected install folder\>:"$PYTHONPATH"
```
And launch.

The `make install` step is crucial so that PRETUS finds the plug-in.

## Troubleshooting

* You will need to set `CMAKE_INSTALL_PREFIX` to a path where your python scripts will go, e.g. <your selected install folder\> .
* Make sure that `PYTHON_INCLUDE_DIR` and `PYTHON_LIBRARY` are correctly set to Python 3.

