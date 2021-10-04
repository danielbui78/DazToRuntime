// OpenSubdiv
#define _USE_MATH_DEFINES // for C++
#include <cmath>

#include <opensubdiv/far/topologyDescriptor.h>
#include <opensubdiv/far/primvarRefiner.h>

//#include <nlohmann/json.hpp>
#include <iostream>
#include <map>

// FBX
//#include "DisplayMesh.h"

//#include "DisplayMaterial.h"
//#include "DisplayTexture.h"
//#include "DisplayLink.h"
//#include "DisplayShape.h"
//#include "DisplayCache.h"

#if defined (FBXSDK_ENV_MAC)
// disable the “format not a string literal and no format arguments” warning since
// the FBXSDK_printf calls made here are all valid calls and there is no secuity risk
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

#include "SubdivideMesh.h"

using namespace OpenSubdiv;
//using json = nlohmann::json;

std::map<std::string, FbxMesh*> subdMeshMap;

//extern bool gVerbose;
//extern json gDtuJson;

FbxMesh* SubdivideMesh(FbxScene* pScene, FbxNode* pParentNode, FbxNode* pNode, FbxMesh* pMesh, int subdLevel)
{
    // Get topology info from FBX mesh object
    int numVertices = pMesh->GetControlPointsCount();
    int numFaces = pMesh->GetPolygonCount();
    std::vector<int> numVertsPerFace(numFaces);
    std::vector<int> vertIndicesPerFace;
    for (int i = 0; i < numFaces; i++)
    {
        numVertsPerFace[i] = pMesh->GetPolygonSize(i);
        for (int j = 0; j < numVertsPerFace[i]; j++)
        {
            int lControlPointIndex = pMesh->GetPolygonVertex(i, j);
            if (lControlPointIndex < 0)
            {
//                DisplayString("            Control Point Index: Invalid index found!");
                continue;
            }
            vertIndicesPerFace.push_back(lControlPointIndex);
        } // for polygonSize
    } // for polygonCount

//    if (gVerbose)
//    {
//        // dump topology info
//        int vertexId = 0;
//        //DisplayString("Mesh Name: ", (char*)pNode->GetName());
//        //DisplayString("    Verts");
//        //DisplayInt("        Number of Verts: ", numVertices);
//        //DisplayString("    Polygons");
//        //DisplayInt("        Number of Faces: ", numFaces);
//        for (int i = 0; i < numFaces; i++)
//        {
//            //DisplayInt("        Polygon ", i);
//            //DisplayInt("            Polygon Size: ", numVertsPerFace[i]);
//            for (int j = 0; j < numVertsPerFace[i]; j++)
//            {
////                DisplayInt("            Control Point Index: ", vertIndicesPerFace[vertexId]);
//                vertexId++;
//            } // for polygonSize
//        } // for polygonCount
////        DisplayString("");
//    }

    // Populate OpenSubdiv topology descriptor
    typedef Far::TopologyDescriptor Descriptor;
    Sdc::SchemeType type = OpenSubdiv::Sdc::SCHEME_CATMARK;
    Sdc::Options options;
    options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_EDGE_ONLY);

    Descriptor desc;
    desc.numVertices = numVertices;
    desc.numFaces = numFaces;
    desc.numVertsPerFace = numVertsPerFace.data();
    desc.vertIndicesPerFace = vertIndicesPerFace.data();

    // Instantiate a Far::TopologyRefiner from the descriptor
    Far::TopologyRefiner* refiner = Far::TopologyRefinerFactory<Descriptor>::Create(desc,
        Far::TopologyRefinerFactory<Descriptor>::Options(type, options));

    // Refine topology
    refiner->RefineUniform(Far::TopologyRefiner::UniformOptions(subdLevel));

    // Determine the sizes for our needs:
    int nCoarseVerts = numVertices;
    int nFineVerts = refiner->GetLevel(subdLevel).GetNumVertices();
    int nTotalVerts = refiner->GetNumVerticesTotal();
    int nTempVerts = nTotalVerts - nCoarseVerts - nFineVerts;

    // Get vertex data from FBX mesh
    std::vector<VertexPosition> coarsePosBuffer(nCoarseVerts);
    FbxVector4* lControlPoints = pMesh->GetControlPoints();
    for (int i = 0; i < nCoarseVerts; i++)
    {
        //coarsePosBuffer[i].SetPosition(lControlPoints[i][0], lControlPoints[i][1], lControlPoints[i][2]);
        coarsePosBuffer[i].SetVector(lControlPoints[i]);
    }

    //if(gVerbose)
    //{   
    //    // dump vertex data
    //    DisplayString("    Verts");
    //    for (int i = 0; i < nCoarseVerts; i++)
    //    {
    //        DisplayInt("        Control Point ", i);
    //        Display3DVector("            Coordinates: ", coarsePosBuffer[i].GetVector());
    //    }
    //    DisplayString("");
    //}

    // Allocate intermediate and final storage to be populated:
    std::vector<VertexPosition> tempPosBuffer(nTempVerts);
    std::vector<VertexPosition> finePosBuffer(nFineVerts);

    VertexPosition* srcPos = &coarsePosBuffer[0];
    VertexPosition* dstPos = &tempPosBuffer[0];

    // Instnatiate a Far::PrimvarRefiner from topology refiner
    Far::PrimvarRefiner primvarRefiner(*refiner);

    for (int level = 1; level < subdLevel; ++level) 
    {
        primvarRefiner.Interpolate(level, srcPos, dstPos);
        srcPos = dstPos, dstPos += refiner->GetLevel(level).GetNumVertices();
    }

    // Interpolate the last level into the separate buffers for our final data:
    primvarRefiner.Interpolate(subdLevel, srcPos, finePosBuffer);

    // save subdivided mesh to FBX
    FbxString meshName = FbxString(pNode->GetName()) + FbxString("_subd");
    FbxMesh* mesh = FbxMesh::Create(pScene, meshName);

    // save interpolated control points
    Far::TopologyLevel topo = refiner->GetLevel(subdLevel);
