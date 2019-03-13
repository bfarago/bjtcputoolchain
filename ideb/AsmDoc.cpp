// CAsmDoc.cpp : implementation file
//

#include "stdafx.h"
#include "ideb.h"
#include "AsmDoc.h"


// CAsmDoc

IMPLEMENT_DYNCREATE(CAsmDoc, CDocument)
BEGIN_MESSAGE_MAP(CAsmDoc, CDocument)
END_MESSAGE_MAP()

const COLORREF CAsmDoc::TokenColors[] = { //T_NUMBEROFTOKENIDS
	RGB(0x00,0x09,0x00),		//T_LABEL
	RGB(0x90,0x20,0x20),		//T_MNEMONIC
	RGB(0x00,0x00,0xa0),		//T_NUMBER
	RGB(0xa0,0x09,0x00),		//T_OPERATOR
	RGB(0x00,0xa0,0x00),		//T_COMMENT
	RGB(0x20,0xa0,0x90), 		//T_KEYWORD
	RGB(0x40,0x40,0xb0)			//T_SYMBOLE
};
CMap<CString, LPCWSTR, CAsmDoc::TokenDataId, CAsmDoc::TokenDataId> CAsmDoc::mapKeywords;

CAsmDoc::CAsmDoc()
	:m_ReadOnly(TRUE)
{
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

void CAsmDoc::CursorValidate(int & x, int & y, int & xWish, UINT nChar)
{
	int maxrow = m_lines.GetCount();
	if (y >= maxrow) y = maxrow - 1;
	if (y < 0)y = 0;
	const CAsmDoc::LineData& ld = m_lines[y];
	int maxcolumn = ld.maxcolumn;
	if ((nChar != VK_LEFT) && (nChar != VK_RIGHT)) {
		x = xWish;
	}
	if (x > maxcolumn) x = maxcolumn;
}

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

BOOL CAsmDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	RegisterToActiveWorkspace();
	if (!__super::OnOpenDocument(lpszPathName))
		return FALSE;
	CStdioFile  f;
	if (f.Open(lpszPathName, CFile::modeRead)) {
		CSimulator* pSimulator = GetSimulator();
		m_lines.RemoveAll();
		int number = 0;
		CString strLine;
		//int len= f.GetLength();
		while (f.ReadString(strLine))
		{
			LineData data;
			data.number = ++number;
			data.memAddress = -1;
			if (pSimulator) data.memAddress = pSimulator->SearchLine(number);
			data.modified = 0;
			data.text = strLine;
			data.Parse();
			m_lines.Add(data);
			//sscanf_s(strLine, "%g %g\n", &data.x, &data.y);
		}
		f.Close();
	}

	return TRUE;
}


BOOL CAsmDoc::OnSaveDocument(LPCTSTR lpszPathName)
{



	return __super::OnSaveDocument(lpszPathName);
}


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
		case ';':
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
		case ':':
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
		case '>':
		case '<':
			if (cprev != c) {
				if ((i - first > 0) && !inWhiteSpace) {
					td.set(td.id, first, i - first, firstColumn);
					tokens.Add(td);
					firstColumn = column;
				}
				column++;
				first = i;
			}
			else {
				td.set(T_OPERATOR, first, 2, firstColumn);
				tokens.Add(td);
				column++;
				firstColumn = column;
				inWhiteSpace = TRUE; //nothing left to store
			}
			break;
		case '@':
		case '*':
		case '/':
		case ',':
		case '-':
		case '+':
			if ((i - first > 0) && !inWhiteSpace) {
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
		case '\t':
#define TABULATORLEN (4)
			column += TABULATORLEN;
			column &= ~(TABULATORLEN - 1);

			if (!inWhiteSpace) {
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
				td.set(td.id, first, i - first, firstColumn);
				tokens.Add(td);
				first = i;
				inWhiteSpace = TRUE;
			}
			firstColumn = column;
			break;
		default:
			column += 1;
			if (inWhiteSpace) {
				first = i;
				inWhiteSpace = FALSE;
				td.id = T_SYMBOLE;
			}
		}
		if (i - first > 0 )
		{
			CString s = text.Mid(first, i - first+1);
			s.MakeLower();
			TCHAR q = ' ';
			if (i+1< len) {
				q = text[i + 1];
			}
			switch (q) {
			case ' ':
			case '\t':
				if (mapKeywords.Lookup(s, td.id)) {
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
		td.set(td.id, first, len - first + 1, firstColumn);
		tokens.Add(td);
		column += len - first+1;
	}
	maxcolumn = column; //TODO: tabs in comment and rest of the line
}
