/** @file idebDoc.cpp
*
* @brief ideb project document model implementation
* The goal of this class, to implement the logic behind the project file document.
*
* The project file in this system contains additional informations related to ideb
* integrated environment.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ideb.h"
#endif

#include "idebDoc.h"
#include "DNewProject.h"
#include <propkey.h>
#include "MainFrm.h"
#include "OutputWnd.h"

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
	ON_COMMAND(ID_VIEW_MEASUREMENT, &CidebDoc::OnViewMeasurement)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MEASUREMENT, &CidebDoc::OnUpdateViewMeasurement)
	ON_COMMAND(ID_VIEW_DISASSEMBLER, &CidebDoc::OnViewDisassembler)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DISASSEMBLER, &CidebDoc::OnUpdateViewDisassembler)
	ON_COMMAND(ID_VIEW_MEMORYMAP, &CidebDoc::OnViewMemorymap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MEMORYMAP, &CidebDoc::OnUpdateViewMemorymap)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_PAUSE, &CidebDoc::OnUpdateDebugPause)
	ON_COMMAND(ID_VIEW_TIMELINE, &CidebDoc::OnViewTimeline)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TIMELINE, &CidebDoc::OnUpdateViewTimeline)
	ON_COMMAND(ID_VIEW_PROFILELAYOUT, &CidebDoc::OnViewProfilelayout)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PROFILELAYOUT, &CidebDoc::OnUpdateViewProfilelayout)
	ON_COMMAND(ID_VIEW_DISASSEMBLERLAYOUT, &CidebDoc::OnViewDisassemblerlayout)
END_MESSAGE_MAP()


// CidebDoc construction/destruction

CidebDoc::CidebDoc()
	:m_SimulationRunning(FALSE), m_AsmbDbg(TRUE)
{
}

CidebDoc::~CidebDoc()
{
	CMainFrame* mf = ((CMainFrame*)AfxGetMainWnd());
	if (mf) mf->RegisterWorkspace(NULL);
}
int CidebDoc::GetFileId(LPCTSTR FileName) {
	int n = (int)m_AsmFiles.GetSize();
	for (int i = 0; i < n; i++) {
		const CString & s = m_AsmFiles.GetAt(i);
		if (s.Compare(FileName) == 0) {
			return i;
		}
	}
	return -1;
}

BOOL CidebDoc::OnWorkspaceEvent(CWorkspaceEvent * pEvent)
{
	switch (pEvent->EventType) {
	case CWorkspaceEvent::OpenDocumentAsm:
		{
			int id = GetFileId(pEvent->FileName);
			if (id < 0) {
				m_AsmbDirIn = pEvent->Path;
				m_AsmFiles.RemoveAll();
				m_AsmFiles.Add(pEvent->FileName);
				m_Simulator.Reset(); //New document
				CMainFrame* mf = ((CMainFrame*)AfxGetMainWnd());
				mf->RegisterWorkspace(this);
				//mf->WorkspaceUpdated();
				//OnDebugReload();
				pEvent->FileId = 0;
				return TRUE;
			}
			pEvent->FileId = id;
		}
		break;
	}
	return 0;
}

BOOL CidebDoc::OnNewDocument()
{
	CDNewProject dlg;
	dlg.SetProject(this);
	INT_PTR r = dlg.DoModal();
	if (r != IDOK) return FALSE;

	if (!CDocument::OnNewDocument())
		return FALSE;

	CString sPath;
	CString sName = dlg.m_FileN;
	int pos = sName.ReverseFind('\\');
	if (pos > 0) {
		sPath = sName.Left(pos+1);
		sName= sName.Right(sName.GetLength() - pos-1);
	}

	m_AsmFiles.RemoveAll();
	m_AsmbFlags = "-l"; // -e
 	m_AsmbVerbose = FALSE;
	m_ToolchainDir = dlg.m_ToolChDir; 
	m_AsmbDirIn = sPath;
	m_AsmbDirOut = sPath;
	/*
	m_AsmbDirIn = L"..\\sample\\";
	m_AsmbDirOut = L"..\\sample\\";
	*/
	m_DocVersionMajor = ACTUAL_DOCVERSION_MAJOR;
	m_DocVersionMinor = ACTUAL_DOCVERSION_MINOR;
	m_SimbSteps = 5;
	m_ProjectName = sName; // L"Test";
	//m_SimTargetBinFileName = _T("a.bin");
	//m_SimTargetBinFileName = "..\\sample\\test_screen.bin";
	m_SimTargetBinFileName.Format(L"%s.bin", sName); 	//= "ScopeRunner_7.bin";
	m_SimulationRunning = FALSE;
	CString sAsm;
	sAsm.Format(L"%s.asm", sName);
	m_AsmFiles.Add(sAsm);  // _T("ScopeRunner_7.asm"));
	
	CMainFrame* mf = ((CMainFrame*)AfxGetMainWnd());
	mf->RegisterWorkspace(this);
	//mf->WorkspaceUpdated();
	OnDebugReload();
	return TRUE;
}

