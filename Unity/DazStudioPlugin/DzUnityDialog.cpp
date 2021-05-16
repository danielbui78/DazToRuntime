#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <QtGui/QToolTip>
#include <QtGui/QWhatsThis>
#include <QtGui/qlineedit.h>
#include <QtGui/qboxlayout.h>
#include <QtGui/qfiledialog.h>
#include <QtCore/qsettings.h>
#include <QtGui/qformlayout.h>
#include <QtGui/qcombobox.h>
#include <QtGui/qdesktopservices.h>
#include <QtGui/qcheckbox.h>
#include <QtGui/qlistwidget.h>
#include <QtGui/qgroupbox.h>

#include "dzapp.h"
#include "dzscene.h"
#include "dzstyle.h"
#include "dzmainwindow.h"
#include "dzactionmgr.h"
#include "dzaction.h"
#include "dzskeleton.h"

#include "DzUnityDialog.h"
#include "DzUnityMorphSelectionDialog.h"
#include "DzUnitySubdivisionDialog.h"

/*****************************
Local definitions
*****************************/
#define DAZ_TO_UNITY_PLUGIN_NAME "UnofficialDazToUnity"


UnofficialDzUnityDialog::UnofficialDzUnityDialog(QWidget* parent) :
	 DzBasicDialog(parent, DAZ_TO_UNITY_PLUGIN_NAME)
{
	 assetNameEdit = NULL;
	 projectEdit = NULL;
	 projectButton = NULL;
	 assetTypeCombo = NULL;
	 assetsFolderEdit = NULL;
	 assetsFolderButton = NULL;
	 morphsButton = NULL;
	 morphsEnabledCheckBox = NULL;
	 subdivisionButton = NULL;
	 subdivisionEnabledCheckBox = NULL;
	 advancedSettingsGroupBox = NULL;
#ifdef FBXOPTIONS
	 showFbxDialogCheckBox = NULL;
#endif
	 installUnityFilesCheckBox = NULL;

	 settings = new QSettings("Daz 3D", "UnofficialDazToUnity");

	 // Declarations
	 int margin = style()->pixelMetric(DZ_PM_GeneralMargin);
	 int wgtHeight = style()->pixelMetric(DZ_PM_ButtonHeight);
	 int btnMinWidth = style()->pixelMetric(DZ_PM_ButtonMinWidth);

	 // Set the dialog title
	 setWindowTitle(tr("Daz To Unity"));

	 QFormLayout* mainLayout = new QFormLayout(this);
	 QFormLayout* advancedLayout = new QFormLayout(this);

	 // Asset Name
	 assetNameEdit = new QLineEdit(this);
	 assetNameEdit->setValidator(new QRegExpValidator(QRegExp("[A-Za-z0-9_]*"), this));

	 // Intermediate Folder
	 QHBoxLayout* assetsFolderLayout = new QHBoxLayout(this);
	 assetsFolderEdit = new QLineEdit(this);
	 assetsFolderButton = new QPushButton("...", this);
	 connect(assetsFolderButton, SIGNAL(released()), this, SLOT(HandleSelectAssetsFolderButton()));

	 assetsFolderLayout->addWidget(assetsFolderEdit);
	 assetsFolderLayout->addWidget(assetsFolderButton);

	 // Asset Transfer Type
	 assetTypeCombo = new QComboBox(this);
	 assetTypeCombo->addItem("Skeletal Mesh");
	 assetTypeCombo->addItem("Static Mesh");
	 assetTypeCombo->addItem("Animation");
	 //assetTypeCombo->addItem("Pose");
	 connect(assetTypeCombo, SIGNAL(activated(int)), this, SLOT(HandleAssetTypeComboChange(int)));

	 // Morphs
	 QHBoxLayout* morphsLayout = new QHBoxLayout(this);
	 morphsButton = new QPushButton("Choose Morphs", this);
	 connect(morphsButton, SIGNAL(released()), this, SLOT(HandleChooseMorphsButton()));
	 morphsEnabledCheckBox = new QCheckBox("", this);
	 morphsEnabledCheckBox->setMaximumWidth(25);
	 morphsLayout->addWidget(morphsEnabledCheckBox);
	 morphsLayout->addWidget(morphsButton);
	 connect(morphsEnabledCheckBox, SIGNAL(stateChanged(int)), this, SLOT(HandleMorphsCheckBoxChange(int)));

	 // Subdivision
	 QHBoxLayout* subdivisionLayout = new QHBoxLayout(this);
	 subdivisionButton = new QPushButton("Choose Subdivisions", this);
	 connect(subdivisionButton, SIGNAL(released()), this, SLOT(HandleChooseSubdivisionsButton()));
	 subdivisionEnabledCheckBox = new QCheckBox("", this);
	 subdivisionEnabledCheckBox->setMaximumWidth(25);
	 subdivisionLayout->addWidget(subdivisionEnabledCheckBox);
	 subdivisionLayout->addWidget(subdivisionButton);
	 connect(subdivisionEnabledCheckBox, SIGNAL(stateChanged(int)), this, SLOT(HandleSubdivisionCheckBoxChange(int)));

	 // Show FBX Dialog option
	 installUnityFilesCheckBox = new QCheckBox("", this);

	 // Add the widget to the basic dialog
	 mainLayout->addRow("Asset Name", assetNameEdit);
	 mainLayout->addRow("Asset Type", assetTypeCombo);
	 mainLayout->addRow("Enable Morphs", morphsLayout);
	 mainLayout->addRow("Enable Subdivision", subdivisionLayout);
	 mainLayout->addRow("Unity Assets Folder", assetsFolderLayout);
	 mainLayout->addRow("Install Unity Files", installUnityFilesCheckBox);
	 connect(installUnityFilesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(HandleInstallUnityFilesCheckBoxChange(int)));
	 addLayout(mainLayout);
#ifdef FBXOPTIONS
	 showFbxDialogCheckBox = new QCheckBox("", this);
	 advancedLayout->addRow("Show FBX Dialog", showFbxDialogCheckBox);

	 // Advanced
	 advancedSettingsGroupBox = new QGroupBox("Advanced Settings", this);
	 advancedSettingsGroupBox->setLayout(advancedLayout);
	 addWidget(advancedSettingsGroupBox);
#endif

	 // Make the dialog fit its contents, with a minimum width, and lock it down
	 resize(QSize(500, 0).expandedTo(minimumSizeHint()));
	 setFixedWidth(width());
	 setFixedHeight(height());

	 // Help
	 assetNameEdit->setWhatsThis("This is the name the asset will use in Unity.");
	 assetTypeCombo->setWhatsThis("Skeletal Mesh for something with moving parts, like a character\nStatic Mesh for things like props\nAnimation for a character animation.");
	 assetsFolderEdit->setWhatsThis("Unity Assets folder. DazToUnity will collect the assets in a subfolder under this folder.");
	 assetsFolderButton->setWhatsThis("Unity Assets folder. DazToUnity will collect the assets in a subfolder under this folder.");

	 // Load Settings
	 if (!settings->value("AssetsPath").isNull())
	 {
		 // DB (2021-05-15): check AssetsPath folder and set InstallUnityFiles if Daz3D subfolder does not exist
		 QString directoryName = settings->value("AssetsPath").toString();
		  assetsFolderEdit->setText(directoryName);
		  if (QDir(directoryName + QDir::separator() + "Daz3D").exists())
		  {
			  // deselect install unity files
			  settings->setValue("InstallUnityFiles", false);
			  installUnityFilesCheckBox->setChecked(false);
		  }
		  else
		  {
			  settings->setValue("InstallUnityFiles", true);
			  installUnityFilesCheckBox->setChecked(true);
		  }

	 }
	 else
	 {
		  QString DefaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QDir::separator() + "UnofficialDazToUnity";
		  assetsFolderEdit->setText(DefaultPath);
	 }
	 if (!settings->value("MorphsEnabled").isNull())
	 {
		  morphsEnabledCheckBox->setChecked(settings->value("MorphsEnabled").toBool());
	 }
	 if (!settings->value("SubdivisionEnabled").isNull())
	 {
		  subdivisionEnabledCheckBox->setChecked(settings->value("SubdivisionEnabled").toBool());
	 }
#ifdef FBXOPTIONS
	 if (!settings->value("ShowFBXDialog").isNull())
	 {
		  showFbxDialogCheckBox->setChecked(settings->value("ShowFBXDialog").toBool());
	 }
#endif
	 // DB (2021-05-15): installUnityFiles is now handled by existence of "Assets/Daz3D" subfolder
	 //if (!settings->value("InstallUnityFiles").isNull())
	 //{
		//  installUnityFilesCheckBox->setChecked(settings->value("InstallUnityFiles").toBool());
	 //}
	 //else
		//  installUnityFilesCheckBox->setChecked(true);

	 // Set Defaults
	 DzNode* Selection = dzScene->getPrimarySelection();
	 if (dzScene->getFilename().length() > 0)
	 {
		  QFileInfo fileInfo = QFileInfo(dzScene->getFilename());
		  assetNameEdit->setText(fileInfo.baseName().remove(QRegExp("[^A-Za-z0-9_]")));
	 }
	 else if (dzScene->getPrimarySelection())
	 {
		  assetNameEdit->setText(Selection->getLabel().remove(QRegExp("[^A-Za-z0-9_]")));
	 }

	 if (qobject_cast<DzSkeleton*>(Selection))
	 {
		  assetTypeCombo->setCurrentIndex(0);
	 }
	 else
	 {
		  assetTypeCombo->setCurrentIndex(1);
	 }
}

