# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTORCC ON)

# Please don't remove the win32 code!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
if(WIN32)
    SET(PLUGIN_FOLDER "${CMAKE_BINARY_DIR}/bin/Debug" CACHE FILEPATH "Folder where plugins will be")
    STRING(REGEX REPLACE "\\\\" "\\\\\\\\" PLUGIN_FOLDER ${PLUGIN_FOLDER} )
else()
    SET(PLUGIN_FOLDER "${CMAKE_BINARY_DIR}/lib" CACHE FILEPATH "Folder (or colon separated folders) where plugins will be")
endif()
# include dl library below!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

add_definitions( -DFOLDER_PLUGIN=${PLUGIN_FOLDER} )
configure_file( my_config.h.in ${CMAKE_BINARY_DIR}/generated/my_config.h )
include_directories( ${CMAKE_BINARY_DIR}/generated/ )

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")

# Boost (for signals slots)
find_package(Boost REQUIRED COMPONENTS filesystem thread chrono)#COMPONENTS system filesystem thread date_time iostreams serialization)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

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

include_directories("${CMAKE_SOURCE_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")


add_executable(pretus
  MACOSX_BUNDLE
  main.cxx
   )

TARGET_LINK_LIBRARIES(pretus
  Qt5::Core # need this for QT signal/slot system
  Qt5::Widgets
  ${Boost_LIBRARIES}
  ifind2_common
  Plugin
  dl
)

#install( DIRECTORY html/  DESTINATION "${SCRIPT_INSTALL_PATH}/${HTML_FOLDERS}" )

install(TARGETS pretus
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  COMPONENT libraries
  )


# create the launcher
get_filename_component(PARENT_QT_DIR ${Qt5Core_DIR} DIRECTORY)
get_filename_component(LOCAL_QT_LIB_DIR ${PARENT_QT_DIR} DIRECTORY)
get_filename_component(LOCAL_QT_DIR ${LOCAL_QT_LIB_DIR} DIRECTORY)

file(WRITE /tmp/launcher_pretus.sh "#! /bin/bash\n")
file(APPEND /tmp/launcher_pretus.sh "export QT_PLUGIN_PATH=${LOCAL_QT_DIR}/plugins/\n")
file(APPEND /tmp/launcher_pretus.sh "export LD_LIBRARY_PATH=${LOCAL_QT_LIB_DIR}/\n")
file(APPEND /tmp/launcher_pretus.sh "export QTDIR=${LOCAL_QT_DIR}/\n")
file(APPEND /tmp/launcher_pretus.sh "$PWD/bin/pretus \${\@}\n")

install(
    FILES /tmp/launcher_pretus.sh
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    DESTINATION "${CMAKE_INSTALL_PREFIX}"
)