//    if (nFineVerts != topo.GetNumVertices()) DisplayInt("num vertices error: ", nFineVerts);
    mesh->InitControlPoints(nFineVerts);
    FbxVector4* cp = mesh->GetControlPoints();
    for (int vert = 0; vert < nFineVerts; ++vert)
        cp[vert] = finePosBuffer[vert].GetVector();

    // save refined indices
    int numSubdFaces = topo.GetNumFaces();
    for (int i = 0; i < numSubdFaces; i++)
    {
        Far::ConstIndexArray faceVertices = topo.GetFaceVertices(i);
        mesh->BeginPolygon(-1, -1, false);
        for (int j = 0; j < faceVertices.size(); j++)
            mesh->AddPolygon(faceVertices[j]);
        mesh->EndPolygon();
    } 

    //if (gVerbose)
    //{    
    //    // dump refined mesh
    //    DisplayString("Subdivided: ");
    //    DisplayString("    Verts");
    //    int numVertices = mesh->GetControlPointsCount();
    //    DisplayInt("        Number of Verts: ", nFineVerts);
    //    FbxVector4* lControlPoints = mesh->GetControlPoints();
    //    for (int vert = 0; vert < numVertices; ++vert)
    //        Display3DVector("            Coordinates: ", lControlPoints[vert]);

    //    DisplayString("    Polygons");
    //    int numSubdFaces = mesh->GetPolygonCount();
    //    DisplayInt("        Number of Faces: ", numSubdFaces);
    //    for (int i = 0; i < numSubdFaces; i++)
    //    {
    //        DisplayInt("        Polygon ", i);
    //        int polygonSize = mesh->GetPolygonSize(i);
    //        DisplayInt("            Polygon Size: ", polygonSize);
    //        for (int j = 0; j < polygonSize; j++)
    //            DisplayInt("            Control Point Index: ", mesh->GetPolygonVertex(i, j));
    //    }
    //    DisplayString("");
    //}

    // Get cluster and skin weight data from FBX geometry links and interplate them
    FbxGeometry* pGeometry = pMesh;
    int lSkinCount = pGeometry->GetDeformerCount(FbxDeformer::eSkin);
    for (int i = 0; i != lSkinCount; ++i)
    {
        //if (gVerbose) 
        //    DisplayInt("  Skin ", i);

        FbxSkin* skin = FbxSkin::Create(pScene, "");
        int lClusterCount = ((FbxSkin*)pGeometry->GetDeformer(i, FbxDeformer::eSkin))->GetClusterCount();
        for (int j = 0; j != lClusterCount; ++j)
        {
            // get cluster data from FBX
            FbxCluster* lCluster = ((FbxSkin*)pGeometry->GetDeformer(i, FbxDeformer::eSkin))->GetCluster(j);
            const char* lClusterModes[] = { "Normalize", "Additive", "Total1" };

            int lIndexCount = lCluster->GetControlPointIndicesCount();
            int* lIndices = lCluster->GetControlPointIndices();
            double* lWeights = lCluster->GetControlPointWeights();

            // populate coarse skin weight buffer
            std::vector<SkinWeight>    coarseSkinWeightBuffer(nCoarseVerts);
            for (int k = 0; k < lIndexCount; k++)
                coarseSkinWeightBuffer[lIndices[k]].SetWeight((float)lWeights[k]);

            // Allocate intermediate and final storage to be populated:
            std::vector<SkinWeight> tempSkinWeightBuffer(nTempVerts);
            std::vector<SkinWeight> fineSkinWeightBuffer(nFineVerts);

            // interpolate skin weights
            SkinWeight* src = &coarseSkinWeightBuffer[0];
            SkinWeight* dst = &tempSkinWeightBuffer[0];
            for (int level = 1; level < subdLevel; ++level) 
            {
                primvarRefiner.Interpolate(level, src, dst);
                src = dst, dst += refiner->GetLevel(level).GetNumVertices();
            }

            // Interpolate the last level into the separate buffers for our final data:
            primvarRefiner.Interpolate(subdLevel, src, fineSkinWeightBuffer);

            // save cluster data
            FbxCluster* cluster = FbxCluster::Create(pScene, lCluster->GetName());
            cluster->SetLink(lCluster->GetLink());
            cluster->SetLinkMode(lCluster->GetLinkMode());
            for (int k = 0; k < nFineVerts; k++)
            {
                float weight = fineSkinWeightBuffer[k].GetWeight();
                if (weight > 0.0f)
                    cluster->AddControlPointIndex(k, weight);
            }

            // copy matrix
            FbxAMatrix lMatrix;
            cluster->SetTransformMatrix(lCluster->GetTransformMatrix(lMatrix));
            cluster->SetTransformLinkMatrix(lCluster->GetTransformLinkMatrix(lMatrix));
            if (lCluster->GetAssociateModel() != NULL)
                cluster->SetTransformAssociateModelMatrix(lCluster->GetTransformAssociateModelMatrix(lMatrix));
            skin->AddCluster(cluster);

            //if (gVerbose) 
            //{
            //    // dump refined cluster data
            //    DisplayInt("    Cluster ", j);
            //    const char* lClusterModes[] = { "Normalize", "Additive", "Total1" };
            //    DisplayString("    Mode: ", lClusterModes[cluster->GetLinkMode()]);

            //    if (cluster->GetLink() != NULL)
            //        DisplayString("        Name: ", (char*)cluster->GetLink()->GetName());

            //    FbxString lString1 = "        Link Indices: ";
            //    FbxString lString2 = "        Weight Values: ";

            //    int lIndexCount = cluster->GetControlPointIndicesCount();
            //    int* lIndices = cluster->GetControlPointIndices();
            //    double* lWeights = cluster->GetControlPointWeights();

            //    for (int k = 0; k < lIndexCount; k++)
            //    {
            //        lString1 += lIndices[k];
            //        lString2 += (float)lWeights[k];

            //        if (k < lIndexCount - 1)
            //        {
            //            lString1 += ", ";
            //            lString2 += ", ";
            //        }
            //    }

            //    lString1 += "\n";
            //    lString2 += "\n";

            //    FBXSDK_printf(lString1);
            //    FBXSDK_printf(lString2);

            //    DisplayString("");

            //    {
            //        DisplayString("      Interpolated: ");

            //        FbxString lString1 = "        Link Indices: ";
            //        FbxString lString2 = "        Weight Values: ";

            //        int lIndexCount = nFineVerts;
            //        for (int k = 0; k < lIndexCount; k++)
            //        {
            //            lString1 += k;
            //            lString2 += fineSkinWeightBuffer[k].GetWeight();

            //            if (k < lIndexCount - 1)
            //            {
            //                lString1 += ", ";
            //                lString2 += ", ";
            //            }
            //        }

            //        lString1 += "\n";
            //        lString2 += "\n";

            //        FBXSDK_printf(lString1);
            //        FBXSDK_printf(lString2);

            //        DisplayString("");
            //    }

            //    FbxAMatrix lMatrix;
            //    lMatrix = cluster->GetTransformMatrix(lMatrix);
            //    Display3DVector("        Transform Translation: ", lMatrix.GetT());
            //    Display3DVector("        Transform Rotation: ", lMatrix.GetR());
            //    Display3DVector("        Transform Scaling: ", lMatrix.GetS());

            //    lMatrix = cluster->GetTransformLinkMatrix(lMatrix);
            //    Display3DVector("        Transform Link Translation: ", lMatrix.GetT());
            //    Display3DVector("        Transform Link Rotation: ", lMatrix.GetR());
            //    Display3DVector("        Transform Link Scaling: ", lMatrix.GetS());

            //    if (cluster->GetAssociateModel() != NULL)
            //    {
            //        lMatrix = cluster->GetTransformAssociateModelMatrix(lMatrix);
            //        DisplayString("        Associate Model: ", (char*)cluster->GetAssociateModel()->GetName());
            //        Display3DVector("        Associate Model Translation: ", lMatrix.GetT());
            //        Display3DVector("        Associate Model Rotation: ", lMatrix.GetR());
            //        Display3DVector("        Associate Model Scaling: ", lMatrix.GetS());
            //    }

            //    DisplayString("");
            //}
        } // for each cluster

        mesh->AddDeformer(skin);
    } // for each skin

    return mesh;
}

