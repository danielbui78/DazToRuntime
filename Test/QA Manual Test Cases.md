## QA Manual Test Cases: Unreal script support ##

1. Load and Export Genesis 8 Basic Female to Unreal
   - a. Start Daz Studio.
   - b. Confirm correct version number of pre-release Daz Studio bridge plugin.
   - c. Start Unreal Engine.
   - d. Confirm correct version number of pre-release UnrealEngine bridge plugin.
   - e. Load Genesis 8 Basic Female.
   - f. Select File->Send To->Daz To Unreal.
   - g. Confirm Asset Name is "Genesis8Female" in the Daz To Unreal dialog window.
   - h. Click "Accept".
   - i. Confirm Unreal Engine has successfully generated a new "Genesis8Female" asset in the Content Browser Pane.
   - j. Confirm that a "Genesis8Female" subfolder was generated in the Intermediate Folder, with "Genesis8Female.dtu" and "Genesis8Female.fbx" files.

2. Load and Export Additional Genesis 8.1 Basic Female to Unreal
   - a. Continue from previous Daz Studio and Unreal Engine session (test case 1).
   - b. Load Genesis 8.1 Basic Female.
   - c. Select File->Send To->Daz To Unreal.
   - d. Confirm Asset Name is "Genesis81Female" in the Daz To Unreal dialog window.
   - e. Click "Accept".
   - f. Confirm UnrealEngine has successfully generated a new "Genesis81Female" asset in the Content Browser Pane.
   - g. Confirm that a "Genesis81Female" subfolder was generated in the Intermediate Folder, with "Genesis81Female.dtu" and "Genesis81Female.fbx" files.

3. Load and Export Genesis 8.1 Basic Female with Custom Scene Node Label.
   - a. Start Daz Studio.
   - b. Confirm correct version number of pre-release Daz Studio bridge plugin.
   - c. Start Unreal Engine.
   - d. Confirm correct version number of pre-release UnrealEngine bridge plugin.
   - e. Load Genesis 8.1 Basic Female.
   - f. Select the Genesis 8.1 Basic Female from the Scene Pane.
   - g. Rename the node to "CustomSceneLabel".
   - h. Select File->Send To->Daz To Unreal.
   - i. Confirm Asset Name is "CustomSceneLabel" in the Daz To Unreal dialog window.
   - j. Click "Accept".
   - k. Confirm Unreal Engine has successfully generated a new "CustomSceneLabel" asset in the Content Browser Pane.
   - l. Confirm that a "CustomSceneLabel" subfolder was generated in the Intermediate Folder, with "CustomSceneLabel.dtu" and "CustomSceneLabel.fbx" files.

4. Load and Export Genesis 8.1 Basic Female with Custom Asset Name to Unreal
   - a. Start Daz Studio.
   - b. Confirm correct version number of pre-release Daz Studio bridge plugin.
   - c. Start Unreal Engine.
   - d. Confirm correct version number of pre-release UnrealEngine bridge plugin.
   - e. Load Genesis 8.1 Basic Female.
   - f. Select File->Send To->Daz To Unreal.
   - g. Confirm Asset Name is "Genesis81Female" in the Daz To Unreal dialog window.
   - h. Change Asset Name to "CustomAssetName".
   - i. Click "Accept".
   - j. Confirm Unreal Engine has successfully generated a new "CustomAssetName" asset in the Content Browser Pane.
   - k. Confirm that a "CustomAssetName" subfolder was generated in the Intermediate Folder, with "CustomAssetName.dtu" and "CustomAssetName.fbx" files.

5. Load and Export Genesis 8.1 Basic Female with Custom Intermediate Folder to Unreal
   - a. Start Daz Studio.
   - b. Confirm correct version number of pre-release Daz Studio bridge plugin.
   - c. Start Unreal Engine.
   - d. Confirm correct version number of pre-release UnrealEngine bridge plugin.
   - e. Load Genesis 8.1 Basic Female.
   - f. Select File->Send To->Daz To Unreal.
   - g. Confirm Intermediate Folder is "C:\Users\<username>\Documents\DazToUnreal" in the Daz To Unreal dialog window.
   - h. Change Intermediate Folder to "C:\CustomRoot".
   - i. Click "Accept".
   - j. Confirm Unreal Engine has successfully generated a new "Genesis81Female" asset in the Content Browser Pane.
   - k. Confirm there is a "C:\CustomRoot" folder with "Genesis81Female" subfolder containing "Genesis81Female.dtu" and "Genesis81Female.fbx".

