#pragma once
#include "afxdialogex.h"
class CidebDoc;

class CDNewProject : public CDialogEx
{
protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CDNewProject)
public:
	// Dialog Data
	//enum { IDD = IDD_DIALOG_NEWPROJECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

														// Implementation
	CidebDoc* m_Proj;

public:
	CDNewProject();
	~CDNewProject();
	void SetProject(CidebDoc* pProj) { m_Proj = pProj; }
	afx_msg void OnClickedButtonBrowse();
	// Filename
	CString m_FileN;
	CString m_ToolChDir;
	virtual BOOL OnInitDialog();
};
