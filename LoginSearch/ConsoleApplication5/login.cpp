#pragma comment(lib, "wevtapi.lib")
#pragma comment(lib, "Netapi32.lib")
#ifndef UNICODE
#define UNICODE
#endif
#include <Windows.h>
#include <winevt.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <fstream>
#include <regex>
#include <numeric>
#include <string>
#include <set> 
#include <lm.h>
#include <assert.h>
#include "tchar.h"
#include "conio.h"
using namespace std;




const int MAX_BUFFER = 2048;


int execcmd(char* cmd, char* result)
{
	char buffer[MAX_BUFFER];                         // 缓冲区                        
	FILE* pipe = _popen(cmd, "r");            // 管道 

	// 管道打开失败
	if (!pipe) { return 0; }

	// 检测管道中的结束符，0表示没有结束
	while (!feof(pipe)) {
		// 从管道中读取数据
		if (fgets(buffer, 128, pipe)) {
			// 拼接 char
			strcat(result, buffer);
		}
	}

	//关闭管道 
	_pclose(pipe);

	return 1;
}

static auto search_by_regex(const char* regex_s, const string& s)
{ // 
	regex reg_ex(regex_s);
	smatch match_result;
	vector<std::string> dipole_list;
	std::string lastdata;

	auto iter_begin = sregex_iterator(s.begin(), s.end(), reg_ex);
	auto iter_end = sregex_iterator();
	for (auto iter = iter_begin; iter != iter_end; iter++) {
		dipole_list.push_back(iter->str(0) + "\n");

	}
	lastdata = accumulate(dipole_list.begin(), dipole_list.end(), lastdata);
	return lastdata;

}

int write_string_to_file_append(const std::string & file_string, const std::string str)
{
	std::ofstream	OsWrite(file_string, std::ofstream::app);
	OsWrite << str;
	OsWrite << std::endl;
	OsWrite.close();
	return 0;
}

string readFileIntoString(char * filename)
{
	ifstream ifile(filename);
	//将文件读入到ostringstream对象buf中
	ostringstream buf;
	char ch;
	while (buf&&ifile.get(ch))
		buf.put(ch);
	//返回与流对象buf关联的字符串
	return buf.str();
}



ifstream & open_file(ifstream &in, const string & file)
{
	in.close();
	in.clear();
	in.open(file.c_str());
	return in;
}

void regexit()
{
	char result[0x7ffff] = "";        // 存放结果
	char buf[1024];
	string temp;
	ifstream fin("LogParser.exe");
	if (fin)
	{
		char cmd[] = "LogParser.exe -i:EVT -o csv \"SELECT TO_UPPERCASE(EXTRACT_TOKEN(Strings, 5, '|')) as NAME, TO_UPPERCASE(EXTRACT_TOKEN(Strings, 18, '|')) as IP FROM C:\\Windows\\Temp\\temp.xlsx\"  1>C:\\Windows\\Temp\\null";
		execcmd(cmd, result);
		char * fn = (char *)"C:\\Windows\\Temp\\null";
		string str;
		str = readFileIntoString(fn);
		char reg[] = "[0-9a-zA-Z]+,([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)";
		string ipadder = search_by_regex(reg, str);       // ⑦
		string filename = "C:\\Windows\\Temp\\microsoft";
		write_string_to_file_append(filename, ipadder.c_str());
		set <string> ss;
		set <string>::iterator iter;
		ifstream infile;
		if (!open_file(infile, filename))
		{
			cout << "No input file!" << endl;
		}
		string textline;
		while (getline(infile, textline))
			ss.insert(textline);
		for (iter = ss.begin(); iter != ss.end(); iter++)
			cout << *iter << endl;
	}
	else
	{
		cout << "Error:Please install logparser" << endl;
	}
	char delcmd[] = "del /s /q C:\\Windows\\Temp\\microsoft";
	char dellcmd[] = "del /s /q C:\\Windows\\Temp\\temp.xlsx";
	char delllcmd[] = "del /s /q C:\\Windows\\Temp\\null";
	execcmd(delcmd, result);
	execcmd(dellcmd, result);
	execcmd(delllcmd, result);
}


void wcharTochar(const wchar_t *wchar, char *chr, int length)
{
	WideCharToMultiByte(CP_ACP, 0, wchar, -1,
		chr, length, NULL, NULL);
}

