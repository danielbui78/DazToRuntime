#include "dzplugin.h"
#include "dzapp.h"

#include "version.h"
#include "DzUnrealAction.h"
#include "DzUnrealDialog.h"

DZ_PLUGIN_DEFINITION("DazToUnreal");

DZ_PLUGIN_AUTHOR("Daz 3D, Inc");

DZ_PLUGIN_VERSION(PLUGIN_MAJOR, PLUGIN_MINOR, PLUGIN_REV, PLUGIN_BUILD);

DZ_PLUGIN_DESCRIPTION(QString(
"<b>Pre-Release DazToUnreal Bridge v%1.%2.%3.%4 </b><br>\
Bridge Collaboration Project<br><br>\
<a href = \"https://github.com/danielbui78-bridge-collab/DazToRuntime/tree/unreal-main\">Github</a><br><br>"
).arg(PLUGIN_MAJOR).arg(PLUGIN_MINOR).arg(PLUGIN_REV).arg(PLUGIN_BUILD));

DZ_PLUGIN_CLASS_GUID(DzUnrealAction, 99F42CAE-CD02-49BC-A7CE-C0CF4EDD7609);
