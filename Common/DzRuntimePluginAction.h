#pragma once
#include <dzaction.h>
#include <dznode.h>
#include <DzFileIOSettings.h>

#include "QtCore/qfile.h"
#include "QtCore/qtextstream.h"

class UnofficialDzRuntimePluginAction : public DzAction {
	 Q_OBJECT
public:

	 UnofficialDzRuntimePluginAction(const QString& text = QString::null, const QString& desc = QString::null);
	 virtual ~UnofficialDzRuntimePluginAction();

// DB (2021-05-24): hybrid C++/script system
signals:
	void ScriptReturn(QObject* obj);
	void ScriptReturn(bool result);

protected:
	 QString CharacterName;
	 QString ImportFolder;
	 QString CharacterFolder;
	 QString CharacterFBX;
	 QString AssetType;
	 QString MorphString;
	 QString FBXVersion;
	 QMap<QString,QString> MorphMapping;

	 bool ExportMorphs;
	 bool ExportSubdivisions;
	 bool ShowFbxDialog;
	 DzNode* Selection;

	 virtual QString getActionGroup() const { return tr("Bridges"); }
	 virtual QString getDefaultMenuPath() const { return tr("&File/Send To"); }

	 virtual void Export();

	 virtual void WriteConfiguration() = 0;
	 virtual void SetExportOptions(DzFileIOSettings &ExportOptions) = 0;

	 // Need to temporarily rename surfaces if there is a name collision
	 void RenameDuplicateMaterials(DzNode* Node, QList<QString>& MaterialNames, QMap<DzMaterial*, QString>& OriginalMaterialNames);
	 void UndoRenameDuplicateMaterials(DzNode* Node, QList<QString>& MaterialNames, QMap<DzMaterial*, QString>& OriginalMaterialNames);
	 
// DB (2021-05-24): hybrid C++/script system
	 static QObject* m_ScriptReturn_Object;
	 static int m_ScriptReturn_ReturnCode;
private slots:
	void HandleScriptReturn(QObject* obj);
	void HandleScriptReturn(bool result);

};