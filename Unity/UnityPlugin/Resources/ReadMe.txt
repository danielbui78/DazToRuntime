Unofficial DTU Bridge version 1.2

This is an unofficial update project for the opensource Daz To Unity Bridge released by Daz3D.  It contains a number of bugfixes and feature additions not yet incorporated in the official Daz To Unity project.

New in this version:
====================
- Bugfixed crash when exporting subdivisions for more than one figure in a single session.
- Initial refactoring for planned open-source FBX and OpenSubdiv Daz-plugins.
- Updated CMakeFiles for more convenient building of FBX/OpenSubdiv/Mac support.
- Full MacOS support on versions 10.9 to 11.
- (Baked) Subdivision support, up to Subdivision Level 4.  Based on https://github.com/cocktailboy/DazToRuntime implementation.
- FBX SDK and OpenSubDiv are static linked into the Unofficial DTU Bridge and requires both libraries to build the Daz Plugin.
- OpenSubDiv is used under a Modified Apache License 2.0: https://github.com/PixarAnimationStudios/OpenSubdiv/blob/release/LICENSE.txt
- FBX SDK is used under the Autodesk® FBX® SDK 2020 license: https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2

Known Issues:
=============
- Subdivision procedure is currently only single-threaded CPU-based and may take several minutes to bake Subdivision level 3 and 4.
- dForce strand-based hair is not yet supported.
- dForce clothing with high poly counts will cause heavy performance slowdowns in Unity.
- Geograft morphs are not yet supported.
- Genesis 8/8.1 prop rigging is not yet 100% accurately converted.
- Identical / duplicate materials are not yet detected for merger in Unity.
- Cloth physics colliders are not yet resized to Daz figures.
- Daz dForce weight maps are not yet converted properly to Unity Cloth physics weight maps.

macOS Installation Instructions:
==========================
Unzip the "libunofficialdzunitybridge.dylib.v1_2.zip" file and move the "libunofficialdzunitybridge.dylib" file into the plugins folder of Daz Studio.  The plugin folder is found in the same folder containing the main Daz Studio application icon. The bridge can then be accessed from the main menu: File->Send To->Unofficial DTU. The embedded Unity plugin can be installed with the "Install Unity Files" option, just like the official DazToUnity Bridge.  Once the Unity plugin is installed, be sure to follow directions to add the IrayUberSkinDiffusionProfile if you are using HDRP.


Windows Installation Instructions:
==========================
The "unofficialdzunitybridge_v1_2.7z" file should be unzipped and the "unofficialdaztounitybridge.dll" file should be placed in the plugins folder of Daz Studio (example: "\Daz 3D\Applications\64-bit\DAZ 3D\DAZStudio4"). Daz Studio can then be started, and the bridge can be accessed from the main menu: File->Send To->Unofficial DTU. The embedded Unity plugin can be installed with the "Install Unity Files" option, just like the official DazToUnity Bridge.  Once the Unity plugin is installed, be sure to follow directions to add the IrayUberSkinDiffusionProfile if you are using HDRP.


HDRP-ONLY: Adding HDRP Diffusion Profile (aka subsurface-scattering):
==========================
In order to use the HDRP Daz skin shaders, you must manually add the IrayUberSkinDiffusionProfile to the Diffusion Profile List.  Until this is done, materials using the HDRP Daz skin shader will have a Green Tint.
Unity 2019: This list is found in the Material section of each HD RenderPipeline Asset, which can be found in the Quality->HDRP panel of the Project Settings dialog.
Unity 2020 and above: This list is found at the bottom of the HDRP Default Settings panel in the Project Settings dialog.


To Test New Shader support:
==========================
1. In the Unity Editor, select main menu "Daz3D->Open DazToUnity Bridge Window".
2. The DTU Bridge window should open.  Click the "Options" tab.
3. Check the box next to "Use New Shaders (experimental)".
4a. Go to Daz and import your figure as usual
OR
4b. To update an already imported figure to use the New Shaders: In the Project pane, find the folder for your previously imported Daz figure. Right-click the DTU file and select "Reimport".


