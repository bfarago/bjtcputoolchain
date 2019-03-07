
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
	ON_COMMAND(ID_BUILD_CLEAN, &CidebDoc::OnBuildClean)
	ON_COMMAND(ID_BUILD_BUILDALL, &CidebDoc::OnBuildBuildall)
	ON_COMMAND(ID_DEBUG_STEP_INSTRUCTION, &CidebDoc::OnDebugStepInstruction)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_STEP_INSTRUCTION, &CidebDoc::OnUpdateDebugStepInstruction)
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
	m_Simulator.SetRunMode(rm_StepState);
	m_Simulator.SetStop(FALSE);
	m_Simulator.Step();
	UpdateAllViews(NULL);
}

void CidebDoc::OnUpdateDebugStep(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_SimulationRunning);
}

void CidebDoc::OnDebugStepInstruction()
{
	m_Simulator.SetDocument(this);
	m_Simulator.SetRunMode(rm_StepInstruction);
	m_Simulator.SetStop(FALSE);
	m_Simulator.RunQuick();
	UpdateAllViews(NULL);
}

void CidebDoc::OnUpdateDebugStepInstruction(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_SimulationRunning);
}


void CidebDoc::OnDebugReload()
{
	m_Simulator.SetDocument(this);
	//m_SimTargetBinFileName = "..\\sample\\test_screen.bin";
	m_SimTargetBinFileName = "..\\sample\\ScopeRunner_7.bin";
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
			//m_Simulator.ProcessHeatMaps();
			for (int i = 0; i < SIM_STEPS_PER_TIMER; i++) {
				if (!m_Simulator.RunQuick()) {
					m_SimulationRunning = FALSE;
					m_Simulator.SetRunMode(rm_StepInstruction);
					break;
				}
			}
			UpdateAllViews(NULL);
		}
	}
}

void CidebDoc::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!m_SimulationRunning) return;
	m_Simulator.OnKeyDown(nChar, nRepCnt, nFlags);
}

void CidebDoc::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!m_SimulationRunning) return;
	m_Simulator.OnKeyUp(nChar, nRepCnt, nFlags);
}

void CidebDoc::OnDebugRun()
{
	m_SimulationRunning = TRUE;
	m_Simulator.SetDocument(this);
	m_Simulator.SetStop(FALSE);
	m_Simulator.SetRunMode(rm_Run);
	UpdateAllViews(NULL);
}

void CidebDoc::OnUpdateDebugRun(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	//m_SimulationRunning
}


void CidebDoc::OnDebugPause()
{
	//KillTimer(NULL, 1000);
	m_Simulator.SetRunMode(rm_StepInstruction);
	m_SimulationRunning = FALSE;
	UpdateAllViews(NULL);
}


void CidebDoc::OnDebugBrakepointpc()
{
	m_Simulator.SetDocument(this);
	m_Simulator.BrakePC(TRUE);
	UpdateAllViews(NULL);
}





void CidebDoc::OnDebugClearheatmap()
{
	m_Simulator.SetDocument(this);
	m_Simulator.ClearHeatMap();
	UpdateAllViews(NULL);
}
#include "MainFrm.h"
#include "OutputWnd.h"
void CidebDoc::OnBuildCompile()
{
	CString s;
	CString cmd;
	CString param, dir, dirout;
	CString logfile;
	CString inputname;
	CString flags;
	COutputWnd* ow = ((CMainFrame*)AfxGetMainWnd())->GetOutputWnd();

	inputname = L"ScopeRunner_7"; // .asm
	dirout = L"..\\sample\\";
	flags = L"-e -l";
	dir = L".\\";
	cmd = TEXT("..\\bin\\asmb.exe");

	param.Format(L"-i %s%s.asm %s", dirout, inputname, flags);
	s.Format(_T("%s%s %s "), dir, cmd, param);
	logfile.Format(L"%s%s.log",dirout, inputname);

	ow->GetBuildOutput()->ResetContent();
	ow->FillBuildWindow(s);
	m_ipcAsmb.setHide(true);
	m_ipcAsmb.setDir(dir);
	m_ipcAsmb.setParam(param);
	m_ipcAsmb.setCmd(cmd);
	m_ipcAsmb.setPipeOutFileName(logfile);
	m_ipcAsmb.StartExecute();
	m_ipcAsmb.Open();
	bool inLoop;
	do {
		//Sleep(10);
		inLoop = false;
		if (m_ipcAsmb.ReadString(s)) {
			if (!s.IsEmpty())
				ow->FillBuildWindow(s);
			inLoop |= true;
		}
		inLoop |= m_ipcAsmb.isRunning();
	} while (inLoop);
}


void CidebDoc::OnBuildClean()
{
	CString param, dir;
	param = L"clean";
	dir = L"..\\sample";
	m_ipcAsmb.setHide(true);
	m_ipcAsmb.setDir(dir);
	m_ipcAsmb.setParam(param);
	m_ipcAsmb.setCmd(TEXT("test.bat"));
	m_ipcAsmb.setPipeOutFileName(L"ideb.log");
	m_ipcAsmb.StartExecute();
	do {
		Sleep(10);
	} while (m_ipcAsmb.isRunning());
}


void CidebDoc::OnBuildBuildall()
{
	CString param, dir;
	param = L"";
	dir = L"..\\sample";
	m_ipcAsmb.setHide(true);
	m_ipcAsmb.setDir(dir);
	m_ipcAsmb.setParam(param);
	m_ipcAsmb.setCmd(TEXT("test.bat"));
	m_ipcAsmb.setPipeOutFileName(L"ideb.log");
	m_ipcAsmb.StartExecute();
	do {
		Sleep(10);
	} while (m_ipcAsmb.isRunning());
}



