#pragma once

#include <fbxsdk.h>

class OpenSubdivInterface
{

};

// utility class for subdividing FbxScene
class SubdivideFbxScene
{
public:
	SubdivideFbxScene(FbxScene* pScene, std::map<std::string, int>* pLookupTable)
	{
		m_Scene = pScene;
		m_SubDLevel_NameLookup = pLookupTable;
	}
	virtual ~SubdivideFbxScene() {};

    bool ProcessScene();
    bool SaveClustersToScene(FbxScene* pDestScene);

protected:
    bool ProcessNode(FbxNode* pNode);
    FbxMesh* SubdivideMesh(FbxNode* pNode, FbxMesh* pMesh, int subdLevel);
    bool SaveClustersToNode(FbxScene* pDestScene, FbxNode* pNode);
    FbxMesh* SaveClustersToMesh(FbxScene* pDestScene, FbxNode* pNode, FbxMesh* pMesh);

    FbxScene* m_Scene;
	std::map<std::string, int>* m_SubDLevel_NameLookup;
	std::map<std::string, FbxMesh*> m_FbxMesh_NameLookup;

    struct Vertex {

        // Minimal required interface ----------------------
        Vertex() { }

        Vertex(Vertex const& src) {
            _position[0] = src._position[0];
            _position[1] = src._position[1];
            _position[2] = src._position[2];
        }

        void Clear(void* = 0) {
            _position[0] = _position[1] = _position[2] = 0.0f;
        }

        void AddWithWeight(Vertex const& src, float weight) {
            _position[0] += weight * src._position[0];
            _position[1] += weight * src._position[1];
            _position[2] += weight * src._position[2];
        }

        // Public interface ------------------------------------
        void SetPosition(float x, float y, float z) {
            _position[0] = x;
            _position[1] = y;
            _position[2] = z;
        }

        void SetPosition(double x, double y, double z) {
            _position[0] = (float)x;
            _position[1] = (float)y;
            _position[2] = (float)z;
        }

        void SetVector(FbxVector4 vec) {
            _position[0] = (float)vec[0];
            _position[1] = (float)vec[1];
            _position[2] = (float)vec[2];
        }

        const float* GetPosition() const {
            return _position;
        }

        FbxVector4 GetVector() {
            return FbxVector4(_position[0], _position[1], _position[2], 1.0f);
        }

    private:
        float _position[3];
    };

    typedef Vertex VertexPosition;

    struct SkinWeight {

        // Minimal required interface ----------------------
        SkinWeight() {
            _weight = 0.0f;
        }

        SkinWeight(SkinWeight const& src) {
            _weight = src._weight;
        }

        void Clear(void* = 0) {
            _weight = 0.0f;
        }

        void AddWithWeight(SkinWeight const& src, float weight) {
            _weight += weight * src._weight;
        }

        // Public interface ------------------------------------
        void SetWeight(float weight) {
            _weight = weight;
        }

        const float GetWeight() const {
            return _weight;
        }

    private:
        float _weight;
    };

};