To Test Subdivision support:
=========================
1. Load Daz Figure.
2. Select "File->Send To->Unofficial DTU".  The "Unofficial DTU Bridge" main dialog window should appear.
3. If the "Enable Subdivision" checkbox is unchecked, click to place a checkmark.
4. Click "Choose Subdivisions". The "Choose Subdivision Levels" dialog window should appear.
5. Set the desired Subdivision Level for each Object/Figure by clicking the corresponding "Subdivision Level" drop-down menu and selecting a subdivision level from 0 to 4.
6. Click Accept for the "Choose Subdivision Levels" dialog.
7. Click Accept for the "Unofficial DTU Bridge" dialog.


To Test dForce Clothing to Unity Cloth Physics support:
=======================================================
*Make sure you save your DazStudio project before taking these steps, some will Not revert with Undo command.*
*Preparation:*
0. In the Unity Editor, open the Unofficial DTU Bridge window.  Go to the Options tab.  Make sure the "Enable dForce Support" option is checked on.  Close the Unofficial DTU Bridge window.  Switch to Daz Studio.
1. Load dForce compliant clothing onto your figure.
2. Clear any dForce simulation calculations in the scene by selecting the Simulations Settings Pane and clicking "Clear".  Warning: you will NOT be able to undo this procedure.
3. Select the Surfaces Pane and select the clothing surface materials.  For each material, double-check the Simulation Properties Tab to make sure that Dynamics Strength is set correctly: a value of 1.00 will often cause clothing to fall off or explode in Unity.  Try setting this to 0.5 if this happens.
4. Recommend setting one Material's Dynamics Strength to 0.2 or less, to act as an anchor for the rest of the clothing -- example: waist-line of pants and skirts, neck-line of shirts and jackets.
5. With dForce Clothing still selected, add a Push Modifier via the "Edit->Object->Geometry->Add Push Modifier..." menu command.  This will add a new property in the Parameters Pane called "PushModifier" in the "Mesh Offset" Property Group.  Try 0.1 as an initial value.  It can be increased if you are having cloth collision issues such as clothes exploding.
6. Select "File->Send To->Unofficial Daz to Unity Bridge".
7. Open Unity Editor window and wait for DTU import to complete and Prefab to be created in the Scene.

*Cloth Colliders:*
1. In the Scene Pane, open the imported prefab and select the "Cloth Collision Rig".  In the Inspector Pane, you will see the "Cloth Collision Assigner" Script Component.
2. There are three sections that can be expanded: "Upperbody Colliders" contains a list of *paired* sphere colliders for the upper body.  "Lowerbody Colliders" contains a similar list of *paired* sphere colliders for the lower body.  *"Paired" sphere colliders* are a two sphere colliders that joined together by the Cloth Physics component into a collision cylinder with differing sized round ends.  This way, you can make custom shapes for limbs, torso and other body parts.
3. You may click on each sphere collider to access the specific sphere collider component within the animation skeleton.  You can then adjust the position or size of the collider.
4. The third section is "Cloth Configuration List".  Expand this section and you will see a list of all imported dForce items that have been converted to use Unity Cloth Physics components.  Select each item and uncheck "Upper Body" or "Lower Body" to disable any cloth collision skeleton parts that are unneeded by that clothing.
5. Double click on a Cloth component here will take you to the clothing object that it controls.  Do this for each one: Find the Cloth component and check the Sphere Colliders section.  You will notice that this is empty.  It is filled at runtime (when the project is Played) by the "Cloth Collision Assigner" Script.  If you want the list to be populated at Edit-Time, then during runtime, click on the three dots of the component and select "Copy Component".  Then stop the project and click the three dots again, now select "Paste Component Values".

