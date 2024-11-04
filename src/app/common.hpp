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

struct AssetInfo {
	QString title = "";
	QString type = "";
	QString filename = "";
};

struct AssetTypeDescriptor {
	QString code = "";
	QStringList extensions = {};
};

extern const QList<QPair<QString, AssetTypeDescriptor>> assetTypes;

extern const QStringList cipherMethods;

class AssetTreeNode {
public:
	AssetTreeNode();
	AssetTreeNode(const AssetInfo& assetInfo);
	~AssetTreeNode();

	AssetTreeNode* addChild();
	AssetTreeNode* addChild(const AssetInfo& assetInfo);
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

struct ProjectFileInfo {
	QString m_filename = "";
	AssetTreeNode* m_assetTree = nullptr;
	QString m_cipherMethod = "";
	bool m_useCompression = false;
};