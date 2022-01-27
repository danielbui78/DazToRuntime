#pragma once
#include "dzbasicdialog.h"
#include <QtGui/qcombobox.h>
#include <QtCore/qsettings.h>

class QPushButton;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QGroupBox;

class DzUnrealDialog : public DzBasicDialog {
	Q_OBJECT
	Q_PROPERTY(QWidget* wAssetNameEdit READ getAssetNameEdit)
	Q_PROPERTY(QWidget* wIntermediateFolderEdit READ getIntermediateFolderEdit)
	Q_PROPERTY(QWidget* wAssetTypeCombo READ getAssetTypeCombo)
	Q_PROPERTY(QWidget* wMorphsEnabledCheckBox READ getMorphsEnabledCheckBox)
	Q_PROPERTY(QWidget* wSubdivisionEnabledCheckBox READ getSubdivisionEnabledCheckBox)
	Q_PROPERTY(QWidget* wAdvancedSettingsGroupBox READ getAdvancedSettingsGroupBox)
	Q_PROPERTY(QWidget* wFbxVersionCombo READ getFbxVersionCombo)
	Q_PROPERTY(QWidget* wShowFbxDialogCheckBox READ getShowFbxDialogCheckBox)
	Q_PROPERTY(QWidget* wExportMaterialPropertyCSVCheckBox READ getExportMaterialPropertyCSVCheckBox)
public:
	QLineEdit* getAssetNameEdit() { return assetNameEdit; }
	QLineEdit* getIntermediateFolderEdit() { return intermediateFolderEdit; }
	QComboBox* getAssetTypeCombo() { return assetTypeCombo; }
	QCheckBox* getMorphsEnabledCheckBox() { return morphsEnabledCheckBox; }
	QCheckBox* getSubdivisionEnabledCheckBox() { return subdivisionEnabledCheckBox; }
	QGroupBox* getAdvancedSettingsGroupBox() { return advancedSettingsGroupBox; }
	QComboBox* getFbxVersionCombo() { return fbxVersionCombo; }
	QCheckBox* getShowFbxDialogCheckBox() { return showFbxDialogCheckBox; }
	QCheckBox* getExportMaterialPropertyCSVCheckBox() { return exportMaterialPropertyCSVCheckBox; }

	/** Constructor **/
	 DzUnrealDialog(QWidget *parent=nullptr);

	/** Destructor **/
	virtual ~DzUnrealDialog() {}

	QLineEdit* assetNameEdit;
//	QLineEdit* projectEdit;
//	QPushButton* projectButton;
	QComboBox* assetTypeCombo;
	QLineEdit* portEdit;
	QLineEdit* intermediateFolderEdit;
	QPushButton* intermediateFolderButton;
	QPushButton* morphsButton;
	QCheckBox* morphsEnabledCheckBox;
	QPushButton* subdivisionButton;
	QCheckBox* subdivisionEnabledCheckBox;
	QGroupBox* advancedSettingsGroupBox;
	QWidget* advancedWidget;
	QComboBox* fbxVersionCombo;
	QCheckBox* showFbxDialogCheckBox;
	QCheckBox* exportMaterialPropertyCSVCheckBox;

	// Pass so the DazTRoUnrealAction can access it from the morph dialog
	Q_INVOKABLE QString GetMorphString();

	// Pass so the DazTRoUnrealAction can access it from the morph dialog
	Q_INVOKABLE QMap<QString,QString> GetMorphMapping() { return morphMapping; }

	Q_INVOKABLE void resetToDefaults();

	void Accepted();
private slots:
	void HandleSelectIntermediateFolderButton();
	void HandlePortChanged(const QString& port);
	void HandleChooseMorphsButton();
	void HandleMorphsCheckBoxChange(int state);
	void HandleChooseSubdivisionsButton();
	void HandleSubdivisionCheckBoxChange(int state);
	void HandleFBXVersionChange(const QString& fbxVersion);
	void HandleShowFbxDialogCheckBoxChange(int state);
	void HandleExportMaterialPropertyCSVCheckBoxChange(int state);
	void HandleShowAdvancedSettingsCheckBoxChange(bool checked);

private:
	QSettings* settings;

	// These are clumsy leftovers from before the dialog were singletons
	QString morphString;
	QMap<QString,QString> morphMapping;
};
