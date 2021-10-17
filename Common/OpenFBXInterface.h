#pragma once

#include <QObject>
#include <QString>

#ifdef __APPLE__
#define USING_LIBSTDCPP     1
#endif
#include <fbxsdk.h>


// FBX Interface class based upon AutoDesk FBX SDK
class OpenFBXInterface : public QObject 
{
	Q_OBJECT

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

	Q_INVOKABLE bool LoadScene(FbxScene* pScene, QString sFilename);
	Q_INVOKABLE bool SaveScene(FbxScene* pScene, QString sFilename, int nFileFormat = -1, bool bEmbedMedia = false);
	Q_INVOKABLE FbxScene* CreateScene(QString sSceneName);

	Q_INVOKABLE bool LoadScene(QString sFilename) { return LoadScene(m_DefaultScene, sFilename); };
	Q_INVOKABLE bool SaveScene(QString sFilename, int nFileFormat = -1, bool bEmbedMedia = false) { return SaveScene(m_DefaultScene, sFilename, nFileFormat, bEmbedMedia); };

	Q_INVOKABLE FbxManager* GetManager() { return m_fbxManager; }
	Q_INVOKABLE FbxIOSettings* GetSettigns() { return m_fbxIOSettings; }
	Q_INVOKABLE QString GetErrorString() { return m_ErrorString; }
	Q_INVOKABLE int GetErrorCode() { return m_ErrorCode; }

protected:
	static OpenFBXInterface* singleton;

	FbxManager* m_fbxManager;
	FbxIOSettings* m_fbxIOSettings;
	FbxScene* m_DefaultScene;

	QString m_ErrorString;
	int m_ErrorCode;

};