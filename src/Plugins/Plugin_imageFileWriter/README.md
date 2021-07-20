# Image file writer plugin.

A plug-in to write images to the file system. Images can be written to an output folder, and they will be organised by output streams. Each output stream can be organised in subfolders to contain the number of images per folder to a maximum value. Images are stored in mhd/raw format, and can include image data and many other types of metadata in the header.

When saving images to different subfolders, sometimes it is convenient to set the number of the first division to something different to zero. This can be done with the argument `-ifw_first_subdivision`.

The user can choose to save any stream, or multiple streams, or all. This can be controlled with the `-ifw_stream` argument. To select multiple streams, put them in a comma-separated list in between double quotation marks, for example: `-ifw_stream "Input, Dummy"`.


## Usage

The following plug-in options can be tweaked via the command line interface:

| Argument           | Values   | Default | Description                            |
|--------------------|----------|---------|----------------------------------------|
|  -ifw_folder  | path to folder |        | Base folder where data will be saved |
|  -ifw_stream     | stream name(s)  |  "Input" | Write images only of a certain stream type, given as a string. If set to "-", it saves all. | 
|  -ifw_max_files     | [0,...] |   0    | Maximum number of images in a single folder, will create another folder if this is exceeded. If 0, all files in same folder.|
|  -ifw_first_subdivision     | [0,...] |   0    | Initial id for the subdivision folder. |



