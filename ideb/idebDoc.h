
// idebDoc.h : interface of the CidebDoc class
//


#pragma once
#define ACTUAL_DOCVERSION_MAJOR (1)
#define ACTUAL_DOCVERSION_MINOR (1)
#include "CSimulator.h"
#include "iipccom.h"
#include "ipccom.h"

class CidebDoc : public CDocument
{
protected: // create from serialization only
	CidebDoc();
	DECLARE_DYNCREATE(CidebDoc)

// Attributes
public:
	CStringArray m_AsmFiles;
	CString m_AsmbFlags;
	BOOL m_AsmbVerbose;
	int m_DocVersionMajor;
	int m_DocVersionMinor;
	int m_SimbSteps;
	CString m_SimTargetBinFileName;
	CSimulator m_Simulator;

	BOOL m_SimulationRunning;
// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	void ClearHeatMap();
	void OnTimer(UINT_PTR nIDEvent);
	virtual ~CidebDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CIpcComSE m_ipcAsmb;
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	afx_msg void OnDebugStep();
	afx_msg void OnUpdateDebugStep(CCmdUI *pCmdUI);
	afx_msg void OnDebugReload();
	afx_msg void OnDebugReset();
	afx_msg void OnDebugRun();
	afx_msg void OnDebugPause();
	afx_msg void OnDebugBrakepointpc();
	afx_msg void OnUpdateDebugRun(CCmdUI *pCmdUI);
	afx_msg void OnUpdateDebugReload(CCmdUI *pCmdUI);
	afx_msg void OnDebugClearheatmap();
	afx_msg void OnBuildCompile();
	afx_msg void OnBuildClean();
	afx_msg void OnBuildBuildall();
};
