#include "stdafx.h"
#include "DNewProject.h"
#include "idebDoc.h"
#include "resource.h"

BEGIN_MESSAGE_MAP(CDNewProject, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CDNewProject::OnClickedButtonBrowse)
END_MESSAGE_MAP()
IMPLEMENT_DYNAMIC(CDNewProject, CDialogEx)


CDNewProject::CDNewProject()
	:CDialogEx(IDD_DIALOG_NEWPROJECT),	m_FileN(_T(""))
{
}


CDNewProject::~CDNewProject()
{
}

void CDNewProject::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILEN, m_FileN);
	DDX_Text(pDX, IDC_TOOLCHAINDIR, m_ToolChDir);
}


void CDNewProject::OnClickedButtonBrowse()
{
	UpdateData(TRUE);
	static TCHAR szFilter[] =
		L"Assembly file(*.asm)|*.asm||";
	CString s;
	s.Format(L"%s.asm", s);
	CFileDialog* dlg = new CFileDialog(true, L"f2lproj", s, 0, szFilter, this);
	if (dlg->DoModal() == IDOK) {
		TCHAR szOut[MAX_PATH];
		TCHAR szCWD[FILENAME_MAX];
		m_FileN = dlg->GetPathName();
		GetCurrentDirectory(FILENAME_MAX, szCWD);
		PathRelativePathTo(szOut, szCWD, FILE_ATTRIBUTE_DIRECTORY, m_FileN, FILE_ATTRIBUTE_NORMAL);
		m_FileN = szOut;

		int pos = m_FileN.ReverseFind('.');
		if (pos > 0) {
			m_FileN = m_FileN.Left(pos);
			//s.Format(L"%s.dbg", s);
		}
		UpdateData(FALSE);
	}

}


BOOL CDNewProject::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_ToolChDir = L"..\\bin\\";
	m_FileN = L"..\\sample\\ScopeRunner_7";
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
