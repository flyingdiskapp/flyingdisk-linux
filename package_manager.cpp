#include <iostream>
#include <fstream>
#include <filesystem>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "package_manager.h"
#include "network.h"
#include "utils.h"
#include "archive_manager.h"
#include "main.h"

int install(std::string package, std::string version, int loglevel, bool reinstall, bool noconfirm)
{
    // Check if already installed
    std::string packageListPath = root + "etc/birdy/packages.json";
    bool isReinstall = false;
    if (isPackageInstalled(packageListPath, package, version))
    {
        isReinstall = true;
        if (!reinstall)
        return 0;

        if (loglevel >= 2)
        std::cout << "warning: " << package << " is already up to date - reinstalling" << std::endl;

    }

    // Fetch package information
     if (loglevel >= 4)
    std::cout << "fetching package metadata...";
    std::vector<std::string> extractedFiles;
    PackageInfo packageInfo = fetchPackageInfo(package, version);
    if (loglevel >= 4)
        std::cout << "done" << std::endl << std::endl;

    if (std::find(packageInfo.platform.begin(), packageInfo.platform.end(), "Linux") == packageInfo.platform.end()) {
        std::cerr << "\033[1;31m[ERROR]\033[0m Package " << package << " does not support Linux." << std::endl;
        return 1;
    }
    int packageCount = 1;
    for (const std::string& dependency : packageInfo.dependencies)
    {
        packageCount++;
    }

    // Display confirmation and packages to install
    if(loglevel >= 3)
    std::cout << "Packages (" << packageCount << ") " << package << " ";
    
    if (loglevel >=3)
    for (const std::string& dependency : packageInfo.dependencies)
    {
        std::cout << dependency << " " << std::endl << std::endl;
    }

    // Ask to procceed with installation
    std::string procceed;
    if (!noconfirm)
    {
        std::cout << "Procceed with installation? [Y/n] ";
        std::cin >> procceed;
    }

    if (procceed != "Y" or procceed != "y")
    {
        if (loglevel >= 2)
        {
            std::cout << "Aborted." << std::endl;
            return 1;
        }
    }

    for (const std::string& dependency : packageInfo.dependencies)
    {
        install(dependency, fetchLatestVersion(dependency), loglevel = 0, reinstall = false, noconfirm = true);
    }

    // Download package
    std::string archivePath = "/tmp/" + packageInfo.files[0];
    if (loglevel >= 2)
    std::cout << "Fetching packages..." << std::endl;
    fetchPackage(package, version, packageInfo.files[0], archivePath);

    // Extract archive
    if (loglevel >= 2)
    std::cout << "Extracting archive...";
    extractArchive(archivePath, root, extractedFiles);
    if (loglevel >= 2)
    std::cout << "done" << std::endl;

    // Update package list
    if (!isReinstall)
    {
        if (loglevel >= 4)
        std::cout << "writing package information...";
        writeExtractedFilesList(packageListPath, extractedFiles, package, version);
        if (loglevel >= 4)
        std::cout << "done" << std::endl;
    }
    if (loglevel >= 1)
    std::cout << "Successfully installed " << package << "!" << std::endl;
    return 0;
}

// Gets the latest version and calls install with that version
int install(std::string package)
{
    std::string version = fetchLatestVersion(package);
    return install(package, version);
}

int uninstall(std::string package)
{
    std::string packageListPath = root + "etc/birdy/packages.json";

    // Check if package is installed
    bool installed = isPackageInstalled(packageListPath, package);
    if (!installed)
    {
        std::cout << "Package is not installed!" << std::endl;
        return 2;
    }
    std::vector<std::string> files = getPackageFiles(packageListPath, package);

    // Remove all files related to the package
    for (std::string file : files)
    {
        try
        {
            if (std::filesystem::remove(file))
                std::cout << "File " << file << " deleted.\n";
            else
                std::cout << "File " << file << " not found.\n";
        }
        catch (const std::filesystem::filesystem_error &err)
        {
            std::cout << "Filesystem error: " << err.what() << '\n';
        }
    }

    // Remove package from package list
    removePackage(packageListPath, package);

    return 0;
}

// Displays package information
int displayPackageInfo(std::string package, std::string version)
{
    PackageInfo packageInfo = fetchPackageInfo(package, version);
    std::cout << "ID: " << packageInfo.id << std::endl;
    std::cout << "Name: " << packageInfo.name << std::endl;
    std::cout << "Description: " << packageInfo.description << std::endl;
    std::cout << "Version: " << packageInfo.version << std::endl;
    std::cout << "Dependencies: ";
    for (std::string dependency : packageInfo.dependencies)
    {
        std::cout << dependency << ", ";
    }
    std::cout << std::endl;

    return 0;
}

// Calls displayPackageInfo with the latest version
int displayPackageInfo(std::string package)
{
    displayPackageInfo(package, "latest");
    return 0;
}