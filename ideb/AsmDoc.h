/** @file CAsmDoc.h
*
* @brief Asm file Document model interface
* The goal of this class, to implement the logic behind the asm file view.
*
* It prepares the syntax highlight, with tokenizing the asm source view.
* The actual syntax highlight implementation is very eary.
* Maybe we want to implement a sophisticated one later.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
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
	CTime m_TimeAsmFIle;
	short m_FileId;
	typedef enum {
		T_LABEL,
		T_MNEMONIC,
		T_NUMBER,
		T_OPERATOR,
		T_COMMENT,
		T_KEYWORD,
		T_SYMBOLE,
		T_STRING,
		T_NUMBEROFTOKENIDS
	} TokenDataId;
	static CMap<CString, LPCWSTR, TokenDataId, TokenDataId> mapKeywords;
	static const COLORREF TokenColors[];

	//internal class, to store tokens in an MFC collection template class
	class TokenData :public CObject
	{
	public:
		TokenDataId id;
		int first, count;
		int column;
		COLORREF color;
		void set(TokenDataId aId, int aFirst, int aCount, int aColumn) //better to use, to keep the caller code clean...
		{
			id = aId;
			first = aFirst;
			count = aCount;
			column = aColumn;
			color = TokenColors[aId];
		}
		const TokenData& operator=(const TokenData& in) //needed to mfc collections to copy an element
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

	//internal class, to store lines in an MFC collection template class
	class LineData {
	public:
		int number;		//original line number in the source file
		int memAddress;	//runtime info, this line is related to that memory address...
		int modified;	//not yet used, but later it holds the editor state...
		int maxcolumn;	//max of the character position x in this line
		CString text;	//unwrapped original line text from the file
		CArray<TokenData> tokens; //array of the tokens
		LineData() //default constructor
			:number(0), memAddress(-1), modified(0), maxcolumn(0)
		{
		}
		void set(int aNumber, const CString& aText) {
			number = aNumber;
			text = aText;
			memAddress = -1;
			maxcolumn = 0;
			modified = 0;
			tokens.RemoveAll();
		}
		const LineData& operator=(const LineData&in) //needed to mfc collections to copy an element
		{
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

	//these are the lines of the asm source file. 
	CArray<LineData> m_lines;
protected:
	BOOL m_ReadOnly;
	DECLARE_MESSAGE_MAP()
public:
	BOOL IsReadOnly() { return m_ReadOnly; }
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
};
