# Video manager plug-in.

A plug-in to read a video file from the file system and transmit it through the pipeline. `Opencv` is used to read the video files so supported format depends on local configuration of opencv.

The video by default loops around when finished, but this can be disabled by the user using the command line argument `-vm_noloop 1`. The video starts from the beginning by default, but an arbitrary start time can be set with `-vm_start_time <mm:ss>`. The video can also be played faster by setting a fast-forward factor with `-vm_ff <factor>`.


## Usage

The following plug-in options can be tweaked via the command line interface:

| Argument           | Values   | Default | Description                            |
|--------------------|----------|---------|----------------------------------------|
|  -vm_framerate  | (0, 30] |   25     | frame rate. This will temporally re-sample the video, but not make it slower or faster. For that use the ff option. |
|  -vm_input_video     | path to file  |  "" | Take images from input file. |
|  -vm_start_time  | <mm:ss\> |   00:00    | Initial time to start getting frames from the video |
|  -vm_verbose  | \{0, 1\} |   0    | isplay extra debugging  info (1) or not (0)|
|  -vm_ff     | [0.0, inf)|   1.25    | Fast forward factor. 1 means native speed, >1 is faster, <1 is slower.|
