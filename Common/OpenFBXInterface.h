#pragma once

#include <QString>

#ifdef __APPLE__
#define USING_LIBSTDCPP     1
#endif
#include <fbxsdk.h>


// FBX Interface class based upon AutoDesk FBX SDK
class OpenFBXInterface
{
public:
	static OpenFBXInterface* GetInterface()
	{
		if (singleton == nullptr)
		{
			singleton = new OpenFBXInterface();
		}
		return singleton;
	}

	OpenFBXInterface();
	~OpenFBXInterface();

	bool LoadScene(FbxScene* pScene, QString sFilename);
	bool SaveScene(FbxScene* pScene, QString sFilename, int nFileFormat = -1, bool bEmbedMedia = false);
	FbxScene* CreateScene(QString sSceneName);

	FbxManager* GetManager() { return m_fbxManager; }
	FbxIOSettings* GetSettigns() { return m_fbxIOSettings; }
	QString GetErrorString() { return m_ErrorString; }
	int GetErrorCode() { return m_ErrorCode; }

protected:
	static OpenFBXInterface* singleton;

	FbxManager* m_fbxManager;
	FbxIOSettings* m_fbxIOSettings;

	QString m_ErrorString;
	int m_ErrorCode;

};