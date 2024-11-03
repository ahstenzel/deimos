#include "projecttab.hpp"

bool itemIsGroup(QTreeWidgetItem* item) {
	return (item && item->text(1).isEmpty());
}

ProjectTabPage::ProjectTabPage(QWidget* parent, const QString& filename, ProjectTabWidget* tabs) :
	QWidget(parent), m_projectFile(nullptr), m_dirty(false), m_assetDialog(nullptr), m_assetDialogEditItem(nullptr), TabWidget_Parent(tabs) {
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
		openFile(filename);
	}
}

ProjectTabPage::~ProjectTabPage() {
	closeFile(); 
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

bool ProjectTabPage::getDirty() {
	return m_dirty;
}

void ProjectTabPage::setDirty(bool dirty) {
	m_dirty = dirty;
	updateTitle();
}

void ProjectTabPage::openFile(const QString &filename) {
	// Verify file
	if (!QFile::exists(filename)) {
		QMessageBox::warning(this, "Error", QString("Failed to open (%0)!").arg(filename));
		return;
	}

	// Close existing project & open new one
	QFileInfo* newProjectFile = new QFileInfo(filename);
	if (m_projectFile) { closeFile(); }
	m_projectFile = newProjectFile;

	// Update widgets
	ProjectFileInfo info = ProjectFile::loadFile(filename);
	CheckBox_UseCompression->setChecked(info.m_useCompression);
	ComboBox_CipherMethod->setCurrentText(info.m_cipherMethod);

	// Switch to populated page
	StackedWidget_Main->setCurrentIndex(1);
	setDirty(false);
}

void ProjectTabPage::closeFile() {
	delete m_projectFile;
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
		ProjectFile::createFile(filename);
		openFile(filename); 
	}
}

void ProjectTabPage::onClicked_PushButton_OpenProjectFile() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("Mars Resource Project (*.mrp)"));
	if (!filename.isEmpty()) { openFile(filename); }
}

void ProjectTabPage::onCheckStateChanged_CheckBox_UseCompression(int state) {
	setDirty(true);
}

void ProjectTabPage::onCurrentIndexChanged_ComboBox_CipherMethod(int index) {
	setDirty(true);
}

void ProjectTabPage::onFinished_AssetDialog(int status) {
	if (status == QDialog::Accepted && m_assetDialog) {
		assetInfo info = m_assetDialog->getValue();
		
		// Check if we're editing an item or creating a new one
		QTreeWidgetItem* item = (m_assetDialogEditItem) ? m_assetDialogEditItem : new QTreeWidgetItem();
		m_assetDialogEditItem = nullptr;
		
		// Set item properties
		item->setFlags(
			Qt::ItemFlag::ItemIsSelectable | 
			Qt::ItemFlag::ItemIsDragEnabled |
			//Qt::ItemFlag::ItemIsDropEnabled |
			Qt::ItemFlag::ItemIsEnabled
		);
		item->setText(0, info.title);
		item->setText(1, info.type);
		item->setText(2, info.filename);
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
	item->setFlags(
		Qt::ItemFlag::ItemIsSelectable | 
		Qt::ItemFlag::ItemIsEditable |
		Qt::ItemFlag::ItemIsDragEnabled |
		Qt::ItemFlag::ItemIsDropEnabled |
		Qt::ItemFlag::ItemIsEnabled
	);
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
		assetInfo info;
		info.title = item->text(0);
		info.type = item->text(1);
		info.filename = item->text(2);
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
	return QTabWidget::addTab(new ProjectTabPage(nullptr, filename, this), filename.isEmpty() ? "<New Project>" : filename);
}

void ProjectTabWidget::onTabCloseRequested_ProjectTabPage(int index) {
	removeTab(index);
	if (count() == 0) { addTab(); }
}

void ProjectTabWidget::onClicked_ToolButton_NewTab() {
	addTab();
}