/** @file CAsmDoc.cpp
*
* @brief Asm file Document model implementation
* The goal of this class, to implement the logic behind the asm file view. 
*
* It prepares the syntax highlight, with tokenizing the asm source view.
* The actual syntax highlight implementation is very eary.
* Maybe we want to implement a sophisticated one later.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/

#include "stdafx.h"
#include "ideb.h"
#include "AsmDoc.h"


// CAsmDoc

IMPLEMENT_DYNCREATE(CAsmDoc, CDocument)
BEGIN_MESSAGE_MAP(CAsmDoc, CDocument)
END_MESSAGE_MAP()

//Todo: design a color scheme system for this
const COLORREF CAsmDoc::TokenColors[] = { //T_NUMBEROFTOKENIDS
	RGB(0x00,0x09,0x00),		//T_LABEL
	RGB(0x90,0x20,0x20),		//T_MNEMONIC
	RGB(0x00,0x00,0xa0),		//T_NUMBER
	RGB(0xa0,0x09,0x00),		//T_OPERATOR
	RGB(0x00,0xa0,0x00),		//T_COMMENT
	RGB(0x20,0xa0,0x90), 		//T_KEYWORD
	RGB(0x40,0x40,0xb0),		//T_SYMBOLE
	RGB(0x40,0x50,0x00)			//T_STRING
};

//reserved keywords list for tokenizer
CMap<CString, LPCWSTR, CAsmDoc::TokenDataId, CAsmDoc::TokenDataId> CAsmDoc::mapKeywords;
//TODO: also we need a symbol table similar way, but maybe not as a singleton, but in the document later...

CAsmDoc::CAsmDoc()
	:m_ReadOnly(TRUE), m_FileId(0)
{
	//init the reserved symbol table
	//TODO: init singleton better way, this will be called now multiple times!
	//mapKeywords.RemoveAll(); //now it is needed or check if it is initialized already:
	if (0 == mapKeywords.GetCount()) {
		mapKeywords[L"equ"] = CAsmDoc::T_KEYWORD;
		mapKeywords[L"db"] = CAsmDoc::T_KEYWORD;
		mapKeywords[L"org"] = CAsmDoc::T_KEYWORD;
		mapKeywords[L"section"] = CAsmDoc::T_KEYWORD;
		mapKeywords[L"end"] = CAsmDoc::T_KEYWORD;

		mapKeywords[L"mvi"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"lda"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"sta"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"ad0"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"ad1"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"adc"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"nand"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"nor"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"rrm"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"jmp"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"jc"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"jnc"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"jz"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"jnz"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"jm"] = CAsmDoc::T_MNEMONIC;
		mapKeywords[L"jp"] = CAsmDoc::T_MNEMONIC;
	}
}

//Change cursor according to the actual row
void CAsmDoc::CursorValidate(int & x, int & y, int & xWish, UINT nChar)
{
	int maxrow = (int)m_lines.GetCount();
	if (y >= maxrow) y = maxrow - 1;
	if (y < 0)y = 0;
	const CAsmDoc::LineData& ld = m_lines[y];
	int maxcolumn = ld.maxcolumn;
	if ((nChar != VK_LEFT) && (nChar != VK_RIGHT)) {
		x = xWish;
	}
	if (x > maxcolumn) x = maxcolumn;
}

//empty doc
BOOL CAsmDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	RegisterToActiveWorkspace();
	return TRUE;
}

CAsmDoc::~CAsmDoc()
{
	UnRegister();
}

void CAsmDoc :: DoWorkspaceUpdate() {
	//TODO: when project and workspace handling changes, may need to update this view as well
	//Invalidate();
}

// CAsmDoc diagnostics

#ifdef _DEBUG
void CAsmDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CAsmDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CAsmDoc serialization

void CAsmDoc::Serialize(CArchive& ar)
{
	//CHECK: Never called by the system, because we use different way,
	//or at least, we dont need to implement the CArchive serializer...
	RegisterToActiveWorkspace();
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		
	}
}
#endif


// CAsmDoc commands
#include "CSimulator.h"

//This will override the serializer, reads the asm file and parse it up.
BOOL CAsmDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	RegisterToActiveWorkspace();
	CString sName;
	CString sPath;
	CString sFileName;
	sName = lpszPathName;
	if (sName.Find(L"\\") > 0) {
		int n = sName.GetLength();
		int r = sName.ReverseFind('\\');
		sPath = sName.Left(r+1);
		sName = sName.Right(n-r-1);
	}
	//TODO: check if we can skip the original methode, maybe need to skip...
	//if (!__super::OnOpenDocument(lpszPathName))
	//	return FALSE;
	CWorkspaceEvent ev;
	ev.EventType = CWorkspaceEvent::OpenDocumentAsm;
	ev.Path = sPath;
	ev.FileName = sName;
	ev.FileId = 0;
	DoWorkspaceEvent(&ev);
	sFileName = sPath;
	sFileName += sName;
	m_FileId = ev.FileId;
	//Open the file...
	CStdioFile  f;
	if (f.Open(sFileName, CFile::modeRead)) { //lpszPathName
		CSimulator* pSimulator = GetSimulator();
		m_lines.RemoveAll();
		int number = 0;
		CString strLine;
		while (f.ReadString(strLine))
		{
			LineData data;
			data.set(++number, strLine);
			if (pSimulator) data.memAddress = pSimulator->SearchLine(number, ev.FileId);
			data.Parse();
			m_lines.Add(data);
		}
		CFileStatus status;
		if (f.GetStatus(status))
		{
			m_TimeAsmFIle = status.m_mtime;
		}
		f.Close();
	}

	return TRUE;
}

//Will be implemented really when the asm editor functionality is needed.
BOOL CAsmDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	//TODO: maybe the super class member function shuldn't be called from here!!!
	//return __super::OnSaveDocument(lpszPathName);
	return TRUE;
}

//This is the tokenizer of the syntax highlite now...
void CAsmDoc::LineData::Parse()
{
	TokenData td;
	int first = 0;
	int firstColumn = 0;
	int column = 0;
	int len = text.GetLength();
	BOOL inWhiteSpace = FALSE;

	td.id = T_LABEL;
	TCHAR cprev = ' ';
	for (int i = 0; i < len; i++) {
		TCHAR c = text[i];
		switch (c) {
		case ';': //from now the rest of the line is commented out
			if (first != i) {
				td.set(td.id, first, i - first, firstColumn);
				tokens.Add(td);
				firstColumn = column;
				first = i;
			}
			td.set(T_COMMENT, first, len - first, firstColumn);
			column += len-first;
			tokens.Add(td);
			i = len;
			first = i;
			break;
		case ':': //the previous part was a label, store the colon, wait for the rest...
			if ((i-first > 0)&& !inWhiteSpace) {
				td.set(T_LABEL, first, i - first, firstColumn);
				tokens.Add(td);
				firstColumn = column;
			}
			first = i;
			td.set(T_OPERATOR, first, 1, firstColumn);
			tokens.Add(td);
			column++;
			firstColumn = column;
			inWhiteSpace = TRUE; //nothing left to store
			//first = i;
			break;
		//this two are double char operators, special case...
		case '>':
		case '<':
			if (cprev != c) {
				//first operator char
				if ((i - first > 0) && !inWhiteSpace) {
					//store the text before the operator
					td.set(td.id, first, i - first, firstColumn);
					tokens.Add(td);
					firstColumn = column;
				}
				column++;
				first = i;
			}
			else {
				//previous character was the same, so this is a double chars operator...
				td.set(T_OPERATOR, first, 2, firstColumn);
				tokens.Add(td);
				column++;
				firstColumn = column;
				inWhiteSpace = TRUE; //nothing left to store
			}
			break;
		//these are simple char operators, mostly common behaviour can handle all
		case '@':
		case '*':
		case '/':
		case ',':
		case '-':
		case '+':
			if ((i - first > 0) && !inWhiteSpace) {
				//store the text before the operator
				td.set(td.id, first, i-first, firstColumn);
				tokens.Add(td);
				firstColumn = column;
			}
			first = i;
			td.set(T_OPERATOR, first, 1, firstColumn);
			tokens.Add(td);
			column++;
			firstColumn = column;
			inWhiteSpace = TRUE; //nothing left to store
			break;
		case '\t':	//tab char handling
#define TABULATORLEN (4)
			column += TABULATORLEN;
			column &= ~(TABULATORLEN - 1);

			if (!inWhiteSpace) {
				//when previous character was not a whitespace, store it...
				td.set(td.id, first, i - first, firstColumn);
				tokens.Add(td);
				first = i;
				inWhiteSpace = TRUE;
			}
			firstColumn = column;
			break;
		case ' ':
			column++;
			if (!inWhiteSpace) {
				//when previous character was not a whitespace, store it...
				td.set(td.id, first, i - first, firstColumn);
				tokens.Add(td);
				first = i;
				inWhiteSpace = TRUE;
			}
			firstColumn = column;
			break;
		case '"':
			column += 1;
			if (inWhiteSpace) {
				//previously we were in a whitespace block... so, do some book-keeping...
				first = i;
				inWhiteSpace = FALSE;
				td.id = T_STRING;
			}
			do
			{
				i++;
			} while ((i < len) && (text[i] != '"'));
			i++;
			td.set(td.id, first, i - first, firstColumn);
			tokens.Add(td);
			break;
		default:
			//the actual character is nono of the above, so it can be letter or number, or other not spec. char.
			column += 1;
			if (inWhiteSpace) {
				//previously we were in a whitespace block... so, do some book-keeping...
				first = i;
				inWhiteSpace = FALSE;
				td.id = T_SYMBOLE;
			}
		}

		if (i - first > 0 )
		{
			//there was some characters, unprocessed
			CString s = text.Mid(first, i - first+1);
			s.MakeLower();
			TCHAR q = ' ';
			if (i+1< len) {
				q = text[i + 1]; //we lookahead for one more, next character
			}
			switch (q) {
			case ' ':
			case '\t':
				//if no more chars or there is a whitespace, then check the keywors
				if (mapKeywords.Lookup(s, td.id)) {
					//store, when the text is in the reserved keywords list
					td.set(td.id, first, i - first + 1, firstColumn);
					tokens.Add(td);
					column++;
					firstColumn = column;
					first = i+1;
					inWhiteSpace = TRUE;
				}
				break;
			}
		}
		cprev = c;
	}//for
	
	if (first < len) {//rest of the line
		//so, the previous processing left some characters behind, store it as is...
		td.set(td.id, first, len - first + 1, firstColumn);
		tokens.Add(td);
		column += len - first+1;
	}
	maxcolumn = column; //TODO: tabs in comment and rest of the line are not handled well right now
}
