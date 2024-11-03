#include "assettreewidget.hpp"

AssetTreeWidget::AssetTreeWidget(QWidget* parent) : QTreeWidget(parent) {}

AssetTreeWidget::~AssetTreeWidget() {}

QTreeWidgetItem *AssetTreeWidget::singleSelectedItem() {
	auto items = selectedItems();
	if (items.size() > 0) { return items[0]; }
	return nullptr;
}

void AssetTreeWidget::removeItem(QTreeWidgetItem *item) {
	if (!item) { return; }

	// Remove this item
	QTreeWidgetItem* parent = item->parent();
	if (parent) {
		delete parent->takeChild(parent->indexOfChild(item));
	} else {
		delete takeTopLevelItem(indexOfTopLevelItem(item));
	}
}

void AssetTreeWidget::mousePressEvent(QMouseEvent *event) {
	QModelIndex item = indexAt(event->pos());
	bool selected = selectionModel()->isSelected(indexAt(event->pos()));
	QTreeView::mousePressEvent(event);
	if ((item.row() == -1 && item.column() == -1) || selected) {
		clearSelection();
		const QModelIndex index;
		selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
	}
}