*Cloth Physics Weight Maps:*
1. Select dforce clothing in the Scene Pane and you should see the Cloth Physics Component in the Inspector Pane.
2. Below the Cloth Physics Component is a new "Cloth Tools" component that contains convenience features for working with cloth physics weight maps.
3. The first section lists all the material groups within the current dforce item.  Each one contains a slider and number textfield and a "Set" and "Clear" button.
4. Use the slider or textfield to enter a decimal number then click "Set" to assign that number as the "Max Distance" to all vertices within that material group.  Note: "Max Distance" is the maximum distance a vertex can travel from its original position due to cloth physics and is measured in Unity worldspace units.  That means a value of "1.0" will let a vertex move up to 1 meter in the world.  The slider will move up to 0.2 units (20 centimeters) but the textfield can be used to enter a value of any size.
5. Click "Clear" to remove all constraints, represented by a black sphere.  This will let the vertex travel an infinite distance from its original position due to cloth physics.
6. "Load Weightmap data" can be used to load Unity Weight Maps or Dforce Weight Maps.  You can use the drop down menu in the load file dialog to choose which file types to filter or to see all files.  The Unity Weight Maps are raw binary dumps of the floating point information for MaxDistance values of the selected item.  The Dforce Weight Maps are raw binary dumps of the unsigned shorts (16bit) information from the Daz Studio "Dforce Weight Node Modifier / Influence Maps".  Dforce Weight map files are automatically created if Daz item contains a Dforce weightmap.
*Note*: In Daz Studio, this value is multiplied by the Dynamics Strength for the corresponding material group.  However, the imported Unity value is not currently multiplied by the corresponding material group because the vertex ordering of the raw Dforce weight map has not yet been decoded.
7. "Save Weightmap data" will popup the save file dialog to let you save the current weightmap in a Unity-compatible Weight Map format.
8. "Load Gradient Pattern" will create a gradient pattern from 0.0 to 1.0 from index 0 of the weightmap to the last index of the weightmap.
9. "Zero All Weights" will set the entire weightmap to 0.0.
10. "Clear All Weights" will remove all constraints from the entire weightmap and allow infinite distance to be travelled by cloth physics.

Tips: if clothing falls off or explodes, try decreasing the Dynamics Strength inside Daz Studio and/or increasing the Push Modifier Offset.

Change Log:
===========
Version 1.1:
- MacOS filesystem support.
- Experimental uDTU shaders added for HDRP and URP, made from refactored and unified shadersubgraph codebase.
   - "Use New Shaders" option added to DTU Bridge Options panel (disabled by default).
   - Translucency Map, SSS, Dual Lobe Specular, Glossy Specular, Specular Strength, Top Coat implemented.
   - URP Transparency support via URP-Transparent shading mode.
   - Dual Lobe Specular and Glossy Specular simultaneously supported in all shading modes (SSS, Metallic, Specular, URP-Transparent).
   - Metallic emulation implemented in Specular and URP-Transparent shading modes.
   - SSS supported for all non-transparent materials (previously only Skin).
- Fixed Alpha Clip Threshold bug in URP: affected depth-testing, especially hair.
- Glossy Anisotropy, Roughness and Weight fixes.
- "eyelash" material assigned to Hair shader.
- Changed window titles to "uDTU".

Version 1.0:
- Bugfix: Imported asset files with different hash values are appropriately overwritten.
- Bugfix: Emission strength values are properly set for IrayUber materials.
- Bugfix: Emission Color now working for URP and Built-in RenderPipeline.
- "Enable dForce" checkbox added to Options tab of DTU Bridge window.
- RenderPipeline Detection procedure will ask to confirm Symbol Definition updates before proceeding.
- Notification windows will popup when Daz Export and Unity Import steps are complete.
- Daz Studio Subdivision settings are restored after Send To operation.
- Changed plugin name and window titles to "Unofficial DTU Bridge".

Version 0.5-alpha:
- Smoother Unity Files installation with automatic dialog popup, RP detection and proper importing of first asset.
- UI tweaks such as Daz3D menu command order, Install/Overwrite Unity Files checkbox.
- New Unity Cloth Tools component to bulk edit weights by material groups, save/load weight maps.
- Optimized HDRP and URP shadergraphs to use a single Sampler node.

