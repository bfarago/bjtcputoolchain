/* Written by Barna Farago <brown@weblapja.com> 2006-2018
 */
#pragma once
#include "iipccom.h"
#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>


class CIpcCom : public IIpcCom
{
public:
    CIpcCom(void);
    ~CIpcCom(void);
    void setHide(bool hide);
    void setDir(LPCTSTR dir){m_dir=dir;}
    LPCTSTR getDir(){return m_dir;}
    void setCmd(LPCTSTR cmd){m_cmd=cmd;}
    LPCTSTR getCmd(){return m_cmd;}
    void setParam(LPCTSTR param){m_param=param;}
    LPCTSTR getParam(){return m_param;}
    void setPipeOutFileName(LPCTSTR fname){m_pipeOutFileName=fname;}
    void ErrorExit(PTSTR lpszFunction);
    bool isRunning();
    DWORD getExitCode(){return m_exitCode;}

    virtual HANDLE getPipeOutFile()=0;

protected:
    CString m_dir;
    CString m_cmd;
    CString m_param;
    CString m_pipeOutFileName; //optional
    INT m_nShowCmd;
    HINSTANCE m_inst;
    HANDLE m_hChild;
    DWORD m_exitCode;
};


class CIpcComSE : public CIpcCom
{
public:
    CIpcComSE(void);
    ~CIpcComSE(void);
    virtual void StartExecute();
    virtual HANDLE getPipeOutFile();
    virtual bool ReadString(CString &s);
    virtual bool SeekEnd();
    virtual void Open();
    virtual void Close();
    virtual void Kill();
    virtual bool isOpen(){
        return m_isOpen;
    }
protected:
    HANDLE m_hChildStd_OUT_Rd;
    bool m_isOpen;
};

class CIpcComCP : public CIpcCom
{
public:
    CIpcComCP(void);
    ~CIpcComCP(void);
    virtual void StartExecute();
    virtual HANDLE getPipeOutFile();
    virtual bool ReadString(CString &s);
    virtual void Open(){}
    virtual void Close();
    virtual bool isOpen();
    virtual void Kill(){}
    virtual bool SeekEnd(){return false;}
protected:
    void InitPipe();
    
    HANDLE m_hChildStd_IN_Rd;
    HANDLE m_hChildStd_IN_Wr;
    HANDLE m_hChildStd_OUT_Rd;
    HANDLE m_hChildStd_OUT_Wr;
};
