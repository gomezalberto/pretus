## #################################################################
## Qt required for signals and slots (even if no GUI is required)
## #################################################################

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(Qt5 REQUIRED Gui Core Widgets Concurrent Xml)

if (UNIX AND NOT APPLE)
find_package(Qt5 REQUIRED X11Extras PrintSupport)
endif()
ADD_DEFINITIONS(-DQT_NO_KEYWORDS) # needed because python has something called "slots" and may class with Qt definitions

file(GLOB SOURCES "*.h" "*.cxx" "*.md")

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(Plugin "${PROJECT_BUILD_TYPE}" ${SOURCES}
  )

include(GenerateExportHeader)
generate_export_header( Plugin
    BASE_NAME Plugin
    EXPORT_MACRO_NAME PLUGIN_EXPORT
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/PluginExport.h
    STATIC_DEFINE PLUGIN_BUILT_AS_STATIC
)

target_link_libraries(Plugin
  ifind2_common
  Qt5::Core # need this for QT signal/slot system
  Qt5::Widgets
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  )

file(GLOB HEADERS "*.h")

set_target_properties(Plugin PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS Plugin
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  COMPONENT libraries  
  PUBLIC_HEADER DESTINATION include
  )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
install (DIRECTORY ${CMAKE_BINARY_DIR}/exports/  DESTINATION include/exports)
link_directories("${CMAKE_INSTALL_PREFIX}/lib")
