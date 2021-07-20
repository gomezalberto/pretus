# Framegrabber plugin.

A plug-in to read real time images using an EPIPHAN DVI2USB framegrabber.

## Usage

The following plug-in options can be tweaked via the command line interface:

| Argument           | Values   | Default | Description                            |
|--------------------|----------|---------|----------------------------------------|
|  -fg_studio_swing  | \{1, 0\} |   1     | Correct for studio swing (1) or not (0)|
|  -fg_framerate     | (0, 30)  |  0 (max)| Frame rate of the capture for the frame grabber. If <=0, then maximum framerate is used |
|  -fg_pixelsize     | px py |   1 1    | Pixel size (x and y), in mm. |
|  -fg_verbose     | \{1, 0\} |   0    | Output more information for debugging. |


# Build and configuration

This plug-in requires that the SDK is available and that the drivers are installed.

## Epiphan SDK
For this plug-in you need the driver and the EPIPHAN SDK. This folder must be indicated in the `EpiphanSDK_DIR` CMake tag. For example:

```
EpiphanSDK_DIR  <your folder here>/epiphan_sdk-3.30.3.0007
```

The SDK can be downloaded from [here](https://www.epiphan.com/downloads/products/epiphan_sdk-3.30.3.0007.zip).

## Epiphan drivers for the framegrabber

If you have issues with the framegrabber, the drivers can be downloaded from [here](https://ssl.epiphan.com/downloads/linux/). *It is very important that, for linux users, the driver version matches the kernel version*.

The kernel check can be done as follows:

1. Check the available kernel drivers in the link above. For example, for Ubuntu 20.04 there is drivers for kernels 5.4.0.42 and 5.4.0.70
2. Check the kernel installed in your system. In a terminal, type:

```
$ uname -r
5.4.0-74-generic
```

Install the corresponding kernel. In a terminal:

```
sudo dpkg -i vga2usb-3.33.0.15-ubuntu-5.4.0-70-generic-x86_64-52005-1829.deb
```

Often there is no need to reboot, just unplug and re-plug the framegrabber again.


# Troubleshooting:

In linux, you will need to be in the group 'video' to have read access to the framegrabber. In a terminal type:
```sudo usermod -a -G video $LOGNAME```


