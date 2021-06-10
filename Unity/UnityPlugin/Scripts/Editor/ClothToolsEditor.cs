﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

[CustomEditor(typeof(ClothTools))]
[CanEditMultipleObjects]

public class ClothToolsEditor : Editor
{
    //private SerializedObject m_Object;

    //public void OnEnable()
    //{
    //    m_Object = new SerializedObject(target);
    //}

    public override void OnInspectorGUI()
    {
        ClothTools clothtools = (ClothTools)target;

        //m_Object.Update();

        GUILayout.Label("**Load dForce Weight Map**", EditorStyles.boldLabel);
        GUILayout.Space(10);

        DrawDefaultInspector();

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
                    clothtools.ClearSubMeshWeights(matIndex);
                    //Debug.Log("Clear Material Weights: " + mat.name);
                }
                if (GUILayout.Button("Zero"))
                {
                    clothtools.SetSubMeshWeights(matIndex, 0.0f);
                    //Debug.Log("Zero Material Weights: " + mat.name);
                }
                if (GUILayout.Button("One"))
                {
                    clothtools.SetSubMeshWeights(matIndex, 1.0f);
                    //Debug.Log("Set Material Weights to 1.0: " + mat.name);
                }
                GUILayout.EndHorizontal();
            }
        }

        GUILayout.Space(10);
        if (GUILayout.Button("Clear All Weights"))
        {
            Undo.RecordObject(clothtools.m_Cloth, "Clear All Weights");
            clothtools.ClearWeightMap();
            //Debug.Log("Clear Weights.");
        }

        GUILayout.Space(10);
        if (GUILayout.Button("Load Weightmap data"))
        {
            clothtools.LoadRawWeightMap();
            //Debug.Log("Load Weightmap data.");
        }

        GUILayout.Space(10);
        if (GUILayout.Button("Load Gradient Pattern"))
        {
            clothtools.LoadGradientPattern();
            //Debug.Log("Load Gradient Pattern.");
        }

        //GUILayout.Space(10);
        //if (GUILayout.Button("Load Stepped Gradient"))
        //{
        //    clothtools.LoadSteppedGradient();
        //    //Debug.Log("Load Stepped Gradient.");
        //}

        //GUILayout.Space(10);
        //if (GUILayout.Button("Generate Lookup Tables"))
        //{
        //    clothtools.GenerateLookupTables();
        //    //Debug.Log("Generate Lookup Tables Called....");
        //}

        //GUILayout.Space(10);
        //if (GUILayout.Button("Run Vertex Data Test"))
        //{
        //    clothtools.TestVertData();
        //    //Debug.Log("Running Vertex Data Test....");
        //}

        //m_Object.ApplyModifiedProperties();
    }

}
