### Slightly more comprehensively/verbosely:

**Configure build environment**

* Install gcc compiler https://linuxize.com/post/how-to-install-gcc-compiler-on-ubuntu-18-04/
* Install OpenGL http://www.codebind.com/linux-tutorials/install-opengl-ubuntu-linux/
* Install git https://linuxize.com/post/how-to-install-git-on-ubuntu-18-04/
* Install CMake (in Ubuntu Software)
    * This may fail to run with no error, nice! In which case uninstall and then
        * sudo snap install cmake --classic
        * sudo apt install cmake-gui
* QT
    * Install the latest version of QT from download
        * https://www.qt.io/download-qt-installer?hsCtaTracking=99d9dd4f-5681-48d2-b096-470725510d34%7C074ddad0-fdef-4e53-8aa8-5e8a876d6ab4
        * Install the required version of QT libraries (Android modules are not required)
        * tick `QT WebEngine` and `Sources`
    * To update QT using the maintenance tool a url for and update source needs to be set (durr). To user defined repositories add
        * http://download.qt-project.org/online/qtsdkrepository/linux_x64/desktop/tools_maintenance/
        * There are other repositories - I had the maintenance tool complain about being too old (v3.0.2) when I tried, them - results was not fix, massive frustration
    * You can try and install QT the console (but when last tried this was an old version) and these will (hopefully) suffice:
        * sudo apt install qt5-default
        * [sudo apt install libqt5x11extras5] this one may not be necessary, but the following one is
        * sudo apt install libqt5x11extras5-dev
        * sudo apt install qttools5-dev
        * sudo apt install qtdeclarative5-dev
        * And to install QT creator
            * sudo apt install qtcreator
