#include <dzapp.h>
#include <dzscene.h>
#include <dzexportmgr.h>
#include <dzexporter.h>
#include <dzmainwindow.h>
#include <dzmaterial.h>
#include <dzproperty.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <dzimageproperty.h>
#include <dzstringproperty.h>
#include <dznumericproperty.h>
#include <dzfloatproperty.h>
#include <dzcolorproperty.h>
#include <dzstringproperty.h>
#include <dzenumproperty.h>
#include <dzboolproperty.h>
#include <dzobject.h>
#include <dzskeleton.h>
#include <dzfigure.h>
#include <dzshape.h>
#include <dzassetmgr.h>
#include <dzuri.h>
#include <dzcontentmgr.h>
#include <dzassetmetadata.h>
#include <dzbone.h>
#include <dzskeleton.h>
#include <dzpresentation.h>
#include <dzmodifier.h>
#include <dzmorph.h>
#include <dzprogress.h>

#include <QtCore/qdir.h>
#include <QtGui/qlineedit.h>
#include <QtNetwork/qudpsocket.h>
#include <QtNetwork/qabstractsocket.h>
#include <QtGui/qcheckbox.h>
#include <QtGui/QMessageBox>


#include "DzRuntimePluginAction.h"

DzRuntimePluginAction::DzRuntimePluginAction(const QString& text, const QString& desc) :
	 DzAction(text, desc)
{
	 ExportMorphs = false;
	 ExportSubdivisions = false;
	 ShowFbxDialog = false;
	 ControllersToDisconnect.append("facs_bs_MouthClose_div2");
	 UseRelativePaths = false;
}

DzRuntimePluginAction::~DzRuntimePluginAction()
{
}

bool DzRuntimePluginAction::preProcessScene(DzNode* parentNode)
{
	// placeholder settings, TODO: move to member variable / Q_PROPERTY
	bool bInjectNormalMaps = true;
	
	///////////////////////
	// Create JobPool
	// Iterate through all children of "parentNode", create jobpool of nodes to process later
	// Nodes are added to nodeJobList in breadth-first order (parent,children,grandchildren)
	///////////////////////
	DzNodeList nodeJobList;
	DzNodeList tempQueue;
	DzNode *node_ptr = parentNode;
	if (node_ptr == nullptr)
		node_ptr = dzScene->getPrimarySelection();
	tempQueue.append(node_ptr);
	while (!tempQueue.isEmpty())
	{
		node_ptr = tempQueue.first();
		tempQueue.removeFirst();
		nodeJobList.append(node_ptr);
		DzNodeListIterator Iterator = node_ptr->nodeChildrenIterator();
		while (Iterator.hasNext())
		{
			DzNode* tempChild = Iterator.next();
			tempQueue.append(tempChild);
		}
	}

	///////////////////////
	// Process JobPool (DzNodeList nodeJobList)
	///////////////////////
	QList<QString> existingMaterialNameList;
	for (int i = 0; i < nodeJobList.length(); i++)
	{
		DzNode *node = nodeJobList[i];
		DzObject* object = node->getObject();
		DzShape* shape = object ? object->getCurrentShape() : NULL;
		if (shape)
		{
			for (int i = 0; i < shape->getNumMaterials(); i++)
			{
				DzMaterial* material = shape->getMaterial(i);
				if (material)
				{
					//////////////////
					// Rename Duplicate Material
					/////////////////
					int nameIndex = 0;
					QString newMaterialName = material->getName();
					while (existingMaterialNameList.contains(newMaterialName) )
					{
						newMaterialName = material->getName() + QString("%1").arg(++nameIndex);
					}
					if (newMaterialName != material->getName())
					{
						m_undoTable_DuplicateMaterialRename.insert(material, material->getName());
						material->setName(newMaterialName);
					}
					existingMaterialNameList.append(newMaterialName);

					/////////////////
					// Inject Normal Maps
					/////////////////
					if (bInjectNormalMaps)
					{
						// Check if normal map missing
						// Check if height map present
						// Generate normal map from height map
					}
				}
			}
		}
	}

	return true;
}


