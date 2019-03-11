#pragma once
#include "WorkspaceView.h"
// CAsmDoc document

class CAsmDoc : public CDocument, public CWorkspaceView
{
	DECLARE_DYNCREATE(CAsmDoc)

public:
	CAsmDoc();
	virtual ~CAsmDoc();
	virtual void DoWorkspaceUpdate();
	void OnWorkspaceUpdate() {}
#ifndef _WIN32_WCE
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual BOOL OnNewDocument();
public: //temporary
	class LineData {
	public:
		int number;
		int memAddress;
		int modified;
		CString text;
	};

	CArray<LineData> m_lines;
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
};
