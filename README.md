# Unofficial DazToRuntime
Unofficial updates to DazToUnity Bridge.

BRANCH: unity-URP-support

Requires: Unity 2020, URP 10.4, ShaderGraph 10.4

Must add following Symbol Definition: USING_URP

Removes HDRP dependencies from DTU scripts and adds custom IrayUberShaders for Unity Universal render pipeline (URP), based on original Daz DTU HDRP shadergraph files. My goal is to keep these custom shaders as compatible as possible with the original DazToUnity Bridge HDRP IrayUber shaders, so that they are nearly plug-in replacements for the original shaders.

Supports:
- Diffuse Maps
- Normal Maps
- Height Maps
- Alpha Maps
- Metallic Maps
- Specular Maps

Not working / not yet implemented:
- Dual Lobe Specular
