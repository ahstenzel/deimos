#pragma once
/**
 * browselineedit.hpp
 * 
 * Simple wrapper for a line edit plus an attached browse button.
 */
#include "common.hpp"

class BrowseLineEdit : public QGroupBox {
	Q_OBJECT
public:
	BrowseLineEdit(QWidget* parent = nullptr);
	BrowseLineEdit(const QString &contents, QWidget* parent = nullptr);
	~BrowseLineEdit();

	QLineEdit* getLineEdit();
	QPushButton* getPushButton();
	QString text();
	void setText(const QString& text);

private:
	QHBoxLayout* Layout_Internal;
	QLineEdit* LineEdit_Internal;
	QPushButton* PushButton_Internal;
};