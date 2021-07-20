option(BUILD_STANDALONE_PRETUS_APP "Build standalone PRETUS: Plug-in based Real Time Ultrasound app" ON)
if (BUILD_STANDALONE_PRETUS_APP)
    add_subdirectory(PRETUS)
endif()
