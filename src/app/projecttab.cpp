#include "projecttab.hpp"

bool itemIsGroup(QTreeWidgetItem* item) {
	return (item && item->text(1).isEmpty());
}

NoEditDelegate::NoEditDelegate(QObject* parent) : 
	QStyledItemDelegate(parent) {}

QWidget* NoEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	return nullptr;
}

ProjectTabPage::ProjectTabPage(QWidget* parent, const QString& filename, ProjectTabWidget* tabs) :
	QWidget(parent), m_projectFile(nullptr), m_dirty(false), m_assetDialog(nullptr), m_assetDialogEditItem(nullptr), TabWidget_Parent(tabs) {
	m_assetFlags = 
		Qt::ItemFlag::ItemIsSelectable | 
		Qt::ItemFlag::ItemIsDragEnabled |
		Qt::ItemFlag::ItemIsEnabled;
	m_groupFlags = 
		Qt::ItemFlag::ItemIsSelectable | 
		Qt::ItemFlag::ItemIsEditable |
		Qt::ItemFlag::ItemIsDragEnabled |
		Qt::ItemFlag::ItemIsDropEnabled |
		Qt::ItemFlag::ItemIsEnabled;
	
	// Top level container widget
	Layout_Main = new QVBoxLayout();
	setLayout(Layout_Main);
	StackedWidget_Main = new QStackedWidget();
	Widget_EmptyTab = new QWidget();
	Widget_PopulatedTab = new QWidget();
	StackedWidget_Main->addWidget(Widget_EmptyTab);
	StackedWidget_Main->addWidget(Widget_PopulatedTab);
	Layout_Main->addWidget(StackedWidget_Main);

	// Empty tab widgets
	Layout_EmptyTab = new QVBoxLayout();
	Widget_EmptyTab->setLayout(Layout_EmptyTab);
	PushButton_CreateProjectFile = new QPushButton("Create Project File");
	connect(PushButton_CreateProjectFile, &QPushButton::clicked, this, &ProjectTabPage::onClicked_PushButton_CreateProjectFile);
	PushButton_OpenProjectFile = new QPushButton("Open Project File");
	connect(PushButton_OpenProjectFile, &QPushButton::clicked, this, &ProjectTabPage::onClicked_PushButton_OpenProjectFile);
	Layout_EmptyTab->addStretch(1);
	Layout_EmptyTab->addWidget(PushButton_CreateProjectFile, false, Qt::AlignCenter);
	Layout_EmptyTab->addWidget(PushButton_OpenProjectFile, false, Qt::AlignCenter);
	Layout_EmptyTab->addStretch(1);

	// Populated tab widgets
	Layout_PopulatedTab = new QVBoxLayout();
	Widget_PopulatedTab->setLayout(Layout_PopulatedTab);
	GroupBox_ProjectOptions = new QGroupBox();
	Layout_ProjectOptions = new QFormLayout();
	GroupBox_ProjectOptions->setLayout(Layout_ProjectOptions);
	Layout_PopulatedTab->addWidget(GroupBox_ProjectOptions);

	// Project options
	CheckBox_UseCompression = new QCheckBox();
	Layout_ProjectOptions->addRow("Compress Assets", CheckBox_UseCompression);
	ComboBox_CipherMethod = new QComboBox();
	ComboBox_CipherMethod->setEditable(false);
	ComboBox_CipherMethod->addItems(cipherMethods);
	Layout_ProjectOptions->addRow("Encryption Method", ComboBox_CipherMethod);
	connect(CheckBox_UseCompression, &QCheckBox::checkStateChanged, this, &ProjectTabPage::onCheckStateChanged_CheckBox_UseCompression);
	connect(ComboBox_CipherMethod, &QComboBox::currentIndexChanged, this, &ProjectTabPage::onCurrentIndexChanged_ComboBox_CipherMethod);

	// Asset tree
	TreeWidget_Assets = new AssetTreeWidget();
	TreeWidget_Assets->setColumnCount(3);
	TreeWidget_Assets->setHeaderLabels({"Title", "Type", "Location"});
	TreeWidget_Assets->setContextMenuPolicy(Qt::CustomContextMenu);
	TreeWidget_Assets->setDragEnabled(true);
	TreeWidget_Assets->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
	TreeWidget_Assets->setExpandsOnDoubleClick(false);
	// Workaround to make only the first column editable
	// Because I dont feel like implementing a whole QTreeView
	TreeWidget_Assets->setItemDelegateForColumn(1, new NoEditDelegate(this));
	TreeWidget_Assets->setItemDelegateForColumn(2, new NoEditDelegate(this));
	connect(TreeWidget_Assets, &AssetTreeWidget::customContextMenuRequested, this, &ProjectTabPage::assetTree_PrepareContextMenu);
	connect(TreeWidget_Assets, &AssetTreeWidget::itemDoubleClicked, this, &ProjectTabPage::onDoubleClicked_AssetTreeItem);
	Layout_PopulatedTab->addWidget(TreeWidget_Assets);

	// Asset buttons
	GroupBox_AssetButtons = new QGroupBox();
	GroupBox_AssetButtons->setFlat(true);
	GroupBox_AssetButtons->setObjectName("GroupBox_AssetButtons");
	GroupBox_AssetButtons->setStyleSheet("QGroupBox#GroupBox_AssetButtons {border:0;}");
	Layout_AssetButtons = new QHBoxLayout();
	GroupBox_AssetButtons->setLayout(Layout_AssetButtons);
	Layout_PopulatedTab->addWidget(GroupBox_AssetButtons);
	PushButton_AddFile = new QPushButton("Add Asset");
	PushButton_AddDirectory = new QPushButton("Add Group");
	Layout_AssetButtons->addWidget(PushButton_AddFile);
	Layout_AssetButtons->addWidget(PushButton_AddDirectory);
	Layout_AssetButtons->addStretch(1);
	connect(PushButton_AddFile, &QPushButton::clicked, this, &ProjectTabPage::assetTree_AddFile);
	connect(PushButton_AddDirectory, &QPushButton::clicked, this, &ProjectTabPage::assetTree_AddDirectory);

	// Set layout
	if (filename.isEmpty()) {
		StackedWidget_Main->setCurrentIndex(0);
	} else {
		ProjectFileInfo info;
		info.m_filename = filename;
		if (!ProjectFile::readFile(&info)) {
			QMessageBox::warning(this, "Error", "Failed to open project file!");
		} else { openFile(info); }
	}
}

