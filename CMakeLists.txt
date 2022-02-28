set(TARGET_NAME Serial)

set(LIBRARY_OUTPUT_PATH ${BUILD_DIR}/${TARGET_NAME})

add_library(${TARGET_NAME} STATIC Serial.cpp)

