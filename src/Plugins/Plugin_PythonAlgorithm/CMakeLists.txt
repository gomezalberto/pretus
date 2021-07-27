set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")

find_package(Boost REQUIRED COMPONENTS filesystem thread chrono)#COMPONENTS system filesystem thread date_time iostreams serialization)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})


## #################################################################
## Required for python bindings system
## #################################################################

# For python stuff
set(Python_ADDITIONAL_VERSIONS 3.6)
find_package(PythonLibs 3.0  REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

find_package(pybind11 REQUIRED)
if(pybind11_INCLUDE_DIR)
    include_directories(${pybind11_INCLUDE_DIR})
else()
    message(FATAL "Cannot find PYBIND11 include files. Please install/download PyBind11 at https://github.com/pybind/pybind11")
endif()

get_filename_component(PYTHON_FOLDERS ${CMAKE_CURRENT_LIST_DIR} NAME)
set(SCRIPT_INSTALL_PATH "${PLUGIN_SCRIPT_INSTALL_PREFIX}/")
set(PYTHON_INSTALL_PATH "${SCRIPT_INSTALL_PATH}${PYTHON_FOLDERS}")
configure_file( plugin_PythonAlgorithm_config.h.in ${CMAKE_BINARY_DIR}/generated/plugin_PythonAlgorithm_config.h )


## #################################################################
## Qt required for signals and slots (even if no GUI is required)
## #################################################################

find_package(Qt5 REQUIRED Gui Core Widgets Concurrent Xml)

if (UNIX AND NOT APPLE)
    find_package(Qt5 REQUIRED X11Extras PrintSupport)
endif()
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

## #################################################################
## Actual code
## #################################################################

#include_directories("${CMAKE_SOURCE_DIR}")

set(PYTHON_FILES
    python/demo.py
    python/worker.py
    )

file(GLOB SOURCES "*.h" "*.cxx")

add_library(Plugin_PythonAlgorithm "${PROJECT_BUILD_TYPE}"
    ${SOURCES}
    # python stuff
    ${PYTHON_FILES}
    )

target_link_libraries(Plugin_PythonAlgorithm
    Plugin
    Qt5::Widgets
    Qt5::Core # need this for QT signal/slot system
    Qt5::Widgets
    ${PYTHON_LIBRARIES}
    ${Boost_LIBRARIES}
    ${ITK_LIBRARIES}
    ${VTK_LIBRARIES}
    ${pybind11_LIBRARIES}
    )

install( DIRECTORY python/  DESTINATION "${SCRIPT_INSTALL_PATH}/${PYTHON_FOLDERS}" )

install(TARGETS Plugin_PythonAlgorithm
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION pluginlib
    ARCHIVE DESTINATION pluginlib
    COMPONENT libraries
    )