bool DzRuntimePluginAction::undoPreProcessScene()
{
	// placeholder for settings
	bool bUndoInjectNormalMaps = false;

	// Undo Rename Duplicate Materials
	QMap<DzMaterial*, QString>::iterator iter;
	for (iter = m_undoTable_DuplicateMaterialRename.begin(); iter != m_undoTable_DuplicateMaterialRename.end(); ++iter)
	{
		iter.key()->setName(iter.value());
	}
	m_undoTable_DuplicateMaterialRename.clear();

	// Undo Inject Normal Maps
	if (bUndoInjectNormalMaps)
	{
		// for each material in undotable for normalmap injection, remove normal map property
	}

	return true;
}

void DzRuntimePluginAction::Export()
{
	// FBX Export
	Selection = dzScene->getPrimarySelection();
	if (!Selection)
		return;

	QMap<QString, DzNode*> PropToInstance;
	if (AssetType == "Environment")
	{
		// Store off the original export information
		QString OriginalCharacterName = CharacterName;
		DzNode* OriginalSelection = Selection;

		// Find all the different types of props in the scene
		GetScenePropList(Selection, PropToInstance);
		QMap<QString, DzNode*>::iterator iter;
		for (iter = PropToInstance.begin(); iter != PropToInstance.end(); ++iter)
		{
			// Override the export info for exporting this prop
			AssetType = "StaticMesh";
			CharacterName = iter.key();
			CharacterName = CharacterName.remove(QRegExp("[^A-Za-z0-9_]"));
			DestinationPath = RootFolder + "/" + CharacterName + "/";
			CharacterFBX = DestinationPath + CharacterName + ".fbx";
			DzNode* Node = iter.value();

			// If this is a figure, send it as a skeletal mesh
			if (DzSkeleton* Skeleton = Node->getSkeleton())
			{
				if (DzFigure* Figure = qobject_cast<DzFigure*>(Skeleton))
				{
					AssetType = "SkeletalMesh";
				}
			}

			//Unlock the transform controls so the node can be moved to root
			UnlockTranform(Node);

			// Disconnect the asset being sent from everything else
			QList<AttachmentInfo> AttachmentList;
			DisconnectNode(Node, AttachmentList);

			// Set the selection so this will be the exported asset
			Selection = Node;

			// Store the current transform and zero it out.
			DzVec3 Location;
			DzQuat Rotation;
			DzMatrix3 Scale;

			Node->getWSTransform(Location, Rotation, Scale);
			Node->setWSTransform(DzVec3(0.0f, 0.0f, 0.0f), DzQuat(), DzMatrix3(true));

			// Export
			ExportNode(Node);

			// Put the item back where it was
			Node->setWSTransform(Location, Rotation, Scale);

			// Reconnect all the nodes
			ReconnectNodes(AttachmentList);
		}

		// After the props have been exported, export the environment
		CharacterName = OriginalCharacterName;
		DestinationPath = RootFolder + "/" + ExportFolder + "/";
		CharacterFBX = DestinationPath + CharacterName + ".fbx";
		Selection = OriginalSelection;
		AssetType = "Environment";
		ExportNode(Selection);
	}
	else if (AssetType == "Pose")
	{
		if (CheckIfPoseExportIsDestructive())
		{
			if (QMessageBox::question(0, tr("Continue"),
				tr("Proceeding will delete keyed values on some properties. Continue?"),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			{
				return;
			}
		}

		PoseList.clear();
		DzNode* Selection = dzScene->getPrimarySelection();
		int poseIndex = 0;
		DzNumericProperty* previousProperty = nullptr;
		for (int index = 0; index < Selection->getNumProperties(); index++)
		{
			DzProperty* property = Selection->getProperty(index);
			DzNumericProperty* numericProperty = qobject_cast<DzNumericProperty*>(property);
			QString propName = property->getName();
			if (numericProperty)
			{
				QString propName = property->getName();
				if (MorphMapping.contains(propName))
				{
					poseIndex++;
					numericProperty->setDoubleValue(0.0f, 0.0f);
					for (int frame = 0; frame < MorphMapping.count() + 1; frame++)
					{
						numericProperty->setDoubleValue(dzScene->getTimeStep() * double(frame), 0.0f);
					}
					numericProperty->setDoubleValue(dzScene->getTimeStep() * double(poseIndex),1.0f);
					PoseList.append(propName);
				}
			}
		}

		DzObject* Object = Selection->getObject();
		if (Object)
		{
			for (int index = 0; index < Object->getNumModifiers(); index++)
			{
				DzModifier* modifier = Object->getModifier(index);
				DzMorph* mod = qobject_cast<DzMorph*>(modifier);
				if (mod)
				{
					for (int propindex = 0; propindex < modifier->getNumProperties(); propindex++)
					{
						DzProperty* property = modifier->getProperty(propindex);
						QString propName = property->getName();
						QString propLabel = property->getLabel();
						DzNumericProperty* numericProperty = qobject_cast<DzNumericProperty*>(property);
						if (numericProperty)
						{
							QString propName = property->getName();
							//qDebug() << propName;
							if (MorphMapping.contains(modifier->getName()))
							{
								poseIndex++;
								numericProperty->setDoubleValue(0.0f, 0.0f);
								for (int frame = 0; frame < MorphMapping.count() + 1; frame++)
								{
									numericProperty->setDoubleValue(dzScene->getTimeStep() * double(frame), 0.0f);
								}
								numericProperty->setDoubleValue(dzScene->getTimeStep() * double(poseIndex), 1.0f);
								PoseList.append(modifier->getName());
							}
						}
					}

				}

			}
		}

		dzScene->setAnimRange(DzTimeRange(0, poseIndex * dzScene->getTimeStep()));
		dzScene->setPlayRange(DzTimeRange(0, poseIndex * dzScene->getTimeStep()));

		ExportNode(Selection);
	}
	else if (AssetType == "SkeletalMesh")
	{
		QList<QString> DisconnectedModifiers = DisconnectOverrideControllers();
		DzNode* Selection = dzScene->getPrimarySelection();
		ExportNode(Selection);
		ReconnectOverrideControllers(DisconnectedModifiers);
	}
	else
	{
		DzNode* Selection = dzScene->getPrimarySelection();
		ExportNode(Selection);
	}
}

void DzRuntimePluginAction::DisconnectNode(DzNode* Node, QList<AttachmentInfo>& AttachmentList)
{
	AttachmentInfo ParentAttachment;
	if (Node->getNodeParent())
	{
		// Don't disconnect a figures bones
		if (DzBone* Bone = qobject_cast<DzBone*>(Node))
		{

		}
		else
		{
			ParentAttachment.Parent = Node->getNodeParent();
			ParentAttachment.Child = Node;
			AttachmentList.append(ParentAttachment);
			Node->getNodeParent()->removeNodeChild(Node);
		}
	}

	QList<DzNode*> ChildNodes;
	for (int ChildIndex = Node->getNumNodeChildren() - 1; ChildIndex >= 0; ChildIndex--)
	{
		DzNode* ChildNode = Node->getNodeChild(ChildIndex);
		if (DzBone* Bone = qobject_cast<DzBone*>(ChildNode))
		{

		}
		else
		{
			DzNode* ChildNode = Node->getNodeChild(ChildIndex);
			AttachmentInfo ChildAttachment;
			ChildAttachment.Parent = Node;
			ChildAttachment.Child = ChildNode;
			AttachmentList.append(ChildAttachment);
			Node->removeNodeChild(ChildNode);
		}
		DisconnectNode(ChildNode, AttachmentList);
	}
}

void DzRuntimePluginAction::ReconnectNodes(QList<AttachmentInfo>& AttachmentList)
{
	foreach(AttachmentInfo Attachment, AttachmentList)
	{
		Attachment.Parent->addNodeChild(Attachment.Child);
	}
}


void DzRuntimePluginAction::ExportNode(DzNode* Node)
{
	dzScene->selectAllNodes(false);
	 dzScene->setPrimarySelection(Node);

	 if (AssetType == "Environment")
	 {
		 QDir dir;
		 dir.mkpath(DestinationPath);
		 WriteConfiguration();
		 return;
	 }

	 DzExportMgr* ExportManager = dzApp->getExportMgr();
	 DzExporter* Exporter = ExportManager->findExporterByClassName("DzFbxExporter");

	 if (Exporter)
	 {
		  DzFileIOSettings ExportOptions;
		  ExportOptions.setBoolValue("doSelected", true);
		  ExportOptions.setBoolValue("doVisible", false);
		  if (AssetType == "SkeletalMesh" || AssetType == "StaticMesh" || AssetType == "Environment")
		  {
				ExportOptions.setBoolValue("doFigures", true);
				ExportOptions.setBoolValue("doProps", true);
		  }
		  else
		  {
				ExportOptions.setBoolValue("doFigures", true);
				ExportOptions.setBoolValue("doProps", false);
		  }
		  ExportOptions.setBoolValue("doLights", false);
		  ExportOptions.setBoolValue("doCameras", false);
		  ExportOptions.setBoolValue("doAnims", false);
		  if ((AssetType == "Animation" || AssetType == "SkeletalMesh") && ExportMorphs && MorphString != "")
		  {
				ExportOptions.setBoolValue("doMorphs", true);
				ExportOptions.setStringValue("rules", MorphString);
		  }
		  else
		  {
				ExportOptions.setBoolValue("doMorphs", false);
				ExportOptions.setStringValue("rules", "");
		  }

		  ExportOptions.setStringValue("format", FBXVersion);
		  ExportOptions.setIntValue("RunSilent", !ShowFbxDialog);

		  ExportOptions.setBoolValue("doEmbed", false);
		  ExportOptions.setBoolValue("doCopyTextures", false);
		  ExportOptions.setBoolValue("doDiffuseOpacity", false);
		  ExportOptions.setBoolValue("doMergeClothing", true);
		  ExportOptions.setBoolValue("doStaticClothing", false);
		  ExportOptions.setBoolValue("degradedSkinning", true);
		  ExportOptions.setBoolValue("degradedScaling", true);
		  ExportOptions.setBoolValue("doSubD", false);
		  ExportOptions.setBoolValue("doCollapseUVTiles", false);

		  // get the top level node for things like clothing so we don't get dupe material names
		  DzNode* Parent = Node;
		  if (AssetType != "Environment")
		  {
			  while (Parent->getNodeParent() != NULL)
			  {
				  Parent = Parent->getNodeParent();
			  }
		  }

		  // rename duplicate material names
		  QList<QString> MaterialNames;
		  QMap<DzMaterial*, QString> OriginalMaterialNames;
		  RenameDuplicateMaterials(Parent, MaterialNames, OriginalMaterialNames);

		  QDir dir;
		  dir.mkpath(DestinationPath);

		  SetExportOptions(ExportOptions);

		  if (ExportSubdivisions)
		  {
			  if (ExportBaseMesh)
			  {
				  QString CharacterBaseFBX = CharacterFBX;
				  CharacterBaseFBX.replace(".fbx", "_base.fbx");
				  Exporter->writeFile(CharacterBaseFBX, &ExportOptions);
			  }
			  else
			  {
				  QString CharacterHDFBX = CharacterFBX;
				  CharacterHDFBX.replace(".fbx", "_HD.fbx");
				  Exporter->writeFile(CharacterHDFBX, &ExportOptions);

				  WriteConfiguration();
			  }
		  }
		  else
		  {
			  Exporter->writeFile(CharacterFBX, &ExportOptions);
			  WriteConfiguration();
		  }

		  // Change back material names
		  UndoRenameDuplicateMaterials(Parent, MaterialNames, OriginalMaterialNames);
	 }
}

// If there are duplicate material names, save off the original and rename one
void DzRuntimePluginAction::RenameDuplicateMaterials(DzNode* Node, QList<QString>& MaterialNames, QMap<DzMaterial*, QString>& OriginalMaterialNames)
{
	 DzObject* Object = Node->getObject();
	 DzShape* Shape = Object ? Object->getCurrentShape() : NULL;

	 if (Shape)
	 {
		  for (int i = 0; i < Shape->getNumMaterials(); i++)
		  {
				DzMaterial* Material = Shape->getMaterial(i);
				if (Material)
				{
					 OriginalMaterialNames.insert(Material, Material->getName());
					 while (MaterialNames.contains(Material->getName()))
					 {
						  Material->setName(Material->getName() + "_1");
					 }
					 MaterialNames.append(Material->getName());
				}
		  }
	 }
	 DzNodeListIterator Iterator = Node->nodeChildrenIterator();
	 while (Iterator.hasNext())
	 {
		  DzNode* Child = Iterator.next();
		  RenameDuplicateMaterials(Child, MaterialNames, OriginalMaterialNames);
	 }
}

// Restore the original material names
void DzRuntimePluginAction::UndoRenameDuplicateMaterials(DzNode* Node, QList<QString>& MaterialNames, QMap<DzMaterial*, QString>& OriginalMaterialNames)
{
	 QMap<DzMaterial*, QString>::iterator iter;
	 for (iter = OriginalMaterialNames.begin(); iter != OriginalMaterialNames.end(); ++iter)
	 {
		  iter.key()->setName(iter.value());
	 }
}

void DzRuntimePluginAction::GetScenePropList(DzNode* Node, QMap<QString, DzNode*>& Types)
{
	DzObject* Object = Node->getObject();
	DzShape* Shape = Object ? Object->getCurrentShape() : NULL;
	DzGeometry* Geometry = Shape ? Shape->getGeometry() : NULL;
	DzSkeleton* Skeleton = Node->getSkeleton();
	DzFigure* Figure = Skeleton ? qobject_cast<DzFigure*>(Skeleton) : NULL;
	//QString AssetId = Node->getAssetId();
	//IDzSceneAsset::AssetType Type = Node->getAssetType();

	// Use the FileName to generate a name for the prop to be exported
	QString Path = Node->getAssetFileInfo().getUri().getFilePath();
	QFile File(Path);
	QString FileName = File.fileName();
	QStringList Items = FileName.split("/");
	QStringList Parts = Items[Items.count() - 1].split(".");
	QString Name = Parts[0].remove(QRegExp("[^A-Za-z0-9_]"));

	if (Figure)
	{
		QString FigureAssetId = Figure->getAssetId();
		if (!Types.contains(Name))
		{
			Types.insert(Name, Node);
		}
	}
	else if (Geometry)
	{
		if (!Types.contains(Name))
		{
			Types.insert(Name, Node);
		}
	}

	// Looks through the child nodes for more props
	for (int ChildIndex = 0; ChildIndex < Node->getNumNodeChildren(); ChildIndex++)
	{
		DzNode* ChildNode = Node->getNodeChild(ChildIndex);
		GetScenePropList(ChildNode, Types);
	}
}

QList<QString> DzRuntimePluginAction::DisconnectOverrideControllers()
{
	QList<QString> ModifiedList;
	DzNode* Selection = dzScene->getPrimarySelection();
	int poseIndex = 0;
	DzNumericProperty* previousProperty = nullptr;
	for (int index = 0; index < Selection->getNumProperties(); index++)
	{
		DzProperty* property = Selection->getProperty(index);
		DzNumericProperty* numericProperty = qobject_cast<DzNumericProperty*>(property);
		QString propName = property->getName();
		if (numericProperty && !numericProperty->isOverridingControllers())
		{
			QString propName = property->getName();
			if (MorphMapping.contains(propName) && ControllersToDisconnect.contains(propName))
			{
				numericProperty->setOverrideControllers(true);
				ModifiedList.append(propName);
			}
		}
	}

	DzObject* Object = Selection->getObject();
	if (Object)
	{
		for (int index = 0; index < Object->getNumModifiers(); index++)
		{
			DzModifier* modifier = Object->getModifier(index);
			DzMorph* mod = qobject_cast<DzMorph*>(modifier);
			if (mod)
			{
				for (int propindex = 0; propindex < modifier->getNumProperties(); propindex++)
				{
					DzProperty* property = modifier->getProperty(propindex);
					QString propName = property->getName();
					QString propLabel = property->getLabel();
					DzNumericProperty* numericProperty = qobject_cast<DzNumericProperty*>(property);
					if (numericProperty && !numericProperty->isOverridingControllers())
					{
						QString propName = property->getName();
						if (MorphMapping.contains(modifier->getName()) && ControllersToDisconnect.contains(modifier->getName()))
						{
							numericProperty->setOverrideControllers(true);
							ModifiedList.append(modifier->getName());
						}
					}
				}

			}

		}
	}

	return ModifiedList;
}

void DzRuntimePluginAction::ReconnectOverrideControllers(QList<QString>& DisconnetedControllers)
{
	DzNode* Selection = dzScene->getPrimarySelection();
	int poseIndex = 0;
	DzNumericProperty* previousProperty = nullptr;
	for (int index = 0; index < Selection->getNumProperties(); index++)
	{
		DzProperty* property = Selection->getProperty(index);
		DzNumericProperty* numericProperty = qobject_cast<DzNumericProperty*>(property);
		QString propName = property->getName();
		if (numericProperty && numericProperty->isOverridingControllers())
		{
			QString propName = property->getName();
			if (DisconnetedControllers.contains(propName))
			{
				numericProperty->setOverrideControllers(false);
			}
		}
	}

	DzObject* Object = Selection->getObject();
	if (Object)
	{
		for (int index = 0; index < Object->getNumModifiers(); index++)
		{
			DzModifier* modifier = Object->getModifier(index);
			DzMorph* mod = qobject_cast<DzMorph*>(modifier);
			if (mod)
			{
				for (int propindex = 0; propindex < modifier->getNumProperties(); propindex++)
				{
					DzProperty* property = modifier->getProperty(propindex);
					QString propName = property->getName();
					QString propLabel = property->getLabel();
					DzNumericProperty* numericProperty = qobject_cast<DzNumericProperty*>(property);
					if (numericProperty && numericProperty->isOverridingControllers())
					{
						QString propName = property->getName();
						if (DisconnetedControllers.contains(modifier->getName()))
						{
							numericProperty->setOverrideControllers(false);
						}
					}
				}

			}

		}
	}
}

bool DzRuntimePluginAction::CheckIfPoseExportIsDestructive()
{
	DzNode* Selection = dzScene->getPrimarySelection();
	int poseIndex = 0;
	DzNumericProperty* previousProperty = nullptr;
	for (int index = 0; index < Selection->getNumProperties(); index++)
	{
		DzProperty* property = Selection->getProperty(index);
		DzNumericProperty* numericProperty = qobject_cast<DzNumericProperty*>(property);
		QString propName = property->getName();
		if (numericProperty)
		{
			QString propName = property->getName();
			if (MorphMapping.contains(propName))
			{
				if (!(numericProperty->getKeyRange().getEnd() == 0.0f && numericProperty->getDoubleValue(0.0f) == 0.0f)) return true;
			}
		}
	}

	DzObject* Object = Selection->getObject();
	if (Object)
	{
		for (int index = 0; index < Object->getNumModifiers(); index++)
		{
			DzModifier* modifier = Object->getModifier(index);
			DzMorph* mod = qobject_cast<DzMorph*>(modifier);
			if (mod)
			{
				for (int propindex = 0; propindex < modifier->getNumProperties(); propindex++)
				{
					DzProperty* property = modifier->getProperty(propindex);
					QString propName = property->getName();
					QString propLabel = property->getLabel();
					DzNumericProperty* numericProperty = qobject_cast<DzNumericProperty*>(property);
					if (numericProperty)
					{
						QString propName = property->getName();
						if (MorphMapping.contains(modifier->getName()))
						{
							if (!(numericProperty->getKeyRange().getEnd() == 0.0f && numericProperty->getDoubleValue(0.0f) == 0.0f)) return true;
						}
					}
				}

			}

		}
	}

	return false;
}

void DzRuntimePluginAction::UnlockTranform(DzNode* NodeToUnlock)
{
	DzFloatProperty* Property = nullptr;
	Property = NodeToUnlock->getXPosControl();
	Property->lock(false);
	Property = NodeToUnlock->getYPosControl();
	Property->lock(false);	
	Property = NodeToUnlock->getZPosControl();
	Property->lock(false);

	Property = NodeToUnlock->getXRotControl();
	Property->lock(false);
	Property = NodeToUnlock->getYRotControl();
	Property->lock(false);
	Property = NodeToUnlock->getZRotControl();
	Property->lock(false);

	Property = NodeToUnlock->getXScaleControl();
	Property->lock(false);
	Property = NodeToUnlock->getYScaleControl();
	Property->lock(false);
	Property = NodeToUnlock->getZScaleControl();
	Property->lock(false);
}

bool DzRuntimePluginAction::isTemporaryFile(QString sFilename)
{
	QString cleanedFilename = sFilename.toLower().replace("\\", "/");
	QString cleanedTempPath = dzApp->getTempPath().toLower().replace("\\", "/");

	if (cleanedFilename.contains(cleanedTempPath))
	{
		return true;
	}

	return false;
}

QString DzRuntimePluginAction::exportWithDTU(QString sFilename, QString sAssetMaterialName)
{
	if (sFilename.isEmpty()) 
		return sFilename;

	QString cleanedFilename = sFilename.toLower().replace("\\", "/");
	QString cleanedTempPath = dzApp->getTempPath().toLower().replace("\\", "/");
	QString cleanedAssetMaterialName = sAssetMaterialName;
	cleanedAssetMaterialName.remove(QRegExp("[^A-Za-z0-9_]"));

	QString exportPath = this->RootFolder.replace("\\","/") + "/" + this->ExportFolder.replace("\\", "/");
	QString fileStem = QFileInfo(sFilename).fileName();

	exportPath += "/ExportTextures/";
	QDir().mkpath(exportPath);
//	QString exportFilename = exportPath + cleanedAssetMaterialName + "_" + fileStem;
	QString exportFilename = exportPath + fileStem;

	exportFilename = makeUniqueFilename(exportFilename);

	if (QFile(sFilename).copy(exportFilename) == true)
	{
		return exportFilename;
	}

	// copy method may fail if file already exists,
	// if exists and same file size, then proceed as if successful
	if ( QFileInfo(exportFilename).exists() &&
		QFileInfo(sFilename).size() == QFileInfo(exportFilename).size())
	{
		return exportFilename;
	}

	// return original source string if failed
	return sFilename;

}

QString DzRuntimePluginAction::makeUniqueFilename(QString sFilename)
{
	if (QFileInfo(sFilename).exists() != true)
		return sFilename;

	QString newFilename = sFilename;
	int duplicate_count = 0;

	while ( 
		QFileInfo(newFilename).exists() &&
		QFileInfo(sFilename).size() != QFileInfo(newFilename).size()
		)
	{
		newFilename = sFilename + QString("_%i").arg(duplicate_count++);
	}

	return newFilename;

}

void DzRuntimePluginAction::writeJSON_Property_Texture(DzJsonWriter& Writer, QString sName, QString sValue, QString sType, QString sTexture)
{
	Writer.startObject(true);
	Writer.addMember("Name", sName);
	Writer.addMember("Value", sValue);
	Writer.addMember("Data Type", sType);
	Writer.addMember("Texture", sTexture);
	Writer.finishObject();

}

void DzRuntimePluginAction::writeJSON_Property_Texture(DzJsonWriter& Writer, QString sName, double dValue, QString sType, QString sTexture)
{
	Writer.startObject(true);
	Writer.addMember("Name", sName);
	Writer.addMember("Value", dValue);
	Writer.addMember("Data Type", sType);
	Writer.addMember("Texture", sTexture);
	Writer.finishObject();

}


// ------------------------------------------------
// PixelIntensity
// ------------------------------------------------
double DzRuntimePluginAction::getPixelIntensity(const  QRgb& pixel) 
{
	const double r = double(qRed(pixel));
	const double g = double(qGreen(pixel));
	const double b = double(qBlue(pixel));
	const double average = (r + g + b) / 3.0;
	return average / 255.0;
}

// ------------------------------------------------
// MapComponent
// ------------------------------------------------
uint8_t DzRuntimePluginAction::getNormalMapComponent(double pX) 
{
	return (pX + 1.0) * (255.0 / 2.0);
}

// ------------------------------------------------
// intclamp
// ------------------------------------------------
int DzRuntimePluginAction::getIntClamp(int x, int low, int high)
{
	if (x < low) { return low; }
	else if (x > high) { return high; }
	return x;
}

// ------------------------------------------------
// map_component
// ------------------------------------------------
QRgb DzRuntimePluginAction::getSafePixel(const QImage& img, int x, int y) 
{
	int ix = this->getIntClamp(x, 0, img.size().width() - 1);
	int iy = this->getIntClamp(y, 0, img.size().height() - 1);
	return img.pixel(ix, iy);
}

// ------------------------------------------------
// makeNormalMapFromBumpMap
// ------------------------------------------------
QImage DzRuntimePluginAction::makeNormalMapFromBumpMap(const QImage& image, double normalStrength) {
	QImage result = QImage(image.size().width(), image.size().height(), QImage::Format_ARGB32_Premultiplied);
	QRect rect = result.rect();
	int r1 = rect.top();
	int r2 = rect.bottom();
	int c1 = rect.left();
	int c2 = rect.right();
	DzProgress progress = DzProgress(QString("Generating Normalmap"), 100, false, true);
	float step = ((float)(r2 - r1)) / 100.0;
	float current = 0;

	// row loop
	for (int row = r1; row <= r2; row++) {
		current++;
		if (current >= step) {
			progress.step();
			current = 0;
		}

		// col loop
		for (int col = c1; col <= c2; col++) {

			// Pixel Picker
			const QRgb topLeft = this->getSafePixel(image, col - 1, row - 1);
			const QRgb top = this->getSafePixel(image, col, row - 1);
			const QRgb topRight = this->getSafePixel(image, col + 1, row - 1);
			const QRgb right = this->getSafePixel(image, col + 1, row);
			const QRgb bottomRight = this->getSafePixel(image, col + 1, row + 1);
			const QRgb bottom = this->getSafePixel(image, col, row + 1);
			const QRgb bottomLeft = this->getSafePixel(image, col - 1, row + 1);
			const QRgb left = this->getSafePixel(image, col - 1, row);

			// calculating pixel intensities
			const double tl = this->getPixelIntensity(topLeft);
			const double t = this->getPixelIntensity(top);
			const double tr = this->getPixelIntensity(topRight);
			const double r = this->getPixelIntensity(right);
			const double br = this->getPixelIntensity(bottomRight);
			const double b = this->getPixelIntensity(bottom);
			const double bl = this->getPixelIntensity(bottomLeft);
			const double l = this->getPixelIntensity(left);

			// Sobel filter
			const double dX = (tr + 2.0 * r + br) - (tl + 2.0 * l + bl);
			const double dY = 1.0 / normalStrength;
			const double dZ = (bl + 2.0 * b + br) - (tl + 2.0 * t + tr);
			const DzVec3 vec = DzVec3(dX, dY, dZ).normalized();

			// DS uses Y as up, not Z, Normalmaps uses Z
			const QColor color = QColor(this->getNormalMapComponent(vec.m_x), this->getNormalMapComponent(vec.m_z), this->getNormalMapComponent(vec.m_y));
			result.setPixel(col, row, color.rgb());
		}
	}

	return result;
}



#include "moc_DzRuntimePluginAction.cpp"
