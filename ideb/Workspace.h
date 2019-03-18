#pragma once
//forward
class CidebDoc; 
class CSimulator;
class CWorkspace;
class CWorkspaceView;
class CWorkspaceEvent;

class CWorkspaceSingleton {
public:
	static void RegisterWorkspace(CWorkspace* ws);
	static CWorkspace* GetActiveWorkspace() { return g_ActiveWorkspace; }
	//static void RegisterWorkspaceView(CWorkspaceView* ws);
protected:
	static CWorkspace* g_ActiveWorkspace;
};

class CWorkspace
{
public:
	CWorkspace();
	~CWorkspace();
	virtual CStringArray* GetAsmFileList() = 0;
	virtual const CString& GetToolchainDir()const = 0;
	virtual const CString& GetTargetBinFileName()const = 0;
	virtual void SetTargetBinFileName(const CString& s) = 0;
	virtual CidebDoc* GetProjectDocument() = 0;
	virtual CSimulator* GetSimulator() = 0;
	virtual const CString& GetProjectName() = 0;
	virtual void SetProjectName(const CString& s) = 0;

	virtual CString& AsmbFlags() = 0;
	virtual BOOL& AsmbVerbose() = 0;
	virtual CString& AsmbDirIn() = 0;
	virtual CString& AsmbDirOut() = 0;
	virtual void RegisterView(CWorkspaceView* pV);
	virtual void UnRegisterView(CWorkspaceView* pV);
	virtual void Update();
	virtual BOOL SendEvent(CWorkspaceEvent* pEvent, BOOL bBroadcast=TRUE);
	virtual BOOL OnWorkspaceEvent(CWorkspaceEvent* pEvent) { return FALSE; }
protected:
	CArray<CWorkspaceView*> m_Views;
};

