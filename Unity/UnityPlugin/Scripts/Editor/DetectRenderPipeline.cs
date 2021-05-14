using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.Linq;

#if UNITY_EDITOR
//[InitializeOnLoad]
public static class DetectRenderPipeline
{

	public static readonly string[] usingRPSymbols = new string[] {
			 "USING_HDRP",
			 "USING_URP",
			 "USING_BUILTIN",
		 };

	static string CleanPreprocessorDirectives(string definedSymbolsString)
    {
		List<string> allDefinedSymbols = definedSymbolsString.Split(';').ToList();

		foreach (string removeSymbol in usingRPSymbols)
		{
			if (allDefinedSymbols.Contains(removeSymbol))
			{
				allDefinedSymbols.Remove(removeSymbol);
				if (!allDefinedSymbols.Contains(removeSymbol))
                {
//					Debug.Log("Defined Symbol removed: " + removeSymbol);
				}
				else
                {
//					Debug.LogError("Defined Symbol could not be removed: " + removeSymbol);
				}
			}
			else
            {
//				Debug.Log("Defined Symbol not detected: " + removeSymbol);
			}
		}

		return string.Join(";", allDefinedSymbols.ToArray());
	}

	static string DetectAndSetSymbolString(string definedSymbols)
    {
		string newSymbolString = "";

		if (definedSymbols != "")
		{
			newSymbolString = definedSymbols + ";";
		}
        if (IsHDRPInstalled())
        {
			newSymbolString += "USING_HDRP";
        }
        else if (IsURPInstalled())
        {
			newSymbolString += "USING_URP";
		}
		else
        {
			newSymbolString += "USING_BUILTIN";
		}

		return newSymbolString;
	}

	//[UnityEditor.Callbacks.DidReloadScripts]
	static void CommitDefinedSymbols(string newSymbolsString)
    {
		Debug.Log("Attempting to write new PreprocessorDirectives: " + newSymbolsString);
		PlayerSettings.SetScriptingDefineSymbolsForGroup(EditorUserBuildSettings.selectedBuildTargetGroup, newSymbolsString);
	}

	static bool IsHDRPInstalled()
    {
		if (UnityEngine.Rendering.GraphicsSettings.currentRenderPipeline != null)
		{
			string renderAssetType = "dummy string";
			renderAssetType = UnityEngine.Rendering.GraphicsSettings.renderPipelineAsset.GetType().ToString();
			if (renderAssetType.Contains("HDRenderPipeline"))
			{
				return true;
			}
		}

		return false;
    }

	static bool IsURPInstalled()
	{
		if (UnityEngine.Rendering.GraphicsSettings.currentRenderPipeline != null)
		{
			string renderAssetType = "dummy string";
			renderAssetType = UnityEngine.Rendering.GraphicsSettings.renderPipelineAsset.GetType().ToString();
			if (renderAssetType.Contains("UniversalRenderPipeline"))
			{
				return true;
			}
		}

		return false;
	}

	public static void RunOnce()
	{
		string oldSymbolsString = PlayerSettings.GetScriptingDefineSymbolsForGroup(EditorUserBuildSettings.selectedBuildTargetGroup);

		string newSymbolsString = oldSymbolsString;

		newSymbolsString = CleanPreprocessorDirectives(newSymbolsString);
		newSymbolsString = DetectAndSetSymbolString(newSymbolsString);

//		Debug.Log("OldString=" + oldSymbolsString + ", NewString=" + newSymbolsString);
		if (oldSymbolsString != newSymbolsString)
        {
			CommitDefinedSymbols(newSymbolsString);
		}

	}
}
#endif // UNITY_EDITOR