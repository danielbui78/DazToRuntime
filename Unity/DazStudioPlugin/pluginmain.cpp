#include "dzplugin.h"
#include "dzapp.h"

#include "version.h"
#include "DzUnityAction.h"
#include "DzUnityDialog.h"

DZ_PLUGIN_DEFINITION("Unofficial DTU Bridge - experimental");

DZ_PLUGIN_AUTHOR("danielbui78");

DZ_PLUGIN_VERSION(PLUGIN_MAJOR, PLUGIN_MINOR, PLUGIN_REV, PLUGIN_BUILD);

DZ_PLUGIN_DESCRIPTION(QString("Export assets from Daz Studio To Unity Engine."));

DZ_PLUGIN_CLASS_GUID(UnofficialDzUnityAction, 34b4d340-6e80-4b59-9db4-9551c6cd7146);