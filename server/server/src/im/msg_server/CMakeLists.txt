cmake_minimum_required(VERSION 2.8.5)
PROJECT(msgserver)

AUX_SOURCE_DIRECTORY(./ SRC_LIST)

#SET(LIBRARY_OUTPUT_PATH ./)
SET(EXECUTABLE_OUTPUT_PATH  ./)
if(APPLE)
    SET(PB_LIB_DIR ../../base/pb/lib/mac)
    SET(SECURITY_LIB_DIR ../../base/security/lib/mac)
elseif(UNIX)
    SET(PB_LIB_DIR ../../base/pb/lib/linux)
    SET(SECURITY_LIB_DIR ../../base/security/lib/linux)
endif()


ADD_DEFINITIONS( -g -W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11)
include_directories(
  include
  ../../base/include)
INCLUDE_DIRECTORIES(./ ../../base ../../base/pb/protocol ../../base/pb/ ../../base/security/)
LINK_DIRECTORIES(./ ../../3rd/tinyxml2 ../../base ${PB_LIB_DIR} ../../base/slog/lib ${SECURITY_LIB_DIR})

#ADD_XXXX必须在TARGET_LINK_LIBRARIES前面，否则会报错
#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})
ADD_EXECUTABLE(msg_server ${SRC_LIST})

TARGET_LINK_LIBRARIES(msg_server base protobuf-lite slog crypto pthread curl tinyxml2)
 