// CidebDoc serialization

void CidebDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_DocVersionMajor;
		ar << m_DocVersionMinor;
		ar << m_ProjectName;
		ar << m_AsmbFlags;
		ar << m_AsmbVerbose;
		ar << m_AsmbDirIn;
		ar << m_AsmbDirOut;
		ar << m_SimbSteps;
		ar << m_SimTargetBinFileName;
		ar << m_ToolchainDir;
	}
	else
	{
		ar >> m_DocVersionMajor;
		ar >> m_DocVersionMinor;
		ar >> m_ProjectName;
		ar >> m_AsmbFlags;
		ar >> m_AsmbVerbose;
		ar >> m_AsmbDirIn;
		ar >> m_AsmbDirOut;
		ar >> m_SimbSteps;
		ar >> m_SimTargetBinFileName;
		ar >> m_ToolchainDir;
	}
	m_AsmFiles.Serialize(ar);
	CMainFrame* mf = ((CMainFrame*)AfxGetMainWnd());
	mf->RegisterWorkspace(this);
	OnDebugReload();
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

/*
TODO: Refactoring ideas:
probably the simulator should run on a project, which is good right now.
But the user interface should handle the project's debugger user-interactions
even if the project document is in the background. In example, one of the related
asm file is opened, then somehow an active project should be there, and this is
missing from the actual implementation.
Keypad matrix and screen obviously needed/used when the project document is
visible and in focus. (also important)
Building or compile also important. Maybe there is a difference between build.
Need to decide, how the workspace and project(s) are handled first, then MDI
multi document view/controller behaviours will be the next level to decide...
*/
void CidebDoc::OnDebugStep()
{
	m_Simulator.SetDocument(this);
	m_Simulator.SetRunMode(rm_StepState);
	m_Simulator.SetStop(FALSE);
	m_Simulator.Step();
	UpdateAllViews(NULL);
	SimAddress_t pc = m_Simulator.GetPc();
	memoryMetaData_t* pMeta = m_Simulator.GetMemoryMetaData(pc);
	CWorkspaceEvent event;
	event.EventType = CWorkspaceEvent::EnsureSourceLineVisible;
	event.Line = m_Simulator.GetLine(pc);
	event.FileId = pMeta->fileId;
	SendEvent(&event);
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
	// CWorkspace* pWs = CWorkspaceSingleton::GetActiveWorkspace();
	// if (pWs) pWs->Update();
	SimAddress_t pc = m_Simulator.GetPc();
	memoryMetaData_t* pMeta = m_Simulator.GetMemoryMetaData(pc);
	CWorkspaceEvent event;
	event.EventType = CWorkspaceEvent::EnsureSourceLineVisible;
	event.Line = m_Simulator.GetLine(pc);
	event.FileId = pMeta->fileId;
	SendEvent(&event);
}

void CidebDoc::OnUpdateDebugStepInstruction(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_SimulationRunning);
}


