# File manager plugin.

A plug-in to read images from a file system. Images can be organised in folders and subfolders, and they can be 2D o 3D. 

Images are read using the `itk::ImageFileReader` from the ITK library, and currently the mhd/raw format is supported.

By default, images are transmitted in alphabetical order, so the file name will dictate the transmission order. Also, by default, images are transmitted at a constant frame rate of 20 images per second. 

There are two ways of setting the frame rate. First, a constant frame rate between 0 and 200 can be set using the command line argument `-fm_framerate <f>`. Second, if the mhd headers have the field `AcquisitionFrameRate`, then this value will be used, and can be different for each image.

## Usage

The following plug-in options can be tweaked via the command line interface:

| Argument           | Values   | Default | Description                            |
|--------------------|----------|---------|----------------------------------------|
|  -fm_framerte  | (0, 200] |   -1 (=20)     | frame rate (if <=0 or >200, will be set to 20) |
|  -fm_loop     | \{0, 1\}  |  0 | loop around (1) or not (0). |
|  -fm_input     | path to folder |   current folder    | Take images from a folder and subfolders |
|  -fm_asraw     | \{0, 1\} |   0    | Load the data as raw (1), without any preprocessing or information available in the header, or not (0). |
|  -fm_checkMhdConsistency     | \{0, 1\} |   1    | Check that data is consistent and ignore inconsistent files (1), or not (0). |
|  -fg_verbose     | \{1, 0\} |   0    | Output more information for debugging. |


