#pragma once
#include "common.hpp"
#include <QWidget>
#include <QMainWindow>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QTabWidget>
#include <QScrollArea>
#include <QMessageBox>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:
	QWidget* Widget_Main;
	QVBoxLayout* Layout_Main;
	QMenu* Menu_File;
	QMenu* Menu_Help;
};