#include <iostream>
#include <curl/curl.h>
#include "network.h"
#include "utils.h"
#include <nlohmann/json.hpp>

// Define the default server address
std::string serverAddress = "https://fdrepo.natesworks.com";
// Define the auth token variable
std::string authToken;

// Callback function for libcurl to write downloaded data
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    // Append the downloaded data to the string pointed to by userp
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    // Return the number of bytes processed
    return size * nmemb;
}

// Logs in to the package server
int login(const std::string &username, const std::string &password)
{
    // Initialize libcurl
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    // If curl initialization was successful
    if (curl)
    {
        // Initialize a string to store the response
        std::string readBuffer;
        // Construct the login URL
        const char *const url = (serverAddress + "/login").c_str();
        // Set the request method to POST
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // Create a JSON payload with the username and password
        nlohmann::json payload = {{"username", username}, {"password", password}};
        // Convert the JSON payload to a string
        std::string payload_str = payload.dump();

        // Set the POST fields and their size
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload_str.size());

        // Set the Content-Type header to application/json
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set the write callback function and data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Set the cookie jar and file
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookies.txt");
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "cookies.txt");

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK)
        {
            // Print an error message if the request failed
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else
        {
            // Get the HTTP response code
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            // If the response code is 200 (OK)
            if (http_code == 200)
            {
                // Store the auth token
                authToken = readBuffer;
                // Print a success message
                std::cout << "\033[32mLogin successful\033[0m" << std::endl;
            }
            else
            {
                // Print an error message if the login failed
                std::cerr << "\033[31mLogin failed\033[0m" << std::endl;
            }
        }

        // Clean up libcurl resources
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();
    return 0;
}

// Fetches package information from the server
PackageInfo fetchPackageInfo(const std::string &packageName, const std::string &packageVersion)
{
    // Initialize libcurl
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    PackageInfo packageInfo;

    curl = curl_easy_init();
    if (curl)
    {
        // Construct the package info URL
        std::string url = (serverAddress + "/packages/" + packageName + "/" + packageVersion + ".json");
        // Set the URL to fetch
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // Set the write callback function and data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK)
        {
            // Print an error message if the request failed
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Clean up libcurl resources
        curl_easy_cleanup(curl);

        // Parse the JSON response
        try
        {
            auto jsonResponse = nlohmann::json::parse(readBuffer);
            packageInfo.id = jsonResponse["id"];
            packageInfo.name = jsonResponse["name"];
            packageInfo.description = jsonResponse["description"];
            packageInfo.version = jsonResponse["version"];
            packageInfo.dependencies = jsonResponse["dependencies"];
            packageInfo.files = jsonResponse["files"];
            packageInfo.platform = jsonResponse["platform"];
        }
        catch (const nlohmann::json::parse_error &e)
        {
            // Print an error message if the JSON parsing failed
            std::cerr << "Json parse error: " << e.what() << std::endl;
        } // Correctly closed catch block
    } 

    // Return the package information
    return packageInfo;
}

// Fetches a package file from the server
int fetchPackage(const std::string &packageName, const std::string &packageVersion, const std::string &file, const std::string &outputFile)
{
    // Initialize libcurl
    CURL *curl;
    CURLcode res;
    // Open the output file for writing
    FILE *fp = fopen(outputFile.c_str(), "wb");
    if (!fp)
    {
        // Print an error message if the file could not be opened
        std::cerr << "Failed to open file: " << outputFile << std::endl;
        return 1;
    }

    curl = curl_easy_init();
    if (curl)
    {
        // Construct the package file URL
        std::string url = (serverAddress + "/packages/" + packageName + "/" + packageVersion + "/" + file);
        // Set the URL to fetch
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // Set the write function to NULL (default)
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        // Set the write data to the output file pointer
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Set up the progress bar
        ProgressData progressData = {file.c_str(), 0.0};
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressBar);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progressData);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK)
        {
            // Print an error message if the request failed
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            // Close the output file
            fclose(fp);
            return 1;
        }

        // Clean up libcurl resources
        curl_easy_cleanup(curl);
    }

    // Close the output file
    fclose(fp);
    // Print a newline after the progress bar
    std::cout << std::endl;
    return 0;
}
