#pragma once

#define DEIMOS_VERSION_MAJOR 1
#define DEIMOS_VERSION_MINOR 0
#define DEIMOS_VERSION_PATCH 0

#define STRINGIFY_(X) #X
#define STRINGIFY(X) STRINGIFY_(X)
#define MAKE_VERSION_STR(major, minor, patch) (STRINGIFY(major) "." STRINGIFY(minor) "." STRINGIFY(patch))
#define DEIMOS_VERSION_STR MAKE_VERSION_STR(DEIMOS_VERSION_MAJOR, DEIMOS_VERSION_MINOR, DEIMOS_VERSION_PATCH)

// Standard libraries
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

// Qt libraries
#include <QWidget>
#include <QMainWindow>
#include <QGroupBox>
#include <QScrollArea>
#include <QTreeWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QStackedWidget>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QVariant>
#include <QVariantList>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QStyledItemDelegate>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QXMLStreamWriter>
#include <QXMLStreamReader>
#include <QEvent>
#include <QMouseEvent>