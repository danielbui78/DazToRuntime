#pragma once
#include <dzaction.h>
#include <dznode.h>
#include <dzjsonwriter.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <DzRuntimePluginAction.h>
#include "DzUnitySubdivisionDialog.h"
#include "DzUnityMorphSelectionDialog.h"
#include "DzUnityDialog.h"

#include "dzweightmap.h"

class UnofficialDzUnityAction : public UnofficialDzRuntimePluginAction {
	 Q_OBJECT
	 Q_PROPERTY(bool InstallUnityFiles READ getInstallUnityFiles WRITE setInstallUnityFiles)
	 Q_PROPERTY(UnofficialDzUnityDialog* BridgeDialog READ getBridgeDialog)
	 Q_PROPERTY(DzUnitySubdivisionDialog* SubdivisionDialog READ getSubdivisionDialog)
     Q_PROPERTY(DzUnityMorphSelectionDialog* MorphSelectionDialog READ getMorphSelectionDialog)
	 Q_PROPERTY(int NonInteractiveMode READ getNonInteractiveMode WRITE setNonInteractiveMode)

public:
	 UnofficialDzUnityAction();

	class MaterialGroupExportOrderMetaData
	{
	public:
		int materialIndex;
		int vertex_offset;
		int vertex_count;

		MaterialGroupExportOrderMetaData(int a_index, int a_offset)
		{
			materialIndex = a_index;
			vertex_offset = a_offset;
			vertex_count = -1;
		}

		bool operator< (MaterialGroupExportOrderMetaData b) const
		{
			if (vertex_offset < b.vertex_offset)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

	 };

	void setInstallUnityFiles(bool arg) { InstallUnityFiles = arg; }
	bool getInstallUnityFiles() { return InstallUnityFiles; }
	UnofficialDzUnityDialog* getBridgeDialog() { return BridgeDialog; }
	DzUnitySubdivisionDialog* getSubdivisionDialog() { return SubdivisionDialog; }
	DzUnityMorphSelectionDialog* getMorphSelectionDialog() { return MorphSelectionDialog; }
	void setNonInteractiveMode(int arg) { NonInteractiveMode = arg; }
	int getNonInteractiveMode() { return NonInteractiveMode; }

protected:
	 int NonInteractiveMode;
	 UnofficialDzUnityDialog* BridgeDialog;
	 DzUnityMorphSelectionDialog* MorphSelectionDialog;
	 DzUnitySubdivisionDialog* SubdivisionDialog;
	 bool InstallUnityFiles;

	 void executeAction();
	 Q_INVOKABLE bool UpgradeToHD(QString baseFilePath, QString hdFilePath, QString outFilePath, std::map<std::string, int>* pLookupTable);
	 Q_INVOKABLE bool CreateUI();
	 Q_INVOKABLE void WriteMaterials(DzNode* Node, DzJsonWriter& Stream);
	 Q_INVOKABLE void WriteWeightMaps(DzNode* Node, DzJsonWriter& Stream);
	 Q_INVOKABLE void WriteConfiguration();
	 Q_INVOKABLE void SetExportOptions(DzFileIOSettings& ExportOptions);
	 Q_INVOKABLE void CreateUnityFiles(bool replace = true);

	 bool metaInvokeMethod(QObject* object, const char* methodSig, void** returnPtr);
	 Q_INVOKABLE DzWeightMapPtr getWeightMapPtr(DzNode* Node);

	 Q_INVOKABLE bool CopyFile(QFile *file, QString *dst, bool replace = true, bool compareFiles = true);
	 Q_INVOKABLE QString GetMD5(const QString &path);
};
