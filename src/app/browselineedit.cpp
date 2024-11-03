#include "browselineedit.hpp"

BrowseLineEdit::BrowseLineEdit(QWidget *parent) : BrowseLineEdit("", parent) {}

BrowseLineEdit::BrowseLineEdit(const QString &contents, QWidget *parent) :
	QGroupBox(parent) {
	setFlat(true);
	setStyleSheet("BrowseLineEdit {border:0;}");
	Layout_Internal = new QHBoxLayout();
	setLayout(Layout_Internal);
	LineEdit_Internal = new QLineEdit(contents);
	PushButton_Internal = new QPushButton("...");
	Layout_Internal->addWidget(LineEdit_Internal);
	Layout_Internal->addWidget(PushButton_Internal);
	Layout_Internal->setSpacing(0);
	Layout_Internal->setContentsMargins(0, 0, 0, 0);
}

BrowseLineEdit::~BrowseLineEdit() {
	delete Layout_Internal;
}

QLineEdit *BrowseLineEdit::getLineEdit() {
	return LineEdit_Internal;
}

QPushButton *BrowseLineEdit::getPushButton() {
	return PushButton_Internal;
}

QString BrowseLineEdit::text() {
	return LineEdit_Internal->text();
}

void BrowseLineEdit::setText(const QString &text) {
	LineEdit_Internal->setText(text);
}
