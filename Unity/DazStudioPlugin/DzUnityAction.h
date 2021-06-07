#pragma once
#include <dzaction.h>
#include <dznode.h>
#include <dzjsonwriter.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <DzRuntimePluginAction.h>
#include "DzUnitySubdivisionDialog.h"

#include "dzweightmap.h"

class UnofficialDzUnityAction : public UnofficialDzRuntimePluginAction {
	 Q_OBJECT
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

		bool operator< (MaterialGroupExportOrderMetaData b)
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

protected:
	 DzUnitySubdivisionDialog* SubdivisionDialog;
	 bool InstallUnityFiles;

	 void executeAction();
	 void WriteMaterials(DzNode* Node, DzJsonWriter& Stream);
	 void WriteWeightMaps(DzNode* Node, DzJsonWriter& Stream);
	 void WriteConfiguration();
	 void SetExportOptions(DzFileIOSettings& ExportOptions);
	 void CreateUnityFiles(bool replace = true);

	 bool metaInvokeMethod(QObject* object, const char* methodSig, void** returnPtr);
	 DzWeightMapPtr getWeightMapPtr(DzNode* Node);

	 bool CopyFile(QFile *file, QString *dst, bool replace = true, bool compareFiles = true);
	 QString GetMD5(const QString &path);
};