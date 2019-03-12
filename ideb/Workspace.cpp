#include "stdafx.h"
#include "Workspace.h"
#include "idebDoc.h"
#include "WorkspaceVIew.h"

CWorkspace::CWorkspace()
{
}


CWorkspace::~CWorkspace()
{
	for (int i = 0; i < m_Views.GetCount(); i++) {
		CWorkspaceView* pV = m_Views.GetAt(i);
		pV->RegisterWorkspace(NULL);
	}
	CWorkspaceSingleton::RegisterWorkspace(NULL);
}

CWorkspace* CWorkspaceSingleton::g_ActiveWorkspace = NULL;

void CWorkspaceSingleton::RegisterWorkspace(CWorkspace * ws)
{
	g_ActiveWorkspace = ws;
}

void CWorkspace::RegisterView(CWorkspaceView * pV)
{
	for (int i = 0; i < m_Views.GetCount(); i++) {
		if (m_Views.GetAt(i) == pV) {
			return;
		}
	}
	m_Views.Add(pV);
}

void CWorkspace::UnRegisterView(CWorkspaceView * pV)
{
	for (int i = 0; i < m_Views.GetCount(); i++) {
		if (m_Views.GetAt(i) == pV) {
			m_Views.RemoveAt(i); i--;
		}
	}
}

void CWorkspace::Update()
{
	for (int i = 0; i < m_Views.GetCount(); i++) {
		CWorkspaceView* pV = m_Views.GetAt(i);
		pV->OnWorkspaceUpdate();
	}
}
