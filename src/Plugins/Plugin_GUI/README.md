# GUI Plug-in


# Summary

This plug-in integrates and organises widgets from previous plug-ins to display information and images in real time to the user.The organisation of the visualization window in shown below:

![visualization figure](visualization.png)

The central frame (A) displays the images from a given stream in real time. The side frames (B and C) can be used to display widgets from individual plugins upstream the pipeline. 

## Usage

WIP

The following plug-in options can be tweaked via the command line interface:

| Argument           | Values   | Default | Description                            |
|--------------------|----------|---------|----------------------------------------|
|  -vs_framerate  | (0, 200] |   -1 (=20)     | frame rate (if <=0 or >200, will be set to 20) |
|  -vs_stream     | string  |  "Input" | Stream to be visualized. |
|  -vs_showruler  | \{0, 1\} |   0    | Whether to show (1) or not (0) a ruler |
|  -vs_layer  | \{0, 1\} |   0    | Display (if available) the requested layer |
|  -vs_overlay     | \{-1, 0, 1, ...\} |   0    | desactivate (0) or activate (1,... n_overlays)  displaying multiple layers as overlay. If -1, display all. |
|  -vs_lut     | \{0, 1, ...\} |   15    | Lookup table (int) (0: SPECTRUM, 1: WARM, 2: COOL, 3: BLUES, 4: WILD_FLOWER, 5: CITRUS, etc). |


## Build and install


Launch CMake configure and generate. Then make and install
``` bash
$ make && make install
```
And launch.

The `make install` step is crucial so that PRETUS finds the plug-in.

## Troubleshooting

* You will need to set `CMAKE_INSTALL_PREFIX` to a path where your python scripts will go, e.g. <your selected install folder\> .

