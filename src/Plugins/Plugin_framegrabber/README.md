# Framegrabber plugin.

A plug-in to read real time images using an EPIPHAN DVI2USB framegrabber.

## Usage

The following plug-in options can be tweaked via the command line interface:

``` bash
# PLUGIN Frame grabber
   Reads real-time imaging from a video source using the Epiphan DVI2USB 3.0  grabber.
	--framegrabber_verbose <val> [ type: BOOL]	Whether to print debug information (1) or not (0). (Default: 0) 
	--framegrabber_showimage <val> [ type: INT]	Whether to display realtime image outputs in the central window (1) or not (0). 
                                            		(Default: <1 for input plugins, 0 for the rest>) 
	--framegrabber_showwidget <val> [ type: INT]	Whether to display widget with plugin information (1-4) or not (0). Location is 
                                             		1- top left, 2- top right, 3-bottom left, 4-bottom right. (Default: visible, 
                                             		default location depends on widget.) 
   Plugin-specific arguments:
        --framegrabber_studioswing <val> [ type: BOOL]	Correct for studio swing (1) or not (0). (Default: 0) 
        --framegrabber_pixelsize <val> [ type: FLOAT]	Value, in mm, of the pixel size (isotropic). (Default: 1) 
        --framegrabber_framerate <val> [ type: FLOAT]	Frame rate at which the plugin does the work. (Default: 20) 
        --framegrabber_demo filename with demo frame [ type: STRING]	Filename (typically .bin) with the frame values. (Default: )
```


# Build and configuration

This plug-in requires setting new CMake variables, the Epiphan SDK available and that the drivers are installed. More details below. This plug-in also needs OpenCV.

## CMake configuration

This plug-in requires to fill the following CMake variables:

* EpiphanSDK_DIR (see subsection below)
* z_lib {include, library} dirs: It is very importatnt that these folders are consistent with any potential pythoin installation. For example, if you have zlib installed within your anaconda path, you should use those (e.g. `<HOME>/anaconda3/lib/libz.so`)




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

### Linux users

If you need to install a specific kernel in Ubuntu, you can do so by doing:

```
$ sudo apt-get install linux-headers-5.4.0-70-generic
$ sudo apt-get install linux-image-5.4.0-70-generic
```
And then rebooting and selecting that kernel version using GRUB.


# Troubleshooting:

In linux, you will need to be in the group 'video' to have read access to the framegrabber. In a terminal type:
```sudo usermod -a -G video $LOGNAME```


