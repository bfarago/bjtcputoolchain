#pragma once
#include "Workspace.h"
class CWorkspaceView
{
public:
	CWorkspaceView();
	~CWorkspaceView();
	void RegisterWorkspace(CWorkspace* ws);
	CWorkspace* GetWorkspace() { return m_Workspace; }
	virtual void DoWorkspaceUpdate() = 0;
protected:
	CWorkspace* m_Workspace;
};

