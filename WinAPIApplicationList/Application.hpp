#pragma once

#include <string>

class Application {
public:
	Application(const std::string& appName,
		const std::string& publisher,
		const std::string& version,
		const std::string& installDate,
		const std::string& location);

	friend std::ostream& operator << (std::ostream& stream, Application& application);

private:
	std::string mAppName;
	std::string mPublisher;
	std::string mVersion;
	std::string mInstallDate;
	std::string mLocation;

	std::string parseDate(const std::string& date);
};