bool QueryRegKey(const TCHAR *keyName, LPCWSTR strSubKey, LPCWSTR strValueName, char *strValue, int length)
{
	DWORD dwType = REG_SZ;//定义数据类型
	DWORD dwLen = MAX_PATH;

	wchar_t data[MAX_PATH];
	HKEY hKey = NULL;
	HKEY hSubKey;


	if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, strSubKey, &hSubKey))
	{
		TCHAR buf[256] = { 0 };

		if (ERROR_SUCCESS == RegQueryValueEx(hSubKey, strValueName, 0, &dwType, (LPBYTE)data, &dwLen))
		{
			wcharTochar(data, strValue, length);
			wprintf(L"%s ----- username: %s \n", keyName, data);
			RegCloseKey(hKey); //关闭注册表
			return true;
		}
	}
	RegCloseKey(hKey); //关闭注册表


	return false;
}

void Session()
{
	LPSESSION_INFO_10 pBuf = NULL;
	LPSESSION_INFO_10 pTmpBuf;
	DWORD dwLevel = 10;
	DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwResumeHandle = 0;
	DWORD i;
	DWORD dwTotalCount = 0;
	LPTSTR pszServerName = NULL;
	LPTSTR pszClientName = NULL;
	LPTSTR pszUserName = NULL;
	NET_API_STATUS nStatus;
	//
	// Check command line arguments.
	//
	//
	// Call the NetSessionEnum function, specifying level 10.
	//
	do // begin do
	{
		nStatus = NetSessionEnum(pszServerName,
			pszClientName,
			pszUserName,
			dwLevel,
			(LPBYTE*)&pBuf,
			dwPrefMaxLen,
			&dwEntriesRead,
			&dwTotalEntries,
			&dwResumeHandle);
		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
		{
			if ((pTmpBuf = pBuf) != NULL)
			{
				//
				// Loop through the entries.
				//
				for (i = 0; (i < dwEntriesRead); i++)
				{
					assert(pTmpBuf != NULL);

					//
					// Print the retrieved data. 
					//
					wprintf(L"\n\tClient: %s\n", pTmpBuf->sesi10_cname);
					wprintf(L"\tUser:   %s\n", pTmpBuf->sesi10_username);
					printf("\tActive: %d\n", pTmpBuf->sesi10_time);
					printf("\tIdle:   %d\n", pTmpBuf->sesi10_idle_time);

					pTmpBuf++;
					dwTotalCount++;
				}
			}
		}
		//
		// Otherwise, indicate a system error.
		//
		else
			fprintf(stderr, "A system error has occurred: %d\n", nStatus);
		//
		// Free the allocated memory.
		//
		if (pBuf != NULL)
		{
			NetApiBufferFree(pBuf);
			pBuf = NULL;
		}
	}
	// 
	// Continue to call NetSessionEnum while 
	//  there are more entries. 
	// 
	while (nStatus == ERROR_MORE_DATA); // end do

	// Check again for an allocated buffer.
	//
	if (pBuf != NULL)
		NetApiBufferFree(pBuf);

}

int main(int argc, char* argv[])
{
	{
		_wsetlocale(LC_ALL, _T("")); //设置或检索本地运行环境,避免wprintf打印中文乱码
		cout << "Get the data to read the RegKey...\n" << endl;
		HKEY hKey = NULL; //保存注册表的句柄 
		DWORD dwIndexs = 0; //需要返回子项的索引 
		TCHAR keyName[MAX_PATH] = { 0 }; //保存子键的名称 
		DWORD charLength = 600;  //想要读取多少字节并返回实际读取到的字符长度
		const TCHAR *subKey = _T("Software\\Microsoft\\Terminal Server Client\\Servers\\");
		if (RegOpenKeyEx(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			while (RegEnumKeyEx(hKey, dwIndexs, keyName, &charLength, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
			{
				wchar_t test[888] = L"Software\\Microsoft\\Terminal Server Client\\Servers\\";
				++dwIndexs;
				charLength = 600;
				wcscat_s(test, keyName);
				LPCWSTR strValueName = _T("UsernameHint");
				char strValue[256];
				int length = 256;
				bool status = QueryRegKey(keyName, test, strValueName, strValue, length);

			}
		}
		if (hKey != NULL)
		{
			RegCloseKey(hKey);
		}

	}
	//cout << "\nGet Windows Log EvtEid 4624 and 4625..." << endl;
	//wchar_t logpath[100] = L"C:\\Windows\\Temp\\temp.xlsx";
	//BOOL flag = EvtExportLog(
	//	NULL,
	//	L"Security",
	//	L"Event/System[(EventID=4624)]",
	//	logpath,
	//	EvtExportLogChannelPath
	//);
	//int i = GetLastError();
	//regexit();
	cout << "\nAccessing network NetSessionEnum..." << endl;
	Session();

}