ProjectTabPage::~ProjectTabPage() {
	delete m_projectFile;
	delete m_assetDialog;
	delete Layout_AssetButtons;
	delete Layout_ProjectOptions;
	delete Layout_EmptyTab;
	delete Layout_PopulatedTab;
	delete Layout_Main;
}

bool ProjectTabPage::isEmpty() {
	return (m_projectFile == nullptr);
}

bool ProjectTabPage::isDirty() {
	return m_dirty;
}

void ProjectTabPage::setDirty(bool dirty) {
	m_dirty = dirty;
	updateTitle();
}

void ProjectTabPage::openFile(const ProjectFileInfo &projectFileInfo) {
	// Update widgets
	setProjectFilename(projectFileInfo.m_filename);
	CheckBox_UseCompression->setChecked(projectFileInfo.m_useCompression);
	ComboBox_CipherMethod->setCurrentText(projectFileInfo.m_cipherMethod);

	// Add top level items to the list
	typedef QPair<AssetTreeNode*, QTreeWidgetItem*> AssetItemPair;
	QList<AssetItemPair> assetList;
	if (projectFileInfo.m_assetTree) {
		for(auto child : *projectFileInfo.m_assetTree) {
			assetList.push_back(AssetItemPair::pair(child, nullptr));
		}
	}

	// Iterate through list, adding children to asset tree
	while(!assetList.isEmpty()) {
		// Extract asset & item references
		AssetItemPair pair = assetList.front();
		assetList.pop_front();
		AssetTreeNode* assetCurrent = pair.first;
		QTreeWidgetItem* itemCurrent = pair.second;

		// Create new tree item
		QTreeWidgetItem* itemNew = new QTreeWidgetItem();
		if (assetCurrent->isGroup()) {
			itemNew->setFlags(m_groupFlags);

			// Add children to list
			for(auto child : *assetCurrent) {
				assetList.push_back(AssetItemPair::pair(child, itemNew));
			}
		} else { itemNew->setFlags(m_assetFlags); }
		assetInfoToItem(assetCurrent->assetInfo(), itemNew);

		// Add to tree
		if (itemCurrent) { itemCurrent->addChild(itemNew); } 
		else { TreeWidget_Assets->addTopLevelItem(itemNew); }
	}

	// Switch to populated page
	StackedWidget_Main->setCurrentIndex(1);
	setDirty(false);
}