6. Load and Export Genesis 8.1 Basic Female with Enable Morphs to Unreal
   - a. Start Daz Studio.
   - b. Confirm correct version number of pre-release Daz Studio bridge plugin.
   - c. Start Unreal Engine.
   - d. Confirm correct version number of pre-release UnrealEngine bridge plugin.
   - e. Load Genesis 8.1 Basic Female.
   - f. Select File->Send To->Daz To Unreal.
   - g. Check "Enable Morphs", then Click "Choose Morphs".
   - h. Select a Morph such as "Genesis 8.1 Female -> Actor -> Bodybuilder" from the left and middle panes. Then click "Add For Export" so that it appears in the right pane.
   - i. Click "Accept" for the Morph Selection dialog.
   - j. Click "Accept" for the Daz To Unreal dialog.
   - k. Confirm Unreal Engine has successfully generated a new "Genesis81Female" asset in the Content Browser Pane.
   - l. Double-click the "Genesis81Female" asset to show the asset viewer window.
   - m. Confirm that the exported morph appears in the "Morph Target Preview" pane on the right side of the asset viewer window.
   - n. Confirm that moving the slider to 1.0 fully applies the morph.
   - o. Confirm that moving the slider to 0.0 fully removes the morph.

7. Load and Export Genesis 8.1 Basic Female with Enable Subdivisions to Unreal
   - a. Start Daz Studio.
   - b. Confirm correct version number of pre-release Daz Studio bridge plugin.
   - c. Start Unreal Engine.
   - d. Confirm correct version number of pre-release UnrealEngine bridge plugin.
   - e. Load Genesis 8.1 Basic Female.
   - f. Select File->Send To->Daz To Unreal.
   - g. Check "Enable Subdivision", then Click "Choose Subdivisions".
   - h. Select the drop-down for Genesis 8.1 Female, and change to Subdivision Level 2.
   - i. Click "Accept" for the Subdivision Levels dialog.
   - j. Click "Accept" for the Daz To Unreal dialog.
   - k. Confirm Unreal Engine has successfully generated a new "Genesis81Female" asset in the Content Browser Pane.
   - l. Double-click the "Genesis81Female" asset to show the asset viewer window.
   - m. Confirm that the Vertices info printed in the top left corner of the preview window shows 271,418 instead of 19,775.
   - n. Confirm that a "Genesis81Female" subfolder was generated in the Intermediate Folder, with "Genesis81Female.dtu", "Genesis81Female.fbx", "Genesis81Female_base.fbx" and "Genesis81Female_HD.fbx" files.

8. Load and Export Custom Scene File to Unreal
   - a. Start Daz Studio.
   - b. Confirm correct version number of pre-release Daz Studio bridge plugin.
   - c. Start Unreal Engine.
   - d. Confirm correct version number of pre-release UnrealEngine bridge plugin.
   - e. Load Genesis 8.1 Basic Female.
   - f. Select File->Send To->Daz To Unreal.
   - g. Click "Accept".
   - h. Confirm Unreal Engine has successfully generated a new "Genesis81Female" asset in the Content Browser Pane.
   - i. Confirm that a "Genesis81Female" subfolder was generated in the Intermediate Folder, with "Genesis81Female.dtu" and "Genesis81Female.fbx" files.

9. Load and Export Victoria 8.1 to Unreal
   - a. Start Daz Studio.
   - b. Confirm correct version number of pre-release Daz Studio bridge plugin.
   - c. Start Unreal Engine.
   - d. Confirm correct version number of pre-release UnrealEngine bridge plugin.
   - e. Load Victoria 8.1.
   - f. Select File->Send To->Daz To Unreal.
   - g. Confirm the Asset Name is "Victoria81".
   - h. Click "Accept".
   - i. Confirm Unreal Engine has successfully generated a new "Victoria81" asset in the Content Browser Pane.
   - j. Confirm that a "Victoria81" subfolder was generated in the Intermediate Folder, with "Victoria81.dtu" and "Victoria81.fbx" files.

10. Load and Export Victoria 8.1 with "Victoria 8.1 Tattoo All - Add" to Unreal
   - a. Start Daz Studio.
   - b. Confirm correct version number of pre-release Daz Studio bridge plugin.
   - c. Start Unreal Engine.
   - d. Confirm correct version number of pre-release UnrealEngine bridge plugin.
   - e. Load Victoria 8.1.
   - f. Open the Materials section and load "Victoria 8.1 Tattoo All - Add" onto Victoria 8.1.
   - g. Wait for the L.I.E. textures to be baked and updated in the Viewport.  This can take a few minutes.
   - h. Select File->Send To->Daz To Unreal.
   - i. Confirm the Asset Name is "Victoria81".
   - j. Click "Accept".
   - k. Confirm Unreal Engine has successfully generated a new "Victoria81" asset in the Content Browser Pane.
   - l. Double-click the "Victoria81" asset to show the asset viewer window.
   - m. Confirm that the asset has full body tattoos visible in the asset viewer.
   - n. Confirm that a "Victoria81" subfolder was generated in the Intermediate Folder, with "Victoria81.dtu" and "Victoria81.fbx" files, and additional "ExportTextures" folder with 8 PNG texture files (d10.png to d17.png).
