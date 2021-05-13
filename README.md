# Unofficial DazToRuntime
Unofficial updates to DazToUnity Bridge.

BRANCH: unity-URP2019-support

NOTE: This is a special version of URP build which contains Unity 2019-compatible shadergraphs.

Must add following Symbol Defintions: USING_URP, USING_2019

Removes HDRP dependencies from DTU scripts and adds custom IrayUberShaders for Unity Universal render pipeline (URP), based on original Daz DTU HDRP shadergraph files. My goal is to keep these custom shaders as compatible as possible with the original DazToUnity Bridge HDRP IrayUber shaders, so that they are nearly plug-in replacements for the original shaders.

Currently supports:
- Diffuse Maps
- Normal Maps
- Height Maps
- Alpha Maps
- Metallic Maps
- Roughness Maps
- Specular Maps

Not yet working / not yet implemented:
- Dual lobe Specular
- SSS
