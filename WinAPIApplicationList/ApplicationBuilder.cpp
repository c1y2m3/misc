#include "ApplicationBuilder.hpp"

ApplicationBuilder::ApplicationBuilder() { }

ApplicationBuilder* ApplicationBuilder::setName(const std::string& appName) {
	this->mAppName = appName;
	this->mAppNameSet = true;
	return this;
}

ApplicationBuilder* ApplicationBuilder::setPublisher(const std::string& appPublisher) {
	this->mPublisher = appPublisher;
	this->mPublisherSet = true;
	return this;
}

ApplicationBuilder* ApplicationBuilder::setVersion(const std::string& appVersion) {
	this->mVersion = appVersion;
	this->mVersionSet = true;
	return this;
}

ApplicationBuilder* ApplicationBuilder::setInstallDate(const std::string& appInstallDate) {
	this->mInstallDate = appInstallDate;
	this->mInstallDateSet = true;
	return this;
}

ApplicationBuilder* ApplicationBuilder::setLocation(const std::string& appLocation) {
	this->mLocation = appLocation;
	this->mLocationSet = true;
	return this;
}

bool ApplicationBuilder::isApplicationReady() {
	return
		this->mAppNameSet &&
		this->mInstallDateSet &&
		this->mPublisherSet &&
		this->mVersionSet &&
		this->mLocationSet;
}

Application* ApplicationBuilder::build() {
	if (!isApplicationReady())
		return nullptr;
	return new Application(mAppName, mPublisher, mVersion, mInstallDate, mLocation);
}

void ApplicationBuilder::reset() {
	this->mAppNameSet = false;
	this->mInstallDateSet = false;
	this->mPublisherSet = false;
	this->mVersionSet = false;
	this->mLocationSet = false;
}

bool ApplicationBuilder::tryToApplyValue(char* cValueName, uint64_t type, unsigned char* data) {
	std::string valueName(cValueName);

	if (type == REG_SZ || type == REG_EXPAND_SZ) {

		if (valueName == "DisplayName") {
			std::string displayName((char*)data);
			setName(displayName);
			return true;
		}

		if (valueName == "DisplayVersion") {
			std::string displayVersion((char*)data);
			setVersion(displayVersion);
			return true;
		}

		if (valueName == "InstallDate") {
			std::string installDate((char*)data);
			setInstallDate(installDate);
			return true;
		}

		if (valueName == "InstallLocation") {
			std::string installLocation((char*)data);
			setLocation(installLocation);
			return true;
		}

		if (valueName == "Publisher") {
			std::string publisher((char*)data);
			setPublisher(publisher);
			return true;
		}
	}

	return false;
}
