#pragma once
/**
 * projecttab.hpp
 * 
 * Tab interface for project files.
 */
#include "common.hpp"
#include "projectfile.hpp"
#include "assetdialog.hpp"
#include "assettreewidget.hpp"

class NoEditDelegate: public QStyledItemDelegate {
public:
	NoEditDelegate(QObject* parent = nullptr): QStyledItemDelegate(parent) {}
	virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
		return nullptr;
	}
};

bool itemIsGroup(QTreeWidgetItem* item);

class ProjectTabWidget : public QTabWidget {
	Q_OBJECT
public:
	explicit ProjectTabWidget(QWidget* parent = nullptr);
	~ProjectTabWidget();
	int addTab(const QString& filename = "");

private:
	QToolButton* toolButton_NewTab;

public slots:
	void onTabCloseRequested_ProjectTabPage(int index);
	void onClicked_ToolButton_NewTab();
};

class ProjectTabPage : public QWidget {
	Q_OBJECT
public:
	explicit ProjectTabPage(QWidget* parent = nullptr, const QString& filename = "", ProjectTabWidget* tabs = nullptr);
	~ProjectTabPage();
	bool isEmpty();

	bool getDirty();
	void setDirty(bool dirty);

	void openFile(const QString& filename);
	void closeFile();

private:
	ProjectTabWidget* TabWidget_Parent;
	QStackedWidget* StackedWidget_Main;
	QWidget* Widget_EmptyTab;
	QWidget* Widget_PopulatedTab;
	QVBoxLayout* Layout_Main;
	QVBoxLayout* Layout_EmptyTab;
	QVBoxLayout* Layout_PopulatedTab;
	QGroupBox* GroupBox_ProjectOptions;
	QFormLayout* Layout_ProjectOptions;
	QPushButton* PushButton_CreateProjectFile;
	QPushButton* PushButton_OpenProjectFile;
	QCheckBox* CheckBox_UseCompression;
	QComboBox* ComboBox_CipherMethod;
	AssetTreeWidget* TreeWidget_Assets;
	QGroupBox* GroupBox_AssetButtons;
	QHBoxLayout* Layout_AssetButtons;
	QPushButton* PushButton_AddFile;
	QPushButton* PushButton_AddDirectory;
	AssetDialog* m_assetDialog;
	QFileInfo* m_projectFile;
	QTreeWidgetItem* m_assetDialogEditItem;
	bool m_dirty;

	void updateTitle();
	void addItem(QTreeWidgetItem* item, bool edit);

public slots:
	void onClicked_PushButton_CreateProjectFile();
	void onClicked_PushButton_OpenProjectFile();
	void onCheckStateChanged_CheckBox_UseCompression(int state);
	void onCurrentIndexChanged_ComboBox_CipherMethod(int index);
	void onFinished_AssetDialog(int status);
	void onDoubleClicked_AssetTreeItem(QTreeWidgetItem *item, int column);

	void assetTree_PrepareContextMenu(const QPoint& pos);
	void assetTree_AddFile();
	void assetTree_AddDirectory();
	void assetTree_DeleteFile();
	void assetTree_DeleteDirectory();
	void assetTree_EditItem();
};