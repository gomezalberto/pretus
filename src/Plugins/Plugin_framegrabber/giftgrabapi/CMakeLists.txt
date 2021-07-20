CMAKE_MINIMUM_REQUIRED(VERSION 3.2.0)

ADD_LIBRARY(giftgrabapi STATIC
	broadcastdaemon.cpp
	broadcastdaemon.h
	macros.h
	ivideosource.h
	videoframe.cpp
	videoframe.h
        maskframe.cpp
        maskframe.h
        except.cpp
        except.h
        iobservable.cpp
        iobservable.h
        iobserver.h
	)

TARGET_LINK_LIBRARIES(giftgrabapi 
	#${EpiphanSDK_LIBS}
	)

target_include_directories(giftgrabapi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
