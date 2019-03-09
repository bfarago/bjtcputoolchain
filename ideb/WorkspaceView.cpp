#include "stdafx.h"
#include "WorkspaceView.h"


CWorkspaceView::CWorkspaceView()
	:m_Workspace(NULL)
{
}


CWorkspaceView::~CWorkspaceView()
{
}

void CWorkspaceView::RegisterWorkspace(CWorkspace * ws)
{
	m_Workspace = ws;
}