//static int GetNodeSubdivisionLevel(std::string name)
//{
//    for (auto s : gDtuJson["Subdivisions"])
//        if (name == s["Asset Name"])
//            return s["Value"];
//    return -1;
//}

bool ProcessNode(FbxScene* pScene, FbxNode* pParentNode, FbxNode* pNode, int SubDLevel)
{
    FbxNodeAttribute::EType lAttributeType;

    if (pNode->GetNodeAttribute() == NULL)
    {
        FBXSDK_printf("NULL Node Attribute\n\n");
    }
    else
    {
        lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

        switch (lAttributeType)
        {
        default:
            break;
        case FbxNodeAttribute::eMarker:
            break;

        case FbxNodeAttribute::eSkeleton:
            break;

        case FbxNodeAttribute::eMesh:
        {
            std::string name = pNode->GetName();
            //int nodeSubdLevel = GetNodeSubdivisionLevel(name);
            int nodeSubdLevel = SubDLevel;
            if (nodeSubdLevel > 0)
            {
                //DisplayString("Interpolating skin weights of mesh: ", name.c_str());
                //DisplayInt("Sudivision level: ", nodeSubdLevel);
                subdMeshMap[name] = SubdivideMesh(pScene, pParentNode, pNode, (FbxMesh*)pNode->GetNodeAttribute(), nodeSubdLevel);
            }
            break;
        }
        case FbxNodeAttribute::eNurbs:
            break;

        case FbxNodeAttribute::ePatch:
            break;

        case FbxNodeAttribute::eCamera:
            break;

        case FbxNodeAttribute::eLight:
            break;

        case FbxNodeAttribute::eLODGroup:
            break;
        }
    }

    for (int i = 0; i < pNode->GetChildCount(); i++)
    {
        ProcessNode(pScene, pNode, pNode->GetChild(i), SubDLevel);
    }

    return true;
}

