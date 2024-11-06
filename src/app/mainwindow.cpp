#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent) {
	// Set window properties
	setWindowTitle("Deimos");
	setWindowIcon(QIcon(":/images/mars.ico"));

	// Main container
	Widget_Main = new QWidget();
	Layout_Main = new QVBoxLayout();
	Widget_Main->setLayout(Layout_Main);
	setCentralWidget(Widget_Main);

	// Menu Bar
	Menu_File = menuBar()->addMenu("&File");
	QAction* Action_File_NewProject = new QAction("&New Project");
	QAction* Action_File_OpenProject = new QAction("&Open Project");
	QAction* Action_File_SaveProject = new QAction("&Save Project");
	QAction* Action_File_SaveAsProject = new QAction("Save Project As");
	Action_File_NewProject->setShortcut(Qt::Key_N | Qt::CTRL);
	Action_File_OpenProject->setShortcut(Qt::Key_O | Qt::CTRL);
	Action_File_SaveProject->setShortcut(Qt::Key_S | Qt::CTRL);
	Action_File_SaveAsProject->setShortcut(Qt::Key_S | Qt::SHIFT | Qt::CTRL);
	connect(Action_File_NewProject, &QAction::triggered, this, &MainWindow::onMenu_File_NewProject);
	connect(Action_File_OpenProject, &QAction::triggered, this, &MainWindow::onMenu_File_OpenProject);
	connect(Action_File_SaveProject, &QAction::triggered, this, &MainWindow::onMenu_File_SaveProject);
	connect(Action_File_SaveAsProject, &QAction::triggered, this, &MainWindow::onMenu_File_SaveAsProject);
	Menu_File->addAction(Action_File_NewProject);
	Menu_File->addAction(Action_File_OpenProject);
	Menu_File->addAction(Action_File_SaveProject);
	Menu_File->addAction(Action_File_SaveAsProject);
	Menu_Help = menuBar()->addMenu("&Help");
	QAction* Action_Help_About = new QAction("About");
	connect(Action_Help_About, &QAction::triggered, this, &MainWindow::onMenu_Help_About);
	Menu_Help->addAction(Action_Help_About);

	// Tabs
	TabWidget_Main = new ProjectTabWidget();
	Layout_Main->addWidget(TabWidget_Main);
}

MainWindow::~MainWindow() {
	delete Layout_Main;
}

void MainWindow::onMenu_File_NewProject() {
	// If the current tab is empty, overwrite it
	ProjectTabPage* page = TabWidget_Main->currentPage();
	if (!page->isEmpty()) {
		TabWidget_Main->onClicked_ToolButton_NewTab();
		page = TabWidget_Main->currentPage();
	}
	page->onClicked_PushButton_CreateProjectFile();
}

void MainWindow::onMenu_File_OpenProject() {
	// If the current tab is empty, overwrite it
	ProjectTabPage* page = TabWidget_Main->currentPage();
	if (!page->isEmpty()) {
		TabWidget_Main->onClicked_ToolButton_NewTab();
		page = TabWidget_Main->currentPage();
	}
	page->onClicked_PushButton_OpenProjectFile();
}

void MainWindow::onMenu_File_SaveProject() {
	ProjectTabPage* page = TabWidget_Main->currentPage();
	ProjectFileInfo info;
	page->saveFile(&info);
	if (!ProjectFile::writeFile(info)) {
		QMessageBox::warning(this, "Error", "Failed to save project file!");
	} else { page->setDirty(false); }
}

void MainWindow::onMenu_File_SaveAsProject() {
	ProjectTabPage* page = TabWidget_Main->currentPage();
	QString newFilename = QFileDialog::getSaveFileName(this, tr("Save File"), ".", tr("Mars Resource Project (*.mrp)"));
	if (!newFilename.isEmpty()) {
		page->setProjectFilename(newFilename);
		onMenu_File_SaveProject();
	}
}

void MainWindow::onMenu_Help_About() {
	QMessageBox::information(this, "About", QString("DEIMOS\nVersion (%0)").arg(DEIMOS_VERSION_STR));
}