void CidebDoc::OnDebugReload()
{
	CWorkspaceSingleton::RegisterWorkspace(this);
	m_Simulator.SetDocument(this);
	m_Simulator.LoadBinToMemory();
	UpdateAllViews(NULL);
	CWorkspace* pWs = CWorkspaceSingleton::GetActiveWorkspace();
	if (pWs) pWs->Update();
	CWorkspaceEvent event;
	event.EventType = CWorkspaceEvent::Reload;
	SendEvent(&event);
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
	CWorkspace* pWs = CWorkspaceSingleton::GetActiveWorkspace();
	if (pWs) pWs->Update();
	SimAddress_t pc = m_Simulator.GetPc();
	memoryMetaData_t* pMeta = m_Simulator.GetMemoryMetaData(pc);
	CWorkspaceEvent event;
	event.EventType = CWorkspaceEvent::EnsureSourceLineVisible;
	event.Line = m_Simulator.GetLine(pc);
	event.FileId = pMeta->fileId;
	SendEvent(&event);
}
void CidebDoc::ClearHeatMap()
{
	//TODO: obviously not used, remove it... Or implement, when needed...
}

void CidebDoc::OnTimer(UINT_PTR nIDEvent)
{
	if (1000 == nIDEvent) {
		if (m_SimulationRunning) {
			m_Simulator.SetDocument(this);
			
			LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
			LARGE_INTEGER Frequency;
			QueryPerformanceFrequency(&Frequency);
			QueryPerformanceCounter(&StartingTime);

			for (int i = 0; i < SIM_STEPS_PER_TIMER; i++) {
				if (!m_Simulator.RunQuick()) {
					m_SimulationRunning = FALSE;
					m_Simulator.SetRunMode(rm_StepInstruction);
					SimAddress_t pc = m_Simulator.GetPc();
					CWorkspace* pWs = CWorkspaceSingleton::GetActiveWorkspace();
					memoryMetaData_t* pMeta = m_Simulator.GetMemoryMetaData(pc);
					CWorkspaceEvent event;
					event.EventType = CWorkspaceEvent::EnsureSourceLineVisible;
					event.Line = m_Simulator.GetLine(pc);
					event.FileId = pMeta->fileId;
					SendEvent(&event);

					if (pWs) pWs->Update();
					break;
				}
			}
			QueryPerformanceCounter(&EndingTime);
			ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
			ElapsedMicroseconds.QuadPart *= 1000000;
			ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

			m_Simulator.m_ExecTimeActual = (int) ElapsedMicroseconds.QuadPart;
			m_Simulator.ProcessMeasurement();
			//UpdateAllViews(NULL);
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
	pCmdUI->Enable(!m_SimulationRunning);
}


void CidebDoc::OnDebugPause()
{
	//KillTimer(NULL, 1000);
	m_Simulator.SetRunMode(rm_StepInstruction);
	m_SimulationRunning = FALSE;
	UpdateAllViews(NULL);
	CWorkspace* pWs = CWorkspaceSingleton::GetActiveWorkspace();
	if (pWs) pWs->Update();

	SimAddress_t pc = m_Simulator.GetPc();
	CWorkspaceEvent event;
	event.EventType = CWorkspaceEvent::EnsureSourceLineVisible;
	memoryMetaData_t* pMeta = m_Simulator.GetMemoryMetaData(pc);
	event.Line = m_Simulator.GetLine(pc);
	event.FileId = pMeta->fileId;
	SendEvent(&event);
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

/**OnBuildCompile
refactoring needed.
We have a list of .asm files. We dont use more than one, while we dont have a linker...
This can be accepted as is, but on user interface we also need to handle and access to the main asm file...
This command will based on the active workspace or project ?
Shouldn't need to be accessed the same from the asm doc/view ?
Maybe better to implement somehow in mainfrm or workspace level... Think twice later.
*/
void CidebDoc::OnBuildCompile()
{
	CString s;
	CString cmd;
	CString param, dir, dirin, dirout;
	CString logfile;
	CString inputname;
	CString inputSuffix;
	CString flags;
	COutputWnd* ow = ((CMainFrame*)AfxGetMainWnd())->GetOutputWnd();
	dirin = m_AsmbDirIn;
	dirout = m_AsmbDirOut;
	// dir = L".\\"; //cd to here 
	dir = m_AsmbDirIn;
	flags = m_AsmbFlags;
	if (m_AsmbVerbose) {
		flags.AppendFormat(L" -v");
	}
	m_AsmbDbg = TRUE; //always
	if (m_AsmbDbg) {
		flags.AppendFormat(L" -g");
	}

	cmd.Format(L"%sasmb.exe", m_ToolchainDir);
	TCHAR sBuf[FILENAME_MAX];
	TCHAR** lppPart = { NULL };
	GetFullPathName(cmd, FILENAME_MAX, sBuf, lppPart);
	cmd = sBuf;

	//cmd = TEXT("..\\bin\\asmb.exe"); //good idea to get the executable nem from project settings rather...
	int n = (int)m_AsmFiles.GetCount();
	for (int i = 0; i < n; i++) {
		inputname = m_AsmFiles.GetAt(i);  // .asm
		
		param.Format(L"-i %s%s%s %s", L"", inputname, inputSuffix, flags);
		s.Format(_T("%s%s %s "), dir, cmd, param);
		logfile.Format(L"%s%s.log", dirout, inputname);

		ow->GetBuildOutput()->ResetContent();
		ow->FillBuildWindow(s);
		m_ipcAsmb.setHide(true);
		m_ipcAsmb.setDir(dir);
		m_ipcAsmb.setParam(param);
		m_ipcAsmb.setCmd(cmd);
		//m_ipcAsmb.setPipeOutFileName(logfile);
		m_ipcAsmb.StartExecute();
		m_ipcAsmb.Open();
		bool inLoop;
		do {
			inLoop = false;
			if (m_ipcAsmb.ReadString(s)) {
				if (!s.IsEmpty())
					ow->FillBuildWindow(s);
				inLoop |= true;
			}
			inLoop |= m_ipcAsmb.isRunning();
		} while (inLoop);
	}
	OnDebugReload();
}

/**OnBuildClean
call a batch file with command line argument "clean".
*/
void CidebDoc::OnBuildClean()
{
	CString param, dir;
	param = L"clean";
	dir = m_AsmbDirIn;
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

/**OnBuildBuildall:
ok, right now this will call a batch file, which similar than a "make all" command.
TODO: think through the project and workspace internal behaviours regarding the compile and build...
TODO: process the output of the process better.
*/
void CidebDoc::OnBuildBuildall()
{
	CString param, dir;
	param = L"";
	dir = m_AsmbDirIn;
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

void CidebDoc::OnViewMeasurement()
{
	m_Simulator.m_DisplayMeasurement ^= 1;
}

void CidebDoc::OnUpdateViewMeasurement(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_Simulator.m_DisplayMeasurement ? 1 : 0);
}


void CidebDoc::OnViewDisassembler()
{
	m_Simulator.m_DisplayDebugMonitor ^= 1;
}


void CidebDoc::OnUpdateViewDisassembler(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_Simulator.m_DisplayDebugMonitor ? 1 : 0);
}


void CidebDoc::OnViewMemorymap()
{
	m_Simulator.m_DisplayMemory ^= 1;
}


void CidebDoc::OnUpdateViewMemorymap(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_Simulator.m_DisplayMemory ? 1 : 0);
}


void CidebDoc::OnUpdateDebugPause(CCmdUI *pCmdUI)
{
	//TODO: check how can we change this state from elsewhere!
	//BUG: the bool member can change while the mfc skip checking this function
	//therefore occasionally the button stay gray.
	pCmdUI->Enable(m_SimulationRunning);
}
void CidebDoc::OnViewTimeline()
{
	m_Simulator.m_DisplayTimeLine ^= 1;
}


void CidebDoc::OnUpdateViewTimeline(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_Simulator.m_DisplayTimeLine ? 1 : 0);
}

void CidebDoc::OnViewProfilelayout()
{
	m_Simulator.m_DisplayDebugMonitor = FALSE;
	m_Simulator.m_DisplayMemory = FALSE;
	m_Simulator.m_DisplayMeasurement= FALSE;
	m_Simulator.m_DisplayTimeLine = TRUE;
	Update();
}


void CidebDoc::OnUpdateViewProfilelayout(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}


void CidebDoc::OnViewDisassemblerlayout()
{
	m_Simulator.m_DisplayDebugMonitor = TRUE;
	m_Simulator.m_DisplayMemory = TRUE;
	m_Simulator.m_DisplayMeasurement = FALSE;
	m_Simulator.m_DisplayTimeLine = FALSE;
	Update();
}
