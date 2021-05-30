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

#include "DzUnityDialog.h"
#include "DzUnityAction.h"

#include "QtCore/qmetaobject.h"
#include "dzmodifier.h"


UnofficialDzUnityAction::UnofficialDzUnityAction() :
	 UnofficialDzRuntimePluginAction(tr("&Unofficial Daz to Unity"), tr("Send the selected node to Unity."))
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
		  ImportFolder = dlg->assetsFolderEdit->text() + "\\Daz3D";
		  dir.mkpath(ImportFolder);

		  // Collect the values from the dialog fields
		  CharacterName = dlg->assetNameEdit->text();
		  CharacterFolder = ImportFolder + "\\" + CharacterName + "\\";
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
		  SubdivisionDialog->LockSubdivisionProperties(ExportSubdivisions);
		  FBXVersion = QString("FBX 2014 -- Binary");

		  Export();

		  //Rename the textures folder
		  QDir textureDir(CharacterFolder + "\\" + CharacterName + ".images");
		  textureDir.rename(CharacterFolder + "\\" + CharacterName + ".images", CharacterFolder + "\\Textures");
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
		QFile::setPermissions(*dst, QFile::ReadOther | QFile::WriteOther);
	}

	return result;
}

void UnofficialDzUnityAction::CreateUnityFiles(bool replace)
{
	 if (!InstallUnityFiles)
		  return;

	 //Create shader folder if it doesn't exist
	 QDir dir;
	 QString scriptsFolder = ImportFolder + "\\Scripts";
	 dir.mkpath(scriptsFolder);

	 QStringList scripts = QDir(":/UnofficialDaz/Scripts/").entryList();
	 for (int i = 0; i < scripts.size(); i++)
	 {
		  QString script = scriptsFolder + "\\" + scripts[i];
		  QFile file(":/UnofficialDaz/Scripts/" + scripts[i]);
		  CopyFile(&file, &script, replace);
		  file.close();
	 }

	 //Create editor folder if it doesn't exist
	 QString editorFolder = ImportFolder + "\\Scripts\\Editor";
	 dir.mkpath(editorFolder);

	 QStringList editorScripts = QDir(":/UnofficialDaz/Editor/").entryList();
	 for (int i = 0; i < editorScripts.size(); i++)
	 {
		  QString script = editorFolder + "\\" + editorScripts[i];
		  QFile file(":/UnofficialDaz/Editor/" + editorScripts[i]);
		  CopyFile(&file, &script, replace);
		  file.close();
	 }

	 //Create shader folder if it doesn't exist
	 QString shaderFolder = ImportFolder + "\\Shaders";
	 dir.mkpath(shaderFolder);

	 QStringList shaders = QDir(":/UnofficialDaz/Shaders/").entryList();
	 for (int i = 0; i < shaders.size(); i++)
	 {
		  QString shader = shaderFolder + "\\" + shaders[i];
		  QFile file(":/UnofficialDaz/Shaders/" + shaders[i]);
		  CopyFile(&file, &shader, replace);
		  file.close();
	 }

	 //Create shader CGInc folder if it doesn't exist
	 QString shaderCGIncFolder = ImportFolder + "\\Shaders\\CGInc";
	 dir.mkpath(shaderCGIncFolder);

	 QStringList shaderCGIncStringList = QDir(":/UnofficialDaz/ShaderCGInc/").entryList();
	 for (int i = 0; i < shaderCGIncStringList.size(); i++)
	 {
		 QString shaderCGIncFilename = shaderCGIncFolder + "\\" + shaderCGIncStringList[i];
		 QFile file(":/UnofficialDaz/ShaderCGInc/" + shaderCGIncStringList[i]);
		 CopyFile(&file, &shaderCGIncFilename, replace);
		 file.close();
	 }

	 //Create shader helpers folder if it doesn't exist
	 QString shaderHelperFolder = ImportFolder + "\\Shaders\\Helpers";
	 dir.mkpath(shaderHelperFolder);

	 QStringList shaderHelpers = QDir(":/UnofficialDaz/ShaderHelpers/").entryList();
	 for (int i = 0; i < shaderHelpers.size(); i++)
	 {
		  QString shaderHelper = shaderHelperFolder + "\\" + shaderHelpers[i];
		  QFile file(":/UnofficialDaz/ShaderHelpers/" + shaderHelpers[i]);
		  CopyFile(&file, &shaderHelper, replace);
		  file.close();
	 }

	 //Create vendors folder if it doesn't exist
	 QString vendorsFolder = ImportFolder + "\\Vendors";
	 dir.mkpath(vendorsFolder);

	 QStringList vendors = QDir(":/UnofficialDaz/Vendors/").entryList();
	 for (int i = 0; i < vendors.size(); i++)
	 {
		  QString vendor = vendorsFolder + "\\" + vendors[i];
		  QFile file(":/UnofficialDaz/Vendors/" + vendors[i]);
		  CopyFile(&file, &vendor, replace);
		  file.close();
	 }

	 //Create DiffusionProfiles folder if it doesn't exist
	 QString profilesFolder = ImportFolder + "\\DiffusionProfiles";
	 dir.mkpath(profilesFolder);

	 QStringList profiles = QDir(":/UnofficialDaz/DiffusionProfiles/").entryList();
	 for (int i = 0; i < profiles.size(); i++)
	 {
		  QString profile = profilesFolder + "\\" + profiles[i];
		  QFile file(":/UnofficialDaz/DiffusionProfiles/" + profiles[i]);
		  CopyFile(&file, &profile, replace);
		  file.close();
	 }

	 //Create Resources folder if it doesn't exist
	 QString resourcesFolder = ImportFolder + "\\Resources";
	 dir.mkpath(resourcesFolder);

	 QStringList resources = QDir(":/UnofficialDaz/Resources/").entryList();
	 for (int i = 0; i < resources.size(); i++)
	 {
		  QString resource = resourcesFolder + "\\" + resources[i];
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
		 DzModifierIterator* modIter = &Object->modifierIterator();
		 while (modIter->hasNext())
		 {
			 DzModifier* modifier = modIter->next();
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
		 DzElement* elSimulationSettingsProvider;
		 bool ret = false;
		 int methodIndex = Shape->metaObject()->indexOfMethod("findSimulationSettingsProvider");
		 if (methodIndex != -1)
		 {
			 QMetaMethod method = Shape->metaObject()->method(methodIndex);
			 ret = method.invoke(Shape, Q_RETURN_ARG(DzElement*, elSimulationSettingsProvider), Q_ARG(QString&, QString("Skirt1")));
		 }

		 ret = QMetaObject::invokeMethod(Shape, "findSimulationSettingsProvider", Q_RETURN_ARG(DzElement*, elSimulationSettingsProvider), Q_ARG(QString*,&QString("Skirt1")));
		 if (ret)
		 {
			 if (elSimulationSettingsProvider)
			 {
				 int numProperties = elSimulationSettingsProvider->getNumProperties();
			 }
		 }

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
								DzPropertyListIterator* propIter = &elSimulationSettingsProvider->propertyListIterator();
								QString propString = "";
								int propIndex = 0;
								while (propIter->hasNext())
								{
									DzProperty* Property = propIter->next();
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


					 }

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


#include "moc_DzUnityAction.cpp"
