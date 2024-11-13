#pragma once
/**
 * mainwindow.hpp
 * 
 * Main window class.
 */
#include "common.hpp"
#include "projecttab.hpp"

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

public slots:
	void onMenu_File_NewProject();
	void onMenu_File_OpenProject();
	void onMenu_File_SaveProject();
	void onMenu_File_SaveAsProject();
	void onMenu_File_ExportProject();
	void onMenu_Help_About();

private:
	QMenu* Menu_File;
	QMenu* Menu_Help;
	QWidget* Widget_Main;
	QVBoxLayout* Layout_Main;
	ProjectTabWidget* TabWidget_Main;
	QToolButton* ToolButton_NewTab;
};