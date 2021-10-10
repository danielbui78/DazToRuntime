#ifndef _SUBDIVIDE_MESH_H
#define _SUBDIVIDE_MESH_H

#define USING_LIBSTDCPP 1
#include <fbxsdk.h>

//------------------------------------------------------------------------------
// Vertex container implementation.
//
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

//------------------------------------------------------------------------------
// Skin weight container implementation.
//
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

FbxMesh* SubdivideMesh(FbxScene* pScene, FbxNode* pParentNode, FbxNode* pNode, FbxMesh* pMesh, int subdLevel);
bool ProcessNode(FbxScene* pScene, FbxNode* pParentNode, FbxNode* pNode, int SubDLevel=-1);
bool ProcessScene(FbxScene* pScene, int SubDLevel=-1);

FbxMesh* SaveClustersToMesh(FbxScene* pScene, FbxNode* pParentNode, FbxNode* pNode, FbxMesh* pMesh);
bool SaveClustersToNode(FbxScene* pScene, FbxNode* pParentNode, FbxNode* pNode);
bool SaveClustersToScene(FbxScene* pScene);

#endif // #ifndef _SUBDIVIDE_MESH_H