bool ProcessScene(FbxScene* pScene, int SubDLevel)
{
    int i;
    FbxNode* lNode = pScene->GetRootNode();

    if (lNode)
    {
        for (i = 0; i < lNode->GetChildCount(); i++)
        {
            ProcessNode(pScene, lNode, lNode->GetChild(i), SubDLevel);
        }
    }

    return true;
}

FbxMesh* SaveClustersToMesh(FbxScene* pScene, FbxNode* pParentNode, FbxNode* pNode, FbxMesh* pMesh)
{
    std::string name = pNode->GetName();
    auto it = subdMeshMap.find(name);
    if (it == subdMeshMap.end())
        return nullptr;

//    DisplayString("Saving clusters to mesh name: ", name.c_str());

    FbxMesh* subdMesh = it->second;

//    if (gVerbose)
//        DisplayString("subd mesh name: ", subdMesh->GetName());

    // Get cluster and skin weight data from subd geometry links and save them
    FbxGeometry* geometry = pMesh;
    FbxGeometry* subdGeometry = subdMesh;
    int skinCount = geometry->GetDeformerCount(FbxDeformer::eSkin);
    for (int i = 0; i != skinCount; ++i)
    {
  //      if (gVerbose)
//            DisplayInt("  Skin ", i);

        FbxSkin* skin = (FbxSkin*)geometry->GetDeformer(i, FbxDeformer::eSkin);
        FbxSkin* subdSkin = (FbxSkin*)subdGeometry->GetDeformer(i, FbxDeformer::eSkin);
        int lClusterCount = subdSkin->GetClusterCount();
        for (int j = 0; j != lClusterCount; ++j)
        {
            FbxCluster* cluster = skin->GetCluster(j);
            FbxCluster* tmp = FbxCluster::Create(pScene, cluster->GetName());

            // set the original cluster data aside to tmp cluster
            FbxAMatrix lMatrix;
            tmp->SetLink(cluster->GetLink());
            tmp->SetLinkMode(cluster->GetLinkMode());
            tmp->SetTransformMatrix(cluster->GetTransformMatrix(lMatrix));
            tmp->SetTransformLinkMatrix(cluster->GetTransformLinkMatrix(lMatrix));
            if (cluster->GetAssociateModel() != NULL)
                tmp->SetTransformAssociateModelMatrix(cluster->GetTransformAssociateModelMatrix(lMatrix));

            // reset cluster and restore data from tmp
            cluster->Reset();
            cluster->SetLink(tmp->GetLink());
            cluster->SetLinkMode(tmp->GetLinkMode());
            cluster->SetTransformMatrix(tmp->GetTransformMatrix(lMatrix));
            cluster->SetTransformLinkMatrix(tmp->GetTransformLinkMatrix(lMatrix));
            if (tmp->GetAssociateModel() != NULL)
                cluster->SetTransformAssociateModelMatrix(tmp->GetTransformAssociateModelMatrix(lMatrix));

            // get interpolated skin weight data from subd cluster and save it to the original cluster
            FbxCluster* subdCluster = subdSkin->GetCluster(j);
            int lIndexCount = subdCluster->GetControlPointIndicesCount();
            int* lIndices = subdCluster->GetControlPointIndices();
            double* lWeights = subdCluster->GetControlPointWeights();
            for (int k = 0; k < lIndexCount; k++)
                cluster->AddControlPointIndex(lIndices[k], lWeights[k]);

            //if (gVerbose)
            //{
            //    // dump refined cluster data
            //    DisplayInt("    Cluster ", j);
            //    const char* lClusterModes[] = { "Normalize", "Additive", "Total1" };
            //    DisplayString("    Mode: ", lClusterModes[cluster->GetLinkMode()]);

            //    if (cluster->GetLink() != NULL)
            //        DisplayString("        Name: ", (char*)cluster->GetLink()->GetName());

            //    FbxString lString1 = "        Link Indices: ";
            //    FbxString lString2 = "        Weight Values: ";

            //    int lIndexCount = cluster->GetControlPointIndicesCount();
            //    int* lIndices = cluster->GetControlPointIndices();
            //    double* lWeights = cluster->GetControlPointWeights();

            //    for (int k = 0; k < lIndexCount; k++)
            //    {
            //        lString1 += lIndices[k];
            //        lString2 += (float)lWeights[k];

            //        if (k < lIndexCount - 1)
            //        {
            //            lString1 += ", ";
            //            lString2 += ", ";
            //        }
            //    }

            //    lString1 += "\n";
            //    lString2 += "\n";

            //    FBXSDK_printf(lString1);
            //    FBXSDK_printf(lString2);

            //    DisplayString("");

            //    FbxAMatrix lMatrix;
            //    lMatrix = cluster->GetTransformMatrix(lMatrix);
            //    Display3DVector("        Transform Translation: ", lMatrix.GetT());
            //    Display3DVector("        Transform Rotation: ", lMatrix.GetR());
            //    Display3DVector("        Transform Scaling: ", lMatrix.GetS());

            //    lMatrix = cluster->GetTransformLinkMatrix(lMatrix);
            //    Display3DVector("        Transform Link Translation: ", lMatrix.GetT());
            //    Display3DVector("        Transform Link Rotation: ", lMatrix.GetR());
            //    Display3DVector("        Transform Link Scaling: ", lMatrix.GetS());

            //    if (cluster->GetAssociateModel() != NULL)
            //    {
            //        lMatrix = cluster->GetTransformAssociateModelMatrix(lMatrix);
            //        DisplayString("        Associate Model: ", (char*)cluster->GetAssociateModel()->GetName());
            //        Display3DVector("        Associate Model Translation: ", lMatrix.GetT());
            //        Display3DVector("        Associate Model Rotation: ", lMatrix.GetR());
            //        Display3DVector("        Associate Model Scaling: ", lMatrix.GetS());
            //    }

            //    DisplayString("");
            //}
        } // for each cluster
    } // for each skin

    return pMesh;
}