void UnofficialDzUnityDialog::HandleSelectAssetsFolderButton()
{
	 // DB (2021-05-15): prepopulate with existing folder string
	 QString directoryName = "/home";
	 if (!settings->value("AssetsPath").isNull())
	 {
		 directoryName = settings->value("AssetsPath").toString();
	 }
	 directoryName = QFileDialog::getExistingDirectory(this, tr("Choose Directory"),
		  directoryName,
		  QFileDialog::ShowDirsOnly
		  | QFileDialog::DontResolveSymlinks);

	 if (directoryName != NULL)
	 {
		  QDir parentDir = QFileInfo(directoryName).dir();
		  if (!parentDir.exists())
		  {
				QMessageBox::warning(0, tr("Error"), tr("Please select Unity Root Assets Folder."), QMessageBox::Ok);
				return;
		  }
		  else
		  {
				bool found1 = false;
				bool found2 = false;
				QFileInfoList list = parentDir.entryInfoList(QDir::NoDot | QDir::NoDotDot | QDir::Dirs);
				for (int i = 0; i < list.size(); i++)
				{
					 QFileInfo file = list[i];
					 if (file.baseName() == QString("ProjectSettings"))
						  found1 = true;
					 if (file.baseName() == QString("Library"))
						  found2 = true;
				}

				if (!found1 || !found2)
				{
					 QMessageBox::warning(0, tr("Error"), tr("Please select Unity Root Assets Folder."), QMessageBox::Ok);
					 return;
				}

				// DB (2021-05-15): Check for presence of Daz3D folder, and set installUnityFiles if not present
				if ( QDir(directoryName + QDir::separator() + "Daz3D").exists() )
				{
					// deselect install unity files
					settings->setValue("InstallUnityFiles", false);
					installUnityFilesCheckBox->setChecked(false);
				}
				else
				{
					settings->setValue("InstallUnityFiles", true);
					installUnityFilesCheckBox->setChecked(true);
				}

				assetsFolderEdit->setText(directoryName);
				settings->setValue("AssetsPath", directoryName);
		  }
	 }
}

