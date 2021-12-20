#pragma once
#include <dzaction.h>
#include <dznode.h>
#include <dzgeometry.h>
#include <dzfigure.h>
#include <dzjsonwriter.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QUuid.h>
#include <DzRuntimePluginAction.h>
#include "DzUnrealSubdivisionDialog.h"

class DzUnrealAction : public DzRuntimePluginAction {
	Q_OBJECT
    Q_PROPERTY(int NonInteractiveMode READ getNonInteractiveMode WRITE setNonInteractiveMode)
public:
	 DzUnrealAction();

protected:
	 int Port;
     int NonInteractiveMode;
	 DzUnrealSubdivisionDialog* SubdivisionDialog;

	 void executeAction();
	 void WriteMaterials(DzNode* Node, DzJsonWriter& Writer, QTextStream& Stream);
	 void WriteInstances(DzNode* Node, DzJsonWriter& Writer, QMap<QString, DzMatrix3>& WritenInstances, QList<DzGeometry*>& ExportedGeometry, QUuid ParentID = QUuid());
	 QUuid WriteInstance(DzNode* Node, DzJsonWriter& Writer, QUuid ParentID);
	 void WriteConfiguration();
	 void SetExportOptions(DzFileIOSettings& ExportOptions);
     int getNonInteractiveMode();
     void setNonInteractiveMode(int);

};
