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


set(SCRIPT_INSTALL_PATH "${PLUGIN_SCRIPT_INSTALL_PREFIX}/")

## #################################################################
## Qt required for signals and slots (even if no GUI is required)
## #################################################################

find_package(Qt5 REQUIRED Gui Core Widgets Concurrent Xml)

if (UNIX AND NOT APPLE)
    find_package(Qt5 REQUIRED X11Extras PrintSupport)
endif()
ADD_DEFINITIONS(-DQT_NO_KEYWORDS) # needed because python has something called "slots" and may class with Qt definitions

## #################################################################
## Actual code
## #################################################################

#include_directories("${CMAKE_SOURCE_DIR}")


file(GLOB SOURCES "*.h" "*.cxx")

add_library(Plugin_GUI "${PROJECT_BUILD_TYPE}"
    ${SOURCES}
    )

target_link_libraries(Plugin_GUI
    Plugin
    Qt5::Widgets
    Qt5::Core # need this for QT signal/slot system
    Qt5::Widgets
    ${Boost_LIBRARIES}
    ${ITK_LIBRARIES}
    ${VTK_LIBRARIES}
    )


#install(FILES ${PYTHON_FILES} DESTINATION ${SCRIPT_INSTALL_PATH})
#message("Run the following command: export  PYTHONPATH=""${SCRIPT_INSTALL_PATH}${PYTHON_FOLDERS}":"$PYTHONPATH")

install(TARGETS Plugin_GUI
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION pluginlib
    ARCHIVE DESTINATION pluginlib
    COMPONENT libraries
    )

