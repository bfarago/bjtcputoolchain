#pragma once
//#include "idebDoc.h"
class CidebDoc; //forward

class CWorkspace
{
public:
	CWorkspace();
	~CWorkspace();
	virtual CStringArray* GetAsmFileList() = 0;
	virtual const CString& GetTargetBinFileName()const = 0;
	virtual void SetTargetBinFileName(const CString& s) = 0;
	inline CidebDoc* GetProjectDocument(){
			return reinterpret_cast<CidebDoc*>(this);
	};
	virtual const CString& GetProjectName() = 0;
	virtual void SetProjectName(const CString& s) = 0;

	virtual CString& AsmbFlags() = 0;
	virtual BOOL& AsmbVerbose() = 0;
	virtual CString& AsmbDirIn() = 0;
	virtual CString& AsmbDirOut() = 0;
};