bool SaveClustersToNode(FbxScene* pScene, FbxNode* pParentNode, FbxNode* pNode)
{
    FbxNodeAttribute::EType lAttributeType;

    if (pNode->GetNodeAttribute() == NULL)
    {
        FBXSDK_printf("NULL Node Attribute\n\n");
    }
    else
    {
        lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

        switch (lAttributeType)
        {
        default:
            break;
        case FbxNodeAttribute::eMarker:
            break;

        case FbxNodeAttribute::eSkeleton:
            break;

        case FbxNodeAttribute::eMesh:
            SaveClustersToMesh(pScene, pParentNode, pNode, (FbxMesh*)pNode->GetNodeAttribute());
            break;

        case FbxNodeAttribute::eNurbs:
            break;

        case FbxNodeAttribute::ePatch:
            break;

        case FbxNodeAttribute::eCamera:
            break;

        case FbxNodeAttribute::eLight:
            break;

        case FbxNodeAttribute::eLODGroup:
            break;
        }
    }

    for (int i = 0; i < pNode->GetChildCount(); i++)
    {
        SaveClustersToNode(pScene, pNode, pNode->GetChild(i));
    }

    return true;
}

bool SaveClustersToScene(FbxScene* pScene)
{
    int i;
    FbxNode* lNode = pScene->GetRootNode();

    if (lNode)
    {
        for (i = 0; i < lNode->GetChildCount(); i++)
        {
            SaveClustersToNode(pScene, lNode, lNode->GetChild(i));
        }
    }

    return true;
}



