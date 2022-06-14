# Plug & Play Framegrabber plugin.
A plug-in to read real time images using a plug and play framegrabber that supports V4L2.

## Build and configuration
This plug-in requires opencv. In Linux it requires the V4L2 libraries (vidoe for linux) installed, otherwise it is just plug and play.
The framegrabber can be tested in advance with OBS or VLC players to check that it is correctly recognised by the system.


## Usage
``` 
cd $HOME/local/pretus
conda activate pretus
sh launcher_pretus.sh -pipeline "pnpframegrabber>gui"
```

## Help for `sh launcher_pretus.sh -h`
The following plug-in options can be tweaked via the command line interface.
```
(N) Plugin Name: 'PnP Frame grabber'

# PLUGIN PnP Frame grabber
   Reads real-time imaging from a video source using a Plug & PLay capture card through V4L / OBS.
	--pnpframegrabber_verbose <val> [ type: BOOL]	Whether to print debug information (1) or not (0). (Default: 0) 
	--pnpframegrabber_showimage <val> [ type: INT]	Whether to display realtime image outputs in the central window (1) or not (0). 
                                               		(Default: <1 for input plugins, 0 for the rest>) 
	--pnpframegrabber_showwidget <val> [ type: INT]	Whether to display widget with plugin information (1-4) or not (0). Location is 
                                                		1- top left, 2- top right, 3-bottom left, 4-bottom right. (Default: visible, 
                                                		default location depends on widget.) 
   Plugin-specific arguments:
	--pnpframegrabber_studioswing <val> [ type: INT]	Correct for studio swing (>0) or not (0). The value (typically 16) indicates how 
                                                 		much to correct for. (Default: 0) 
	--pnpframegrabber_resolution width.height [ type: STRING]	Number of pixels of the video stream, separated by a dot. Accepted values are, 
                                                          		in 16:9: 1920.1080, 1360.768, 1280.720; in 4:3: 1600.1200, 1280.960, 1024.768, 
                                                          		800.600, 640.480; and other: 1280.1024, 720.576, 720.480 (Default: 1024.768) 
	--pnpframegrabber_pixelsize <val> [ type: FLOAT]	Value, in mm, of the pixel size (isotropic). (Default: 1) 
	--pnpframegrabber_color <0/1> [ type: BOOL]	USe color images (1) or not (0). (Default: 0) 
	--pnpframegrabber_camid <val> [ type: INT]	camera id. (Default: 4) 
	--pnpframegrabber_framerate <val> [ type: FLOAT]	Frame rate at which the framegrabber captures data. (Default: 30) 

```

## Resolution tests Using Frame-grabber MiraBox Video Capture
* Scripts 
```
cd $HOME/repositories/pretus/src/Plugins/Plugin_PnPframegrabber
conda activate pretus 
python framegrabber_capturing_video.py --fW 1024 --fH 768 --FPS 60 --buffer_size 1 
```

* Terminal logs

 * --fW 1024 --fH 768 --FPS 60 --buffer_size 1 
``` 
python framegrabber_capturing_video.py --fW 1024 --fH 768 --FPS 60 --buffer_size 1 
fps: 60.0
resolution: 1024.0x768.0
mode: MJPG
Buffer size: 1.0
```

 * --fW 1280 --fH 960 --FPS 30 --buffer_size 1
``` 
python framegrabber_capturing_video.py --fW 1280 --fH 960 --FPS 30 --buffer_size 1
fps: 30.0
resolution: 1280.0x960.0
mode: MJPG
Buffer size: 1.0
``` 

 * --fW 1920 --fH 1080 --FPS 60 --buffer_size 1
``` 
python framegrabber_capturing_video.py --fW 1920 --fH 1080 --FPS 60 --buffer_size 1
fps: 60.0
resolution: 1920.0x1080.0
mode: MJPG
Buffer size: 1.0
```