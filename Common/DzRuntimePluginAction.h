#pragma once
#include <dzaction.h>
#include <dznode.h>
#include <DzFileIOSettings.h>

#include "QtCore/qfile.h"
#include "QtCore/qtextstream.h"

// Struct to remember attachment info
struct AttachmentInfo
{
	DzNode* Parent;
	DzNode* Child;
};

class DzRuntimePluginAction : public DzAction {
	Q_OBJECT
	Q_PROPERTY(QString AssetType READ getAssetType WRITE setAssetType)
	Q_PROPERTY(QString ExportFilename READ getExportFilename WRITE setExportFilename)
	Q_PROPERTY(QString ExportFolder READ getExportFolder WRITE setExportFolder)
	Q_PROPERTY(QString RootFolder READ getRootFolder WRITE setRootFolder)
public:

	 DzRuntimePluginAction(const QString& text = QString::null, const QString& desc = QString::null);
	 virtual ~DzRuntimePluginAction();

protected:
	 QString CharacterName; // Exported filename without extension
	 QString RootFolder; // The destination Root Folder
	 QString DestinationPath; // Path to destination files: <RootFolder> + "/" + <CharacterName (folder)> + "/"
	 QString CharacterFBX;
	 QString CharacterBaseFBX;
	 QString CharacterHDFBX;
	 QString AssetType;
	 QString MorphString;
	 QString FBXVersion;
	 QMap<QString,QString> MorphMapping;
	 QList<QString> PoseList;

	 // Used only by script system
	 QString ExportFolder; // over-rides bridge use of <CharacterName> for the destination folder

	 bool ExportMorphs;
	 bool ExportSubdivisions;
	 bool ExportBaseMesh;
	 bool ShowFbxDialog;
	 bool ExportMaterialPropertiesCSV;
	 DzNode* Selection;

	 virtual QString getActionGroup() const { return tr("Bridges"); }
	 virtual QString getDefaultMenuPath() const { return tr("&File/Send To"); }

	 virtual void Export();
	 virtual void ExportNode(DzNode* Node);

	 virtual void WriteConfiguration() = 0;
	 virtual void SetExportOptions(DzFileIOSettings &ExportOptions) = 0;

	 // Need to temporarily rename surfaces if there is a name collision
	 void RenameDuplicateMaterials(DzNode* Node, QList<QString>& MaterialNames, QMap<DzMaterial*, QString>& OriginalMaterialNames);
	 void UndoRenameDuplicateMaterials(DzNode* Node, QList<QString>& MaterialNames, QMap<DzMaterial*, QString>& OriginalMaterialNames);

	 // Used to find all the unique props in a scene for Environment export
	 void GetScenePropList(DzNode* Node, QMap<QString, DzNode*>& Types);

	 // During Environment export props need to get disconnected as they are exported.
	 void DisconnectNode(DzNode* Node, QList<AttachmentInfo>& AttachmentList);
	 void ReconnectNodes(QList<AttachmentInfo>& AttachmentList);

	 // During Skeletal Mesh Export Disconnect Override Controllers
	 QList<QString> DisconnectOverrideControllers();
	 void ReconnectOverrideControllers(QList<QString>& DisconnetedControllers);
	 QList<QString> ControllersToDisconnect;

	 // For Pose exports check if writing to the timeline will alter existing keys
	 bool CheckIfPoseExportIsDestructive();

	 // Need to be able to move asset instances to origin during environment export
	 void UnlockTranform(DzNode* NodeToUnlock);

	 QString getAssetType() { return this->AssetType; };
	 void setAssetType(QString arg_AssetType) { this->AssetType = arg_AssetType; };
	 QString getExportFilename() { return this->CharacterName; };
	 void setExportFilename(QString arg_Filename) { this->CharacterName = arg_Filename; };

	 QString getExportFolder() { return this->ExportFolder; };
	 void setExportFolder(QString arg_Folder) { this->ExportFolder = arg_Folder; };
	 QString getRootFolder() { return this->RootFolder; };
	 void setRootFolder(QString arg_Root) { this->RootFolder = arg_Root; };

};