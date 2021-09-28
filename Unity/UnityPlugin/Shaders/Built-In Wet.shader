// Based on Unity built-in "Standard" shader.
// Unity built-in shader source. Copyright (c) 2016 Unity Technologies. MIT license (see license.txt)
Shader "Daz3D/Built-In Wet"
{
    Properties
    {
//        _Color("Color", Color) = (1,1,1,1)
        _Diffuse("Diffuse Color", Color) = (1,1,1,1)
//        _MainTex("Albedo", 2D) = "white" {}
        _DiffuseMap("Diffuse Map", 2D) = "white" {}

//        _Cutoff("Alpha Cutoff", Range(0.0, 1.0)) = 0.5
        _AlphaClipThreshold("Alpha Cutoff", Range(0.0, 1.0)) = 0.5

//        _Glossiness("Smoothness", Range(0.0, 1.0)) = 0.5
//        _GlossMapScale("Smoothness Scale", Range(0.0, 1.0)) = 1.0
//        [Enum(Metallic Alpha,0,Albedo Alpha,1)] _SmoothnessTextureChannel("Smoothness texture channel", Float) = 0
        _Roughness("Roughness", Range(0.0, 1.0)) = 0.5
        _RoughnessMap("Roughness Map", 2D) = "white" {}

//        [Gamma] _Metallic("Metallic", Range(0.0, 1.0)) = 0.0
        [Gamma] _Metallic("Metallic Weight", Range(0.0, 1.0)) = 0.0
//        _METALLICMAP("Metallic", 2D) = "white" {}
        _MetallicMap("Metallic Map", 2D) = "white" {}

        _SpecularColor("Specular Color", Color) = (0.5,0.5,0.5,1)
        _SpecularColorMap("Specular Color Map", 2D) = "grey" {}

        [ToggleOff] _SpecularHighlights("Specular Highlights", Float) = 1.0
        [ToggleOff] _GlossyReflections("Glossy Reflections", Float) = 1.0

//        _BumpScale("Scale", Float) = 1.0
        _Normal("Normal Strength", Range(0.0, 2.0)) = 1.0
//        [Normal] _BumpMap("Normal Map", 2D) = "bump" {}
        [Normal] _NormalMap("Normal Map", 2D) = "bump" {}

//        _Parallax("Height Scale", Range(0.005, 0.08)) = 0.02
        _Height("Height", Range(0.005, 0.08)) = 0.02
//        _ParallaxMap("Height Map", 2D) = "black" {}
        _HeightMap("Height Map", 2D) = "black" {}

        _OcclusionStrength("Occlusion Strength", Range(0.0, 1.0)) = 1.0
//        _OcclusionMap("Occlusion", 2D) = "white" {}
        _OcclusionMap("Occlusion Map", 2D) = "white" {}

//        _EmissionColor("Color", Color) = (0,0,0)
        _Emission("Emission Color", Color) = (0,0,0)
//        _EmissionMap("Emission", 2D) = "white" {}
        _EmissionMap("Emission Map", 2D) = "white" {}

        _Alpha("Cutout Opacity", Float) = 1.0
        _AlphaMap("Cutout Opacity Map", 2D) = "white" {}

//        _DetailMask("Detail Mask", 2D) = "white" {}
//        _DetailAlbedoMap("Detail Albedo x2", 2D) = "grey" {}
//        _DetailNormalMapScale("Scale", Float) = 1.0
//        [Normal] _DetailNormalMap("Normal Map", 2D) = "bump" {}
//        [Enum(UV0,0,UV1,1)] _UVSec("UV Set for secondary textures", Float) = 0


            // Blending state
//            _Mode("__mode", Float) = 0.0
//            _SrcBlend("__src", Float) = 1.0
//            _DstBlend("__dst", Float) = 0.0
//            _ZWrite("__zw", Float) = 1.0

//            [Enum(Opaque, 0, Cutout, 1, Fade, 2, Transparent, 3)]
//            _Mode("BlendMode", Float) = 0.0
//            [Enum(One, (int)UnityEngine.Rendering.BlendMode.One, SrcAlpha, (int)UnityEngine.Rendering.BlendMode.SrcAlpha)]
            _SrcBlend("SrcBlend", Float) = 1.0
//            [Enum(Zero, (int)UnityEngine.Rendering.BlendMode.Zero, OneMinusSrcAlpha, (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha)]
            _DstBlend("DstBlend", Float) = 0.0
            [ToggleOff]
            _ZWrite("Zwrite", Float) = 1.0
    }

        CGINCLUDE
#define _SPECULARCOLORMAP 1
#define _NORMALMAP 1
//#define _PARALLAXMAP 1
#define _ALPHAMAP 1
//#define UNITY_SETUP_BRDF_INPUT MetallicSetup
#define UNITY_SETUP_BRDF_INPUT SpecularSetup
            ENDCG

            SubShader
        {
            Tags { "RenderType" = "Opaque" "PerformanceChecks" = "False" }
            LOD 300


            // ------------------------------------------------------------------
            //  Base forward pass (directional light, emission, lightmaps, ...)
            Pass
            {
                Name "FORWARD"
                Tags { "LightMode" = "ForwardBase" }

                Blend[_SrcBlend][_DstBlend]
                ZWrite[_ZWrite]

                CGPROGRAM
                #pragma target 3.0

            // -------------------------------------

            #pragma shader_feature_local _NORMALMAP
            #pragma shader_feature_local _ _ALPHATEST_ON _ALPHABLEND_ON _ALPHAPREMULTIPLY_ON
            #pragma shader_feature _EMISSION
            #pragma shader_feature_local _METALLICMAP
            #pragma shader_feature_local _DETAIL_MULX2
            #pragma shader_feature_local _SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A
            #pragma shader_feature_local _SPECULARHIGHLIGHTS_OFF
            #pragma shader_feature_local _GLOSSYREFLECTIONS_OFF
            #pragma shader_feature_local _PARALLAXMAP

            #pragma multi_compile_fwdbase
            #pragma multi_compile_fog
            #pragma multi_compile_instancing
            // Uncomment the following line to enable dithering LOD crossfade. Note: there are more in the file to uncomment for other passes.
            //#pragma multi_compile _ LOD_FADE_CROSSFADE

            #pragma vertex vertBase
            #pragma fragment fragBase
            #include "CGInc/DazStandardCoreForward.cginc"

            ENDCG
        }
            // ------------------------------------------------------------------
            //  Additive forward pass (one light per pass)
            Pass
            {
                Name "FORWARD_DELTA"
                Tags { "LightMode" = "ForwardAdd" }
                Blend[_SrcBlend] One
                Fog { Color(0,0,0,0) } // in additive pass fog should be black
                ZWrite Off
                ZTest LEqual

                CGPROGRAM
                #pragma target 3.0

            // -------------------------------------


            #pragma shader_feature_local _NORMALMAP
            #pragma shader_feature_local _ _ALPHATEST_ON _ALPHABLEND_ON _ALPHAPREMULTIPLY_ON
            #pragma shader_feature_local _METALLICMAP
            #pragma shader_feature_local _SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A
            #pragma shader_feature_local _SPECULARHIGHLIGHTS_OFF
            #pragma shader_feature_local _DETAIL_MULX2
            #pragma shader_feature_local _PARALLAXMAP

            #pragma multi_compile_fwdadd_fullshadows
            #pragma multi_compile_fog
            // Uncomment the following line to enable dithering LOD crossfade. Note: there are more in the file to uncomment for other passes.
            //#pragma multi_compile _ LOD_FADE_CROSSFADE

            #pragma vertex vertAdd
            #pragma fragment fragAdd
            #include "CGInc/DazStandardCoreForward.cginc"

            ENDCG
        }
            // ------------------------------------------------------------------
            //  Shadow rendering pass
            Pass {
                Name "ShadowCaster"
                Tags { "LightMode" = "ShadowCaster" }

                ZWrite On ZTest LEqual

                CGPROGRAM
                #pragma target 3.0

            // -------------------------------------


            #pragma shader_feature_local _ _ALPHATEST_ON _ALPHABLEND_ON _ALPHAPREMULTIPLY_ON
            #pragma shader_feature_local _METALLICMAP
            #pragma shader_feature_local _SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A
            #pragma shader_feature_local _PARALLAXMAP
            #pragma multi_compile_shadowcaster
            #pragma multi_compile_instancing
            // Uncomment the following line to enable dithering LOD crossfade. Note: there are more in the file to uncomment for other passes.
            //#pragma multi_compile _ LOD_FADE_CROSSFADE

            #pragma vertex vertShadowCaster
            #pragma fragment fragShadowCaster

            #include "CGInc/DazStandardShadow.cginc"

            ENDCG
        }
            // ------------------------------------------------------------------
            //  Deferred pass
            Pass
            {
                Name "DEFERRED"
                Tags { "LightMode" = "Deferred" }

                CGPROGRAM
                #pragma target 3.0
                #pragma exclude_renderers nomrt


            // -------------------------------------

            #pragma shader_feature_local _NORMALMAP
            #pragma shader_feature_local _ _ALPHATEST_ON _ALPHABLEND_ON _ALPHAPREMULTIPLY_ON
            #pragma shader_feature _EMISSION
            #pragma shader_feature_local _METALLICMAP
            #pragma shader_feature_local _SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A
            #pragma shader_feature_local _SPECULARHIGHLIGHTS_OFF
            #pragma shader_feature_local _DETAIL_MULX2
            #pragma shader_feature_local _PARALLAXMAP

            #pragma multi_compile_prepassfinal
            #pragma multi_compile_instancing
            // Uncomment the following line to enable dithering LOD crossfade. Note: there are more in the file to uncomment for other passes.
            //#pragma multi_compile _ LOD_FADE_CROSSFADE

            #pragma vertex vertDeferred
            #pragma fragment fragDeferred

            #include "CGInc/DazStandardCore.cginc"

            ENDCG
        }

            // ------------------------------------------------------------------
            // Extracts information for lightmapping, GI (emission, albedo, ...)
            // This pass it not used during regular rendering.
            Pass
            {
                Name "META"
                Tags { "LightMode" = "Meta" }

                Cull Off

                CGPROGRAM
                #pragma vertex vert_meta
                #pragma fragment frag_meta

                #pragma shader_feature _EMISSION
                #pragma shader_feature_local _METALLICMAP
                #pragma shader_feature_local _SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A
                #pragma shader_feature_local _DETAIL_MULX2
                #pragma shader_feature EDITOR_VISUALIZATION

                #include "CGInc/DazStandardMeta.cginc"
                ENDCG
            }
        }

            SubShader
        {
            Tags { "RenderType" = "Opaque" "PerformanceChecks" = "False" }
            LOD 150

            // ------------------------------------------------------------------
            //  Base forward pass (directional light, emission, lightmaps, ...)
            Pass
            {
                Name "FORWARD"
                Tags { "LightMode" = "ForwardBase" }

                Blend[_SrcBlend][_DstBlend]
                ZWrite[_ZWrite]

                CGPROGRAM
                #pragma target 2.0

                #pragma shader_feature_local _NORMALMAP
                #pragma shader_feature_local _ _ALPHATEST_ON _ALPHABLEND_ON _ALPHAPREMULTIPLY_ON
                #pragma shader_feature _EMISSION
                #pragma shader_feature_local _METALLICMAP
                #pragma shader_feature_local _SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A
                #pragma shader_feature_local _SPECULARHIGHLIGHTS_OFF
                #pragma shader_feature_local _GLOSSYREFLECTIONS_OFF
            // SM2.0: NOT SUPPORTED shader_feature_local _DETAIL_MULX2
            // SM2.0: NOT SUPPORTED shader_feature_local _PARALLAXMAP

            #pragma skip_variants SHADOWS_SOFT DIRLIGHTMAP_COMBINED

            #pragma multi_compile_fwdbase
            #pragma multi_compile_fog

            #pragma vertex vertBase
            #pragma fragment fragBase
            #include "CGInc/DazStandardCoreForward.cginc"

            ENDCG
        }
            // ------------------------------------------------------------------
            //  Additive forward pass (one light per pass)
            Pass
            {
                Name "FORWARD_DELTA"
                Tags { "LightMode" = "ForwardAdd" }
                Blend[_SrcBlend] One
                Fog { Color(0,0,0,0) } // in additive pass fog should be black
                ZWrite Off
                ZTest LEqual

                CGPROGRAM
                #pragma target 2.0

                #pragma shader_feature_local _NORMALMAP
                #pragma shader_feature_local _ _ALPHATEST_ON _ALPHABLEND_ON _ALPHAPREMULTIPLY_ON
                #pragma shader_feature_local _METALLICMAP
                #pragma shader_feature_local _SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A
                #pragma shader_feature_local _SPECULARHIGHLIGHTS_OFF
                #pragma shader_feature_local _DETAIL_MULX2
            // SM2.0: NOT SUPPORTED shader_feature_local _PARALLAXMAP
            #pragma skip_variants SHADOWS_SOFT

            #pragma multi_compile_fwdadd_fullshadows
            #pragma multi_compile_fog

            #pragma vertex vertAdd
            #pragma fragment fragAdd
            #include "CGInc/DazStandardCoreForward.cginc"

            ENDCG
        }
            // ------------------------------------------------------------------
            //  Shadow rendering pass
            Pass {
                Name "ShadowCaster"
                Tags { "LightMode" = "ShadowCaster" }

                ZWrite On ZTest LEqual

                CGPROGRAM
                #pragma target 2.0

                #pragma shader_feature_local _ _ALPHATEST_ON _ALPHABLEND_ON _ALPHAPREMULTIPLY_ON
                #pragma shader_feature_local _METALLICMAP
                #pragma shader_feature_local _SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A
                #pragma skip_variants SHADOWS_SOFT
                #pragma multi_compile_shadowcaster

                #pragma vertex vertShadowCaster
                #pragma fragment fragShadowCaster

                #include "CGInc/DazStandardShadow.cginc"

                ENDCG
            }

            // ------------------------------------------------------------------
            // Extracts information for lightmapping, GI (emission, albedo, ...)
            // This pass it not used during regular rendering.
            Pass
            {
                Name "META"
                Tags { "LightMode" = "Meta" }

                Cull Off

                CGPROGRAM
                #pragma vertex vert_meta
                #pragma fragment frag_meta

                #pragma shader_feature _EMISSION
                #pragma shader_feature_local _METALLICMAP
                #pragma shader_feature_local _SMOOTHNESS_TEXTURE_ALBEDO_CHANNEL_A
                #pragma shader_feature_local _DETAIL_MULX2
                #pragma shader_feature EDITOR_VISUALIZATION

                #include "CGInc/DazStandardMeta.cginc"
                ENDCG
            }
        }


            FallBack "VertexLit"
//            CustomEditor "StandardShaderGUI"
}
