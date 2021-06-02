using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

[CustomEditor(typeof(LoadDforceMap))]
[CanEditMultipleObjects]

public class LoadDforceMapEditor : Editor
{
    private SerializedObject m_Object;

    public void OnEnable()
    {
        m_Object = new SerializedObject(target);
    }

    public override void OnInspectorGUI()
    {
        LoadDforceMap loadDforce = (LoadDforceMap)target;

        m_Object.Update();

        GUILayout.Label("**Load dForce Weight Map**", EditorStyles.boldLabel);
        GUILayout.Space(10);

        base.OnInspectorGUI();

        GUILayout.Space(10);
        GameObject parent = Selection.activeGameObject;
        SkinnedMeshRenderer skinned = parent.GetComponent<SkinnedMeshRenderer>();
        foreach (Material mat in skinned.sharedMaterials)
        {
            if (mat)
            {
                int matIndex = System.Array.IndexOf(skinned.sharedMaterials, mat);

                GUILayout.BeginHorizontal();
                GUILayout.Label(mat.name);
                if (GUILayout.Button("Clear"))
                {
                    loadDforce.ClearSubMeshWeights(matIndex);
                    Debug.Log("Clear Material Weights: " + mat.name);
                }
                if (GUILayout.Button("Zero"))
                {
                    loadDforce.SetSubMeshWeights(matIndex, 0.0f);
                    Debug.Log("Zero Material Weights: " + mat.name);
                }
                if (GUILayout.Button("One"))
                {
                    loadDforce.SetSubMeshWeights(matIndex, 1.0f);
                    Debug.Log("Set Material Weights to 1.0: " + mat.name);
                }
                GUILayout.EndHorizontal();
            }
        }

        GUILayout.Space(10);
        if (GUILayout.Button("Clear Weights"))
        {
            loadDforce.ClearWeightMap();
            Debug.Log("Clear Weights.");
        }

        GUILayout.Space(10);
        if (GUILayout.Button("Load Weightmap data"))
        {
            loadDforce.LoadRawWeightMap();
            Debug.Log("Load Weightmap data.");
        }

        GUILayout.Space(10);
        if (GUILayout.Button("Load Gradient Pattern"))
        {
            loadDforce.LoadGradientPattern();
            Debug.Log("Load Gradient Pattern.");
        }

        GUILayout.Space(10);
        if (GUILayout.Button("Load Stepped Gradient"))
        {
            loadDforce.LoadSteppedGradient();
            Debug.Log("Load Stepped Gradient.");
        }

        m_Object.ApplyModifiedProperties();
    }

}
