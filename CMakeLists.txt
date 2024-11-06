cmake_minimum_required(VERSION 3.16)

project(deimos VERSION 1.0.0 LANGUAGES CXX)

# Set CMake language requirements
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set CMake path variables
set(QT_FULL_PATH "$ENV{QT_PATH}/${DEIMOS_GEN}_64")
set(OUTPUT_DIR "${PROJECT_SOURCE_DIR}/out/dist/${OS}/$<IF:$<CONFIG:Debug>,Debug,Release>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

# Build main app
add_subdirectory("${PROJECT_SOURCE_DIR}/src/app")

# Set executable properties
set_target_properties(deimos PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
	MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

# Set install rules
install(TARGETS deimos
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Copy MinGW libraries
if (${DEIMOS_GEN} STREQUAL "mingw")
	add_custom_command(TARGET deimos POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_FULL_PATH}/bin/libgcc_s_seh-1.dll ${OUTPUT_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_FULL_PATH}/bin/libstdc++-6.dll ${OUTPUT_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_FULL_PATH}/bin/libwinpthread-1.dll ${OUTPUT_DIR}
		COMMENT "Deploy mingw runtime libraries from ${QT_FULL_PATH}/bin"
	)
endif()