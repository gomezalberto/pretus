# Image file writer plug-in.

A plug-in to write images to the file system. Images can be written to an output folder, and they will be organised by output streams. Each output stream can be organised in subfolders to contain the number of images per folder to a maximum value. Images are stored in mhd/raw format, and can include image data and many other types of metadata in the header.

When saving images to different subfolders, sometimes it is convenient to set the number of the first division to something different to zero. This can be done with the argument `--imagefilewriter_firstsubdivision`.

The user can choose to save any stream, or multiple streams, or all. This can be controlled with the `stream` argument. To select multiple streams, put them in a comma-separated list in between double quotation marks, for example: `--imagefilewriter_stream "Input, Dummy"`.


## Usage

The following plug-in options can be tweaked via the command line interface:

```bash
# PLUGIN Image file writer
   Save image data to the file system.
	--imagefilewriter_stream <val> [ type: STRING]	Name of the stream(s) that this plug-in takes as input. (Default: ) 
	--imagefilewriter_framerate <val> [ type: FLOAT]	Frame rate at which the plugin does the work. (Default: 20) 
	--imagefilewriter_verbose <val> [ type: BOOL]	Whether to print debug information (1) or not (0). (Default: 0) 
	--imagefilewriter_time <val> [ type: BOOL]	Whether to measure execution time (1) or not (0). (Default: 0) 
	--imagefilewriter_showwidget <val> [ type: INT]	Whether to display widget with plugin information (1-4) or not (0). Location is 
                                                		1- top left, 2- top right, 3-bottom left, 4-bottom right. (Default: visible, 
                                                		default location depends on widget.) 
   Plugin-specific arguments:
	--imagefilewriter_folder <folder> [ type: STRING]	Parent folder to save images. Will be created if does not exist. (Default: ) 
	--imagefilewriter_stream <stream name> [ type: STRING]	Write images only of a certain stream type, given as a string. If set to "-", it 
                                                       		saves all. (Default: Input) 
	--imagefilewriter_maxfiles <N> [ type: INT]	Maximum number of images in a single folder, will create another folder if this 
                                            		is exceeded. If 0, all files in same folder. (Default: 0) 
	--imagefilewriter_firstsubdivision <N> [ type: INT]	Initial id for the subdivision folder. (Default: 0) 
```