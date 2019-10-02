#include "stdafx.h"
#include "CppUnitTest.h"
#include "..\asmb\intrep.h"
#include "..\asmb\util.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{		
	TEST_CLASS(UnitTest)
	{
	public:
		/*setAddress and checkAddress
		*/
		TEST_METHOD(TestAddress1)
		{
			SimAddress_t expectedAddress = 1;
			address = 0;
			maxaddress = 0;
			setAddress(1);
			Assert::AreEqual(address, expectedAddress);
			Assert::AreEqual(maxaddress, expectedAddress);
			expectedAddress = 0x0FFFu;
			setAddress(expectedAddress);
			Assert::AreEqual(address, expectedAddress);
			Assert::AreEqual(maxaddress, expectedAddress);
			SimAddress_t expectedAddress2 = 0x0800u;
			setAddress(expectedAddress2);
			Assert::AreEqual(address, expectedAddress2);
			Assert::AreEqual(maxaddress, expectedAddress);
			chkAddress(2);
			Assert::AreEqual(maxaddress, expectedAddress);
			maxaddress = 0;
			SimAddress_t expectedAddress3 = 2;
			chkAddress(expectedAddress3);
			Assert::AreEqual(maxaddress, expectedAddress3);

			setAddress(MAXMEMORY);
			Assert::AreEqual(address, expectedAddress2);
			Assert::AreEqual(maxaddress, expectedAddress3);
			setAddress(-1);
			Assert::AreEqual(address, expectedAddress2);
			Assert::AreEqual(maxaddress, expectedAddress3);

		}

		/* addMemory and getMemoryMeta
		*/
		TEST_METHOD(TestMemory1)
		{
			address = 0;
			maxaddress = 0; //SimAddress_t
			SimAddress_t expectedAddress = 1;
			addMemory(0xc);
			Assert::AreEqual(address, expectedAddress);
			Assert::AreEqual(maxaddress, expectedAddress);
			Assert::AreEqual((int)memory[0], 0x0c);
			memoryMetaData_t* pMeta = getMemoryMeta(0);
			Assert::IsNotNull(pMeta);
			Assert::AreEqual((int)pMeta->fileId, 0); // 0:base file
			Assert::AreEqual((int)pMeta->line, 1);   // first line is 1
			Assert::AreEqual((int)pMeta->sectionId, 0);
			Assert::AreEqual((int)pMeta->sectionType, (int)MT_code);
			yylineno = 2;
			sectionId = 1;
			sectionType = MT_data;
			addMemory(0xa);
			pMeta = getMemoryMeta(1);
			Assert::IsNotNull(pMeta);
			Assert::AreEqual((int)memory[1], 0x0a);
			Assert::AreEqual((int)pMeta->line, 2);
			Assert::AreEqual((int)pMeta->sectionId, 1);
			Assert::AreEqual((int)pMeta->sectionType, (int)MT_data);

			pMeta = getMemoryMeta(MAXMEMORY); //invalid
			Assert::IsNull(pMeta);
		}

		/* SymbolLength and searchSymbol and setSymbol
		*/
		TEST_METHOD(TestSymbols1)
		{
			size_t valueS= SymbolLength();
			size_t expectedS = 0;
			Assert::AreEqual(valueS, expectedS);
			int v= searchSymbol("");
			Assert::AreEqual(v, -1);

			setSymbol("test", 3, ST_LOAD);
			valueS = SymbolLength();
			expectedS = 1;
			Assert::AreEqual(valueS, expectedS);
			v = searchSymbol("");
			Assert::AreEqual(v, -1);
			v = searchSymbol("test");
			Assert::AreEqual(v, 0);
		}
	};
}