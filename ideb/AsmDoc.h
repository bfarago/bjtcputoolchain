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
	void CursorValidate(int& x, int& y, int& xWish, UINT nChar);
protected:
	virtual BOOL OnNewDocument();
public: //temporary
	
	typedef enum {
		T_LABEL,
		T_MNEMONIC,
		T_NUMBER,
		T_OPERATOR,
		T_COMMENT,
		T_KEYWORD,
		T_SYMBOLE,
		T_NUMBEROFTOKENIDS
	} TokenDataId;
	static CMap<CString, LPCWSTR, TokenDataId, TokenDataId> mapKeywords;
	static const COLORREF TokenColors[];
	class TokenData :public CObject
	{
	public:
		TokenDataId id;
		int first, count;
		int column;
		COLORREF color;
		void set(TokenDataId aId, int aFirst, int aCount, int aColumn)
		{
			id = aId;
			first = aFirst;
			count = aCount;
			column = aColumn;
			color = TokenColors[aId];
		}
		const TokenData& operator=(const TokenData& in)
		{
			if (this != &in) {
				id = in.id;
				first = in.first;
				count = in.count;
				column = in.column;
				color = in.color;
			}
			return *this;
		}
	};
	class LineData {
	public:
		int number;
		int memAddress;
		int modified;
		int maxcolumn;
		CString text;
		CArray<TokenData> tokens;
		LineData()
			:number(0), memAddress(-1), modified(0), maxcolumn(0)
		{
		}
		const LineData& operator=(const LineData&in) {
			if (this != &in) {
				number = in.number;
				memAddress = in.memAddress;
				modified = in.modified;
				maxcolumn = in.maxcolumn;
				text = in.text;
				tokens.RemoveAll();
				tokens.Copy(in.tokens);
			}
			return *this;
		}
		void Parse();
	};

	CArray<LineData> m_lines;
protected:
	BOOL m_ReadOnly;
	DECLARE_MESSAGE_MAP()
public:
	BOOL IsReadOnly() { return m_ReadOnly; }
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
};
