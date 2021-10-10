#include <QtGui/qcheckbox.h>
#include <QtGui/QMessageBox>
#include <QtNetwork/qudpsocket.h>
#include <QtNetwork/qabstractsocket.h>
#include <QCryptographicHash>
#include <QtCore/qdir.h>

#include <dzapp.h>
#include <dzscene.h>
#include <dzmainwindow.h>
#include <dzshape.h>
#include <dzproperty.h>
#include <dzobject.h>
#include <dzpresentation.h>
#include <dznumericproperty.h>
#include <dzimageproperty.h>
#include <dzcolorproperty.h>
#include <dpcimages.h>

#include "QtCore/qmetaobject.h"
#include "dzmodifier.h"
#include "dzgeometry.h"
#include "dzweightmap.h"
#include "dzfacetshape.h"
#include "dzfacetmesh.h"
#include "dzfacegroup.h"

#include "DzUnityDialog.h"
#include "DzUnityAction.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SUBDIVISION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __APPLE__
    #define USING_LIBSTDCPP 1
#endif
#include "FbxSdkManager.h"
#include "SubdivideMesh.h"


bool UpgradeToHD(QString baseFilePath, QString hdFilePath, QString outFilePath, int SubDLevel)
{
	FbxManager* lSdkManager = NULL;
	FbxScene* baseMeshScene = NULL;
	bool lResult;

	// Prepare the FBX SDK and load base mesh scene
	InitializeSdkObjects(lSdkManager, baseMeshScene);
	QByteArray base_ba = baseFilePath.toLocal8Bit();
	lResult = LoadScene(lSdkManager, baseMeshScene, base_ba.data(), false);
	if (lResult == false)
	{
		QMessageBox::warning(0, "Error",
			"An error occurred while loading the base scene...", QMessageBox::Ok);
		printf("\n\nAn error occurred while loading the base scene...");
		return false;
	}

	// subdivide
	lResult = ProcessScene(baseMeshScene, SubDLevel);

	// load HD mesh scene
	FbxScene* hdMeshScene = FbxScene::Create(lSdkManager, "HD Mesh Scene");
	QByteArray HD_ba = hdFilePath.toLocal8Bit();
	lResult = LoadScene(lSdkManager, hdMeshScene, HD_ba.data(), false);
	if (lResult == false)
	{
		QMessageBox::warning(0, "Error",
			"An error occurred while loading the HD scene...", QMessageBox::Ok);

		printf("\n\nAn error occurred while loading the HD scene...");
		return false;
	}

	// save clusters to the scene object
	lResult = SaveClustersToScene(hdMeshScene);

	//DisplayString("Saving the output mesh FBX file:  ", outFilePath);
	int fileFormat = lSdkManager->GetIOPluginRegistry()->GetNativeWriterFormat(); // binary file format
	QByteArray final_ba = outFilePath.toLocal8Bit();
	lResult = SaveScene(lSdkManager, hdMeshScene, final_ba.data(), fileFormat);
	if (lResult == false)
	{
		QMessageBox::warning(0, "Error",
			"An error occurred while saving the scene...", QMessageBox::Ok);

		printf("\n\nAn error occurred while saving the scene...");
		return false;
	}

	// Destroy all objects created by the FBX SDK.
	DestroySdkObjects(lSdkManager, lResult);

	return true;
}

//bool UpgradeToHD(std::string fbxFilePath)
//{
//	std::string baseFilePath(fbxFilePath);
//	int pos = baseFilePath.find(".fbx");
//	if (pos <= 0)
//	{
//		printf("FBX filepath is invalid (extension must be .fbx)");
//		return false;
//	}
//
//	int len = strlen("_base.fbx");
//	baseFilePath.replace(pos, len, "_base.fbx");
//
//	std::string  hdFilePath(fbxFilePath);
//	len = strlen("_HD.fbx");
//	hdFilePath.replace(pos, len, "_HD.fbx");
//
//	if (UpgradeToHD(baseFilePath.c_str(), hdFilePath.c_str(), fbxFilePath.c_str(), 0) == false)
//		return false;
//
//	return true;
//}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SUBDIVISION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UnofficialDzUnityAction::UnofficialDzUnityAction() :
	 UnofficialDzRuntimePluginAction(tr("&Unofficial DTU (Daz To Unity)"), tr("Send the selected node to Unity."))
{
	 SubdivisionDialog = nullptr;
	 QAction::setIcon(QIcon(":/UnofficialDaz/Images/icon"));
}

