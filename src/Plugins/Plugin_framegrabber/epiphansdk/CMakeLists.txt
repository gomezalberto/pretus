CMAKE_MINIMUM_REQUIRED(VERSION 3.2.0)

# This CMakeLists.txt is heavily inspired from the GIFT-grab project:
# https://github.com/gift-surg/GIFT-Grab

find_path(EpiphanSDK_DIR "")

# ZLIB
FIND_PACKAGE(ZLIB REQUIRED)

# pthreads
SET(CMAKE_THREAD_PREFER_PTHREAD TRUE)
FIND_PACKAGE(Threads REQUIRED)

# Include directories
SET(INCLUDE_DIRS
    ${EpiphanSDK_DIR}/epiphan/include
    ${EpiphanSDK_DIR}/epiphan/frmgrab/include
    ${EpiphanSDK_DIR}/opensource/slib/include
    ${EpiphanSDK_DIR}/opensource/slib/src
    )

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})


# Frmgrab library
SET(FrmGrab_HEADERS
    ${EpiphanSDK_DIR}/epiphan/include/v2u_compression.h
    ${EpiphanSDK_DIR}/epiphan/include/v2u_defs.h
    ${EpiphanSDK_DIR}/epiphan/include/v2u_dshow.h
    ${EpiphanSDK_DIR}/epiphan/include/v2u_id.h
    ${EpiphanSDK_DIR}/epiphan/include/v2u_ioctl.h
    ${EpiphanSDK_DIR}/epiphan/include/v2u_pack_off.h
    ${EpiphanSDK_DIR}/epiphan/include/v2u_pack_on.h
    ${EpiphanSDK_DIR}/epiphan/include/v2u_version.h
    ${EpiphanSDK_DIR}/epiphan/frmgrab/include/frmgrab.h
    )
SET(FrmGrab_LIB
    ${EpiphanSDK_DIR}/epiphan/frmgrab/lib/linux/x86_64/libfrmgrab.a
    ) # CMAKE_CURRENT_SOURCE_DIR necessary because of tests linking to this

# SLIB
SET(SLIB_HEADERS
    ${EpiphanSDK_DIR}/opensource/slib/include/s_base32.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_base64.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_bitset.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_buf.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_cs.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_def.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_dom.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_event.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_file.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_futil.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_hash.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_hist.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_init.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_itr.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_lib.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_lock.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_math.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_md.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_mem.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_mutex.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_ntk.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_ntutil.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_opt.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_os.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_prop.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_queue.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_random.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_ring.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_rwlock.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_stack.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_strbuf.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_thread.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_unix.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_util.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_vector.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_ver.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_win32.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_wkq.h
    ${EpiphanSDK_DIR}/opensource/slib/include/s_xml.h
    )
SET(SLIB_SOURCES
    ${EpiphanSDK_DIR}/opensource/slib/src/s_base32.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_base64.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_bitset.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_buf.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_cs.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_dom.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_event.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_file.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_fio.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_fio.h
    ${EpiphanSDK_DIR}/opensource/slib/src/s_fmem.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_fnull.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_fsock.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_fsplit.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_fsub.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_futil.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_fwrap.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_fzio.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_fzip.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_hash.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_hist.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_init.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_iop.h
    ${EpiphanSDK_DIR}/opensource/slib/src/s_itra.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_itr.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_itrc.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_itrf.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_itrp.h
    ${EpiphanSDK_DIR}/opensource/slib/src/s_itrs.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_lib.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_libp.h
    ${EpiphanSDK_DIR}/opensource/slib/src/s_lock.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_lockp.h
    ${EpiphanSDK_DIR}/opensource/slib/src/s_math.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_md5.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_md.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_mdp.h
    ${EpiphanSDK_DIR}/opensource/slib/src/s_mem.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_mfp.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_mpm.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_mutex.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_net.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_opt.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_parse.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_prop.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_propx.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_queue.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_random.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_ring.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_rwlock.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_sha1.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_stack.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_strbuf.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_str.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_thread.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_thrp.h
    ${EpiphanSDK_DIR}/opensource/slib/src/s_time.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_trace.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_utf8.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_util.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_vector.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_wkq.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_xml.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_xmlp.c
    ${EpiphanSDK_DIR}/opensource/slib/src/s_xmlp.h
    ${EpiphanSDK_DIR}/opensource/slib/src/unix/u_event.c
    ${EpiphanSDK_DIR}/opensource/slib/src/unix/u_furl.c
    ${EpiphanSDK_DIR}/opensource/slib/src/unix/u_futil.c
    ${EpiphanSDK_DIR}/opensource/slib/src/unix/u_mutex.c
    ${EpiphanSDK_DIR}/opensource/slib/src/unix/u_thread.c
    ${EpiphanSDK_DIR}/opensource/slib/src/unix/u_trace.c
    )
SET(SLIB_LIB slib)
ADD_LIBRARY(${SLIB_LIB} STATIC ${SLIB_HEADERS} ${SLIB_SOURCES})
SET_TARGET_PROPERTIES(${SLIB_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Headers to include
SET(HEADERS ${FrmGrab_HEADERS})
# Libs to link against
SET(LIBS
    ${FrmGrab_LIB}
    ${SLIB_LIB}
    ${CMAKE_THREAD_LIBS_INIT}
    ${ZLIB_LIBRARIES}
    )

# Variables to be used by linking libraries
SET(EpiphanSDK_INCLUDE_DIRS
    ${INCLUDE_DIRS}
    CACHE INTERNAL "Epiphan video grabbing SDK include dirs"
    )
SET(EpiphanSDK_LIBS
    ${LIBS}
    CACHE INTERNAL "Epiphan video grabbing SDK link libraries"
    )

set(INCLUDE_DIRS
    ${INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(videograbber STATIC epiphansdk_video_source.h epiphansdk_video_source.cpp)
TARGET_LINK_LIBRARIES(videograbber
    ${EpiphanSDK_LIBS}
    giftgrabapi)

target_include_directories(videograbber PUBLIC ${INCLUDE_DIRS})
