#pragma once

#include <Windows.h>

#include <string>
#include <vector>

#include "Application.hpp"

class ApplicationBuilder {
public:
	ApplicationBuilder();

	ApplicationBuilder* setName(const std::string& appName);

	ApplicationBuilder* setPublisher(const std::string& appPublisher);

	ApplicationBuilder* setVersion(const std::string& appVersion);

	ApplicationBuilder* setInstallDate(const std::string& appInstallDate);

	ApplicationBuilder* setLocation(const std::string& appLocation);

	bool isApplicationReady();

	Application* build();

	void reset();

	bool tryToApplyValue(char* cValueName, uint64_t type, unsigned char* data);

private:
	std::string mAppName;
	bool mAppNameSet = false;
	std::string mPublisher;
	bool mPublisherSet = false;
	std::string mVersion;
	bool mVersionSet = false;
	std::string mInstallDate;
	bool mInstallDateSet = false;
	std::string mLocation;
	bool mLocationSet = false;
};

