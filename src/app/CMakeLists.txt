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
	"${PROJECT_SOURCE_DIR}/src/app/common.cpp"
	"${PROJECT_SOURCE_DIR}/src/app/mainwindow.cpp"
	"${PROJECT_SOURCE_DIR}/src/app/main.cpp"
)

# Include Qt headers
target_include_directories(deimos PRIVATE 
	"${QT_FULL_PATH}/include/QtCore"
	"${QT_FULL_PATH}/include/QtWidgets"
	"${QT_FULL_PATH}/include/QtGui"
	"${QT_FULL_PATH}/include/QtXml"
)

# Copy dependencies to output folder
if (${DEIMOS_OS} STREQUAL "windows")
	set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
	include(windeployqt)
	windeployqt(deimos)
endif()

# Link Qt Libraries
target_link_libraries(deimos PRIVATE 
	Qt6::Widgets 
	Qt6::Gui
	Qt6::Xml
)