# File manager plug-in.

A plug-in to read images from a file system. Images can be organised in folders and subfolders, and they can be 2D o 3D. 

Images are read using the `itk::ImageFileReader` from the ITK library, and currently the mhd/raw format is supported.

By default, images are transmitted in alphabetical order, so the file name will dictate the transmission order. Also, by default, images are transmitted at a constant frame rate of 20 images per second. 

There are two ways of setting the frame rate. First, a constant frame rate between 0 and 200 can be set using the command line arguments. Second, if the mhd headers have the field `AcquisitionFrameRate`, then this value will be used, and can be different for each image.

## Usage

The following plug-in options can be tweaked via the command line interface:

```bash
# PLUGIN File manager
   Reads and transmits images from a folder hierarchy.
	--filemanager_framerate <val> [ type: FLOAT]	Frame rate at which the plugin does the work. (Default: 20) 
	--filemanager_verbose <val> [ type: BOOL]	Whether to print debug information (1) or not (0). (Default: 0) 
	--filemanager_showimage <val> [ type: INT]	Whether to display realtime image outputs in the central window (1) or not (0). 
                                           		(Default: <1 for input plugins, 0 for the rest>) 
	--filemanager_showwidget <val> [ type: INT]	Whether to display widget with plugin information (1-4) or not (0). Location is 
                                            		1- top left, 2- top right, 3-bottom left, 4-bottom right. (Default: visible, 
                                            		default location depends on widget.) 
   Plugin-specific arguments:
	--filemanager_loop <val> [ type: INT]	loop around (1) or not (0). (Default: 0) 
	--filemanager_asraw <val> [ type: INT]	Load the data as raw (1), without any preprovcessing available in the header 
                                       		(0). (Default: 0) 
	--filemanager_checkMhdConsistency <val> [ type: INT]	Check that data is consistent and ignore inconsistent files (1) or not (0). 
                                                     		(Default: 1) 
	--filemanager_input <path to folder> [ type: STRING]	Take images from a folder. (Default: Currentfolder) 

```
