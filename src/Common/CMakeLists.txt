set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")

find_package(Boost REQUIRED COMPONENTS system filesystem thread chrono)#COMPONENTS system filesystem thread date_time iostreams serialization)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package(Qt5 REQUIRED Gui Core Widgets Concurrent Xml)

if (UNIX AND NOT APPLE)
    find_package(Qt5 REQUIRED X11Extras PrintSupport)
endif()
ADD_DEFINITIONS(-DQT_NO_KEYWORDS) # needed because python has something called "slots" and may class with Qt definitions

file(GLOB SOURCES "*.h" "*.cxx")

find_package(HDF5 REQUIRED COMPONENTS C CXX HL CONFIG)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(ifind2_common "${PROJECT_BUILD_TYPE}" ${SOURCES})

include(GenerateExportHeader)
generate_export_header( ifind2_common
    BASE_NAME ifind2_common
    EXPORT_MACRO_NAME iFIND2COMMON_EXPORT
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/ifind2_common_Export.h
    STATIC_DEFINE ifind2_common_BUILT_AS_STATIC
)

target_link_libraries(ifind2_common
    Qt5::Widgets
    Qt5::Core # need this for QT signal/slot system
    Qt5::Widgets
    ${Boost_LIBRARIES}
    ${ITK_LIBRARIES}
    )

file(GLOB COMMON_INSTALLABLE_HEADERS "*.h")
set_target_properties(ifind2_common PROPERTIES PUBLIC_HEADER "${COMMON_INSTALLABLE_HEADERS}")

install(TARGETS ifind2_common
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    COMPONENT libraries
    PUBLIC_HEADER DESTINATION include
    )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
install (DIRECTORY ${CMAKE_BINARY_DIR}/exports/  DESTINATION include/exports)

