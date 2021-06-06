using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExecuteInEditMode]
public class LoadDforceMap : MonoBehaviour
{
    public TextAsset m_BinaryData;
    public TextAsset m_TestVertData;
    [SerializeField]
    private SkinnedMeshRenderer m_Skinned;
    [SerializeField]
    private Cloth m_Cloth;


    [System.Serializable]
    class SubmeshMeta
    {
        public int vertex_offset;
        public int vertex_count;
        public int submesh_index;

        public SubmeshMeta(int offset, int count, int index)
        {
            vertex_offset = offset;
            vertex_count = count;
            submesh_index = index;
        }

        public int Start()
        {
            return vertex_offset;
        }

        public int Stop()
        {
            return vertex_offset + vertex_count + 1;
        }

    }
    List<SubmeshMeta> m_SubmeshMeta;

    /////////////////////////////////////////////////////////////////
    // SkinnedMesh vertex index TO Cloth vertex index
    /////////////////////////////////////////////////////////////////
    [System.Serializable]
    class CollapsedVertexArray
    {
        private Dictionary<int, List<CollapsedVertex>> m_CollapsedVertices;
        private int m_UniqueVertexCount;
        private Dictionary<int, int> m_LookupTable;

        public class CollapsedVertex
        {
            public Vector3 vertex;
            public int unique_index;
            public List<int> indexes;
            const float near_zero = 0.0001f; // 0.1 mm

            public CollapsedVertex(Vector3 a_vertex, int index, int a_unique_index)
            {
                vertex = a_vertex;
                indexes = new List<int>(1) { index };
                unique_index = a_unique_index;
            }

            public void AddIndex(int index)
            {
                indexes.Add(index);
            }

            public static bool operator ==(CollapsedVertex a, Vector3 b) => a.vertex == b;
            public static bool operator !=(CollapsedVertex a, Vector3 b) => a.vertex != b;

            public static bool operator ==(CollapsedVertex a, CollapsedVertex b) => a.vertex == b.vertex;
            public static bool operator !=(CollapsedVertex a, CollapsedVertex b) => a.vertex != b.vertex;
            public override bool Equals(object obj) => this.vertex.Equals(obj);
            public override int GetHashCode() => this.vertex.GetHashCode();

        }

        public int Length
        {
            get
            {
                return m_UniqueVertexCount;
            }
        }

        public int LookupIndex(int a_index)
        {
            if (m_LookupTable != null)
            {
                if (m_LookupTable.ContainsKey(a_index))
                    return m_LookupTable[a_index];
            }

            return -1;

        }

        public CollapsedVertexArray(Vector3[] a_vertices)
        {
            if (a_vertices.Length <= 0)
            {
                m_UniqueVertexCount = -1;
                return;
            }

            m_CollapsedVertices = new Dictionary<int, List<CollapsedVertex>>(a_vertices.Length);
            for (int i=0; i < a_vertices.Length; i++)
            {
                Vector3 a_vert = a_vertices[i];
                bool vert_is_unique = true;
                if (m_CollapsedVertices.ContainsKey(a_vert.GetHashCode()))
                {
                    // get optimized (hashcode filtered) list of verts, check against each one for uniqueness
                    List<CollapsedVertex> cvert_list = m_CollapsedVertices[a_vert.GetHashCode()];
                    foreach (CollapsedVertex cvert in cvert_list)
                    {
                        if (cvert == a_vert)
                        {
                            vert_is_unique = false;
                            cvert.AddIndex(i);
                            break;
                        }
                    }
                    if (vert_is_unique)
                    {
                        // add to end of unqiue verts array
                        //m_CollapsedVertices[m_UniqueVertexCount++] = new CollapsedVertex(a_vert, i);
                        cvert_list.Add(new CollapsedVertex(a_vert, i, m_UniqueVertexCount++));
                    }

                }
                else
                {
                    // assume unqiue
                    m_CollapsedVertices.Add(a_vert.GetHashCode(), new List<CollapsedVertex>(1) { new CollapsedVertex(a_vert, i, m_UniqueVertexCount++) });
                }

            }

            // build lookup tables / Dictionaries
            m_LookupTable = new Dictionary<int, int>(a_vertices.Length);
            foreach (List<CollapsedVertex> cvert_list in m_CollapsedVertices.Values)
            {
                foreach (CollapsedVertex cvert in cvert_list)
                {
                    foreach (int key in cvert.indexes)
                    {
                        if (m_LookupTable.ContainsKey(key))
                        {
                            m_LookupTable[key] = cvert.unique_index;
                        }
                        else
                        {
                            m_LookupTable.Add(key, cvert.unique_index);
                        }
                    }

                }
            }

            Debug.Log("Finished CollapsedVertexArray: original Verts=" + a_vertices.Length + ", unique Verts=" + m_UniqueVertexCount);
            return;

        }

    }

