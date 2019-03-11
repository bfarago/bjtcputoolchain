// CAsmDoc.cpp : implementation file
//

#include "stdafx.h"
#include "ideb.h"
#include "AsmDoc.h"


// CAsmDoc

IMPLEMENT_DYNCREATE(CAsmDoc, CDocument)
BEGIN_MESSAGE_MAP(CAsmDoc, CDocument)
END_MESSAGE_MAP()

CAsmDoc::CAsmDoc()
{
}

BOOL CAsmDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	RegisterToActiveWorkspace();
	return TRUE;
}

CAsmDoc::~CAsmDoc()
{
}

void CAsmDoc :: DoWorkspaceUpdate() {
	//Invalidate();
}

// CAsmDoc diagnostics

#ifdef _DEBUG
void CAsmDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CAsmDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CAsmDoc serialization

void CAsmDoc::Serialize(CArchive& ar)
{
	RegisterToActiveWorkspace();
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		
	}
}
#endif


// CAsmDoc commands
#include "CSimulator.h"

BOOL CAsmDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	RegisterToActiveWorkspace();
	if (!__super::OnOpenDocument(lpszPathName))
		return FALSE;
	CStdioFile  f;
	if (f.Open(lpszPathName, CFile::modeRead)) {
		CSimulator* pSimulator = GetSimulator();
		m_lines.RemoveAll();
		int number = 0;
		CString strLine;
		//int len= f.GetLength();
		while (f.ReadString(strLine))
		{
			LineData data;
			data.number = ++number;
			data.memAddress = -1;
			if (pSimulator) data.memAddress = pSimulator->SearchLine(number);
			data.modified = 0;
			data.text = strLine;
			m_lines.Add(data);
			//sscanf_s(strLine, "%g %g\n", &data.x, &data.y);
		}
		f.Close();
	}

	return TRUE;
}


BOOL CAsmDoc::OnSaveDocument(LPCTSTR lpszPathName)
{



	return __super::OnSaveDocument(lpszPathName);
}
