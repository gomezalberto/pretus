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

find_package(Qt5 REQUIRED Gui Core Widgets Concurrent Xml)

if (UNIX AND NOT APPLE)
find_package(Qt5 REQUIRED X11Extras PrintSupport)
endif()
ADD_DEFINITIONS(-DQT_NO_KEYWORDS) # needed because python has something called "slots" and may class with Qt definitions


file(GLOB SOURCES "*.h" "*.cxx" "*.md")

add_library(Plugin_PnPframegrabber "${PROJECT_BUILD_TYPE}" ${SOURCES})

target_link_libraries(Plugin_PnPframegrabber
  Plugin
  ifind2_common
  Qt5::Widgets
  Qt5::Core # need this for QT signal/slot system
  Qt5::Widgets
  ${Boost_LIBRARIES}
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  #videograbber
  )


option(BUILD_STANDALONE_PNPFRAMEGRABBER "Build standalone version of the P&P framegrabber plugin" OFF)
if (BUILD_STANDALONE_PNPFRAMEGRABBER)

    #set_property(SOURCE standalone_framegrabber.cxx PROPERTY SKIP_AUTOMOC ON)

    add_executable(standalone_PnPframegrabber
      standalone_PnPframegrabber.cxx
      #FGMiniworker.cxx
      PnPFrameGrabberManager.cxx
      )
    target_link_libraries(standalone_PnPframegrabber
      Plugin
      Qt5::Widgets
      Qt5::Core # need this for QT signal/slot system
      Qt5::Widgets
      ${Boost_LIBRARIES}
      ${ITK_LIBRARIES}
      ${VTK_LIBRARIES}
      #videograbber
      )
    set(EXTRA_TARGETS standalone_PnPframegrabber)

endif() #BUILD_STANDALONE_FRAMEGRABBER

install(TARGETS Plugin_PnPframegrabber ${EXTRA_TARGETS}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION pluginlib
  ARCHIVE DESTINATION pluginlib
  COMPONENT libraries
  )