    /////////////////////////////////////////////////////////////////
    // End: SkinnedMesh vertex index TO Cloth vertex index
    /////////////////////////////////////////////////////////////////
    [SerializeField, HideInInspector]
    CollapsedVertexArray m_CollapsedVerts;

    // Start is called before the first frame update
    void OnEnable()
    {
        GameObject parent = GetComponent<Renderer>().gameObject;
        m_Skinned = parent.GetComponent<SkinnedMeshRenderer>();
        m_Cloth = parent.GetComponent<Cloth>();

        m_SubmeshMeta = new List<SubmeshMeta>();

    }


    public void GenerateLookupTables()
    {
        m_CollapsedVerts = new CollapsedVertexArray(m_Skinned.sharedMesh.vertices);

        if (m_Cloth.vertices.Length == m_CollapsedVerts.Length)
        {
            Debug.Log("collapsed == cloth. Ready for weightmap transfer.");
        }
        else
        {
            Debug.LogError("# collapsed verts (" + m_CollapsedVerts.Length + ") != # cloth verts(" + m_Cloth.vertices.Length + ").  Please fix lookup table.");
        }

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

        for (int vertex_index = submesh.firstVertex; vertex_index < (submesh.firstVertex + submesh.vertexCount); vertex_index++)
        {
            int cloth_vertex = m_CollapsedVerts.LookupIndex(vertex_index);
            newCoefficients[cloth_vertex].maxDistance = weight_value;

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
        //int numVerts = m_Skinned.sharedMesh.vertexCount;
        int numVerts = newCoefficients.Length ;
        ushort[] weights = new ushort[numVerts];
        System.Buffer.BlockCopy(m_BinaryData.bytes, 0, weights, 0, numVerts);

        float simulation_strength = 0.0f;
        //int[] vertex_list = skinned.sharedMesh.GetTriangles(matIndex, false);
        for (int vertex_index = 0; vertex_index < numVerts; vertex_index++)
        {
            //int cloth_index = m_CollapsedVerts.LookupIndex(vertex_index);
            int cloth_index = vertex_index;
            if (cloth_index < 0)
            {
                Debug.LogError("DFORCE IMPORT: dforce weightmap vertex index has no mapping to cloth index");
                continue;
            }
            if (cloth_index >= newCoefficients.Length)
            {
                Debug.LogWarning("DFORCE IMPORT: cloth_corrected_index for dforce weights is greater than coefficient array: " + vertex_index + " vs " + newCoefficients.Length);
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

            newCoefficients[cloth_index].maxDistance = adjusted_simulation_strength;
        }

        m_Cloth.coefficients = newCoefficients;

    }

    struct float3
    {
        public float x;
        public float y;
        public float z;
    };

    public void TestVertData()
    {
        // get verts
        Vector3[] unityVerts = m_Cloth.vertices;

        int numVerts = m_Cloth.vertices.Length;
        float[] dazVertBuffer = new float[numVerts*3];
        int byte_length = sizeof(float) * numVerts*3;
        Debug.Log("byte_length = " + byte_length);
        System.Buffer.BlockCopy(m_TestVertData.bytes, 0, dazVertBuffer, 0, numVerts*3);

        Vector3 unityMax = new Vector3();
        Vector3 unityMin = new Vector3();
        Vector3 dazMax = new Vector3();
        Vector3 dazMin = new Vector3();
        //bool calcDazBounds = true;

        int i;
        for (i=0; i < unityVerts.Length; i++)
        {
            //Vector3 a_vert = m_Skinned.rootBone.TransformPoint(unityVerts[i]);
            //Vector3 a_vert = m_Cloth.transform.TransformPoint(unityVerts[i]);
            Vector3 a_vert = unityVerts[i];
            a_vert *= 100f;

            // calc bounds for unity verts
            unityMax.x = (a_vert.x > unityMax.x) ? a_vert.x : unityMax.x;
            unityMax.y = (a_vert.y > unityMax.y) ? a_vert.y : unityMax.y;
            unityMax.z = (a_vert.z > unityMax.z) ? a_vert.z : unityMax.z;
            unityMin.x = (a_vert.x < unityMin.x) ? a_vert.x : unityMin.x;
            unityMin.y = (a_vert.y < unityMin.y) ? a_vert.y : unityMin.y;
            unityMin.z = (a_vert.z < unityMin.z) ? a_vert.z : unityMin.z;

            //int j;
            //for (j=0; j < numVerts; j++)
            //{
            //    if (calcDazBounds)
            //    {
            //        // calc bounds for unity verts
            //        dazMax.x = (a_vert.x > dazMax.x) ? a_vert.x : dazMax.x;
            //        dazMax.y = (a_vert.y > dazMax.y) ? a_vert.y : dazMax.y;
            //        dazMax.z = (a_vert.z > dazMax.z) ? a_vert.z : dazMax.z;
            //        dazMin.x = (a_vert.x > dazMin.x) ? a_vert.x : dazMin.x;
            //        dazMin.y = (a_vert.y > dazMin.y) ? a_vert.y : dazMin.y;
            //        dazMin.z = (a_vert.z > dazMin.z) ? a_vert.z : dazMin.z;

            //    }

            //    Vector3 b_vert = new Vector3();
            //    b_vert.x = -dazVertBuffer[(j*3)+0] * 0.01f;
            //    b_vert.y = dazVertBuffer[(j*3)+1] * 0.01f;
            //    b_vert.z = dazVertBuffer[(j*3)+2] * 0.01f;

            //    if (Vector3.Distance(a_vert, b_vert) < 0.001f)
            //    {
            //        Debug.Log("unity[" + i + "] == daz[" + j + "]");
            //        break;
            //    }
            //}
            //if (j == numVerts)
            //{
            //    Debug.LogError("Could not match skinned[" + i + "] in any vertex of daz[]");
            //    calcDazBounds = false;
            //}

        }

        int j;
        for (j = 0; j < numVerts; j++)
        {
            Vector3 a_vert = new Vector3();
            a_vert.x = dazVertBuffer[(j*3) + 0] * 1f;
            a_vert.y = dazVertBuffer[(j*3) + 1] * 1f;
            a_vert.z = dazVertBuffer[(j*3) + 2] * 1f;

            // calc bounds for unity verts
            dazMax.x = (a_vert.x > dazMax.x) ? a_vert.x : dazMax.x;
            dazMax.y = (a_vert.y > dazMax.y) ? a_vert.y : dazMax.y;
            dazMax.z = (a_vert.z > dazMax.z) ? a_vert.z : dazMax.z;
            dazMin.x = (a_vert.x < dazMin.x) ? a_vert.x : dazMin.x;
            dazMin.y = (a_vert.y < dazMin.y) ? a_vert.y : dazMin.y;
            dazMin.z = (a_vert.z < dazMin.z) ? a_vert.z : dazMin.z;

        }

        Debug.Log("unityBounds: (" + unityMin.x + " to " + unityMax.x + ", " + unityMin.y + " to " + unityMax.y + ", " + unityMin.z + " to " + unityMax.z + ")");
        Debug.Log("dazBounds: (" + dazMin.x + " to " + dazMax.x + ", " + dazMin.y + " to " + dazMax.y + ", " + dazMin.z + " to " + dazMax.z + ")");

        Debug.Log("Done");

    }

}
