cmake_minimum_required(VERSION 3.0)

project(PRETUS VERSION 1.0)

## #################################################################
## cmake policy warning removal
## #################################################################
set(CMAKE_POLICY_DEFAULT_CMP0054 NEW)
cmake_policy(SET CMP0054 NEW)

## #################################################################
## CMAKE generic options / settings
## #################################################################

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release
RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_STANDARD 11)
option(BUILD_SHARED_LIBS "Build shared libraries, required for the plug-ins" ON)
if (BUILD_SHARED_LIBS)
    set(PROJECT_BUILD_TYPE SHARED)
else()
    set(PROJECT_BUILD_TYPE STATIC)
endif()
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(PLUGIN_SCRIPT_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/Plugins" )
if(WIN32)
    set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/bin)
else()
    set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
endif(WIN32)
if (APPLE)
    set(GUI_EXECUTABLE MACOSX_BUNDLE)
endif()

file(WRITE ${PROJECT_BINARY_DIR}/VERSION "${PROJECT_VERSION}")

## #################################################################
## Increase the template depth under linux
## #################################################################
if (UNIX)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ftemplate-depth=1024")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ftemplate-depth=1024")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=1024 -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftemplate-depth=1024 -fPIC")
elseif (WIN32)
	# here need to clone the dlfcn 32 repo
	# https://github.com/dlfcn-win32/dlfcn-win32
	# and build / link it
endif()


## #################################################################
## Documentation
## #################################################################
option(BUILD_DOC "Build documentation" OFF)
if (BUILD_DOC)
    find_package(Doxygen)
    if (DOXYGEN)
        set(MY_DOXYGEN_BUILD_DIR ${PROJECT_BINARY_DIR}/docs/Doxygen)
        configure_file (${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxygen/doxygen.config.in ${MY_DOXYGEN_BUILD_DIR}/doxygen.config)
        add_custom_target(doc ${DOXYGEN} ${MY_DOXYGEN_BUILD_DIR}/doxygen.config)
    else()
        MESSAGE(WARNING "Doxygen was not found. Documentation cannot be built")
    endif()
endif()

## #################################################################
## Git
## #################################################################

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
    find_package(Git)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short=10 HEAD
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE "${PROJECT_NAME}_BUILD_VERSION"
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        message( STATUS "Git version: ${${PROJECT_NAME}_BUILD_VERSION}" )
    else(GIT_FOUND)
        set(${PROJECT_NAME}_BUILD_VERSION 0)
    endif()
endif()

## #################################################################
## Libraries that I definitely need: VTK ITK Qt and boost
## #################################################################

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost REQUIRED COMPONENTS filesystem thread chrono)#COMPONENTS system filesystem thread date_time iostreams serialization)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package(Qt5 REQUIRED Gui Core Widgets Concurrent Xml)

if (UNIX AND NOT APPLE)
    find_package(Qt5 REQUIRED X11Extras PrintSupport)
endif()
ADD_DEFINITIONS(-DQT_NO_KEYWORDS) # needed because python has something called "slots" and may class with Qt definitions


if (WIN32)
	option(USE_SYSTEM_DLFCWIN32 "Use an existing version of dlfc-win32" OFF)
	if (USE_SYSTEM_DLFCWIN32)
		find_package(dlfcn-win32 REQUIRED)
	else(USE_SYSTEM_DLFCWIN32)
		include(ExternalProject)

		set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

		ExternalProject_Add(dlfcn-win32
			GIT_REPOSITORY https://github.com/dlfcn-win32/dlfcn-win32.git
			CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
		)

		include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
		link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)
	endif()
	set(CMAKE_DL_LIBS dlfcn-win32::dl)
endif()

## #################################################################
## Build stuff
## #################################################################

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)

include_directories("${CMAKE_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/Common")
include_directories("${CMAKE_SOURCE_DIR}/PluginLib")
include_directories("${CMAKE_SOURCE_DIR}/Plugins")
add_subdirectory(Common)
add_subdirectory(PluginLib)
add_subdirectory(Plugins)
add_subdirectory(App)
