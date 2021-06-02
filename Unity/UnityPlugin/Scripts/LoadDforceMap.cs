using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExecuteInEditMode]
public class LoadDforceMap : MonoBehaviour
{
    public TextAsset m_BinaryData;
//    [HideInInspector]
    public SkinnedMeshRenderer m_Skinned;
//    [HideInInspector]
    public Cloth m_Cloth;

    // Start is called before the first frame update
    void OnEnable()
    {
        GameObject parent = GetComponent<Renderer>().gameObject;
        m_Skinned = parent.GetComponent<SkinnedMeshRenderer>();
        m_Cloth = parent.GetComponent<Cloth>();

    }

    // Update is called once per frame
    void Update()
    {
        
    }


    public void SetSubMeshWeights(int submesh_index, float weight_value)
    {
        if (m_Cloth == null)
            return;

        if (submesh_index > m_Skinned.sharedMesh.subMeshCount)
        {
            Debug.LogError("LoadDforceMap::SetSubMeshWeight(): invalid submesh_index=" + submesh_index);
            return;
        }

        ClothSkinningCoefficient[] newCoefficients = new ClothSkinningCoefficient[m_Cloth.coefficients.Length];
        System.Array.Copy(m_Cloth.coefficients, newCoefficients, newCoefficients.Length);
        UnityEngine.Rendering.SubMeshDescriptor submesh = m_Skinned.sharedMesh.GetSubMesh(submesh_index);

        for (int vertex_index = submesh.firstVertex; vertex_index < submesh.vertexCount; vertex_index++)
        {
            newCoefficients[vertex_index].maxDistance = weight_value;
        }

        m_Cloth.coefficients = newCoefficients;

    }

    public void ClearSubMeshWeights(int submesh_index)
    {
        SetSubMeshWeights(submesh_index, float.MaxValue);
    }

    public void ClearWeightMap()
    {

        if (m_Cloth == null)
            return;

        ClothSkinningCoefficient[] newCoefficients = new ClothSkinningCoefficient[m_Cloth.coefficients.Length];
        System.Array.Copy(m_Cloth.coefficients, newCoefficients, newCoefficients.Length);

        for (int i = 0; i < newCoefficients.Length; i++)
        {
            float maxDistance = newCoefficients[i].maxDistance;
            newCoefficients[i].maxDistance = float.MaxValue;
        }

        m_Cloth.coefficients = newCoefficients;

    }

    public void LoadGradientPattern()
    {

        if (m_Cloth == null)
            return;

        ClothSkinningCoefficient[] newCoefficients = new ClothSkinningCoefficient[m_Cloth.coefficients.Length];
        System.Array.Copy(m_Cloth.coefficients, newCoefficients, newCoefficients.Length);

        for (int i = 0; i < newCoefficients.Length; i++)
        {
            float maxDistance = newCoefficients[i].maxDistance;
//            float gradientValue = (float)i / newCoefficients.Length * float.MaxValue;
            float gradientValue = (float)i / newCoefficients.Length;
            newCoefficients[i].maxDistance = gradientValue;
        }

        m_Cloth.coefficients = newCoefficients;

    }

    public void LoadSteppedGradient()
    {

        if (m_Cloth == null)
            return;

        ClothSkinningCoefficient[] newCoefficients = new ClothSkinningCoefficient[m_Cloth.coefficients.Length];
        System.Array.Copy(m_Cloth.coefficients, newCoefficients, newCoefficients.Length);

        for (int i = 0; i < newCoefficients.Length; i++)
        {
            float maxDistance = newCoefficients[i].maxDistance;
            float gradientValue = (float)i / newCoefficients.Length;
            int stepping = (int) (gradientValue * 10.0f);
            gradientValue = stepping / 10.0f;
            newCoefficients[i].maxDistance = gradientValue;
        }

        m_Cloth.coefficients = newCoefficients;

    }


    public void LoadRawWeightMap()
    {
        GameObject workingInstance = this.gameObject;

        ClothSkinningCoefficient[] newCoefficients = new ClothSkinningCoefficient[m_Cloth.coefficients.Length];
        System.Array.Copy(m_Cloth.coefficients, newCoefficients, newCoefficients.Length);

        //UnityEngine.Rendering.SubMeshDescriptor submesh = skinned.sharedMesh.GetSubMesh(matIndex);
        //for (int vertex_index = submesh.firstVertex; vertex_index < submesh.vertexCount; vertex_index++)
        //{
        //    float value = simulation_strength * 0.2f;
        //    newCoefficients[vertex_index].maxDistance = value;
        //}

        /////////////////////////////////////////////////////
        /// Load Raw Weight Map
        /////////////////////////////////////////////////////
        int numVerts = m_Skinned.sharedMesh.vertexCount;
        ushort[] weights = new ushort[numVerts];
        System.Buffer.BlockCopy(m_BinaryData.bytes, 0, weights, 0, numVerts);

        float simulation_strength = 0.0f;
        //int[] vertex_list = skinned.sharedMesh.GetTriangles(matIndex, false);
        for (int vertex_index = 0; vertex_index < numVerts; vertex_index++)
        {
            if (vertex_index >= newCoefficients.Length)
            {
                Debug.LogWarning("DFORCE IMPORT: vertex_index for dforce weights is greater than coefficient array: " + vertex_index + " vs " + newCoefficients.Length);
                continue;
            }

            simulation_strength = (float) weights[vertex_index] / ushort.MaxValue;
//            simulation_strength = 0.5f;

            //float strength_max = 1.0f;
            //float strength_min = 0.0f;
            //float strength_scale_threshold = 0.5f;
            float adjusted_simulation_strength = simulation_strength;

            //// tiered scaling
            //if (simulation_strength <= strength_scale_threshold)
            //{
            //    // stronger compression of values below threshold
            //    float scale = 0.075f;
            //    float offset = 0.2f;
            //    adjusted_simulation_strength = (simulation_strength - offset) * scale;
            //}
            //else
            //{
            //    float offset = (strength_scale_threshold - 0.2f) * 0.075f; // offset = (threshold - previous tier's offset) * previous teir's scale
            //    float scale = 0.2f;
            //    adjusted_simulation_strength = (simulation_strength - offset) / (1 - offset); // apply offset, then normalize to 1.0
            //    adjusted_simulation_strength *= scale;

            //}
            //// clamp to 0.0f to 0.2f
            //float coeff_min = 0.0f;
            //float coeff_max = 0.2f;
            //adjusted_simulation_strength = (adjusted_simulation_strength > coeff_min) ? adjusted_simulation_strength : coeff_min;
            //adjusted_simulation_strength = (adjusted_simulation_strength < coeff_max) ? adjusted_simulation_strength : coeff_max;

            newCoefficients[vertex_index].maxDistance = adjusted_simulation_strength;
        }

        m_Cloth.coefficients = newCoefficients;

    }

}
