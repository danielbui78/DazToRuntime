#include "dzplugin.h"
#include "dzapp.h"

#include "version.h"
#include "DzUnityAction.h"
#include "DzUnityDialog.h"
#include "OpenFBXInterface.h"

DZ_PLUGIN_DEFINITION("Unofficial DTU Bridge");

DZ_PLUGIN_AUTHOR("danielbui78");

DZ_PLUGIN_VERSION(PLUGIN_MAJOR, PLUGIN_MINOR, PLUGIN_REV, PLUGIN_BUILD);

DZ_PLUGIN_DESCRIPTION(QString("Export assets from Daz Studio To Unity Engine."));

DZ_PLUGIN_CLASS_GUID(UnofficialDzUnityAction, 34b4d340-6e80-4b59-9db4-9551c6cd7146);
DZ_PLUGIN_CUSTOM_CLASS_GUID(UnofficialDzUnityDialog, 588ff176-4a32-444d-8134-67b35332351d);
DZ_PLUGIN_CUSTOM_CLASS_GUID(DzUnityMorphSelectionDialog, a438f76c-e9a4-4989-8bc5-cbc7203c7d6b);
DZ_PLUGIN_CUSTOM_CLASS_GUID(DzUnitySubdivisionDialog, 04056c67-fbb3-4ae5-b768-06796cff4894);
DZ_PLUGIN_CLASS_GUID(OpenFBXInterface, 63b7a93e-182f-410d-9ef4-3cb4067a48eb);


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

QObject* UnofficialDzUnityDialogFactory::createInstance(const QVariantList& args) const
{
	QWidget* parent = GetParentArg0(args);
	return (QObject*) new UnofficialDzUnityDialog(parent);
}
QObject* UnofficialDzUnityDialogFactory::createInstance() const
{
	return (QObject*) new UnofficialDzUnityDialog();
}

QObject* DzUnityMorphSelectionDialogFactory::createInstance(const QVariantList& args) const
{
	QWidget* parent = GetParentArg0(args);
	return (QObject*) new DzUnityMorphSelectionDialog(parent);
}
QObject* DzUnityMorphSelectionDialogFactory::createInstance() const
{
	return (QObject*) new DzUnityMorphSelectionDialog();
}

QObject* DzUnitySubdivisionDialogFactory::createInstance(const QVariantList& args) const
{
	QWidget* parent = GetParentArg0(args);
	return (QObject*) new DzUnitySubdivisionDialog(parent);
}
QObject* DzUnitySubdivisionDialogFactory::createInstance() const
{
	return (QObject*) new DzUnitySubdivisionDialog();
}