Version 0.4-alpha:
- Preliminary support for dForce clothing, cloth physics export (only via the Simulation Properties of the Surfaces Pane of DazStudio).
- Pregenerated cloth collision skeleton which is automatically merged into animation skeleton of Prefabs created by the Bridge.

Version 0.3-alpha:
- Animation exporting is now enabled through the Animation asset type and disabled when exporting Skeletal or Static Mesh.
- Timeline animations are exported with sequentially numbered "@anim0000.fbx" filenames, which increment with each export operation.
- Reverted/Fixed issue caused by removal of .meta files in v0.2-alpha which can lead to problems with mismatched GUID files when upgrading the unity plugin.

Version 0.1-alpha:
- Combined support for all three rendering pipelines and an autodetection/configuration system.


Copyright Notice:
==========
This product is based on software by DAZ 3D, Inc. Copyright 2002-2021 DAZ 3D, Inc., used under modified Apache 2.0 License.  All rights reserved.

This software contains Autodesk® FBX® code developed by Autodesk, Inc. Copyright 2019 Autodesk, Inc. All rights, reserved. Such code is provided “as is” and Autodesk, Inc. disclaims any and all warranties, whether express or implied, including without limitation the implied warranties of merchantability, fitness for a particular purpose or non-infringement of third party rights. In no event shall Autodesk, Inc. be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of such code.

It also uses the following libraries:
Qt 4.8, Copyright 2015 The Qt Company Ltd.  All rights reserved.

OpenSubdiv, Copyright 2013 Pixar.  All rights reserved.

----------------------------------------------------------------------------------------------
End of Unofficial DTU Bridge ReadMe
----------------------------------------------------------------------------------------------


==================================================================
Original ReadMe for the official Daz To Unity Bridge, version 1.0:
==================================================================

Daz To Unity Bridge version 1.0

== Description ==
The Daz To Unity Bridge consists of two major parts:
1) a plug-in for Daz3D Studio responsible for exporting models and related data, and
2) a set of UnityEditor scripts (C#) which consume the exported data.

== Setup ==
No setup is required in Unity Editor.
The Daz to Unity plugin will write all the neccessary files (scripts, profiles, export data) into each targeted Unity project.
The import process is automatic.

== Workflow ==
Daz3D Studio - Have the root of one model selected. A menu command File >> Send To >> Daz to Unity summons a dialog, in which you specify the target Unity Project folder and other options.
Unity Editor - Once the export has completed, put the Unity Editor app back into focus in Windows. The importer scripts will activate automatically and begin importing fresh export data.
The first import takes longer than subsequent imports, because the Editor must compile shaders first.
The first import of a given asset takes longer because textures are often copied across from Daz3D studio, and these take time to import.

== Folders ==
The first time an export happens, a folder tree is written into your Unity project.
Assets\Daz3D is the parent folder; the Daz to Unity Bridge only deals with files under \Daz3D\
Exports data are written to subfolders of \Daz3D\ and bear the name of the exported model.

== Files ==
FBX - Much of the model data (skeletal hierarchy, weighted mesh, material mappings, lights, cameras, etc.) are conveyed in FBX format. These FBX files are constructed by Daz3D's built-in FBX exporter, and maintain the same variety of options and level of fidelity as a routine FBX export.
DTU - The Bridge plug-in also writes a Daz-to-Unity file (in JSON format) which details the material properties so the importer can construct and assign high-fidelity Unity materials.
Shaders - A suite of shader graphs are copied into your unity project the first time an export happens. These are imported by Unity Editor and compiled into shaders compatable with Unity's HD Render Pipeline.
Scripts - A suite of C# scripts are copied into your unity project the first time an export happens. These include a custom importer for DTU file types, a custom EditorWindow and helper classes used by the importer.
Diffusion Profiles - A generic diffusion profile and an IrayUberSkinDiffusionProfile are copied into your Unity project the first time an export happens. These are used by the custom shaders to replicate skin surface properties found in Daz3D Studio.