void UnofficialDzUnityAction::executeAction()
{
	 // Check if the main window has been created yet.
	 // If it hasn't, alert the user and exit early.
	 DzMainWindow* mw = dzApp->getInterface();
	 if (!mw)
	 {
		  QMessageBox::warning(0, tr("Error"),
				tr("The main window has not been created yet."), QMessageBox::Ok);

		  return;
	 }

	 // Create and show the dialog. If the user cancels, exit early,
	 // otherwise continue on and do the thing that required modal
	 // input from the user.

	 if (dzScene->getNumSelectedNodes() != 1)
	 {
		  QMessageBox::warning(0, tr("Error"),
				tr("Please select one Character or Prop to send."), QMessageBox::Ok);
		  return;
	 }

	 // Create the dialog
	 UnofficialDzUnityDialog* dlg = new UnofficialDzUnityDialog(mw);

	 // If the Accept button was pressed, start the export
	 if (dlg->exec() == QDialog::Accepted)
	 {
		  //Create Daz3D folder if it doesn't exist
		  QDir dir;
		  ImportFolder = dlg->assetsFolderEdit->text() + "/Daz3D";
		  dir.mkpath(ImportFolder);

		  // Collect the values from the dialog fields
		  CharacterName = dlg->assetNameEdit->text();
		  CharacterFolder = ImportFolder + "/" + CharacterName + "/";
		  CharacterFBX = CharacterFolder + CharacterName + ".fbx";
		  AssetType = dlg->assetTypeCombo->currentText().replace(" ", "");
		  MorphString = dlg->GetMorphString();
		  ExportMorphs = dlg->morphsEnabledCheckBox->isChecked();
		  ExportSubdivisions = dlg->subdivisionEnabledCheckBox->isChecked();
		  MorphMapping = dlg->GetMorphMapping();
#ifdef FBXOPTIONS
		  ShowFbxDialog = dlg->showFbxDialogCheckBox->isChecked();
#endif
		  InstallUnityFiles = dlg->installUnityFilesCheckBox->isChecked();

		  CreateUnityFiles(true);

		  SubdivisionDialog = DzUnitySubdivisionDialog::Get(dlg);
		  FBXVersion = QString("FBX 2014 -- Binary");

		  if (ExportSubdivisions)
		  {
			  SubdivisionDialog->LockSubdivisionProperties(false);
			  ExportBaseMesh = true;
			  Export();
			  SubdivisionDialog->UnlockSubdivisionProperties();
		  }
	  
		  SubdivisionDialog->LockSubdivisionProperties(ExportSubdivisions);
		  ExportBaseMesh = false;
		  Export();

		  if (ExportSubdivisions)
		  {
			  QString BaseCharacterFBX = CharacterFolder + CharacterName + "_base.fbx";
			  //QString HDCharacterFBX = CharacterFolder + CharacterName + "_HD.fbx";
			  // DB 2021-10-02: Upgrade HD
			  int SubDLevel = 2;
			  //UpgradeToHD(BaseCharacterFBX, HDCharacterFBX, CharacterFBX, SubDLevel);
			  if (UpgradeToHD(BaseCharacterFBX, CharacterFBX, CharacterFBX, SubDLevel) == false)
			  {
				  QMessageBox::warning(0, tr("Error"),
					  tr("There was an error during the Subdivision Surface refinement operation, the exported Daz model may not work correctly."), QMessageBox::Ok);
			  }
			  else
			  {
				  // remove intermediate base character fbx
				  // Sanity Check
				  if (QFile::exists(BaseCharacterFBX))
				  {
					  QFile::remove(BaseCharacterFBX);
				  }
			  }
		  }
 
		  // DB 2021-09-02: Unlock and Undo subdivision changes
		  SubdivisionDialog->UnlockSubdivisionProperties();

		  //Rename the textures folder
		  QDir textureDir(CharacterFolder + "/" + CharacterName + ".images");
		  textureDir.rename(CharacterFolder + "/" + CharacterName + ".images", CharacterFolder + "/Textures");

		  // DB 2021-09-02: messagebox "Export Complete"
		  //QMessageBox msgBox;
		  //msgBox.setText(tr("Unofficial DTU Bridge export from Daz Studio complete. Please switch to Unity to continue."));
		  //msgBox.exec();
		  QMessageBox::information(0, "Unofficial DTU Bridge", tr("Export phase from Daz Studio complete. Please switch to Unity to begin Import phase."), QMessageBox::Ok);

	 }
}

QString UnofficialDzUnityAction::GetMD5(const QString &path)
{
	auto algo = QCryptographicHash::Md5;
	QFile sourceFile(path);
    qint64 fileSize = sourceFile.size();
    const qint64 bufferSize = 10240;

    if (sourceFile.open(QIODevice::ReadOnly))
    {
        char buffer[bufferSize];
        int bytesRead;
        int readSize = qMin(fileSize, bufferSize);

        QCryptographicHash hash(algo);
        while (readSize > 0 && (bytesRead = sourceFile.read(buffer, readSize)) > 0)
        {
            fileSize -= bytesRead;
            hash.addData(buffer, bytesRead);
            readSize = qMin(fileSize, bufferSize);
        }

        sourceFile.close();
        return QString(hash.result().toHex());
    }
    return QString();
}

