# Set path variables
set(QT_FULL_PATH "$ENV{QT_PATH}/${DEIMOS_GEN}_64")
set(OUTPUT_DIR "${PROJECT_SOURCE_DIR}/out/dist/${DEIMOS_OS}/$<IF:$<CONFIG:Debug>,Debug,Release>")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

# Load Qt support
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_FULL_PATH})
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Xml)
qt_standard_project_setup()
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Build executable
qt_add_executable(deimos
	"${PROJECT_SOURCE_DIR}/src/app/browselineedit.cpp"
	"${PROJECT_SOURCE_DIR}/src/app/assettreewidget.cpp"
	"${PROJECT_SOURCE_DIR}/src/app/assetdialog.cpp"
	"${PROJECT_SOURCE_DIR}/src/app/projectfile.cpp"
	"${PROJECT_SOURCE_DIR}/src/app/projecttab.cpp"
	"${PROJECT_SOURCE_DIR}/src/app/mainwindow.cpp"
	"${PROJECT_SOURCE_DIR}/src/app/resources.cpp"
	"${PROJECT_SOURCE_DIR}/src/app/common.cpp"
	"${PROJECT_SOURCE_DIR}/src/app/main.cpp"
	"${PROJECT_SOURCE_DIR}/resources/mars.rc"
)

# Build resources
include(resourcecompile)
rcc(deimos "${PROJECT_SOURCE_DIR}/resources/resources.qrc" "${PROJECT_SOURCE_DIR}/src/app/resources.cpp")

# Include directories
target_include_directories(deimos PRIVATE 
	"${QT_FULL_PATH}/include/QtCore"
	"${QT_FULL_PATH}/include/QtWidgets"
	"${QT_FULL_PATH}/include/QtGui"
	"${QT_FULL_PATH}/include/QtXml"
	"${PROJECT_SOURCE_DIR}/src/extern"
)

# Copy Qt libraries to output folder
if (${DEIMOS_OS} STREQUAL "windows")
	include(windeployqt)
	windeployqt(deimos)
endif()

# Link libraries
target_link_libraries(deimos PRIVATE 
	Qt6::Widgets 
	Qt6::Gui
	Qt6::Xml
	liblz4
)

# Set executable properties
set_target_properties(deimos PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
	LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
	RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
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