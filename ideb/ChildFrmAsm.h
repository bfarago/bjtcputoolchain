#pragma once

#ifdef _WIN32_WCE
#error "CMDIChildWnd is not supported for Windows CE."
#endif 

// CChildFrameAsm frame with splitter

class CChildFrameAsm : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CChildFrameAsm)
protected:
	CChildFrameAsm();           // protected constructor used by dynamic creation
	virtual ~CChildFrameAsm();

	CSplitterWnd m_wndSplitter;

protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	DECLARE_MESSAGE_MAP()
};


