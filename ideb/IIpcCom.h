/* Written by Barna Farago <brown@weblapja.com> 2006-2018
*/
#pragma once

class IIpcCom{
public:
    virtual void StartExecute()=0;
    virtual void Open()=0;
    virtual void Close()=0;
    virtual bool isOpen()=0;
    virtual void Kill()=0;
    virtual bool ReadString(CString &s)=0;
    virtual bool SeekEnd()=0;
};
