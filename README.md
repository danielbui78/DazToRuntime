# Unofficial DazToRuntime
Unofficial updates to DazToUnity Bridge.

BRANCH: combined experimental branch

unity-URP-support requires: Unity 2020, URP 10.4, ShaderGraph 10.4
unity-builtin-support confirmed working with: 2019.4.11f1 (LTS), 2020.3.4f1 (LTS), 2021.1.5f1

Must add following Symbol Definition: USING_URP or USING_BUILTIN

Removes HDRP dependencies from DTU scripts and adds custom IrayUberShaders for Unity Universal render pipeline (URP), based on original Daz DTU HDRP shadergraph files. My goal is to keep these custom shaders as compatible as possible with the original DazToUnity Bridge HDRP IrayUber shaders, so that they are nearly plug-in replacements for the original shaders.

Supports:
- Diffuse Maps (URP & Built-in)
- Normal Maps (URP and Built-in)
- Height Maps (URP)
- Alpha Maps (Unity and Built-in)
- Metallic Maps (Unity and Built-in)
- Specular Maps (Unity and Built-in)

Not working / not yet implemented:
- Dual Lobe Specular
- SSS
- Heightmaps (Built-in)
