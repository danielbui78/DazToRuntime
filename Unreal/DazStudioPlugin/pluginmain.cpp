#include "dzplugin.h"
#include "dzapp.h"

#include "version.h"
#include "DzUnrealAction.h"
#include "DzUnrealDialog.h"
#include "DzBridgeMorphSelectionDialog.h"
#include "DzBridgeSubdivisionDialog.h"

DZ_PLUGIN_DEFINITION("DazToUnreal");

DZ_PLUGIN_AUTHOR("Daz 3D, Inc");

DZ_PLUGIN_VERSION(PLUGIN_MAJOR, PLUGIN_MINOR, PLUGIN_REV, PLUGIN_BUILD);

DZ_PLUGIN_DESCRIPTION(QString(
"<b>Pre-Release DazToUnreal Bridge v%1.%2.%3.%4 </b><br>\
Bridge Collaboration Project<br><br>\
<a href = \"https://github.com/danielbui78-bridge-collab/DazToRuntime/tree/unreal-main\">Github</a><br><br>"
).arg(PLUGIN_MAJOR).arg(PLUGIN_MINOR).arg(PLUGIN_REV).arg(PLUGIN_BUILD));

DZ_PLUGIN_CLASS_GUID(DzUnrealAction, 99F42CAE-CD02-49BC-A7CE-C0CF4EDD7609);
DZ_PLUGIN_CUSTOM_CLASS_GUID(DzUnrealDialog, b7c0b573-bd61-452c-92c1-9560459b4e89);
DZ_PLUGIN_CUSTOM_CLASS_GUID(DzBridgeMorphSelectionDialog, 321916ba-0bcc-45d9-8c7e-ebbe80dea51c);
DZ_PLUGIN_CUSTOM_CLASS_GUID(DzBridgeSubdivisionDialog, a2342e17-db3b-4032-a576-75b5843fa893);

static QWidget* GetParentArg0(const QVariantList& args)
{
	QWidget* parent = nullptr;
	QVariant qvar = args[0];
	QObject* obj = qvar.value<QObject*>();
	if (obj && obj->inherits("QWidget"))
	{
		parent = (QWidget*)obj;
	}

	return parent;
}

QObject* DzUnrealDialogFactory::createInstance(const QVariantList& args) const
{
	QWidget* parent = GetParentArg0(args);
	return (QObject*) new DzUnrealDialog(parent);
}
QObject* DzUnrealDialogFactory::createInstance() const
{
	return (QObject*) new DzUnrealDialog(nullptr);
}

QObject* DzBridgeMorphSelectionDialogFactory::createInstance(const QVariantList& args) const
{
	QWidget* parent = GetParentArg0(args);
	return (QObject*) new DzBridgeMorphSelectionDialog(parent);
}
QObject* DzBridgeMorphSelectionDialogFactory::createInstance() const
{
	return (QObject*) new DzBridgeMorphSelectionDialog(nullptr);
}

QObject* DzBridgeSubdivisionDialogFactory::createInstance(const QVariantList& args) const
{
	QWidget* parent = GetParentArg0(args);
	return (QObject*) new DzBridgeSubdivisionDialog(parent);
}
QObject* DzBridgeSubdivisionDialogFactory::createInstance() const
{
	return (QObject*) new DzBridgeSubdivisionDialog(nullptr);
}