void ProjectTabPage::saveFile(ProjectFileInfo* projectFileInfo) {
	if (!projectFileInfo) { return; }

	// Fill in generic project info
	projectFileInfo->m_filename = projectFilename();
	projectFileInfo->m_cipherMethod = ComboBox_CipherMethod->currentText();
	projectFileInfo->m_useCompression = CheckBox_UseCompression->isChecked();
	if (projectFileInfo->m_assetTree) { delete projectFileInfo->m_assetTree; }
	projectFileInfo->m_assetTree = new AssetTreeNode();

	// Add top level widget items to list
	typedef QPair<AssetTreeNode*, QTreeWidgetItem*> AssetItemPair;
	QStack<AssetItemPair> assetList;
	for(int i = 0; i < TreeWidget_Assets->topLevelItemCount(); ++i) {
		assetList.push(AssetItemPair::pair(projectFileInfo->m_assetTree, TreeWidget_Assets->topLevelItem(i)));
	}

	// Iterate through list, adding children to file tree
	while(!assetList.isEmpty()) {
		// Extract parent/widget item pair
		AssetItemPair pair = assetList.front();
		assetList.pop_front();
		AssetTreeNode* assetCurrent = pair.first;
		QTreeWidgetItem* itemCurrent = pair.second;

		// Construct asset info & add to asset tree
		AssetInfo assetCurrentInfo;
		assetItemToInfo(itemCurrent, &assetCurrentInfo);
		AssetTreeNode* assetNew = assetCurrent->addChild(assetCurrentInfo);

		// Add items children to the list
		for(int i = 0; i < itemCurrent->childCount(); ++i) {
			QTreeWidgetItem* child = itemCurrent->child(i);
			assetList.push(AssetItemPair::pair(assetNew, child));
		}
	}
}

void ProjectTabPage::updateTitle() {
	if (TabWidget_Parent) {
		int idx = TabWidget_Parent->indexOf(this);
		if (idx != -1) {
			QString title = m_projectFile->baseName();
			if (m_dirty) { title += "*"; }
			TabWidget_Parent->setTabText(idx, title);
		}
	}
}

void ProjectTabPage::onClicked_PushButton_CreateProjectFile() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save File"), ".", tr("Mars Resource Project (*.mrp)"));
	if (!filename.isEmpty()) { 
		// Create default file
		if (!ProjectFile::createFile(filename)) { return; }

		// Load newly created file
		ProjectFileInfo info;
		info.m_filename = filename;
		if (!ProjectFile::readFile(&info)) {
			QMessageBox::warning(this, "Error", "Failed to open project file!");
		} else { openFile(info); }
	}
}

void ProjectTabPage::onClicked_PushButton_OpenProjectFile() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("Mars Resource Project (*.mrp)"));
	if (!filename.isEmpty()) { 
		ProjectFileInfo info;
		info.m_filename = filename;
		if (!ProjectFile::readFile(&info)) {
			QMessageBox::warning(this, "Error", "Failed to open project file!");
		} else { openFile(info); }
	}
}

