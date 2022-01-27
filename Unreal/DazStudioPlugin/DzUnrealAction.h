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

class DzUnrealDialog;

class DzUnrealAction : public DzRuntimePluginAction {
	Q_OBJECT
	Q_PROPERTY(QWidget* wBridgeDialog READ getBridgeDialog WRITE setBridgeDialog)
public:
	 DzUnrealAction();

	 DzUnrealDialog* getBridgeDialog() { return BridgeDialog; }
	 void setBridgeDialog(QWidget* arg_dlg) { BridgeDialog = qobject_cast<DzUnrealDialog*>(arg_dlg); }
	 Q_INVOKABLE void resetToDefaults();

protected:
	 int Port;
	 DzUnrealSubdivisionDialog* SubdivisionDialog;
	 DzUnrealDialog *BridgeDialog;

	 void executeAction();
	 void WriteMaterials(DzNode* Node, DzJsonWriter& Writer, QTextStream& Stream);
	 void WriteInstances(DzNode* Node, DzJsonWriter& Writer, QMap<QString, DzMatrix3>& WritenInstances, QList<DzGeometry*>& ExportedGeometry, QUuid ParentID = QUuid());
	 QUuid WriteInstance(DzNode* Node, DzJsonWriter& Writer, QUuid ParentID);
	 Q_INVOKABLE void WriteConfiguration();
	 void SetExportOptions(DzFileIOSettings& ExportOptions);

};
