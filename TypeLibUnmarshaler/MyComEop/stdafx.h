// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#ifdef ARRAYSIZE
#define countof(a) (ARRAYSIZE(a)) // (sizeof((a))/(sizeof(*(a))))
#else
#define countof(a) (sizeof((a)) / (sizeof(*(a))))

#endif






#pragma once

#include "targetver.h"
#include "stdafx.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h> 
#include <bits.h>
#include <bits4_0.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <comdef.h>
#include <Shlwapi.h>
#include <strsafe.h>
#include <vector>
#include <Sddl.h>
#include <atlbase.h>
#include <shellapi.h>
#include <AclAPI.h>
#include <map>


#define LEN 1024

bool CreateNativeHardlink(LPCWSTR linkname, LPCWSTR targetname);



// TODO:  在此处引用程序需要的其他头文件


typedef LUID OXID;
typedef LUID OID;
typedef GUID	IPID;

typedef struct tagDUALSTRINGARRAY    {
	unsigned short wNumEntries;     // Number of entries in array.
	unsigned short wSecurityOffset; // Offset of security info.
	unsigned short aStringArray[];
} DUALSTRINGARRAY;

typedef struct tagSTDOBJREF    {
	DWORD   flags;
	DWORD   cPublicRefs;
	OXID           oxid;
	OID            oid;
	IPID           ipid;
} STDOBJREF;

typedef struct tagOBJREF    {
	unsigned long signature;//MEOW
	unsigned long flags;
	GUID          iid;
	union        {
		struct            {
			STDOBJREF       std;
			DUALSTRINGARRAY saResAddr;
		} u_standard;
		struct            {
			STDOBJREF       std;
			CLSID           clsid;
			DUALSTRINGARRAY saResAddr;
		} u_handler;
		struct            {
			CLSID           clsid;
			unsigned long   cbExtension;
			unsigned long   size;
			ULONGLONG pData;
		} u_custom;
	} u_objref;
} OBJREF;

typedef struct ExploitData{
	;

	IID IID_Interface;
	IID TypeLib_Interface;
	OLECHAR extPath[LEN];
	OLECHAR dllPath[LEN];
	OLECHAR Interface_Name[LEN];
	UINT32 InterfaceVersionFirst;
	UINT32 InterfaceVersionNext;
}*PExploitData;