/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*
*/
#include "StdAfx.h"
#include "IpcCom.h"
CIpcCom::CIpcCom(): 
    m_nShowCmd(SW_SHOWNORMAL),m_inst(NULL),m_hChild(NULL),m_exitCode(0)
    {}  //SW_SHOWMINIMIZED
CIpcCom::~CIpcCom(){}
void CIpcCom::ErrorExit(PTSTR lpszFunction) 
// Format a readable error message, display a message box, 
// and exit from the application.
{ 
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
  //  ExitProcess(1);
}
void CIpcCom::setHide(bool hide){
    if (hide)
        m_nShowCmd=SW_SHOWMINIMIZED;
    else
        m_nShowCmd=SW_NORMAL;
}
bool CIpcCom::isRunning(){ //check if process is running.
    
    BOOL b=GetExitCodeProcess(m_hChild, &m_exitCode);
    if (b){
        if (m_exitCode == STILL_ACTIVE )
            return true;
    }
    return false;
}
//------------------------------------------------------------
CIpcComSE::CIpcComSE(void):m_isOpen(false),m_hChildStd_OUT_Rd(NULL)
{
}

CIpcComSE::~CIpcComSE(void)
{
   //m_file.Abort();
    Close();
}
void CIpcComSE::StartExecute()
{
    m_exitCode = STILL_ACTIVE;
    DeleteFile(m_pipeOutFileName);
    m_hChildStd_OUT_Rd=NULL;
    SHELLEXECUTEINFO si;
    ZeroMemory(&si,sizeof(si));
    si.cbSize=sizeof(SHELLEXECUTEINFO);
    si.fMask=SEE_MASK_NOCLOSEPROCESS;
    si.lpDirectory=m_dir;
    si.lpFile=m_cmd;
    si.lpParameters=m_param;
    si.nShow=m_nShowCmd;
    si.lpVerb=L"open";
    BOOL bSuccess = ShellExecuteEx(&si);
    DWORD e=GetLastError();
    m_inst=si.hInstApp;
    m_hChild=si.hProcess;
	// warning C4311, and warning C4302 are normal. Should check if microsoft provides a clear way to get the err code...
    INT_PTR se=(INT_PTR)(m_inst); //ptr or error code
    switch (se){
        case SE_ERR_FNF:    //File not found
            break;
        case SE_ERR_PNF:    //Path not found
        case SE_ERR_ACCESSDENIED:
        case SE_ERR_OOM:
        case SE_ERR_SHARE:
        case SE_ERR_ASSOCINCOMPLETE:
        case SE_ERR_NOASSOC:
            break;
    };
    int dog=100;
    while (!isRunning()){
        Sleep(10);
        if (!dog--) break;
    }
    
}
void CIpcComSE::Open(){
    m_hChildStd_OUT_Rd=getPipeOutFile();
}
HANDLE CIpcComSE::getPipeOutFile(){
    m_isOpen=true;
    HANDLE h=NULL;
    int dog=100;
    while (!h){
        h=CreateFile(m_pipeOutFileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
            NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
        if ((!h)||(h==INVALID_HANDLE_VALUE)){
            Sleep(10);
            h=NULL;
            if (!dog--) break;
        }
        
    };
    m_isOpen=true;
    return h;
}
bool CIpcComSE::ReadString(CString &s){
    bool bEnd=false;
    s.Empty();
    if (!m_hChildStd_OUT_Rd) return false;
    while(!bEnd){
        CHAR c;
        DWORD nBytesRead;
        if (!ReadFile(m_hChildStd_OUT_Rd, &c, 1, &nBytesRead,NULL) || !nBytesRead)
        {
            DWORD e=GetLastError();
            switch (e){
                case 0:
                case ERROR_BROKEN_PIPE:
                case ERROR_HANDLE_EOF:
                    if (!isRunning()) Close();
                    return false;
                    break;
            }
            break;
        }
        switch (c){
           case '\n':
               //break; //no, handle together with \r
           case '\r':
               bEnd=true;
               break;
           default:
            s.AppendChar(c);
        }
    }
    return bEnd;
}
bool CIpcComSE::SeekEnd(){
    if (m_hChildStd_OUT_Rd) SetFilePointer(m_hChildStd_OUT_Rd, 0,0,FILE_END);
    return true;
}
void CIpcComSE::Kill(){
    if (isRunning()){
        TerminateProcess(m_hChild,100);
    }
}
void CIpcComSE::Close(){
    if (m_hChildStd_OUT_Rd){
        CloseHandle(m_hChildStd_OUT_Rd);
        m_hChildStd_OUT_Rd=NULL;
        m_isOpen=false;
    }
}
//------------------------------------------------------------
CIpcComCP::CIpcComCP(void):
    m_hChildStd_IN_Rd(NULL),
    m_hChildStd_IN_Wr(NULL),
    m_hChildStd_OUT_Rd(NULL),
    m_hChildStd_OUT_Wr(NULL)
{
    
}

CIpcComCP::~CIpcComCP(void)
{
}

void CIpcComCP::InitPipe(){
   SECURITY_ATTRIBUTES saAttr; 
// Set the bInheritHandle flag so pipe handles are inherited. 
   saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
   saAttr.bInheritHandle = TRUE; 
   saAttr.lpSecurityDescriptor = NULL; 
// Create a pipe for the child process's STDOUT. 
   if ( ! CreatePipe(&m_hChildStd_OUT_Rd, &m_hChildStd_OUT_Wr, &saAttr, 0) ) 
      ErrorExit(TEXT("StdoutRd CreatePipe")); 
// Ensure the read handle to the pipe for STDOUT is not inherited.
   if ( ! SetHandleInformation(m_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
      ErrorExit(TEXT("Stdout SetHandleInformation")); 
// Create a pipe for the child process's STDIN. 
   if (! CreatePipe(&m_hChildStd_IN_Rd, &m_hChildStd_IN_Wr, &saAttr, 0)) 
      ErrorExit(TEXT("Stdin CreatePipe")); 
// Ensure the write handle to the pipe for STDIN is not inherited. 
   if ( ! SetHandleInformation(m_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
      ErrorExit(TEXT("Stdin SetHandleInformation")); 
 
}

void CIpcComCP::StartExecute()
{
    InitPipe();
    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE; 

    // Set up members of the PROCESS_INFORMATION structure. 

    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

    // Set up members of the STARTUPINFO structure. 
    // This structure specifies the STDIN and STDOUT handles for redirection.

    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.wShowWindow=m_nShowCmd;
    siStartInfo.cb = sizeof(STARTUPINFO); 
    siStartInfo.hStdError = m_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = m_hChildStd_OUT_Wr;
    siStartInfo.hStdInput = m_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
    m_exitCode = STILL_ACTIVE;
    // Create the child process. 
    bSuccess = CreateProcess(m_cmd.GetBuffer(), 
      m_param.GetBuffer(),          // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      0,             // creation flags 
      NULL,          // use parent's environment 
      m_dir.GetBuffer(),          // directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION 

    // If an error occurs, exit the application. 
    if ( ! bSuccess ) 
      ErrorExit(TEXT("CreateProcess"));
    else 
    {
      // Close handles to the child process and its primary thread.
      // Some applications might keep these handles to monitor the status
      // of the child process, for example. 

      //CloseHandle(piProcInfo.hProcess);
     
      m_hChild=piProcInfo.hProcess;
      CloseHandle(piProcInfo.hThread);
    }
}
HANDLE CIpcComCP::getPipeOutFile(){
    return m_hChildStd_OUT_Rd;
}
bool CIpcComCP::ReadString(CString &s){
    bool bEnd=false;
    if (m_hChildStd_OUT_Wr){
        if (!CloseHandle(m_hChildStd_OUT_Wr)) ErrorExit(TEXT("StdOutWr CloseHandle")); 
        m_hChildStd_OUT_Wr=NULL;
    }
    while(!bEnd){
        CHAR c;
        DWORD nBytesRead;
        if (!ReadFile(m_hChildStd_OUT_Rd, &c, 1, &nBytesRead,NULL) || !nBytesRead)
        {
            if (GetLastError() == ERROR_BROKEN_PIPE) break; // pipe done - normal exit path.
            else ErrorExit(L"ReadFile"); // Something bad happened.
            bEnd=true;
            break;
        }
        switch (c){
           case '\n':
               //break;
           case '\r':
               bEnd=true;
               break;
           default:
            s.AppendChar(c);
        }
    }
    return bEnd;
}
void CIpcComCP::Close(){
    //TODO: kill process?
}
bool CIpcComCP::isOpen(){ //check if process is running.
    BOOL b=GetExitCodeProcess(m_hChild, &m_exitCode);
    if (m_exitCode == STILL_ACTIVE ) return true;
    return false;
}