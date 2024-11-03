#include "assetdialog.hpp"

const QList<QPair<QString, assetTypeDescriptor>> assetTypes = {
	{"Image", {
		"IMG_",
		{"png", "bmp", "tiff"}
	}},
	{"Text", {
		"TXT_",
		{"txt", "json", "ini", "xml", "hlsl"}
	}},
	{"Binary", {
		"BIN_",
		{"bin", "spv"}
	}},
};

AssetDialog::AssetDialog(QWidget *parent) :
	QDialog(parent) {
	Layout_Main = new QFormLayout();
	setLayout(Layout_Main);
	setWindowTitle("Add Asset");

	LineEdit_Filename = new BrowseLineEdit();
	connect(LineEdit_Filename->getLineEdit(), &QLineEdit::textChanged, this, &AssetDialog::onChanged_LineEdit_Filename);
	connect(LineEdit_Filename->getPushButton(), &QPushButton::clicked, this, &AssetDialog::onClicked_Filename_Browse);
	Layout_Main->addRow("File", LineEdit_Filename);

	LineEdit_Title = new QLineEdit();
	QRegularExpression regex_title("^[\\w\\-_. ]+${0,31}");
	Validator_Title = new QRegularExpressionValidator(regex_title);
	LineEdit_Title->setValidator(Validator_Title);
	connect(LineEdit_Title, &QLineEdit::textChanged, this, &AssetDialog::onChanged_ValidateAssets);
	Layout_Main->addRow("Title", LineEdit_Title);

	Label_Type = new QLabel("");
	Layout_Main->addRow("Type", Label_Type);

	ButtonBox_Main = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	ButtonBox_Main->button(QDialogButtonBox::Ok)->setDisabled(true);
	connect(ButtonBox_Main, &QDialogButtonBox::accepted, this, &AssetDialog::accept);
	connect(ButtonBox_Main, &QDialogButtonBox::rejected, this, &AssetDialog::reject);
	Layout_Main->addWidget(ButtonBox_Main);
}

AssetDialog::~AssetDialog() {
	delete Validator_Title;
	delete Layout_Main;
}

assetInfo AssetDialog::getAssetInfo() {
	assetInfo info;
	AssetDialog dialog;
	if (dialog.exec() == QDialog::DialogCode::Accepted) {
		info = dialog.getValue();
	}
	return info;
}

assetInfo AssetDialog::getValue() {
	assetInfo info;
	info.title = LineEdit_Title->text();
	info.type = Label_Type->text();
	info.filename = LineEdit_Filename->text();
	return info;
}

void AssetDialog::setValue(const assetInfo &info) {
	LineEdit_Title->setText(info.title);
	Label_Type->setText(info.type);
	LineEdit_Filename->setText(info.filename);
}

void AssetDialog::onChanged_LineEdit_Filename() {
	// Get filename extension
	QString filename = LineEdit_Filename->text();
	QFileInfo fileInfo(filename);
	QString fileExtension = fileInfo.completeSuffix();
	if (filename.isEmpty() || fileExtension.isEmpty()) {
		Label_Type->setText("");
		onChanged_ValidateAssets();
		return;
	} else if (LineEdit_Title->text().isEmpty()) {
		LineEdit_Title->setText(fileInfo.baseName());
	}

	// Compare to known file types
	for(auto type : assetTypes) {
		for(auto extension : type.second.extensions) {
			if (extension == fileExtension) {
				Label_Type->setText(type.first);
				onChanged_ValidateAssets();
				return;
			}
		}
	}
	Label_Type->setText(assetTypes.last().first);
	onChanged_ValidateAssets();
}

void AssetDialog::onChanged_ValidateAssets() {
	ButtonBox_Main->button(QDialogButtonBox::Ok)->setEnabled(
		!LineEdit_Title->text().isEmpty() &&
		!Label_Type->text().isEmpty() &&
		!LineEdit_Filename->text().isEmpty()
	);
}

void AssetDialog::onClicked_Filename_Browse() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), ".", "");
	if (!filename.isEmpty()) {
		LineEdit_Filename->setText(filename);
	}
}
