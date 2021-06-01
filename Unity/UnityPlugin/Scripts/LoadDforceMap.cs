using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LoadDforceMap : MonoBehaviour
{
    public TextAsset bindata;

    // Start is called before the first frame update
    void Start()
    {
        loadRawWeightMap();
    }

    // Update is called once per frame
    void Update()
    {
        
    }


    void loadRawWeightMap()
    {
        GameObject workingInstance = this.gameObject;


        // DB (2021-05-25): dForce import

        //DForceMaterial dforceMat = _dforceMap.Map[key];
        GameObject parent = GetComponent<Renderer>().gameObject;
        SkinnedMeshRenderer skinned = parent.GetComponent<SkinnedMeshRenderer>();
        Cloth cloth;

        // add Unity Cloth Physics component to gameobject parent of the renderer
        if (parent.GetComponent<Cloth>() == null)
        {
            return;

            //cloth = parent.AddComponent<Cloth>();
            //// assign values from dtuMat
            //cloth.stretchingStiffness = dforceMat.dtuMaterial.Get("Stretch Stiffness").Float;
            //cloth.bendingStiffness = dforceMat.dtuMaterial.Get("Bend Stiffness").Float;
            //cloth.damping = dforceMat.dtuMaterial.Get("Damping").Float;
            //cloth.friction = dforceMat.dtuMaterial.Get("Friction").Float;

            // default values
            //ClothSkinningCoefficient[] newCoefficients = new ClothSkinningCoefficient[cloth.coefficients.Length];
            //for (int coeff_index = 0; coeff_index < newCoefficients.Length; coeff_index++)
            //{
            //    newCoefficients[coeff_index].maxDistance = 0.0f;
            //    newCoefficients[coeff_index].collisionSphereDistance = 0.005f;
            //}
            //cloth.coefficients = newCoefficients;

            // fix SkinnedMeshRenderer boundaries bug
            skinned.updateWhenOffscreen = true;

            //// Add G8F cloth collision rig
            //var searchResult = workingInstance.transform.Find("Cloth Collision Rig");
            //GameObject collision_instance = (searchResult != null) ? searchResult.gameObject : null;
            //if (collision_instance == null)
            //{
            //    GameObject collision_prefab = AssetDatabase.LoadAssetAtPath<GameObject>("Assets/Daz3D/Resources/G8F Collision Rig.prefab");
            //    collision_instance = Instantiate<GameObject>(collision_prefab);
            //    collision_instance.name = "Cloth Collision Rig";
            //    collision_instance.transform.parent = workingInstance.transform;
            //    // merge cloth collision rig to figure root bone
            //    collision_instance.GetComponent<ClothCollisionAssigner>().mergeRig(skinned.rootBone);
            //}
            //ClothCollisionAssigner.ClothConfig clothConfig = new ClothCollisionAssigner.ClothConfig();
            //clothConfig.m_ClothToManage = cloth;
            //clothConfig.m_UpperBody = true;
            //clothConfig.m_LowerBody = true;
            //collision_instance.GetComponent<ClothCollisionAssigner>().addClothConfig(clothConfig);

        }
        else
        {
            cloth = parent.GetComponent<Cloth>();
        }

        // map the materical's submesh's vertices to the correct "Dynamics Strength"
        //float simulation_strength = dforceMat.dtuMaterial.Get("Dynamics Strength").Float;
        //Debug.Log("DEBUG INFO: simulation strength: " + simulation_strength);

        // get vertex list from mesh

        ClothSkinningCoefficient[] newCoefficients = new ClothSkinningCoefficient[cloth.coefficients.Length];
        System.Array.Copy(cloth.coefficients, newCoefficients, newCoefficients.Length);

        //UnityEngine.Rendering.SubMeshDescriptor submesh = skinned.sharedMesh.GetSubMesh(matIndex);
        //for (int vertex_index = submesh.firstVertex; vertex_index < submesh.vertexCount; vertex_index++)
        //{
        //    float value = simulation_strength * 0.2f;
        //    newCoefficients[vertex_index].maxDistance = value;
        //}

        /////////////////////////////////////////////////////
        /// Load Raw Weight Map
        /////////////////////////////////////////////////////
        int numVerts = skinned.sharedMesh.vertexCount;
        ushort[] weights = new ushort[numVerts];
        System.Buffer.BlockCopy(bindata.bytes, 0, weights, 0, numVerts);

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

            float strength_max = 1.0f;
            float strength_min = 0.0f;
            float strength_scale_threshold = 0.5f;
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

        cloth.coefficients = newCoefficients;

    }

}
