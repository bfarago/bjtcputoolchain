
// idebDoc.cpp : implementation of the CidebDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ideb.h"
#endif

#include "idebDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CidebDoc

IMPLEMENT_DYNCREATE(CidebDoc, CDocument)

BEGIN_MESSAGE_MAP(CidebDoc, CDocument)
	ON_COMMAND(ID_DEBUG_STEP, &CidebDoc::OnDebugStep)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_STEP, &CidebDoc::OnUpdateDebugStep)
	ON_COMMAND(ID_DEBUG_RELOAD, &CidebDoc::OnDebugReload)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_RELOAD, &CidebDoc::OnUpdateDebugReload)
	ON_COMMAND(ID_DEBUG_RESET, &CidebDoc::OnDebugReset)
	ON_COMMAND(ID_DEBUG_RUN, &CidebDoc::OnDebugRun)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_RUN, &CidebDoc::OnUpdateDebugRun)
	ON_COMMAND(ID_DEBUG_PAUSE, &CidebDoc::OnDebugPause)
	ON_COMMAND(ID_DEBUG_BRAKEPOINTPC, &CidebDoc::OnDebugBrakepointpc)
	ON_COMMAND(ID_DEBUG_CLEARHEATMAP, &CidebDoc::OnDebugClearheatmap)
	ON_COMMAND(ID_BUILD_COMPILE, &CidebDoc::OnBuildCompile)
END_MESSAGE_MAP()


// CidebDoc construction/destruction

CidebDoc::CidebDoc()
	:m_SimulationRunning(FALSE)
{
	// TODO: add one-time construction code here

}

CidebDoc::~CidebDoc()
{
}

BOOL CidebDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	m_AsmFiles.RemoveAll();
	m_AsmbFlags = "-f -l";
	m_AsmbVerbose = FALSE;
	m_DocVersionMajor = ACTUAL_DOCVERSION_MAJOR;
	m_DocVersionMinor = ACTUAL_DOCVERSION_MINOR;
	m_SimbSteps = 5;
	m_SimTargetBinFileName = _T("a.bin");
	m_SimulationRunning = FALSE;
	return TRUE;
}




// CidebDoc serialization

void CidebDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_DocVersionMajor;
		ar << m_DocVersionMinor;
		ar << m_AsmbFlags;
		ar << m_AsmbVerbose;
		ar << m_SimbSteps;
		ar << m_SimTargetBinFileName;
	}
	else
	{
		ar >> m_DocVersionMajor;
		ar >> m_DocVersionMinor;
		ar >> m_AsmbFlags;
		ar >> m_AsmbVerbose;
		ar >> m_SimbSteps;
		ar >> m_SimTargetBinFileName;
	}
	m_AsmFiles.Serialize(ar);
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CidebDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CidebDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data.
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CidebDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CidebDoc diagnostics

#ifdef _DEBUG
void CidebDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CidebDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CidebDoc commands


void CidebDoc::OnDebugStep()
{
	m_Simulator.SetDocument(this);
	m_Simulator.SetStop(FALSE);
	m_Simulator.Step();
	UpdateAllViews(NULL);
}


void CidebDoc::OnUpdateDebugStep(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}


void CidebDoc::OnDebugReload()
{
	m_Simulator.SetDocument(this);
	m_SimTargetBinFileName = "..\\sample\\test_screen.bin";
	m_Simulator.LoadBinToMemory();
	UpdateAllViews(NULL);
}
void CidebDoc::OnUpdateDebugReload(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}


void CidebDoc::OnDebugReset()
{
	m_Simulator.SetDocument(this);
	m_Simulator.Reset();
	UpdateAllViews(NULL);
}
void CidebDoc::ClearHeatMap()
{
}
void CidebDoc::OnTimer(UINT_PTR nIDEvent)
{
	if (1000 == nIDEvent) {
		if (m_SimulationRunning) {
			m_Simulator.SetDocument(this);
			m_Simulator.Step();
			UpdateAllViews(NULL);
		}
	}
}

void CidebDoc::OnDebugRun()
{
	m_SimulationRunning = TRUE;
	//SetTimer(NULL, 1000, 100, 0);
	m_Simulator.SetDocument(this);
	m_Simulator.SetStop(FALSE);
	for (int i = 0; i < 100; i++) {
		if (!m_Simulator.Step())
			break;
	}
	UpdateAllViews(NULL);
}

void CidebDoc::OnUpdateDebugRun(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	//m_SimulationRunning
}


void CidebDoc::OnDebugPause()
{
	KillTimer(NULL, 1000);
	m_SimulationRunning = FALSE;
}


void CidebDoc::OnDebugBrakepointpc()
{
	m_Simulator.SetDocument(this);
	m_Simulator.BrakePC(TRUE);
}





void CidebDoc::OnDebugClearheatmap()
{
	m_Simulator.SetDocument(this);
	m_Simulator.ClearHeatMap();
	UpdateAllViews(NULL);
}


void CidebDoc::OnBuildCompile()
{
	CString param, dir;
	param = L"-i ..\\sample\\test_screen.asm -l";
	dir = L".\\";
	m_ipcAsmb.setHide(true);
	m_ipcAsmb.setDir(dir);
	m_ipcAsmb.setParam(param);
	m_ipcAsmb.setCmd(TEXT("..\\bin\\asmb.exe"));
	m_ipcAsmb.setPipeOutFileName(L"test_screen.log");
	m_ipcAsmb.StartExecute();
	do {
		Sleep(10);
	} while (m_ipcAsmb.isRunning());
}
