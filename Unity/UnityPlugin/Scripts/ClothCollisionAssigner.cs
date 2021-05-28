using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Serialization;
using System;


public class ClothCollisionAssigner : MonoBehaviour
{
    [FormerlySerializedAs("m_collider_list")]
    public SphereCollider[] m_UpperbodyColliders;
    public SphereCollider[] m_LowerbodyColliders;

    [Serializable]
    public class ClothConfig
    {
        public Cloth m_ClothToManage;
        public bool m_UpperBody;
        public bool m_LowerBody;
    }

    public ClothConfig[] m_ClothConfigurationList;

    private void addCollidersToCloth(SphereCollider[] collider_list, Cloth cloth_component)
    {
        int collider_size = 0;
        foreach (SphereCollider sphere_collider in collider_list)
        {
            if (sphere_collider != null)
            {
                collider_size++;
            }
        }
        if (collider_size % 2 != 0)
        {
            collider_size += 1;
        }
        int original_size = 0;
        if (cloth_component.sphereColliders != null)
        {
            original_size = cloth_component.sphereColliders.Length;
        }
        ClothSphereColliderPair[] colliderpair_list = new ClothSphereColliderPair[original_size + (collider_size / 2)];
        if (original_size > 0)
        {
            Array.Copy(cloth_component.sphereColliders, colliderpair_list, original_size);
        }
        for (int i = 0; i < collider_size / 2; i++)
        {

            colliderpair_list[original_size + i].first = collider_list[i * 2];
            colliderpair_list[original_size + i].second = collider_list[i * 2 + 1];
        }
        cloth_component.sphereColliders = colliderpair_list;

    }

    public void addClothConfig(ClothConfig newConfig)
    {
        int new_size = m_ClothConfigurationList.Length + 1;
        Array.Resize<ClothConfig>(ref m_ClothConfigurationList, new_size);
        m_ClothConfigurationList[new_size - 1] = newConfig;
    }

    // Start is called before the first frame update
    void Start()
    {
        foreach (ClothConfig cloth_config in m_ClothConfigurationList)
        {
            if (cloth_config == null)
                continue;

            // delete existing cloth collisions
            if (cloth_config.m_ClothToManage != null && cloth_config.m_ClothToManage.sphereColliders != null)
                cloth_config.m_ClothToManage.sphereColliders = null;

            // add new collisions
            if (cloth_config.m_UpperBody)
                addCollidersToCloth(m_UpperbodyColliders, cloth_config.m_ClothToManage);
            if (cloth_config.m_LowerBody)
                addCollidersToCloth(m_LowerbodyColliders, cloth_config.m_ClothToManage);
        }
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