void ProjectTabPage::onCheckStateChanged_CheckBox_UseCompression(int state) {
	setDirty(true);
}

void ProjectTabPage::onCurrentIndexChanged_ComboBox_CipherMethod(int index) {
	setDirty(true);
}

void ProjectTabPage::onFinished_AssetDialog(int status) {
	if (status == QDialog::Accepted && m_assetDialog) {
		AssetInfo info = m_assetDialog->getValue();
		
		// Check if we're editing an item or creating a new one
		QTreeWidgetItem* item = (m_assetDialogEditItem) ? m_assetDialogEditItem : new QTreeWidgetItem();
		m_assetDialogEditItem = nullptr;
		
		// Set item properties
		item->setFlags(m_assetFlags);
		assetInfoToItem(info, item);
		addItem(item, false);
	}
	delete m_assetDialog;
	m_assetDialog = nullptr;
}

void ProjectTabPage::onDoubleClicked_AssetTreeItem(QTreeWidgetItem *item, int column) {
	if (!itemIsGroup(item)) { assetTree_EditItem(); }
}

void ProjectTabPage::assetTree_PrepareContextMenu(const QPoint &pos) {
	QTreeWidgetItem* item = TreeWidget_Assets->itemAt(pos);
	QMenu contextMenu(this);
	QAction* actionAddGroup = new QAction("Add Group");
	QAction* actionAddAsset = new QAction("Add Asset");
	connect(actionAddGroup, &QAction::triggered, this, &ProjectTabPage::assetTree_AddDirectory);
	connect(actionAddAsset, &QAction::triggered, this, &ProjectTabPage::assetTree_AddFile);
	contextMenu.addAction(actionAddGroup);
	contextMenu.addAction(actionAddAsset);
	
	if (item) {
		if (itemIsGroup(item)) {
			// Right clicked group
			QAction* actionDeleteGroup = new QAction("Delete Group");
			connect(actionDeleteGroup, &QAction::triggered, this, &ProjectTabPage::assetTree_DeleteDirectory);
			contextMenu.addAction(actionDeleteGroup);
		} else {
			// Right clicked asset
			QAction* actionDeleteAsset = new QAction("Delete Asset");
			QAction* actionEditAsset = new QAction("Edit Asset");
			connect(actionDeleteAsset, &QAction::triggered, this, &ProjectTabPage::assetTree_DeleteFile);
			connect(actionEditAsset, &QAction::triggered, this, &ProjectTabPage::assetTree_EditItem);
			contextMenu.addAction(actionDeleteAsset);
			contextMenu.addAction(actionEditAsset);
		}
	}
	
	contextMenu.exec(TreeWidget_Assets->mapToGlobal(QPoint(pos)));
}

void ProjectTabPage::assetTree_AddFile() {
	m_assetDialog = new AssetDialog(this);
	m_assetDialog->open();
	connect(m_assetDialog, &AssetDialog::finished, this, &ProjectTabPage::onFinished_AssetDialog);
	return;
}

void ProjectTabPage::assetTree_AddDirectory() {
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setFlags(m_groupFlags);
	item->setText(0, "New Group");
	addItem(item, true);
}

void ProjectTabPage::assetTree_DeleteFile() {
	QTreeWidgetItem* item = TreeWidget_Assets->singleSelectedItem();
	if (item) {
		TreeWidget_Assets->removeItem(item);
		setDirty(true);
	}
}

void ProjectTabPage::assetTree_DeleteDirectory() {
	QTreeWidgetItem* item = TreeWidget_Assets->singleSelectedItem();
	if (item) {
		if (item->childCount() == 0) {
			TreeWidget_Assets->removeItem(item);
			setDirty(true);
		} else {
			auto result = QMessageBox::question(this, "Delete Group", 
				"Group is not empty! Delete everything in it?", QMessageBox::Yes | QMessageBox::No);
			if (result == QMessageBox::Yes) {
				TreeWidget_Assets->removeItem(item);
				setDirty(true);
			}
		}
	}
}

