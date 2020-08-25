#pragma once

#include <Windows.h>
#include <vector>

#include "ApplicationBuilder.hpp"

class WinAPIHelper {
private:
	static HKEY openApplications() {
		return openKey(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\",
			//"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\",
			KEY_ENUMERATE_SUB_KEYS);
	}

	static std::vector<Application*> queryApplications(HKEY key) {
		CHAR keyName[255] = { 0 };
		DWORD keySize = 255;
		size_t index = 0;

		std::vector<Application*> applications;

		ApplicationBuilder* builder = new ApplicationBuilder;
		for (DWORD regError = ERROR_SUCCESS;
			regError == ERROR_SUCCESS;
			regError = RegEnumKeyExA(key, index, keyName, &keySize, NULL, NULL, NULL, NULL), index++, keySize = 255) {
			try {
				HKEY subKey = openKey(key, keyName, KEY_QUERY_VALUE);
				buildApplication(subKey, builder);

				if (builder->isApplicationReady()) {
					Application* application = builder->build();
					applications.push_back(application);
				}

				builder->reset();
				RegCloseKey(subKey);
			} catch (std::exception & _) {}


		}

		return applications;
	}

	static void buildApplication(HKEY key, ApplicationBuilder* builder) {
		size_t index = 0;
		CHAR valueName[255] = { 0 };
		DWORD valueNameSize = 255;

		BYTE data[1024];
		DWORD dataSize = 1024;

		DWORD type = 0;

		for (DWORD readError = ERROR_SUCCESS;
			readError == ERROR_SUCCESS;
			readError = RegEnumValueA(key,
				index,
				valueName,
				&valueNameSize,
				NULL, &type,
				data, &dataSize), index++, valueNameSize = 255, dataSize = 1024) {
			builder->tryToApplyValue(valueName, type, data);
		}
	}

public:
	static HKEY openKey(HKEY parent, const CHAR* subKey, DWORD access) {
		HKEY key;
		if (RegOpenKeyExA(parent, subKey, 0, access, &key) == ERROR_SUCCESS)
			return key;
		else throw std::exception((std::string("Failed to open register entry ") + subKey).c_str());
	}

	static std::vector<Application*> getApplicationList() {
		HKEY appsKey = openApplications();
		std::vector<Application*> applications = queryApplications(appsKey);
		RegCloseKey(appsKey);
		return applications;
	}
};