bool UnofficialDzUnityAction::CopyFile(QFile *file, QString *dst, bool replace, bool compareFiles)
{
	bool dstExists = QFile::exists(*dst);

	if(replace)
	{
		if(compareFiles && dstExists)
		{
			auto srcFileMD5 = GetMD5(file->fileName());
			auto dstFileMD5 = GetMD5(*dst);

			if(srcFileMD5.length() > 0 && dstFileMD5.length() > 0 && srcFileMD5.compare(dstFileMD5) == 0)
			{
				return false;
			}
		}

		if(dstExists)
		{
			QFile::remove(*dst);
		}
	}

	/*
	if(dstExists)
	{
		QFile::setPermissions(QFile::ReadOther | QFile::WriteOther);
	}
	*/

	//ensure our output destination file has the correct file permissions
	//file->setPermissions(QFile::ReadOther | QFile::WriteOther);

	auto result = file->copy(*dst);

	if(QFile::exists(*dst))
	{
#if __APPLE__
        QFile::setPermissions(*dst, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
#else
        QFile::setPermissions(*dst, QFile::ReadOther | QFile::WriteOther);
#endif
    }

	return result;
}

void UnofficialDzUnityAction::CreateUnityFiles(bool replace)
{
	 if (!InstallUnityFiles)
		  return;

	 //Create shader folder if it doesn't exist
	 QDir dir;
	 QString scriptsFolder = ImportFolder + "/Scripts";
	 dir.mkpath(scriptsFolder);

	 QStringList scripts = QDir(":/UnofficialDaz/Scripts/").entryList();
	 for (int i = 0; i < scripts.size(); i++)
	 {
		  QString script = scriptsFolder + "/" + scripts[i];
		  QFile file(":/UnofficialDaz/Scripts/" + scripts[i]);
		  CopyFile(&file, &script, replace);
		  file.close();
	 }

	 //Create editor folder if it doesn't exist
	 QString editorFolder = ImportFolder + "/Scripts/Editor";
	 dir.mkpath(editorFolder);

	 QStringList editorScripts = QDir(":/UnofficialDaz/Editor/").entryList();
	 for (int i = 0; i < editorScripts.size(); i++)
	 {
		  QString script = editorFolder + "/" + editorScripts[i];
		  QFile file(":/UnofficialDaz/Editor/" + editorScripts[i]);
		  CopyFile(&file, &script, replace);
		  file.close();
	 }

	 //Create shader folder if it doesn't exist
	 QString shaderFolder = ImportFolder + "/Shaders";
	 dir.mkpath(shaderFolder);

	 QStringList shaders = QDir(":/UnofficialDaz/Shaders/").entryList();
	 for (int i = 0; i < shaders.size(); i++)
	 {
		  QString shader = shaderFolder + "/" + shaders[i];
		  QFile file(":/UnofficialDaz/Shaders/" + shaders[i]);
		  CopyFile(&file, &shader, replace);
		  file.close();
	 }

	 //Create shader CGInc folder if it doesn't exist
	 QString shaderCGIncFolder = ImportFolder + "/Shaders/CGInc";
	 dir.mkpath(shaderCGIncFolder);

	 QStringList shaderCGIncStringList = QDir(":/UnofficialDaz/ShaderCGInc/").entryList();
	 for (int i = 0; i < shaderCGIncStringList.size(); i++)
	 {
		 QString shaderCGIncFilename = shaderCGIncFolder + "/" + shaderCGIncStringList[i];
		 QFile file(":/UnofficialDaz/ShaderCGInc/" + shaderCGIncStringList[i]);
		 CopyFile(&file, &shaderCGIncFilename, replace);
		 file.close();
	 }

	 //Create shader UnofficialDTU folder if it doesn't exist
	 QString shaderUDTUFolder = ImportFolder + "/Shaders/UnofficialDTU";
	 dir.mkpath(shaderUDTUFolder);

	 QStringList shaderUDTUStringList = QDir(":/UnofficialDaz/UnofficialDTU/").entryList();
	 for (int i = 0; i < shaderUDTUStringList.size(); i++)
	 {
		 QString shaderUDTUFilename = shaderUDTUFolder + "/" + shaderUDTUStringList[i];
		 QFile file(":/UnofficialDaz/UnofficialDTU/" + shaderUDTUStringList[i]);
		 CopyFile(&file, &shaderUDTUFilename, replace);
		 file.close();
	 }

	 //Create shader helpers folder if it doesn't exist
	 QString shaderHelperFolder = ImportFolder + "/Shaders/Helpers";
	 dir.mkpath(shaderHelperFolder);

	 QStringList shaderHelpers = QDir(":/UnofficialDaz/ShaderHelpers/").entryList();
	 for (int i = 0; i < shaderHelpers.size(); i++)
	 {
		  QString shaderHelper = shaderHelperFolder + "/" + shaderHelpers[i];
		  QFile file(":/UnofficialDaz/ShaderHelpers/" + shaderHelpers[i]);
		  CopyFile(&file, &shaderHelper, replace);
		  file.close();
	 }

	 //Create vendors folder if it doesn't exist
	 QString vendorsFolder = ImportFolder + "/Vendors";
	 dir.mkpath(vendorsFolder);

	 QStringList vendors = QDir(":/UnofficialDaz/Vendors/").entryList();
	 for (int i = 0; i < vendors.size(); i++)
	 {
		  QString vendor = vendorsFolder + "/" + vendors[i];
		  QFile file(":/UnofficialDaz/Vendors/" + vendors[i]);
		  CopyFile(&file, &vendor, replace);
		  file.close();
	 }

	 //Create DiffusionProfiles folder if it doesn't exist
	 QString profilesFolder = ImportFolder + "/DiffusionProfiles";
	 dir.mkpath(profilesFolder);

	 QStringList profiles = QDir(":/UnofficialDaz/DiffusionProfiles/").entryList();
	 for (int i = 0; i < profiles.size(); i++)
	 {
		  QString profile = profilesFolder + "/" + profiles[i];
		  QFile file(":/UnofficialDaz/DiffusionProfiles/" + profiles[i]);
		  CopyFile(&file, &profile, replace);
		  file.close();
	 }

	 //Create Resources folder if it doesn't exist
	 QString resourcesFolder = ImportFolder + "/Resources";
	 dir.mkpath(resourcesFolder);

	 QStringList resources = QDir(":/UnofficialDaz/Resources/").entryList();
	 for (int i = 0; i < resources.size(); i++)
	 {
		  QString resource = resourcesFolder + "/" + resources[i];
		  QFile file(":/UnofficialDaz/Resources/" + resources[i]);
		  CopyFile(&file, &resource, replace);
		  file.close();
	 }
}

void UnofficialDzUnityAction::WriteConfiguration()
{
	 QString DTUfilename = CharacterFolder + CharacterName + ".dtu";
	 QFile DTUfile(DTUfilename);
	 DTUfile.open(QIODevice::WriteOnly);
	 DzJsonWriter writer(&DTUfile);
	 writer.startObject(true);
	 writer.addMember("Asset Id", Selection->getAssetId());
	 writer.addMember("Asset Name", CharacterName);
	 writer.addMember("Asset Type", AssetType);
	 writer.addMember("FBX File", CharacterFBX);
	 writer.addMember("Import Folder", CharacterFolder);

	 writer.startMemberArray("Materials", true);
	 WriteMaterials(Selection, writer);
	 writer.finishArray();

	 writer.startMemberArray("Morphs", true);
	 if (ExportMorphs)
	 {
		  for (QMap<QString, QString>::iterator i = MorphMapping.begin(); i != MorphMapping.end(); ++i)
		  {
				writer.startObject(true);
				writer.addMember("Name", i.key());
				writer.addMember("Label", i.value());
				writer.finishObject();
		  }
	 }
	 writer.finishArray();

	 writer.startMemberArray("Subdivisions", true);
	 if (ExportSubdivisions)
		  SubdivisionDialog->WriteSubdivisions(writer);

	 if (AssetType.toLower().contains("skeletalmesh"))
	 {
		 bool ExportDForce = true;
		 writer.startMemberArray("dForce WeightMaps", true);
		 if (ExportDForce)
		 {
			 WriteWeightMaps(Selection, writer);
		 } 
	 }

	 writer.finishArray();
	 writer.finishObject();

	 DTUfile.close();
}

// Setup custom FBX export options
void UnofficialDzUnityAction::SetExportOptions(DzFileIOSettings& ExportOptions)
{
	 ExportOptions.setBoolValue("doSelected", true);
//	 ExportOptions.setBoolValue("doLights", true);
//	 ExportOptions.setBoolValue("doCameras", true);
//	 ExportOptions.setBoolValue("doAnims", false);

	 if (AssetType == "SkeletalMesh" && ExportMorphs && MorphString != "")
	 {
		  ExportOptions.setBoolValue("doMorphs", true);
		  ExportOptions.setStringValue("rules", MorphString);
	 }
	 else
	 {
		  ExportOptions.setBoolValue("doMorphs", false);
		  ExportOptions.setStringValue("rules", "");
	 }

	 ExportOptions.setIntValue("RunSilent", !ShowFbxDialog);

	 ExportOptions.setBoolValue("doEmbed", true);
	 ExportOptions.setBoolValue("doDiffuseOpacity", true);
	 ExportOptions.setBoolValue("doMergeClothing", true);
	 ExportOptions.setBoolValue("doCopyTextures", true);
	 ExportOptions.setBoolValue("doLocks", true);
	 ExportOptions.setBoolValue("doLimits", true);
}

// Write out all the surface properties
void UnofficialDzUnityAction::WriteMaterials(DzNode* Node, DzJsonWriter& Writer)
{
	 DzObject* Object = Node->getObject();
	 DzShape* Shape = Object ? Object->getCurrentShape() : NULL;

	 // DB (2021-05-24): dForce Additions
	 // 1. check: for dForce modifier in object
	 // 2. if dForce modifier exists, look for SimulationSettingsProvider
	 // 2a. prep script: load FindSimulationProvider script from file/resource
	 // 2b. prep arguments: pass Node object + Material string to script
	 // 2c. execute script
	 // 2d. wait for return
	 // 3. if script call successful, get returnvalue into DzElement* SimulationSettingsProvider
	 // 4. process SimulationsSettingsProvider after other Material properties


	 // 1. check: for dForce modifier in object
	 bool bDForceSettingsAvailable = false;
	 if (Shape)
	 {
		 DzModifierIterator modIter = Object->modifierIterator();
		 while (modIter.hasNext())
		 {
			 DzModifier* modifier = modIter.next();
			 QString mod_Class = modifier->className();
			 if (mod_Class.toLower().contains("dforce"))
			 {
				 bDForceSettingsAvailable = true;
				 break;
			 }
		 }
	 }

	 if (Shape)
	 {
		  for (int i = 0; i < Shape->getNumMaterials(); i++)
		  {
				DzMaterial* Material = Shape->getMaterial(i);
				if (Material)
				{
					 Writer.startObject(true);
					 Writer.addMember("Version", 2);
					 Writer.addMember("Asset Name", Node->getLabel());
					 Writer.addMember("Material Name", Material->getName());
					 Writer.addMember("Material Type", Material->getMaterialName());

					 DzPresentation* presentation = Node->getPresentation();
					 if (presentation != nullptr)
					 {
						  const QString presentationType = presentation->getType();
						  Writer.addMember("Value", presentationType);
					 }
					 else
					 {
						  Writer.addMember("Value", QString("Unknown"));
					 }

					 Writer.startMemberArray("Properties", true);
					 for (int propertyIndex = 0; propertyIndex < Material->getNumProperties(); propertyIndex++)
					 {
						  DzProperty* Property = Material->getProperty(propertyIndex);
						  DzImageProperty* ImageProperty = qobject_cast<DzImageProperty*>(Property);
						  if (ImageProperty)
						  {
								QString Name = Property->getName();
								QString TextureName = "";

								if (ImageProperty->getValue())
								{
									 TextureName = ImageProperty->getValue()->getFilename();
								}

								Writer.startObject(true);
								Writer.addMember("Name", Name);
								Writer.addMember("Value", Material->getDiffuseColor().name());
								Writer.addMember("Data Type", QString("Texture"));
								Writer.addMember("Texture", TextureName);
								Writer.finishObject();
								continue;
						  }

						  DzColorProperty* ColorProperty = qobject_cast<DzColorProperty*>(Property);
						  if (ColorProperty)
						  {
								QString Name = Property->getName();
								QString TextureName = "";

								if (ColorProperty->getMapValue())
								{
									 TextureName = ColorProperty->getMapValue()->getFilename();
								}

								Writer.startObject(true);
								Writer.addMember("Name", Name);
								Writer.addMember("Value", ColorProperty->getColorValue().name());
								Writer.addMember("Data Type", QString("Color"));
								Writer.addMember("Texture", TextureName);
								Writer.finishObject();
								continue;
						  }

						  DzNumericProperty* NumericProperty = qobject_cast<DzNumericProperty*>(Property);
						  if (NumericProperty)
						  {
								QString Name = Property->getName();
								QString TextureName = "";

								if (NumericProperty->getMapValue())
								{
									 TextureName = NumericProperty->getMapValue()->getFilename();
								}

								Writer.startObject(true);
								Writer.addMember("Name", Name);
								Writer.addMember("Value", QString::number(NumericProperty->getDoubleValue()));
								Writer.addMember("Data Type", QString("Double"));
								Writer.addMember("Texture", TextureName);
								Writer.finishObject();
						  }
					 }

					 /***********************************************************************/
					 // Add DForce Settings to Material Properties Array
					 /***********************************************************************/
					 if (bDForceSettingsAvailable)
					 {
						 // look up all Shape methods, find closest match for "findSimulationSettingsProvider"
						 //const QMetaObject *metaObj = Shape->metaObject();
						 //int searchResult = -1;
						 //for (int i = 0; i < metaObj->methodCount(); i++)
						 //{
							//const char* methodSig = metaObj->method(i).signature();
							//if (QString(methodSig).toLower().contains("findsimulationsettingsprovider"))
							//{
							//	searchResult = i;
							//	break;
							//}
						 //}
						 DzElement* elSimulationSettingsProvider;
						 bool ret = false;
						 int methodIndex = -1;
						 methodIndex = Shape->metaObject()->indexOfMethod(QMetaObject::normalizedSignature("findSimulationSettingsProvider(QString)"));
						 if (methodIndex != -1)
						 {
							 QMetaMethod method = Shape->metaObject()->method(methodIndex);
							 QGenericReturnArgument returnArgument(
								 method.typeName(),
								 &elSimulationSettingsProvider
							 );
							 ret = method.invoke(Shape, returnArgument, Q_ARG(QString, Material->getName()));
							 if (elSimulationSettingsProvider)
							 {
								int numProperties = elSimulationSettingsProvider->getNumProperties();
								DzPropertyListIterator propIter = elSimulationSettingsProvider->propertyListIterator();
								QString propString = "";
								int propIndex = 0;
								while (propIter.hasNext())
								{
									DzProperty* Property = propIter.next();
									DzNumericProperty* NumericProperty = qobject_cast<DzNumericProperty*>(Property);
									if (NumericProperty)
									{
										QString Name = Property->getName();
										QString TextureName = "";
										if (NumericProperty->getMapValue())
										{
											TextureName = NumericProperty->getMapValue()->getFilename();
										}
										Writer.startObject(true);
										Writer.addMember("Name", Name);
										Writer.addMember("Value", QString::number(NumericProperty->getDoubleValue()));
										Writer.addMember("Data Type", QString("Double"));
										Writer.addMember("Texture", TextureName);
										Writer.finishObject();
									}
								}

							 }

						 }

						 ////*******************************************************************************
						 //// Below is the old, unused hybrid script/C++ implementation of "findSimulationSettingsProvider"
						 ////*******************************************************************************
						 //// 2. if dForce modifier exists, look for SimulationSettingsProvider
						 //// 2a. prep script: load FindSimulationProvider script from file/resource
						 //DzScript* script = new DzScript();
						 //if (script->loadFromFile(":/UnofficialDaz/dsa/ScriptFunctionFindSimulationSettingsProvider.dsa"))
						 //{
							// // 2b. prep arguments: pass Node object + Material string to script
							// QVariantList args;
							// QVariant varNode;
							// varNode.setValue((QObject*)Node);
							// args.append(varNode);
							// args.append(Material->getName());
							//
							// // 2c. execute script
							// m_ScriptReturn_ReturnCode = 0;
							// m_ScriptReturn_Object = NULL;
							// bool callResult = script->call("ScriptedFindSimulationSettingsProvider", args);
							// if (callResult)
							// {
							//	 // 2d. wait for return
							//	 int timeout = 5;
							//	 bool bTimeout = false;
							//	 while (m_ScriptReturn_Object == NULL && m_ScriptReturn_ReturnCode == 0)
							//	 {
							//		 delay(100);
							//		 if (timeout-- <= 0)
							//		 {
							//			 bTimeout = true;
							//			 break;
							//		 }
							//	 }
							//
							//	 if (bTimeout == false && m_ScriptReturn_ReturnCode > 0)
							//	 {
							//		 // 3. if script call successful, get returnvalue into DzElement* SimulationSettingsProvider
							//		 // 4. process SimulationsSettingsProvider after other Material properties
							//		 DzElement* elSimulationSettingsProvider = (DzElement*) m_ScriptReturn_Object;
							//		 int numProperties = elSimulationSettingsProvider->getNumProperties();
							//		 DzPropertyListIterator* propIter = &elSimulationSettingsProvider->propertyListIterator();
							//		 QString propString = "";
							//		 int propIndex = 0;
							//		 while (propIter->hasNext())
							//		 {
							//			 DzProperty* Property = propIter->next();
							//			 DzNumericProperty* NumericProperty = qobject_cast<DzNumericProperty*>(Property);
							//			 if (NumericProperty)
							//			 {
							//				 QString Name = Property->getName();
							//				 QString TextureName = "";
							//				 if (NumericProperty->getMapValue())
							//				 {
							//					 TextureName = NumericProperty->getMapValue()->getFilename();
							//				 }
							//				 Writer.startObject(true);
							//				 Writer.addMember("Name", Name);
							//				 Writer.addMember("Value", QString::number(NumericProperty->getDoubleValue()));
							//				 Writer.addMember("Data Type", QString("Double"));
							//				 Writer.addMember("Texture", TextureName);
							//				 Writer.finishObject();
							//			 }
							//		 }
							//	 }
							// }
						 //}

					 } // if (bDForceSettingsAvailable)

					 Writer.finishArray();

					 Writer.finishObject();
				}
		  }
	 }

	 DzNodeListIterator Iterator = Node->nodeChildrenIterator();
	 while (Iterator.hasNext())
	 {
		  DzNode* Child = Iterator.next();
		  WriteMaterials(Child, Writer);
	 }
}

// Write weightmaps - recursively traverse parent/children, and export all associated weightmaps
void UnofficialDzUnityAction::WriteWeightMaps(DzNode* Node, DzJsonWriter& Writer)
{
	DzObject* Object = Node->getObject();
	DzShape* Shape = Object ? Object->getCurrentShape() : NULL;

	bool bDForceSettingsAvailable = false;

	if ( Shape && Shape->inherits("DzFacetShape") )
	{
		DzModifier *dforceModifier;
		DzModifierIterator modIter = Object->modifierIterator();
		while (modIter.hasNext())
		{
			DzModifier* modifier = modIter.next();
			QString mod_Class = modifier->className();
			if (mod_Class.toLower().contains("dforce"))
			{
				bDForceSettingsAvailable = true;
				dforceModifier = modifier;
				break;
			}
		}

		if (bDForceSettingsAvailable)
		{
			//////////////////////////////////////////
			//// OLD Method for obtaining weightmaps, relying on dForce Weight Modifier Node
			//////////////////////////////////////////
			//// 1. check if weightmap modifier present
			//// 2. if not add an undoable weightnode
			//// 3. use weightnode to find weightmap
			//// 4. extract weightmap weights to file --> tied to Shape?
			//// 5. undo any added weightmap modifier
			//DzWeightMapPtr weightMap = getWeightMapPtr(Node);
			//if (weightMap != NULL)
			//{
			//	int numVerts = Shape->getAssemblyGeometry()->getNumVertices();
			//	unsigned short* weights = weightMap->getWeights();
			//	char* buffer = (char*)weights;
			//	int byte_length = numVerts * sizeof(unsigned short);
			//	// export to raw file
			//	QString filename = QString("%1-old.raw_dforce_map.bytes").arg(Node->getLabel());
			//	QFile rawWeight(CharacterFolder + filename);
			//	if (rawWeight.open(QIODevice::ReadWrite))
			//	{
			//		int bytesWritten = rawWeight.write(buffer, byte_length);
			//		if (bytesWritten != byte_length)
			//		{
			//			// write error
			//			QString errString = rawWeight.errorString();
			//			QMessageBox::warning(0, tr("Error"),
			//				errString, QMessageBox::Ok);
			//		}
			//		rawWeight.close();
			//	}
			//}

			////////////////////////////////////////////
			//// EVEN OLDER Method
			////////////////////////////////////////////
			//DzNodeListIterator Iterator = Node->nodeChildrenIterator();
			//while (Iterator.hasNext())
			//{
			//	DzNode* Child = Iterator.next();
			//	if (Child->className().contains("DzDForceModifierWeightNode"))
			//	{
			//		QObject *handler;
			//		if (metaInvokeMethod(Child, "getWeightMapHandler()", handler))
			//		{
			//			QObject* weightGroup;
			//			if (metaInvokeMethod(handler, "currentWeightGroup()", weightGroup))
			//			{
			//				QObject* context;
			//				if (metaInvokeMethod(weightGroup, "currentWeightContext()", context))
			//				{
			//					// DzWeightMapPtr
			//					QMetaMethod metaMethod = context->metaObject()->method(30); // getWeightMap()
			//					DzWeightMapPtr weightMap;
			//					QGenericReturnArgument returnArgument(
			//						metaMethod.typeName(),
			//						&weightMap
			//					);
			//					int result = metaMethod.invoke((QObject*)context, returnArgument);
			//					if (result != -1)
			//					{
			//						if (weightMap != NULL)
			//						{										 
			//							int numVerts = Shape->getAssemblyGeometry()->getNumVertices();
			//							unsigned short* weights = weightMap->getWeights();
			//							char* buffer = (char*)weights;
			//
			//							// export to raw file
			//							QString filename = QString("%1.raw_dforce_map").arg(Node->getLabel());
			//							QFile rawWeight(CharacterFolder + filename);
			//							if (rawWeight.open(QIODevice::WriteOnly))
			//							{
			//								int bytesWritten = rawWeight.write(buffer, sizeof(weights) * numVerts);
			//								if (bytesWritten != sizeof(weights) * numVerts)
			//								{
			//									// write error
			//									QString errString = rawWeight.errorString();
			//									QMessageBox::warning(0, tr("Error"),
			//										errString, QMessageBox::Ok);
			//								}
			//								rawWeight.close();
			//							}
			//						}
			//
			//					}
			//				}
			//			}
			//		}
			//
			//		//// DzDForceModifierWeightHandler
			//		//QMetaMethod metaMethod = Child->metaObject()->method(372); // getWeightMapHandler
			//		//DzBase* handler;
			//		//QGenericReturnArgument returnArgument(
			//		//	metaMethod.typeName(),
			//		//	&handler
			//		//);
			//		//int result = metaMethod.invoke((QObject*)Child, returnArgument);
			//		//if (result)
			//		//{
			//		//	if (handler != NULL)
			//		//	{
			//		//		// DzDForceModifierWeightGroup
			//		//		QMetaMethod metaMethod = handler->metaObject()->method(18); // getWeightGroup
			//		//		QObject* weightGroup;
			//		//		QGenericReturnArgument returnArgument(
			//		//			metaMethod.typeName(),
			//		//			&weightGroup
			//		//		);
			//		//		int result = metaMethod.invoke((QObject*)handler, returnArgument, Q_ARG(int, 0));
			//		//		if (result)
			//		//		{
			//		//			if (weightGroup != NULL)
			//		//			{
			//		//				// DzWeightMapContext
			//		//				QMetaMethod metaMethod = weightGroup->metaObject()->method(19); // getWeightMapContext
			//		//				QObject* context;
			//		//				QGenericReturnArgument returnArgument(
			//		//					metaMethod.typeName(),
			//		//					&context
			//		//				);
			//		//				int result = metaMethod.invoke((QObject*)weightGroup, returnArgument, Q_ARG(int, 0));
			//		//				if (result)
			//		//				{
			//		//					if (context != NULL)
			//		//					{
			//		//						// DzWeightMapPtr
			//		//						QMetaMethod metaMethod = context->metaObject()->method(30); // getWeightMap
			//		//						DzWeightMapPtr weightMap;
			//		//						QGenericReturnArgument returnArgument(
			//		//							metaMethod.typeName(),
			//		//							&weightMap
			//		//						);
			//		//						int result = metaMethod.invoke((QObject*)context, returnArgument);
			//		//						if (result)
			//		//						{
			//		//							if (weightMap != NULL)
			//		//							{
			//		//								int numVerts = Shape->getAssemblyGeometry()->getNumVertices();
			//		//								unsigned short *weights = weightMap->getWeights();
			//		//								char *buffer = (char*)weights;
			//		//
			//		//								// export to raw file
			//		//								QString filename = QString("%1.raw_dforce_map").arg(Node->getLabel());
			//		//								QFile rawWeight(CharacterFolder + filename);
			//		//								if (rawWeight.open(QIODevice::WriteOnly))
			//		//								{
			//		//									int bytesWritten = rawWeight.write(buffer, sizeof(weights) * numVerts);
			//		//									if (bytesWritten != sizeof(weights) * numVerts)
			//		//									{
			//		//										// write error
			//		//										QString errString = rawWeight.errorString();
			//		//										QMessageBox::warning(0, tr("Error"),
			//		//											errString, QMessageBox::Ok);
			//		//									}
			//		//									rawWeight.close();
			//		//								}
			//		//							}
			//		//						}
			//		//					}
			//		//				}
			//		//			}
			//		//		}
			//		//	}
			//		//}
			//	}
			//
			//}

			///////////////////////////////////////////////
			// NEW Method for obtaining weightmaps, grab directly from dForce Modifier Node
			///////////////////////////////////////////////
			int methodIndex = dforceModifier->metaObject()->indexOfMethod(QMetaObject::normalizedSignature("getInfluenceWeights()"));
			if (methodIndex != -1)
			{
				QMetaMethod method = dforceModifier->metaObject()->method(methodIndex);
				DzWeightMap *weightMap;
				QGenericReturnArgument returnArg(
					method.typeName(),
					&weightMap
				);
				int result = method.invoke((QObject*)dforceModifier, returnArg);
				if (result != -1)
				{
					if (weightMap)
					{
						int numVerts = Shape->getAssemblyGeometry()->getNumVertices();
						unsigned short* daz_weights = weightMap->getWeights();
						int byte_length = numVerts * sizeof(unsigned short);

						char* buffer = new char[byte_length];
						unsigned short* unity_weights = (unsigned short*) buffer;

						// load material groups to remap weights to unity's vertex order
						DzFacetMesh* facetMesh = dynamic_cast<DzFacetShape*>(Shape)->getFacetMesh();
						if (facetMesh)
						{
							// sanity check
							if (numVerts != facetMesh->getNumVertices())
							{
								// throw error if needed
								dzApp->log("Unofficial DTU Bridge: ERROR Exporting Weight Map to file.");
								return;
							}
							int numMaterials = facetMesh->getNumMaterialGroups();
							std::list<MaterialGroupExportOrderMetaData> exportQueue;
							DzFacet* facetPtr = facetMesh->getFacetsPtr();

							// generate export order queue
							// first, populate export queue with materialgroups
							for (int i = 0; i < numMaterials; i++)
							{
								DzMaterialFaceGroup* materialGroup = facetMesh->getMaterialGroup(i);
								int numFaces = materialGroup->count();
								const int* indexPtr = materialGroup->getIndicesPtr();
								int offset = facetPtr[indexPtr[0]].m_vertIdx[0];
								int count = -1;
								MaterialGroupExportOrderMetaData *metaData = new MaterialGroupExportOrderMetaData(i, offset);
								exportQueue.push_back(*metaData);
							}

							// sort: uses operator< to order by vertex_offset
							exportQueue.sort();

							/////////////////////////////////////////
							// for building vertex index lookup tables
							/////////////////////////////////////////
							int material_vertex_count = 0;
							int material_vertex_offset = 0;
							int* DazToUnityLookup = new int[numVerts];
							for (int i = 0; i < numVerts; i++) { DazToUnityLookup[i] = -1; }
							int* UnityToDazLookup = new int[numVerts];
							for (int i = 0; i < numVerts; i++) { UnityToDazLookup[i] = -1; }

							int unity_weightmap_vertexindex = 0;
							// iterate through sorted material groups...
							for (std::list<MaterialGroupExportOrderMetaData>::iterator export_iter = exportQueue.begin(); export_iter != exportQueue.end(); export_iter++)
							{
								// update the vert_offset for each materialGroup
								material_vertex_offset = material_vertex_offset + material_vertex_count;
								material_vertex_count = 0;
								int check_offset = export_iter->vertex_offset;

								// retrieve material group based on sorted material index list
								int materialGroupIndex = export_iter->materialIndex;
								DzMaterialFaceGroup* materialGroup = facetMesh->getMaterialGroup(materialGroupIndex);
								int numFaces = materialGroup->count();
								// pointer for faces in materialGroup
								const int* indexPtr = materialGroup->getIndicesPtr();

								// get each face in materialGroup, then iterate through all vertex indices in the face
								// copy out weights into buffer using material group's vertex ordering, but cross-referenced with internal vertex array indices

								// get the i-th index into the index array of faces, then retrieve the j-th index into the vertex index array
								// i is 0 to number of faces (aka facets), j is 0 to number of vertices in the face
								for (int i = 0; i < numFaces; i++)
								{
									int vertsPerFacet = (facetPtr->isQuad()) ? 4 : 3;
									for (int j = 0; j < vertsPerFacet; j++)
									{
										// retrieve vertex index into daz internal vertex array (probably a BST in array format)
										int vert_index = facetPtr[indexPtr[i]].m_vertIdx[j];

										///////////////////////////////////
										// NOTE: Since the faces will often share/re-use the same vertex, we need to skip
										// any vertex that has already been recorded, since we want ONLY unique vertices
										// in weightmap.  This is done by creating checking back into a DazToUnity vertex index lookup table
										///////////////////////////////////
										// unique vertices will not yet be written and have default -1 value
										if (DazToUnityLookup[vert_index] == -1)
										{
											// This vertex is unique, record into the daztounity lookup table and proceed with other operations
											// to be performend on unqiue verts.
											DazToUnityLookup[vert_index] = unity_weightmap_vertexindex;

											// use the vertex index to cross-reference to the corresponding weightmap value and copy out to buffer for exporting
											// (only do this for unique verts)
											unity_weights[unity_weightmap_vertexindex] = weightMap->getWeight(vert_index);
											//unity_weights[unity_weightmap_vertexindex] = daz_weights[vert_index];

											// Create the unity to daz vertex lookup table (only do this for unique verts)
											UnityToDazLookup[unity_weightmap_vertexindex] = vert_index;

											// increment the unity weightmap vertex index (only do this for unique verts)
											unity_weightmap_vertexindex++;
										}

									} //for (int j = 0; j < vertsPerFace; j++)

								} // for (int i = 0; i < numFaces; i++)

							} // for (std::list<MaterialGroupExportOrderMetaData>::iterator export_iter = exportQueue.begin(); export_iter != exportQueue.end(); export_iter++)

							// export to dforce_weightmap file
							QString filename = QString("%1.dforce_weightmap.bytes").arg(Node->getLabel());
							QFile rawWeight(CharacterFolder + filename);
							if (rawWeight.open(QIODevice::WriteOnly))
							{
								int bytesWritten = rawWeight.write(buffer, byte_length);
								if (bytesWritten != byte_length)
								{
									// write error
									QString errString = rawWeight.errorString();
									QMessageBox::warning(0, tr("Error writing dforce weightmap. Incorrect number of bytes written."),
										errString, QMessageBox::Ok);
								}
								rawWeight.close();
							}

						} // if (facetMesh) /** facetMesh null? */
					} // if (weightMap) /** weightmap null? */
				} // if (result != -1) /** invokeMethod failed? */
			} // if (methodIndex != -1) /** findMethod failed? */
		} // if (bDForceSettingsAvailable) /** no dforce data found */
	} // if (Shape)

	DzNodeListIterator Iterator = Node->nodeChildrenIterator();
	while (Iterator.hasNext())
	{
		DzNode* Child = Iterator.next();
		WriteWeightMaps(Child, Writer);
	}

}


// OLD Method for obtaining weightmap, relying on dForce Weight Modifier Node
DzWeightMapPtr UnofficialDzUnityAction::getWeightMapPtr(DzNode* Node)
{
	// 1. check if weightmap modifier present
	DzNodeListIterator Iterator = Node->nodeChildrenIterator();
	while (Iterator.hasNext())
	{
		DzNode* Child = Iterator.next();
		if (Child->className().contains("DzDForceModifierWeightNode"))
		{
			QObject* handler;
			if (metaInvokeMethod(Child, "getWeightMapHandler()", (void**) &handler))
			{
				QObject* weightGroup;
				if (metaInvokeMethod(handler, "currentWeightGroup()", (void**) &weightGroup))
				{
					QObject* context;
					if (metaInvokeMethod(weightGroup, "currentWeightContext()", (void**) &context))
					{
						DzWeightMapPtr weightMap;
						// DzWeightMapPtr
						QMetaMethod metaMethod = context->metaObject()->method(30); // getWeightMap()
						QGenericReturnArgument returnArgument(
							metaMethod.typeName(),
							&weightMap
						);
						int result = metaMethod.invoke((QObject*)context, returnArgument);
						if (result != -1)
						{
							return weightMap;
						}
					}
				}
			}
		}

	}

	return NULL;

}

bool UnofficialDzUnityAction::metaInvokeMethod(QObject* object, const char* methodSig, void** returnPtr)
{
	if (object == NULL)
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////
	// REFERENCE Signatures obtained by QMetaObject->method() query
	//////////////////////////////////////////////////////////////////
	//
	// DzDForceModifierWeightNode::getWeightMapHandler() = 372
	//
	// DzDForceModifierWeightHandler::getWeightGroup(int) = 18
	// DzDForceModifierWeightHandler::currentWeightGroup() = 20
	//
	// DzDForceModifierWeightGroup::getWeightMapContext(int) = 19
	// DzDForceModifierWeightGroup::currentWeightContext() = 22
	//
	// DzDForceModiferMapContext::getWeightMap() = 30
	/////////////////////////////////////////////////////////////////////////

	// find the metamethod
	const QMetaObject *metaObject = object->metaObject();
	int methodIndex = metaObject->indexOfMethod(QMetaObject::normalizedSignature(methodSig));
	if (methodIndex == -1)
	{
		// use fuzzy search
		// look up all methods, find closest match for methodSig
		int searchResult = -1;
		QString fuzzySig = QString(QMetaObject::normalizedSignature(methodSig)).toLower().remove("()");
		for (int i = 0; i < metaObject->methodCount(); i++)
		{
			const char* sig = metaObject->method(i).signature();
			if (QString(sig).toLower().contains(fuzzySig))
			{
				searchResult = i;
				break;
			}
		}
		if (searchResult == -1)
		{
			return false;
		}
		else
		{
			methodIndex = searchResult;
		}

	}

	// invoke metamethod
	QMetaMethod metaMethod = metaObject->method(methodIndex); 
	void* returnVal;
	QGenericReturnArgument returnArgument(
		metaMethod.typeName(),
		&returnVal
	);
	int result = metaMethod.invoke((QObject*)object, returnArgument);
	if (result)
	{
		// set returnvalue
		*returnPtr = returnVal;

		return true;
	}

	return false;
}


#include "moc_DzUnityAction.cpp"
