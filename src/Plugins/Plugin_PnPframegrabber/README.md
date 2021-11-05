# Plug & Play Framegrabber plugin.

A plug-in to read real time images using a plug and play framegrabber that supports V4L2.

## Usage


WIP------------

The following plug-in options can be tweaked via the command line interface:

``` bash
# PLUGIN PnP Frame grabber
   Reads real-time imaging from a video source using a Plug & PLay capture card through V4L / OBS.
        --pnpframegrabber_framerate <val> [ type: FLOAT]	Frame rate at which the plugin does the work. (Default: 20) 
        --pnpframegrabber_verbose <val> [ type: BOOL]	Whether to print debug information (1) or not (0). (Default: 0) 
        --pnpframegrabber_showimage <val> [ type: INT]	Whether to display realtime image outputs in the central window (1) or not (0). 
                                                        (Default: <1 for input plugins, 0 for the rest>) 
        --pnpframegrabber_showwidget <val> [ type: INT]	Whether to display widget with plugin information (1-4) or not (0). Location is 
                                                                1- top left, 2- top right, 3-bottom left, 4-bottom right. (Default: visible, 
                                                                default location depends on widget.) 
   Plugin-specific arguments:
        --pnpframegrabber_studioswing <val> [ type: BOOL]	Correct for studio swing (1) or not (0). (Default: 0) 
        --pnpframegrabber_resolution width.height [ type: STRING]	Number of pixels of the video stream, separated by a dot. Accepted values are, 
                                                                        in 16:9: 1920.1080, 1360.768, 1280.720; in 4:3: 1600.1200, 1280.960, 1024.786, 
                                                                        800.600, 640.480; and other: 1280.1024, 720.576, 720.480 (Default: 1280.720) 
        --pnpframegrabber_pixelsize <val> [ type: FLOAT]	Value, in mm, of the pixel size (isotropic). (Default: 1) 
        --pnpframegrabber_color <0/1> [ type: BOOL]	USe color images (1) or not (0). (Default: 0) 

```


# Build and configuration

This plug-in requires opencv. In Linux it requires the V4L2 libraries (vidoe for linux) installed, otherwise it is just plug and play.
The framegrabber can be tested in advance with OBS or VLC players to check that it is correctly recognised by the system.

