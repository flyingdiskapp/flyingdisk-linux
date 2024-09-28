#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include "package_manager.h"

extern std::string serverAddress;
extern std::string authToken;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
int login(const std::string &username, const std::string &password);
PackageInfo fetchPackageInfo(const std::string &packageName, const std::string &packageVersion);
int fetchPackage(const std::string &packageName, const std::string &packageVersion, const std::string &file, const std::string &outputFile);

#endif // NETWORK_H
