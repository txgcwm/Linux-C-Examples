set(EXECUTABLE_OUTPUT_PATH ${OUTDIR}/rtsp/test)

include_directories(
    ${TOPDIR}/rtsp/include
    ${TOPDIR}/rtsp/src
)

link_directories(
    ${TOPDIR}/opensource/prebuild/libs
)


#################################
add_executable(rtspd rtspd.c)
target_link_libraries(rtspd rtsp pthread)