void ProjectTabPage::assetTree_EditItem() {
	QTreeWidgetItem *item = TreeWidget_Assets->singleSelectedItem();
	if (item) {
		// Flag item as being edited
		m_assetDialogEditItem = item;

		// Create asset dialog
		m_assetDialog = new AssetDialog(this);
		AssetInfo info;
		assetItemToInfo(item, &info);
		m_assetDialog->setValue(info);
		m_assetDialog->open();
		connect(m_assetDialog, &AssetDialog::finished, this, &ProjectTabPage::onFinished_AssetDialog);
	}
}

void ProjectTabPage::addItem(QTreeWidgetItem* item, bool edit) {
	// Check for parent item
	QTreeWidgetItem* parentItem = nullptr;
	QTreeWidgetItem* selectedItem = TreeWidget_Assets->singleSelectedItem();
	if (itemIsGroup(selectedItem)) {
		parentItem = selectedItem;
	} else if (selectedItem) {
		parentItem = selectedItem->parent();
	}

	// Either add to existing item or top level
	if (parentItem) {
		parentItem->addChild(item);
	} else {
		TreeWidget_Assets->addTopLevelItem(item);
	}
	TreeWidget_Assets->setCurrentItem(item, 0);
	if (edit) { TreeWidget_Assets->editItem(item, 0); }
	setDirty(true);
}

void ProjectTabPage::assetInfoToItem(const AssetInfo& info, QTreeWidgetItem* item) {
	item->setText(0, info.title);
	item->setText(1, info.type);
	item->setText(2, info.filename);
}

void ProjectTabPage::assetItemToInfo(QTreeWidgetItem* item, AssetInfo* info) {
	info->title = item->text(0);
	info->type = item->text(1);
	info->filename = item->text(2);
}

QString ProjectTabPage::projectFilename() {
	return m_projectFile->absoluteFilePath();
}

void ProjectTabPage::setProjectFilename(const QString &filename) {
	QFileInfo* newProjectFile = new QFileInfo(filename);
	if (m_projectFile) { delete m_projectFile; }
	m_projectFile = newProjectFile;
	updateTitle();
}

ProjectTabWidget::ProjectTabWidget(QWidget *parent) :
	QTabWidget(parent) {
	// Add close button
	setTabsClosable(true);

	// Add new tab button
	QIcon newWindowIcon = QIcon::fromTheme(QIcon::ThemeIcon::WindowNew);
	toolButton_NewTab = new QToolButton();
	toolButton_NewTab->setIcon(newWindowIcon);
	setCornerWidget(toolButton_NewTab);
	connect(this, &QTabWidget::tabCloseRequested, this, &ProjectTabWidget::onTabCloseRequested_ProjectTabPage);
	connect(toolButton_NewTab, &QToolButton::clicked, this, &ProjectTabWidget::onClicked_ToolButton_NewTab);

	// Add empty tab
	addTab();
}

ProjectTabWidget::~ProjectTabWidget() {
	delete toolButton_NewTab;
}

int ProjectTabWidget::addTab(const QString& filename) {
	int idx = QTabWidget::addTab(new ProjectTabPage(nullptr, filename, this), filename.isEmpty() ? "<New Project>" : filename);
	setCurrentIndex(idx);
	return idx;
}

ProjectTabPage* ProjectTabWidget::currentPage() {
	return static_cast<ProjectTabPage*>(currentWidget());
}

void ProjectTabWidget::onTabCloseRequested_ProjectTabPage(int index) {
	removeTab(index);
	if (count() == 0) { addTab(); }
}

void ProjectTabWidget::onClicked_ToolButton_NewTab() {
	addTab();
}