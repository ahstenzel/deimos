# Build LZ4 library
add_library(liblz4 STATIC "${PROJECT_SOURCE_DIR}/src/extern/lz4.c")

# Build AES library
add_library(libaes STATIC "${PROJECT_SOURCE_DIR}/src/extern/aes.c")