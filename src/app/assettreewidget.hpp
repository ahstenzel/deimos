#pragma once
/**
 * assettreewidget.hpp
 * 
 * Custom tree view for organizing assets.
 */
#include "common.hpp"

class AssetTreeWidget : public QTreeWidget {
	Q_OBJECT
public:
	AssetTreeWidget(QWidget* parent = nullptr);
	~AssetTreeWidget();

	QTreeWidgetItem* singleSelectedItem();

	void removeItem(QTreeWidgetItem* item);

	virtual void mousePressEvent(QMouseEvent *event);
};