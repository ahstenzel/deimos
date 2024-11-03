#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent)
{
	// Set window properties
	setWindowTitle("Deimos");

	// Main container
	Widget_Main = new QWidget();
	Layout_Main = new QVBoxLayout();
	Widget_Main->setLayout(Layout_Main);
	setCentralWidget(Widget_Main);

	// Menu Bar
	Menu_File = menuBar()->addMenu("&File");
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

void MainWindow::onMenu_Help_About() {
	QMessageBox::information(this, "About", QString("DEIMOS\nVersion (%0)").arg(DEIMOS_VERSION_STR));
}