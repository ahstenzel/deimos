#pragma once
/**
 * assetdialog.hpp
 * 
 * Modal dialog for adding a new asset to the tree.
 */
#include "common.hpp"
#include "browselineedit.hpp"

struct assetInfo {
	QString title = "";
	QString type = "";
	QString filename = "";
};

struct assetTypeDescriptor {
	QString code = "";
	QStringList extensions = {};
};

extern const QList<QPair<QString, assetTypeDescriptor>> assetTypes;

class AssetDialog : public QDialog {
	Q_OBJECT;
public:
	AssetDialog(QWidget* parent = nullptr);
	~AssetDialog();

	static assetInfo getAssetInfo();
	assetInfo getValue();
	void setValue(const assetInfo& info);

private:
	QFormLayout* Layout_Main;
	QLineEdit* LineEdit_Title;
	QLabel* Label_Type;
	BrowseLineEdit* LineEdit_Filename;
	QDialogButtonBox* ButtonBox_Main;
	QRegularExpressionValidator* Validator_Title;

public slots:
	void onClicked_Filename_Browse();
	void onChanged_LineEdit_Filename();
	void onChanged_ValidateAssets();
};