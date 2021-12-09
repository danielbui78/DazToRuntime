#include "dzplugin.h"
#include "dzapp.h"

#include "version.h"
#include "DzUnityAction.h"
#include "DzUnityDialog.h"
#include "OpenFBXInterface.h"

DZ_PLUGIN_DEFINITION("Unofficial DTU Bridge");

DZ_PLUGIN_AUTHOR("danielbui78");

DZ_PLUGIN_VERSION(PLUGIN_MAJOR, PLUGIN_MINOR, PLUGIN_REV, PLUGIN_BUILD);

DZ_PLUGIN_DESCRIPTION(QString(
"Export assets from Daz Studio To Unity Engine.<br>\n\
<br>\n\
<br>\n\
This product is based on software by DAZ 3D, Inc.Copyright 2002 - 2021 DAZ 3D, Inc., used under modified Apache 2.0 License.All rights reserved.<br>\n\
<br>\n\
This software contains Autodesk® FBX® code developed by Autodesk, Inc.Copyright 2019 Autodesk, Inc.All rights, reserved.Such code is provided “as is”and Autodesk, Inc.disclaims anyand all warranties, whether express or implied, including without limitation the implied warranties of merchantability, fitness for a particular purpose or non - infringement of third party rights.In no event shall Autodesk, Inc.be liable for any direct, indirect, incidental, special, exemplary, or consequential damages(including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort(including negligence or otherwise) arising in any way out of such code.<br>\n\
<br>\n\
It also uses the following libraries :<br>\n\
Qt 4.8, Copyright 2015 The Qt Company Ltd.All rights reserved.<br>\n\
<br>\n\
OpenSubdiv, Copyright 2013 Pixar.All rights reserved.<br>\n\
<br>\n"
));

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
