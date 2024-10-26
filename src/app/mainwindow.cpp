#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent)
{
	// Set window properties
	setWindowTitle("Deimos");

	// Main container
	Widget_Main = new QWidget();
	Layout_Main = new QVBoxLayout(Widget_Main);
	setCentralWidget(Widget_Main);

	// Menu Bar
	Menu_File = menuBar()->addMenu("&File");
	Menu_Help = menuBar()->addMenu("&Help");
}

MainWindow::~MainWindow() {
	delete Menu_Help;
	delete Menu_File;
	delete Layout_Main;
	delete Widget_Main;
}