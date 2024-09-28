#include <iostream>
#include <fstream>
#include "utils.h"
#include "package_manager.h"
#include "network.h"
#include <nlohmann/json.hpp>

// Formats a given size in bytes to a human-readable string with appropriate units (B, KB, MB, GB, TB).
std::string formatSize(double size)
{
    // Array of size suffixes
    const char *suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    // Index of the current suffix
    int suffixIndex = 0;
    // While the size is greater than 1024 and the suffix index is less than 3
    while (size >= 1024 && suffixIndex < 3)
    {
        // Divide the size by 1024
        size /= 1024;
        // Increment the suffix index
        suffixIndex++;
    }
    // Create a buffer to store the formatted string
    char buffer[50];
    // Format the size and suffix into the buffer
    snprintf(buffer, sizeof(buffer), "%.2f %s", size, suffixes[suffixIndex]);
    // Return the formatted string
    return std::string(buffer);
}

// Displays a progress bar in the console.
// This function is designed to be used as a callback function for libcurl's progress meter.
int progressBar(void *ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
{
    // credits: https://stackoverflow.com/a/1639047
    // Cast the void pointer to a ProgressData pointer
    ProgressData *progressData = (ProgressData *)ptr;

    // If the total download size is less than or equal to 0, return
    if (TotalToDownload <= 0.0)
    {
        return 0;
    }

    // Calculate the percentage and number of dots to display
    int totaldotz = 40;
    double fractiondownloaded = NowDownloaded / TotalToDownload;
    int dotz = (int)round(fractiondownloaded * totaldotz);

    // Print the file name
    printf("\r%s ", progressData->fileName);

    // Print the progress bar
    int ii = 0;
    printf("%.0f%%[", fractiondownloaded * 100);
    for (; ii < dotz; ii++)
    {
        printf("=");
    }
    for (; ii < totaldotz; ii++)
    {
        printf(" ");
    }
    printf("]");

    // Format and print the downloaded and total sizes
    std::string nowDownloadedStr = formatSize(NowDownloaded);
    std::string totalToDownloadStr = formatSize(TotalToDownload);

    printf(" %s/%s", nowDownloadedStr.c_str(), totalToDownloadStr.c_str());
    // Clear the rest of the line
    printf("\033[K");
    // Flush the output buffer
    fflush(stdout);
    return 0;
}

// Writes a list of extracted files to a JSON file.
void writeExtractedFilesList(const std::string &listPath, const std::vector<std::string> &extractedFiles, std::string packageName, std::string packageVersion)
{
    // Create a JSON object
    nlohmann::json j;
    // Open the file for reading
    std::ifstream infile(listPath);
    // If the file is open
    if (infile.is_open())
    {
        // Parse the JSON from the file
        infile >> j;
        // Close the file
        infile.close();
    }

    // Create a JSON object for the package information
    nlohmann::json packageInfo;
    // Set the package name
    packageInfo["name"] = packageName;
    // Set the package version
    packageInfo["version"] = packageVersion;
    // Set the extracted files
    packageInfo["files"] = extractedFiles;

    // Add the package information to the JSON object
    j.push_back(packageInfo);

    // Open the file for writing
    std::ofstream outfile(listPath);
    // If the file is open
    if (outfile.is_open())
    {
        // Write the JSON to the file
        outfile << j.dump(4);
        // Close the file
        outfile.close();
    }
    else
    {
        // If the file could not be opened, print an error message
        std::cerr << "Could not open file for writing: " << listPath << std::endl;
    }
}

// Checks if a package is installed by looking for its name and version in the package list file.
bool isPackageInstalled(const std::string &listPath, const std::string &packageName, const std::string &packageVersion) {
    // Create a JSON object
    nlohmann::json j;
    // Open the file for reading
    std::ifstream infile(listPath);
    // If the file is not open
    if (!infile.is_open()) {
        // Print an error message
        std::cerr << "Could not open file for reading: " << listPath << std::endl;
        // Return false
        return false;
    }

    // Parse the JSON from the file
    infile >> j;

    // Iterate over the packages in the JSON object
    for (const auto &packageInfo : j) {
        // If the package name and version match
        if (packageInfo["name"] == packageName && (packageVersion == "_any" || packageInfo["version"] == packageVersion)) {
            // Return true
            return true;
        }
    }
    // If the package is not found, return false
    return false;
}

// Checks if a package is installed by looking for its name in the package list file.
bool isPackageInstalled(const std::string &listPath, const std::string &packageName)
{
    // Call the overloaded isPackageInstalled function with the package version set to "_any"
    return isPackageInstalled(listPath, packageName, "_any");
}

// Gets a list of files belonging to a specific package from the package list file.
std::vector<std::string> getPackageFiles(const std::string &listPath, std::string packageName)
{
    // Create a JSON object
    nlohmann::json j;
    // Open the file for reading
    std::ifstream infile(listPath);

    // If the file is open
    if (infile.is_open())
    {
        // Try to parse the JSON from the file
        try
        {
            infile >> j;
        }
        // If there is a parse error
        catch (const nlohmann::json::parse_error &e)
        {
            // Print an error message
            std::cerr << "nlohmann::json parse error: " << e.what() << std::endl;
        }
        // Close the file
        infile.close();
    }
    else
    {
        // If the file could not be opened, print an error message
        std::cerr << "Could not open file for reading: " << listPath << std::endl;
    }

    // Iterate over the packages in the JSON object
    for (const auto &packageInfo : j)
    {
        // If the package name matches
        if (packageInfo["name"] == packageName)
        {
            // Return the list of files
            return packageInfo["files"];
        }
    }
    // If the package is not found, return an empty vector
    return std::vector<std::string>();
}

// Fetches the latest version of a package from the server.
std::string fetchLatestVersion(const std::string &packageName)
{
    // Fetch the package info for the latest version
    auto packageInfo = fetchPackageInfo(packageName, "latest");
    // Return the version string
    return packageInfo.version;
}

// Removes a package from the package list file.
int removePackage(const std::string &listPath, const std::string &packageName)
{
    // Create a JSON object
    nlohmann::json j;
    // Open the file for reading
    std::ifstream infile(listPath);
    // If the file is open
    if (infile.is_open())
    {
        // Parse the JSON from the file
        infile >> j;
        // Close the file
        infile.close();
    }
    else
    {
        // If the file could not be opened, print an error message
        std::cerr << "Could not open file for reading: " << listPath << std::endl;
        // Return 1
        return 1;
    }

    // Flag to indicate if the package was found
    bool packageFound = false;
    // Iterate over the packages in the JSON object
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        // If the package name matches
        if ((*it)["name"] == packageName)
        {
            // Erase the package from the JSON object
            j.erase(it);
            // Set the package found flag to true
            packageFound = true;
            // Break out of the loop
            break;
        }
    }

    // If the package was not found
    if (!packageFound)
    {
        // Print an error message
        std::cerr << "Package not found: " << packageName << std::endl;
        // Return 2
        return 2;
    }

    // Open the file for writing
    std::ofstream outfile(listPath);
    // If the file is open
    if (outfile.is_open())
    {
        // Write the JSON to the file
        outfile << j.dump(4);
        // Close the file
        outfile.close();
        // Return 0
        return 0;
    }
    else
    {
        // If the file could not be opened, print an error message
        std::cerr << "Could not open file for writing: " << listPath << std::endl;
        // Return 1
        return 1;
    }
}
