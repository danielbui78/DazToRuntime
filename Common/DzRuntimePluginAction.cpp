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

#include <QtCore/qdir.h>
#include <QtGui/qlineedit.h>
#include <QtNetwork/qudpsocket.h>
#include <QtNetwork/qabstractsocket.h>
#include <QtGui/qcheckbox.h>


#include "DzRuntimePluginAction.h"

QObject* UnofficialDzRuntimePluginAction::m_ScriptReturn_Object;
int UnofficialDzRuntimePluginAction::m_ScriptReturn_ReturnCode;


UnofficialDzRuntimePluginAction::UnofficialDzRuntimePluginAction(const QString& text, const QString& desc) :
	 DzAction(text, desc)
{
	 ExportMorphs = false;
	 ExportSubdivisions = false;
	 ShowFbxDialog = false;

	 connect(this, SIGNAL(ScriptReturn(QObject*)), this, SLOT(HandleScriptReturn(QObject*)));
	 connect(this, SIGNAL(ScriptReturn(bool)), this, SLOT(HandleScriptReturn(bool)));

}

UnofficialDzRuntimePluginAction::~UnofficialDzRuntimePluginAction()
{
}

void UnofficialDzRuntimePluginAction::Export()
{
	 // FBX Export
	 Selection = dzScene->getPrimarySelection();
	 if (!Selection)
		  return;

	 DzExportMgr* ExportManager = dzApp->getExportMgr();
	 DzExporter* Exporter = ExportManager->findExporterByClassName("DzFbxExporter");

	 if (Exporter)
	 {
		 bool bDoMaterials = false;
		 bool bDoAnimation = false;


		  DzFileIOSettings ExportOptions;
		  ExportOptions.setBoolValue("doSelected", true);
		  ExportOptions.setBoolValue("doVisible", false);
		  if (AssetType == "SkeletalMesh" || AssetType == "StaticMesh")
		  {
				ExportOptions.setBoolValue("doFigures", true);
				ExportOptions.setBoolValue("doProps", true);
				bDoMaterials = true;
		  }
		  else
		  {
				ExportOptions.setBoolValue("doFigures", true);
				ExportOptions.setBoolValue("doProps", false);
		  }
		  if (AssetType == "Animation")
		  {
			  bDoAnimation = true;
		  }
		  ExportOptions.setBoolValue("doLights", false);
		  ExportOptions.setBoolValue("doCameras", false);
		  ExportOptions.setBoolValue("doAnims", false);
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

		  ExportOptions.setStringValue("format", FBXVersion);
		  ExportOptions.setIntValue("RunSilent", !ShowFbxDialog);

		  ExportOptions.setBoolValue("doEmbed", true);
		  ExportOptions.setBoolValue("doCopyTextures", true);
		  ExportOptions.setBoolValue("doDiffuseOpacity", true);
		  ExportOptions.setBoolValue("doMergeClothing", true);
		  ExportOptions.setBoolValue("doStaticClothing", false);
		  ExportOptions.setBoolValue("degradedSkinning", true);
		  ExportOptions.setBoolValue("degradedScaling", true);
		  ExportOptions.setBoolValue("doSubD", false);
		  ExportOptions.setBoolValue("doCollapseUVTiles", false);
		  ExportOptions.setBoolValue("doLocks", true);
		  ExportOptions.setBoolValue("doLimits", true);

		  ////////////
		  // Prepare for Material exports (Skeleton and Static Mesh)
		  ////////////
		  DzNode* Parent = Selection;
		  QList<QString> MaterialNames;
		  QMap<DzMaterial*, QString> OriginalMaterialNames;
		  if (bDoMaterials)
		  {
			  // get the top level node for things like clothing so we don't get dupe material names
			  while (Parent->getNodeParent() != NULL)
			  {
				  Parent = Parent->getNodeParent();
			  }
			  // rename duplicate material names
			  RenameDuplicateMaterials(Parent, MaterialNames, OriginalMaterialNames);
		  }

		  ///////////////
		  // Prepare Animation exports
		  ///////////////
		  DzNodeList undoHideList;
		  if (bDoAnimation)
		  {
			  // correct CharacterFolder
			  CharacterFolder = ImportFolder + QDir::separator() + CharacterName.left(CharacterName.indexOf("@")) + QDir::separator();
			  // correct animation filename
			  CharacterFBX = CharacterFolder + CharacterName + ".fbx";
			  // correct export options
			  ExportOptions.setBoolValue("doAnims", true);
			  // disable props, morphs, subD, textures, etc
			  ExportOptions.setBoolValue("doVisible", true);
			  ExportOptions.setBoolValue("doMergeClothing", false);
			  ExportOptions.setBoolValue("doEmbed", false);
			  ExportOptions.setBoolValue("doCopyTextures", false);
			  ExportOptions.setBoolValue("doDiffuseOpacity", false);
			  ExportOptions.setBoolValue("doSubD", false);
			  ExportOptions.setBoolValue("doCollapseUVTiles", false);
			  ExportOptions.setBoolValue("doLocks", false);
			  ExportOptions.setBoolValue("doLimits", false);

			  // Loop through and Hide all visible geometry, putting it into list to unhide
			  DzNodeListIterator nodeIterator = dzScene->nodeListIterator();
			  nodeIterator.toFront();
			  while (nodeIterator.hasNext())
			  {
				  DzNode *node = nodeIterator.next();
				  if (node == Selection)
					  continue;
				  if (node->isVisible())
				  {
					  undoHideList.append(node);
					  node->setVisible(false);
				  }
			  }			  
		  }


		  /////////////////////
		  // START: Export FBX
		  ///////////////////////
		  QDir dir;
		  dir.mkpath(CharacterFolder);

		  // Unnecessary, all code is already done above
//		  SetExportOptions(ExportOptions);

		  Exporter->writeFile(CharacterFBX, &ExportOptions);

		  WriteConfiguration();
		  ////////////////////////
		  // DONE: Export FBX
		  ////////////////////////

		  //////////////////////
		  // Undo Export Preparations
		  //////////////////////
		  if (bDoAnimation)
		  {
			  // loop through list to undo hidden geometry
			  while (undoHideList.isEmpty() == false)
			  {
				  DzNode *node = undoHideList.first();
				  node->setVisible(true);
				  undoHideList.pop_front();
			  }

		  }
		  if (bDoMaterials)
		  {
			  // Change back material names
			  UndoRenameDuplicateMaterials(Parent, MaterialNames, OriginalMaterialNames);
		  }

	 } // if (Exporter)
}

