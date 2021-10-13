/****************************************************************************************
 Portions of this file are based on source code from
   https://github.com/cocktailboy/daz-to-ue4-subd-skin
   and is used under license below:
 
MIT License

Copyright(c) 2021 cocktailboy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#define _USE_MATH_DEFINES
#include <cmath>

#include <iostream>
#include <map>

#include <opensubdiv/far/topologyDescriptor.h>
#include <opensubdiv/far/primvarRefiner.h>

#include "OpenFBXInterface.h"
#include "OpenSubdivInterface.h"


bool SubdivideFbxScene::ProcessScene()
{
	FbxNode* pNode = m_Scene->GetRootNode();
	if (pNode)
	{
		for (int i = 0; i < pNode->GetChildCount(); i++)
		{
			if (ProcessNode(pNode->GetChild(i)) == false)
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool SubdivideFbxScene::ProcessNode(FbxNode* pNode)
{
	FbxNodeAttribute::EType nAttributeType;

	if (pNode->GetNodeAttribute() != NULL)
	{
		nAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

		switch (nAttributeType)
		{
		case FbxNodeAttribute::eMesh:
		{
			std::string name = pNode->GetName();
			int nodeSubDLevel = -1;
			try
			{
				nodeSubDLevel = m_SubDLevel_NameLookup->at(name);
			}
			catch (std::out_of_range)
			{
				nodeSubDLevel = -1;
			}
			if (nodeSubDLevel > 0)
			{
				m_FbxMesh_NameLookup[name] = SubdivideMesh(pNode, (FbxMesh*)pNode->GetNodeAttribute(), nodeSubDLevel);
			}
			break;
		}

		case FbxNodeAttribute::eMarker:
			break;
		case FbxNodeAttribute::eSkeleton:
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
		default:
			break;
		}
	}

	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		ProcessNode(pNode->GetChild(i));
	}

	return true;
}

FbxMesh* SubdivideFbxScene::SubdivideMesh(FbxNode* pNode, FbxMesh* pMesh, int subdLevel)
{
	// Get topology from FbxMesh
	int numVertices = pMesh->GetControlPointsCount();
	int numFaces = pMesh->GetPolygonCount();
	std::vector<int> numVertsPerFace(numFaces);
	std::vector<int> vertIndicesPerFace;
	for (int i = 0; i < numFaces; i++)
	{
		numVertsPerFace[i] = pMesh->GetPolygonSize(i);
		for (int j = 0; j < numVertsPerFace[i]; j++)
		{
			int nControlPointIndex = pMesh->GetPolygonVertex(i,j);
			if (nControlPointIndex < 0)
			{
				// skip invalid index
				continue;
			}
			vertIndicesPerFace.push_back(nControlPointIndex);
		}
	}

	// OpenSubdiv topology descriptor
	typedef OpenSubdiv::Far::TopologyDescriptor Descriptor;
	OpenSubdiv::Sdc::SchemeType type = OpenSubdiv::Sdc::SCHEME_CATMARK;
	OpenSubdiv::Sdc::Options options;
	options.SetVtxBoundaryInterpolation(OpenSubdiv::Sdc::Options::VTX_BOUNDARY_EDGE_ONLY);

	Descriptor desc;
	desc.numVertices = numVertices;
	desc.numFaces = numFaces;
	desc.numVertsPerFace = numVertsPerFace.data();
	desc.vertIndicesPerFace = vertIndicesPerFace.data();

	// Make topology refiner
	OpenSubdiv::Far::TopologyRefiner* refiner = OpenSubdiv::Far::TopologyRefinerFactory<Descriptor>::Create(
		desc,
		OpenSubdiv::Far::TopologyRefinerFactory<Descriptor>::Options(type, options)
	);

	// Refine topology
	refiner->RefineUniform(OpenSubdiv::Far::TopologyRefiner::UniformOptions(subdLevel));

	// calculate number of additional vertices
	int nCoarseVerts = numVertices;
	int nFineVerts = refiner->GetLevel(subdLevel).GetNumVertices();
	int nTotalVerts = refiner->GetNumVerticesTotal();
	int nTempVerts = nTotalVerts - nCoarseVerts - nFineVerts;

	// allocate storage for additional vertices
	std::vector<VertexPosition> tempPosBuffer(nTempVerts);
	std::vector<VertexPosition> finePosBuffer(nFineVerts);

	// get vertex data from source mesh
	std::vector<VertexPosition> coarsePosBuffer(nCoarseVerts);
	FbxVector4* nControlPoints = pMesh->GetControlPoints();
	for (int i = 0; i < nCoarseVerts; i++)
	{
		coarsePosBuffer[i].SetVector(nControlPoints[i]);
	}

	VertexPosition *srcPos = &coarsePosBuffer[0];
	VertexPosition *dstPos = &tempPosBuffer[0];
	
	// Make Primvar::refiner from topology refiner
	OpenSubdiv::Far::PrimvarRefiner primvarRefiner(*refiner);

	// Do intermediate interpolation steps
	for (int level = 1; level < subdLevel; ++level)
	{
		primvarRefiner.Interpolate(level, srcPos, dstPos);
		srcPos = dstPos, dstPos += refiner->GetLevel(level).GetNumVertices();
	}

	// Do last Interpolation into buffer for final data
	primvarRefiner.Interpolate(subdLevel, srcPos, finePosBuffer);

	// Create FBX mesh to store subdivided data
	FbxString meshName = FbxString(pNode->GetName()) + FbxString("_subd");
	FbxMesh* mesh = FbxMesh::Create(m_Scene, meshName);
	OpenSubdiv::Far::TopologyLevel topo = refiner->GetLevel(subdLevel);
	mesh->InitControlPoints(nFineVerts);
	FbxVector4* pControlPoints = mesh->GetControlPoints();
	for (int vert_index = 0; vert_index < nFineVerts; ++vert_index)
	{
		pControlPoints[vert_index] = finePosBuffer[vert_index].GetVector();
	}
	int numSubDFaces = topo.GetNumFaces();
	for (int i = 0; i < numSubDFaces; i++)
	{
		OpenSubdiv::Far::ConstIndexArray faceVertices = topo.GetFaceVertices(i);
		mesh->BeginPolygon(-1, -1, false);
		for (int j = 0; j < faceVertices.size(); j++)
		{
			mesh->AddPolygon(faceVertices[j]);
		}
		mesh->EndPolygon();
	}

	// Get cluster and skin weight data from FBX geometry links and interplate them
	FbxGeometry* pGeometry = pMesh;
	int lSkinCount = pGeometry->GetDeformerCount(FbxDeformer::eSkin);
	for (int i = 0; i != lSkinCount; ++i)
	{
		FbxSkin* skin = FbxSkin::Create(m_Scene, "");
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
			FbxCluster* cluster = FbxCluster::Create(m_Scene, lCluster->GetName());
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

		} // for each cluster

		mesh->AddDeformer(skin);
	} // for each skin

	return mesh;
}

bool SubdivideFbxScene::SaveClustersToScene(FbxScene* pDestScene)
{
	FbxNode* pNode = pDestScene->GetRootNode();
	if (pNode)
	{
		for (int i = 0; i < pNode->GetChildCount(); i++)
		{
			if (SaveClustersToNode(pDestScene, pNode->GetChild(i)) == false)
				return false;
		}
	}

	return true;
}

bool SubdivideFbxScene::SaveClustersToNode(FbxScene* pDestScene, FbxNode* pNode)
{
	FbxNodeAttribute::EType nAttributeType;

	if (pNode->GetNodeAttribute() != NULL)
	{
		nAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

		switch (nAttributeType)
		{
		default:
			break;
		case FbxNodeAttribute::eMarker:
			break;

		case FbxNodeAttribute::eSkeleton:
			break;

		case FbxNodeAttribute::eMesh:
			SaveClustersToMesh(pDestScene, pNode, (FbxMesh*)pNode->GetNodeAttribute());
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
		SaveClustersToNode(pDestScene, pNode->GetChild(i));
	}

	return true;
}

FbxMesh* SubdivideFbxScene::SaveClustersToMesh(FbxScene* pDestScene, FbxNode* pNode, FbxMesh* pMesh)
{
	std::string name = pNode->GetName();
	auto it = m_FbxMesh_NameLookup.find(name);
	if (it == m_FbxMesh_NameLookup.end())
		return nullptr;

	FbxMesh* subdMesh = it->second;

	// Get cluster and skin weight data from subd geometry links and save them
	FbxGeometry* geometry = pMesh;
	FbxGeometry* subdGeometry = subdMesh;
	int skinCount = geometry->GetDeformerCount(FbxDeformer::eSkin);
	for (int i = 0; i != skinCount; ++i)
	{
		FbxSkin* skin = (FbxSkin*)geometry->GetDeformer(i, FbxDeformer::eSkin);
		FbxSkin* subdSkin = (FbxSkin*)subdGeometry->GetDeformer(i, FbxDeformer::eSkin);
		int lClusterCount = subdSkin->GetClusterCount();
		for (int j = 0; j != lClusterCount; ++j)
		{
			FbxCluster* cluster = skin->GetCluster(j);
			FbxCluster* tmp = FbxCluster::Create(pDestScene, cluster->GetName());

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
			{
				cluster->AddControlPointIndex(lIndices[k], lWeights[k]);
			}

		} // for each cluster
	} // for each skin

	return pMesh;
}
