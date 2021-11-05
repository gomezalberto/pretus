## #################################################################
## Qt required for signals and slots (even if no GUI is required)
## #################################################################

find_library(PLUGIN_LIBRARY
    NAMES libPlugin.so
    HINTS "${CMAKE_PREFIX_PATH}/"
)

set(PLUGIN_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/PluginLib")
include_directories("${PLUGIN_INCLUDE_DIR}")

# need this to locate the PluginLib
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Non optional plug-ins
add_subdirectory(Plugin_filemanager)
add_subdirectory(Plugin_imageFileWriter)

option(BUILD_PLUGIN_GUI "Build plugin to visualize images and plugin outputs" ON)
if (BUILD_PLUGIN_GUI)
    add_subdirectory(Plugin_GUI)
endif()

option(BUILD_PLUGIN_VIDEOMANAGER "Build plugin to receive images from video file using OpenCV" OFF)
if (BUILD_PLUGIN_VIDEOMANAGER)
    add_subdirectory(Plugin_videomanager)
endif()

option(BUILD_PLUGIN_FRAMEGRABBER "Build plugin to receive images from the Epiphan DVI2USB 3.0 framegrabber" OFF)
if (BUILD_PLUGIN_FRAMEGRABBER)
  add_subdirectory(Plugin_framegrabber)
endif()

option(BUILD_PLUGIN_PNPFRAMEGRABBER "Build plugin to receive images from the a Plug & Play framegrabber (e.g. using v4l)" OFF)
if (BUILD_PLUGIN_PNPFRAMEGRABBER)
  add_subdirectory(Plugin_PnPframegrabber)
endif()

option(BUILD_PLUGIN_PLANEDETECTION "Build plugin to detect standard planes. Requires python bindings, and pytorch" OFF)
if (BUILD_PLUGIN_PLANEDETECTION)
  add_subdirectory(Plugin_planeDetection)
endif()

option(BUILD_PLUGIN_CPPALGORITHM "Build a sample Cpp Algorithm plug-in" OFF)
if (BUILD_PLUGIN_CPPALGORITHM)
  add_subdirectory(Plugin_CppAlgorithm)
endif()
option(BUILD_PLUGIN_PYTHONALGORITHM "Build a sample Python Algorithm plug-in" OFF)
if (BUILD_PLUGIN_PYTHONALGORITHM)
  add_subdirectory(Plugin_PythonAlgorithm)
endif()

