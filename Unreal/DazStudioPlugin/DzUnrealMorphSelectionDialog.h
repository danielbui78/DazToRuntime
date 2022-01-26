#pragma once
#include "dzbasicdialog.h"
#include <QtGui/qlineedit.h>
#include <QtGui/qcombobox.h>
#include <QtGui/qcheckbox.h>
#include <QtCore/qsettings.h>
#include "dznode.h"

class QListWidget;
class QListWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;
class QLineEdit;
class QComboBox;
class QCheckBox;

struct MorphInfo {
	 QString Name;
	 QString Label;
	 QString Type;
	 QString Path;

	 inline bool operator==(MorphInfo other)
	 {
		  if (Name == other.Name)
		  {
				return true;
		  }
		  return false;
	 }

	 MorphInfo()
	 {
		  Name = QString();
		  Label = QString();
		  Type = QString();
		  Path = QString();
	 }
};

struct JointLinkKey
{
	int Angle;
	int Value;
};

struct JointLinkInfo
{
	QString Bone;
	QString Axis;
	QString Morph;
	double Scalar;
	double Alpha;
	QList<JointLinkKey> Keys;
};

class DzUnrealMorphSelectionDialog : public DzBasicDialog {
	 Q_OBJECT
public:

	 DzUnrealMorphSelectionDialog(QWidget* parent=nullptr);
	 virtual ~DzUnrealMorphSelectionDialog() {}

	 // Setup the dialog
	 Q_INVOKABLE void PrepareDialog();

	 // Singleton access
	 Q_INVOKABLE static DzUnrealMorphSelectionDialog* Get(QWidget* Parent)
	 {
		  if (singleton == nullptr)
		  {
				singleton = new DzUnrealMorphSelectionDialog(Parent);
		  }
		  singleton->PrepareDialog();
		  return singleton;
	 }

	 // Get the morph string in the format for the Daz FBX Export
	 Q_INVOKABLE QString GetMorphString();

	 // Get the morph string in the format used for presets
	 Q_INVOKABLE QString GetMorphCSVString();

	 // Get the morph string in an internal name = friendly name format
	 // Used to rename them to the friendly name in Unreal
	 Q_INVOKABLE QMap<QString, QString> GetMorphRenaming();

	 Q_INVOKABLE bool IsAutoJCMEnabled() { return autoJCMCheckBox->isChecked(); }

	 // Recursive function for finding all active JCM morphs for a node
	 Q_INVOKABLE QList<JointLinkInfo> GetActiveJointControlledMorphs(DzNode* Node = nullptr);

	 // Retrieve label based on morph name
	 // DB Dec-21-2021, Created for scripting.
	 Q_INVOKABLE QString GetMorphLabelFromName(QString morphName);

public slots:
	 void FilterChanged(const QString& filter);
	 void ItemSelectionChanged();
	 void HandleAddMorphsButton();
	 void HandleRemoveMorphsButton();
	 void HandleSavePreset();
	 void HandlePresetChanged(const QString& presetName);
	 void HandleArmJCMMorphsButton();
	 void HandleLegJCMMorphsButton();
	 void HandleTorsoJCMMorphsButton();
	 void HandleARKitGenesis81MorphsButton();
	 void HandleFaceFXGenesis8Button();
	 void HandleAutoJCMCheckBoxChange(bool checked);

private:

	 // Refresh the list of possible presets from disk
	 void RefreshPresetsCombo();

	 // Recursive function for finding all morphs for a node
	 QStringList GetAvailableMorphs(DzNode* Node);

	 //
	 QList<JointLinkInfo> GetJointControlledMorphInfo(DzProperty* property);

	 void UpdateMorphsTree();

	 // Returns the tree node for the morph name (with path)
	 // Builds out the structure of the tree as needed.
	 QTreeWidgetItem* FindTreeItem(QTreeWidgetItem* parent, QString name);

	 void SavePresetFile(QString filePath);

	 // Updates selectedInTree to have all the morphs for the nodes
	 // selected in the left tree
	 void SelectMorphsInNode(QTreeWidgetItem* item);

	 // Rebuild the right box that lists all the morphs that will export.
	 void RefreshExportMorphList();

	 // Morphs currently selected in the left tree box
	 QList<MorphInfo> selectedInTree;

	 // List of morphs moved to the export box
	 QList<MorphInfo> morphsToExport;

	 // Store off the presetsFolder path at dialog setup
	 QString presetsFolder;

	 static DzUnrealMorphSelectionDialog* singleton;

	 // A list of all found morphs.
	 QStringList morphList;

	 // Mapping of morph name to label
	 QMap<QString, QString> morphNameMapping;

	 // Mapping of morph name to MorphInfo
	 //TODO: morphNameMapping (and others) should be replaced by this
	 QMap<QString, MorphInfo> morphs;

	 // List of morphs (recursive) under each tree node
	 // For convenience populating the middle box.
	 QMap<QTreeWidgetItem*, QList<MorphInfo>> morphsForNode;

	 // Force the size of the dialog
	 QSize minimumSizeHint() const override;

	 // Widgets the dialog will access after construction
	 QListWidget* morphListWidget;
	 QListWidget* morphExportListWidget;
	 QTreeWidget* morphTreeWidget;
	 QLineEdit* filterEdit;
	 QComboBox* presetCombo;

	 QTreeWidgetItem* fullBodyMorphTreeItem;
	 QTreeWidgetItem* charactersTreeItem;
	 QCheckBox* autoJCMCheckBox;

	 QSettings* settings;
};
