#include "stdafx.h"
#include "WorkspaceView.h"


CWorkspaceView::CWorkspaceView()
	:m_Workspace(NULL)
{
}


CWorkspaceView::~CWorkspaceView()
{
}

void CWorkspaceView::RegisterToActiveWorkspace()
{
	m_Workspace= CWorkspaceSingleton::GetActiveWorkspace();
	if (m_Workspace) {
		m_Workspace->RegisterView(this);
	}
}

void CWorkspaceView::RegisterWorkspace(CWorkspace * ws)
{
	m_Workspace = ws;
	if (m_Workspace) {
		m_Workspace->RegisterView(this);
	}
}