// If there are duplicate material names, save off the original and rename one
void UnofficialDzRuntimePluginAction::RenameDuplicateMaterials(DzNode* Node, QList<QString>& MaterialNames, QMap<DzMaterial*, QString>& OriginalMaterialNames)
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
void UnofficialDzRuntimePluginAction::UndoRenameDuplicateMaterials(DzNode* Node, QList<QString>& MaterialNames, QMap<DzMaterial*, QString>& OriginalMaterialNames)
{
	 QMap<DzMaterial*, QString>::iterator iter;
	 for (iter = OriginalMaterialNames.begin(); iter != OriginalMaterialNames.end(); ++iter)
	 {
		  iter.key()->setName(iter.value());
	 }
}


// DB (2021-05-24): hybrid C++/script system
void UnofficialDzRuntimePluginAction::HandleScriptReturn(bool result)
{
	m_ScriptReturn_Object = NULL;
	if (result == false)
	{
		m_ScriptReturn_ReturnCode = -1;
	}
	else
	{
		m_ScriptReturn_ReturnCode = 1;
	}
}

// DB (2021-05-24): hybrid C++/script system
void UnofficialDzRuntimePluginAction::HandleScriptReturn(QObject* object)
{
	if (object == NULL)
	{
		m_ScriptReturn_Object = NULL;
		m_ScriptReturn_ReturnCode = -1;
	}
	else
	{
		m_ScriptReturn_Object = object;
		m_ScriptReturn_ReturnCode = 1;
	}
}

#include "moc_DzRuntimePluginAction.cpp"
