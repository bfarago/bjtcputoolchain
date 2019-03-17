#pragma once
#include "Workspace.h"

class CWorkspaceEvent {
public:
	enum {
		GoToSourceLine,
		EnsureSourceLineVisible,
		OpenDocumentAsm
	}EventType;
	//For SourceLine
	int Line;
	short FileId;
	// for OpenDocumentAsm
	LPCTSTR FileName;
	LPCTSTR Path;
};

class CWorkspaceView
{
public:
	CWorkspaceView();
	~CWorkspaceView();
	void RegisterToActiveWorkspace();
	void RegisterWorkspace(CWorkspace* ws);
	void UnRegister();
	CWorkspace* GetWorkspace() { return m_Workspace; }
	CSimulator* GetSimulator() { 
		if (m_Workspace)
			return m_Workspace->GetSimulator();
		return NULL;
	}
	virtual void DoWorkspaceUpdate() = 0;
	virtual void OnWorkspaceUpdate() = 0;
	virtual BOOL DoWorkspaceEvent(CWorkspaceEvent* pEvent) {
		if (m_Workspace)
			return m_Workspace->SendEvent(pEvent);
		return  FALSE;
	}
	virtual BOOL OnWorkspaceEvent(CWorkspaceEvent* pEvent) { return FALSE; }
protected:
	CWorkspace* m_Workspace;
};

