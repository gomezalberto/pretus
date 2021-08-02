# Video manager plug-in.

A plug-in to read a video file from the file system and transmit it through the pipeline. `Opencv` is used to read the video files so supported format depends on local configuration of opencv.

The video by default loops around when finished, but this can be disabled by the user using the command line argument `--videomanager_noloop 1`. The video starts from the beginning by default, but an arbitrary start time can be set with `--videomanager_starttime <mm:ss>`. The video can also be played faster by setting a fast-forward factor with `--videomanager_ff <factor>`.


## Usage

The following plug-in options can be tweaked via the command line interface:

```bash
# PLUGIN Video manager
   Reads video files from disk. File format depends on opencv installation.
	--videomanager_framerate <val> [ type: FLOAT]	Frame rate at which the plugin does the work. (Default: 20) 
	--videomanager_verbose <val> [ type: BOOL]	Whether to print debug information (1) or not (0). (Default: 0) 
	--videomanager_showimage <val> [ type: INT]	Whether to display realtime image outputs in the central window (1) or not (0). 
                                            		(Default: <1 for input plugins, 0 for the rest>) 
	--videomanager_showwidget <val> [ type: INT]	Whether to display widget with plugin information (1-4) or not (0). Location is 
                                             		1- top left, 2- top right, 3-bottom left, 4-bottom right. (Default: visible, 
                                             		default location depends on widget.) 
   Plugin-specific arguments:
	--videomanager_loop <val> [ type: INT]	loop around (1) or not (0). (Default: 1) 
	--videomanager_ff <fast forward factor> [ type: FLOAT]	Fast forward factor, in (0, inf). 1 means native speed, >1 is faster, <1 is 
                                                       		slower. (Default: 1.1) 
	--videomanager_starttime <mm:ss> [ type: STRING]	Initial time to start getting frames from the video. (Default: 00:00) 
	--videomanager_input <path to video> [ type: STRING]	Take images from a video file. (Default: N/A (compulsory)) 
```