* Boost
    * sudo apt install libboost-all-dev 
        (or libboost-dev may work - https://stackoverflow.com/questions/12578499/how-to-install-boost-on-ubuntu)
    * (There may be some other boost installs required, but I restarted the VM befoire making a note of what was installed)
* VTK
    * There are many and varied instructions for this...
        * https://gitlab.kitware.com/vtk/vtk/blob/master/Documentation/dev/git/download.md - use this one
        * ( https://vtk.org/Wiki/VTK/Building/Linux )
        * ( https://vtk.org/Wiki/VTK/Configure_and_Build )
    * git clone https://gitlab.kitware.com/vtk/vtk VTK
    * git checkout tags/v8.2.0 (it will whine)
    * git update submodules --init
    * Run CMake/ cmake-gui
        * Enable `VTK_Group_Qt`, `vtkGUISupportQtOpenGL`, `vtkImagingOpenGL2`, 
            * I no longer think `vtkGUISupportQtWebKit` needs to be enabled (QtWebKit is deprecated and not in new QT)
        * Set Qt5_DIR to e.g. /home/ab99/Qt5.14.2/5.14.2/gcc_64/lib/cmake/Qt5 
        * CMAKE_BUILD_TYPE - RelWithDebInfo 
            * If you ever get seriously into debugging VTK choose Debug
            * Release will be ever so slightly quicker then RelWithDebInfo, but no good for identifying problems as we're delivering to evaluation - which is important
        * With the other default settings it should work
        * BUt the build may fail with some error on a test - diabling BUILD_TEST enables VTK to be built (but this does need to be fixed)
        * If you are short of space (e.g. VM) turn off BUILD_TESTING
    * make
* HDF5: We recommend building the c/cpp libraries from source, from the [HDF5 git repository](https://github.com/HDFGroup/hdf5). Tested with version 1.10.6. THen, make sure you install the same python version, and we recommend doing this via anaconda with `conda install -c anaconda hdf5=1.10.6`. Then make sure ITK and opencv use the build version.
* ITK 
    * git clone https://github.com/InsightSoftwareConsortium/ITK ITK
    * git tag (will list all of the tags)
    * git checkout tags/[latest release tag, e.g. v4.9.1 worked]
    * Run CMake/ cmake-gui
        * Enable BUILD_SHARED_LIBS, Module_ITKVtkGlue
        * Set VTK_DIR to the vtk build/release folder
        * CMAKE_BUILD_TYPE - RelWithDebInfo (see VTK section above for rationale)
    * make
* Anaconda
    * Folow https://www.digitalocean.com/community/tutorials/how-to-install-the-anaconda-python-distribution-on-ubuntu-18-04
    * [on a VM you may need to make the number of vCPUs greater than one, no I don't know why]
        * Next time these instructions are followed - is there a way to add python to the path? I was having to manually add the python lib path and include folder in CMake
* PyBind
    * Clone from https://github.com/pybind/pybind11

**Clone, build and run the iFind application**

* git clone https://gitlab.com/ifind/standalone.git standalone 
* setup a build folder for it
* Run CMake
    * Set VTK_DIR and ITK_DIR to their respective build directories
    * Set PYBIND11_INDLUDE_FOLDER to the PYBIND11 include folder :)
    * Disable the plugins  you don't want
    * Enable the standalone application
    * Enable shared libs
    * Set build type to Debug or RelWithDebInfo for dev/deployment
    * To build the debug code set CMAKE_BUILD_TYPE as Debug
    * Configure and Configure and ... and Generate
* Load, Build and Run in QT Creator
    * Open QT Creator, 'Open file or project...'
        * Choose 'CMakeLists.txt' file in top 'standalone' folder
        * This should open the standalone app and the selected plugins
        * Or it may just obliterate your CMake setup, because, reasons...
    * Build Project
    * Run (Ctrl-R) and...


## Dual booting Ubuntu with Windows:

Ask IT to set this up, it's quite quick.

## Remote debugging from Visual Studio

I have tried this with iFind and it worked surprisingly easily.

* Test Environment
    * Windows 10, Visual Studio 2019 - Linux Development with C++ workload installed (installable from the VS installer)
* Linux machine
    * If the application/plugin has a GUI you will need a separate computer
    * In theory an application without a GUI should work on the Ubuntu WSL (Windows Subsystem for Linux, available in the Microsoft Store)
    * Get a build of standalone with the necessary plugins up and running - that way you'll have the required dependencies on the target
* These instructions work to run an example (has a GUI so you will require a second, Linux computer)
    * https://docs.microsoft.com/en-us/cpp/build/get-started-linux-cmake?view=vs-2019
    * Gotchas were
        * Reemember to add the '.isd.kcl.ac.uk' to the remote computers address
        * To set up the Linux Display parameter make sure you have selected 'Linux-Debug' as the active build (this udpdates the configuration file)
* standalone then works out of the box pretty much
    * CMake in VS requires the linux machine paths setting to find VTK, ITK, QT etc.
    * Command line args are set by adding a 'args' parameters to the build  config in the launch json file, e.g. 
        * `"args": [ "-pipeline", "FileManager*Visualization", "-fm_input", "/home/gw17/iFind/data/demo_streams/iFIND00541/Input/General", "-fm_loop", 1 ]`
        * NB the one thing which didn't work was the '>' -plugins parameter, I'll fix this so it will take a '*' on both platforms


## Setting up an Ubuntu VM on Windows:

We haven't yet found the 'right way' to set up an Ubuntu VM on Windows. You might be lucky and get everything to work straight away. Or you might be unlucky and fall into one of the many traps along the way - here are a few of them we found, details of which are there to help to weaery traveller.

There are two ways of setting up an Ubuntu VM:

* Quick setup (generally recommended)
* The long setup (may be necessary)

Plus the choice of Ubuntu

* **18.04 LTS - As at 20/05/2020 use this one**
* 20.04 LTS - I'm using this and tooooooo many things don't work yet/require too much goooglong

Looks out below for some extra tips too 

### Quick setup

* Enable HyperV https://docs.microsoft.com/en-us/virtualization/hyper-v-on-windows/quick-start/enable-hyper-v
* To create a virtual machine with Hyper-V please follow this link https://docs.microsoft.com/en-us/virtualization/hyper-v-on-windows/quick-start/quick-create-virtual-machine
    * Open Hyper-V Quick Create from the start menu
    * Select 'Ubuntu Chosen Version' from the provided list on the left
    * Click the 'Create Virtual Machine' button
* VM settings that runs not too slow
    * running it on SSD
    * 6 virtual processors by default
    * set memory 4096MB minimum 
    * by default the virtual hard drive is 11GB, need to assign bigger space
        * if do need to expand the hard drive, follow attempt 2 in this link https://www.frodehus.dev/resize-disk-for-ubuntu-hyper-v-quick-create-image/
* Install Ubuntu etc. follow the instructions below


### Long setup

This will be slow, but will allow testing

* Enable HyperV https://docs.microsoft.com/en-us/virtualization/hyper-v-on-windows/quick-start/enable-hyper-v
* Download 'Ubuntu Chosen Version' ISO https://ubuntu.com/download/desktop
* Run HyperV and create a new VM, settings are mostly the default
    * Generation 1 (I tried 1 but it dodn't work, even though it should)
    * 4096MB minimum memory
    * Dynamic memory
    * Default network switch
    * Create Virtual Hard disk of 127GB and format it
        * That's quite a lot - 40GB should be sufficient
        * If it is on a spinny disk it'll be slow - so try and put it on an SSD
    * Install OS from ISO
* Install Ubuntu
    * Basically default settings, except
    * UK!
    * Minimal install
    * Create user account
    * Reboot
* Configure Ubuntu 
   * Set screen size to 1920 x 1080 - http://gitwww.computer-server-repair.com/changing-screen-resolution-in-ubuntu-18-04-using-hyper-v/
* Create a shared data folder between windows and linux
    * https://linuxhint.com/shared_folders_hypver-v_ubuntu_guest/
    * Where user name is e.g. bb15@isd.kcl.ac.uk
    * And you have to reset this each time you turn the VM off and on again

### Other notes on installing Hyper-V VMs


#### Disk size 

Quick setup only gives you an inadequate 12GB, Qt alone wants to install 4+GB, and fails 

- In theory 
    - HyperV can expand the disk space 
    - Then 'Disk' in Ubuntu can expand into this space
- But I could not get this to work. Various, more complex ways to achieve this...
    - [shujie's link]
    - https://github.com/microsoft/linux-vm-tools/issues/82
    - However, I had difficulty booting from the gparted disk, and had to set up its security ID
        - These instrtuctions and some gumption enable booting from teh gparted disk
        - https://support.microsoft.com/en-gb/help/2249906/hyper-v-virtual-machine-may-not-start-and-you-receive-a-general-access
    - However, gparted was still upable to edit the partitions and kept crashing. So I gave up and use the long setup method

#### The generation of the VM (Gen 1 or Gen 2)

https://docs.microsoft.com/en-us/windows-server/virtualization/hyper-v/plan/should-i-create-a-generation-1-or-2-virtual-machine-in-hyper-v

- Quick setup is always Gen2
- Long setup you get to pick - you should pick Gen 2, but
    - This introduces various other issues
        - To boot/install from the Ubuntu ISO set the security policy...
        - VM-> Right Click-> Settings...-> Security-> Secure Boot-> MIcrosoft UEFI Certificate Authority

#### Enhanced Mode

- Enhanced mode enables copy-paste clipboard and better file sharing, and so is a good thing
    - Enabled by default for quick create Ubuntu VMs
        - But I think this may only work for 18.04, not 20.04 (as at May 2020)
        - there were problems with 19.10 https://unix.stackexchange.com/questions/571645/hyper-v-enhanced-session-mode-ubuntu-19-10
- Here are a few links
    - https://superuser.com/questions/734880/hyper-v-clipboard-and-integration-services-in-ubuntu
    - https://www.tenforums.com/virtualization/107147-guide-how-run-ubuntu-18-04-enhanced-mode-hyper-v.html

#### Hyper V services available

- https://docs.microsoft.com/en-us/virtualization/hyper-v-on-windows/reference/integration-services#hyper-v-data-exchange-service-kvp

### Plugins which have been built and run under Windows

* Standalone
* Plugin_filenamanager
* Plugin_dummy (has a memory leak)
* Plugin_visualization
* Plugin_imageFileWriter
* Autoreport
    * (but not autoreport standalone at the time of writing)


## Handy things

* To search for apt packages
    * apt-cache search <package name substring>
* Linux will say that it cannot find an executable, when what it reallly means is that it can totally find it, but it cannot find one of its depndencies. (I have no idea what the equivalent of dependency walker is.)
* Attempting to do anything really within QtCreator to do with Project->Build settings (whci ultimately come from CMake) will end up with QT Creator overwriting your CMake cache and you having to re-enter paths. This is far too easily done.
* Install Tweak Tools
    * From app store (doesn't seem to work on 20.04)
    * But this worked for 20.04 https://linuxconfig.org/how-to-install-tweak-tool-on-ubuntu-20-04-lts-focal-fossa-linux
* Update display resolution of VM
    * https://metinsaylan.com/8991/how-to-change-screen-resolution-on-ubuntu-18-04-in-hyper-v/ (tested in 20.04)

## Things to do:

* Build VTK with the tests and run the tests and make sure that they pass
