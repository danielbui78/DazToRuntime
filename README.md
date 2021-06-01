# Unofficial DazToUnity Bridge

EXPERIMENTAL BRANCH: dForce, bugfixes, PBRSkin support, URP support, Built-In support, autodetect active renderpipeline

unity-URP-support for 2019 and 2020
unity-builtin-support confirmed working with: 2019.4.11f1 (LTS), 2020.3.4f1 (LTS), 2021.1.5f1

Updated scripts will add one of the following Symbol Definitions to the Project Settings: USING_HDRP, USING_URP, USING_BUILTIN

Removes HDRP dependencies from DTU scripts and adds custom IrayUberShaders for Unity Universal render pipeline (URP), based on original Daz DTU HDRP shadergraph files; and adds custom Built-In shaders, based on Unity's Standard Shader. My goal is to keep these custom shaders as compatible as possible with the original DazToUnity Bridge HDRP IrayUber shaders, so that they are nearly plug-in replacements for the original shaders.

Supports:
- dForce clothing via Simulation Properties in Surfaces Tab of Daz Studio
- Diffuse Maps (URP & Built-in)
- Normal Maps (URP and Built-in)
- Height Maps (URP)
- Alpha Maps (Unity and Built-in)
- Metallic Maps (Unity and Built-in)
- Specular Maps (Unity and Built-in)

Not working / not yet implemented:
- dForce Weight Maps
- Dual Lobe Specular
- SSS
- Heightmaps (Built-in)
