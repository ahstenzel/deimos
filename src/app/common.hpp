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
#include <algorithm>
#include <exception>
#include <limits>

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
#include <QEvent>
#include <QMouseEvent>
#include <QStyledItemDelegate>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDomDocument>
#include <QXMLStreamWriter>
#include <QXMLStreamReader>
#include <QVariant>
#include <QVariantList>
#include <QVector>
#include <QStack>
#include <QList>
#include <QException>

// External libraries
#define LZ4_HEAPMODE 1
#include "lz4.h"

// ============================================== Helper functions

/// @brief Round the number up to the next multiple.
uint64_t roundUp(uint64_t num, uint64_t multiple);

/// @brief Calcualte the CRC32 value for the given data.
/// @param data Data buffer
/// @param length Buffer length
/// @param previousCRC Previous CRC value (only if data is being appended to an existing CRC value)
/// @return CRC value
uint32_t crc32Calculate(const void* data, size_t length, uint32_t previousCRC = 0);

extern const uint32_t crc32Lookup[256];

// ============================================== Asset functions

/// @brief Descriptor for a single asset- whether its an actual resource or a group of resources.
struct AssetInfo {
	QString title = "";
	QString type = "";
	QString filename = "";
};

/// @brief Descriptor for a type of asset and the file extensions associated with it.
struct AssetTypeDescriptor {
	QString code = "";
	QStringList extensions = {};
};

/// @brief Map of asset type names to their associated descriptor.
extern const QList<QPair<QString, AssetTypeDescriptor>> assetTypes;

/// @brief List of valid encryption algorithms.
extern const QStringList cipherMethods;

/// @brief Single node in a tree of assets.
class AssetTreeNode {
public:
	AssetTreeNode();
	AssetTreeNode(const AssetInfo& assetInfo);
	~AssetTreeNode();

	AssetTreeNode* addChild();
	AssetTreeNode* addChild(const AssetInfo& assetInfo);
	AssetTreeNode* addChild(AssetTreeNode* node);
	void removeChild(AssetTreeNode* child);
	qsizetype childCount();
	QVector<AssetTreeNode*>::iterator begin();
	QVector<AssetTreeNode*>::iterator end();
	QVector<AssetTreeNode*>::reverse_iterator rbegin();
	QVector<AssetTreeNode*>::reverse_iterator rend();

	bool isGroup();

	AssetInfo assetInfo();
	void setAssetInfo(const AssetInfo& assetInfo);

private:
	AssetInfo m_assetInfo;
	QVector<AssetTreeNode*> m_children;
};

/// @brief Descriptor for an entire project, with all the assets and settings associated with it.
struct ProjectFileInfo {
	QString m_filename = "";
	AssetTreeNode* m_assetTree = nullptr;
	QString m_cipherMethod = "";
	bool m_useCompression = false;
};