void UnofficialDzUnityDialog::HandleChooseMorphsButton()
{
	 DzUnityMorphSelectionDialog* dlg = DzUnityMorphSelectionDialog::Get(this);
	 dlg->exec();
	 morphString = dlg->GetMorphString();
	 morphMapping = dlg->GetMorphRenaming();
}

void UnofficialDzUnityDialog::HandleChooseSubdivisionsButton()
{
	 DzUnitySubdivisionDialog* dlg = DzUnitySubdivisionDialog::Get(this);
	 dlg->exec();
}

QString UnofficialDzUnityDialog::GetMorphString()
{
	 morphMapping = DzUnityMorphSelectionDialog::Get(this)->GetMorphRenaming();
	 return DzUnityMorphSelectionDialog::Get(this)->GetMorphString();
}

void UnofficialDzUnityDialog::HandleMorphsCheckBoxChange(int state)
{
	 settings->setValue("MorphsEnabled", state == Qt::Checked);
}

void UnofficialDzUnityDialog::HandleSubdivisionCheckBoxChange(int state)
{
	 settings->setValue("SubdivisionEnabled", state == Qt::Checked);
}

#ifdef FBXOPTIONS
void UnofficialDzUnityDialog::HandleShowFbxDialogCheckBoxChange(int state)
{
	 settings->setValue("ShowFBXDialog", state == Qt::Checked);
}
#endif

void UnofficialDzUnityDialog::HandleInstallUnityFilesCheckBoxChange(int state)
{
	 settings->setValue("InstallUnityFiles", state == Qt::Checked);
}

void UnofficialDzUnityDialog::HandleAssetTypeComboChange(int state)
{
	QString assetNameString = assetNameEdit->text();

	// enable/disable Morphs and Subdivision only if Skeletal selected
	if (assetTypeCombo->currentText() != "Skeletal Mesh")
	{
		morphsEnabledCheckBox->setChecked(false);
		subdivisionEnabledCheckBox->setChecked(false);
	}

	// if "Animation", change assetname
	if (assetTypeCombo->currentText() == "Animation")
	{
		// check assetname is in @anim[0000] format
		if ( !assetNameString.contains("@") || assetNameString.contains(QRegExp("@anim[0-9]*") ) )
		{
			// extract true assetName and recompose animString
			assetNameString = assetNameString.left(assetNameString.indexOf("@"));
			// get importfolder using corrected assetNameString
			QString importFolderPath = settings->value("AssetsPath").toString() + QDir::separator() + "Daz3D" + QDir::separator() + assetNameString + QDir::separator();

			// create anim filepath
			uint animCounter = 0;
			QString animString = assetNameString + QString("@anim%1").arg(animCounter, 4, 10, QChar('0'));
			QString filePath = importFolderPath + animString + ".fbx";

			// if anim file exists, then increment anim filename counter
			while (QFileInfo(filePath).exists())
			{
				if (++animCounter > 9999)
				{
					break;
				}
				animString = assetNameString + QString("@anim%1").arg(animCounter, 4, 10, QChar('0'));
				filePath = importFolderPath + animString + ".fbx";
			}
			assetNameEdit->setText(animString);
		}

	}
	else
	{
		// remove @anim if present
		if (assetNameString.contains("@")) {
			assetNameString = assetNameString.left(assetNameString.indexOf("@"));
		}
		assetNameEdit->setText(assetNameString);
	}

}

#include "moc_DzUnityDialog.cpp"
