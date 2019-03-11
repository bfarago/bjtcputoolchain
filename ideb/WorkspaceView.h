#pragma once
#include "Workspace.h"
class CWorkspaceView
{
public:
	CWorkspaceView();
	~CWorkspaceView();
	void RegisterToActiveWorkspace();
	void RegisterWorkspace(CWorkspace* ws);
	CWorkspace* GetWorkspace() { return m_Workspace; }
	CSimulator* GetSimulator() { 
		if (m_Workspace) return m_Workspace->GetSimulator();
		return NULL;
	}
	virtual void DoWorkspaceUpdate() = 0;
	virtual void OnWorkspaceUpdate() = 0;
protected:
	CWorkspace* m_Workspace;
};

