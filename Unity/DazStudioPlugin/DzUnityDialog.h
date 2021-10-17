#pragma once
#include "dzbasicdialog.h"
#include <QtGui/qcombobox.h>
#include <QtCore/qsettings.h>

class QPushButton;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QWidget;

class UnofficialDzUnityDialog : public DzBasicDialog {
	Q_OBJECT
	Q_PROPERTY(QWidget* assetNameEdit READ getAssetNameEdit)
	Q_PROPERTY(QWidget* assetsFolderEdit READ getAssetsFolderEdit)
	Q_PROPERTY(QWidget* assetsTypeCombo READ getAssetTypeCombo)
	Q_PROPERTY(QWidget* morphsEnabledCheckBox READ getMorphsEnabledCheckBox)
	Q_PROPERTY(QWidget* subdivisionEnabledCheckBox READ getSubdivisionEnabledCheckBox)
	Q_PROPERTY(QWidget* showFbxDialogCheckBox READ getShowFbxDialogCheckBox)

public:
	Q_INVOKABLE void addButton(QAbstractButton* btn, int index = 1) { DzBasicDialog::addButton(btn, index); }
	Q_INVOKABLE void addLayout(QLayout* layout, int stretch = 0) { DzBasicDialog::addLayout(layout, stretch); }
	Q_INVOKABLE void addSpacing(int spacing) { DzBasicDialog::addSpacing(spacing); }
	Q_INVOKABLE void addStretch(int stretch=0) { DzBasicDialog::addStretch(stretch); }
	Q_INVOKABLE void addWidget(QWidget* child, int stretch=0, Qt::Alignment alignment=0) { DzBasicDialog::addWidget(child, stretch, alignment); }

	QLineEdit* getAssetNameEdit() { return assetNameEdit; }
	QLineEdit* getAssetsFolderEdit() { return assetsFolderEdit; }
	QComboBox* getAssetTypeCombo() { return assetTypeCombo; }
	QCheckBox* getMorphsEnabledCheckBox() { return morphsEnabledCheckBox; }
	QCheckBox* getSubdivisionEnabledCheckBox() { return subdivisionEnabledCheckBox; }
	QCheckBox* getShowFbxDialogCheckBox() { return showFbxDialogCheckBox; }



	/** Constructor **/
	 UnofficialDzUnityDialog(QWidget *parent=nullptr);

	/** Destructor **/
	virtual ~UnofficialDzUnityDialog() {}

	QLineEdit* assetNameEdit;
	QLineEdit* projectEdit;
	QPushButton* projectButton;
	QComboBox* assetTypeCombo;
	QLineEdit* assetsFolderEdit;
	QPushButton* assetsFolderButton;
	QPushButton* morphsButton;
	QCheckBox* morphsEnabledCheckBox;
	QPushButton* subdivisionButton;
	QCheckBox* subdivisionEnabledCheckBox;
	QGroupBox* advancedSettingsGroupBox;
#ifdef FBXOPTIONS
	QCheckBox* showFbxDialogCheckBox;
#endif
	QLabel* installOrOverwriteUnityFilesLabel;
	QCheckBox* installUnityFilesCheckBox;

	// Pass so the DazTRoUnityAction can access it from the morph dialog
	Q_INVOKABLE QString GetMorphString();

	// Pass so the DazTRoUnityAction can access it from the morph dialog
	Q_INVOKABLE QMap<QString,QString> GetMorphMapping() { return morphMapping; }

	void Accepted();
private slots:
	void HandleSelectAssetsFolderButton();
	void HandleChooseMorphsButton();
	void HandleMorphsCheckBoxChange(int state);
	void HandleChooseSubdivisionsButton();
	void HandleSubdivisionCheckBoxChange(int state);
#ifdef FBXOPTIONS
	void HandleShowFbxDialogCheckBoxChange(int state);
#endif
	void HandleInstallUnityFilesCheckBoxChange(int state);
	void HandleAssetTypeComboChange(int state);

private:
	QSettings* settings;

	// These are clumsy leftovers from before the dialog were singletons
	QString morphString;
	QMap<QString,QString> morphMapping;
};
