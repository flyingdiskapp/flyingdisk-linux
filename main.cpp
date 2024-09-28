#include <iostream>
#include "argparse/argparse.hpp"
#include "package_manager.h"
#include "network.h"
#include "utils.h"

std::string root = "/";

int main(int argc, char *argv[])
{
    argparse::ArgumentParser parser("Birdy");

    parser.add_argument("-i", "--install")
        .help("Installs a package")
        .nargs(argparse::nargs_pattern::at_least_one);

    parser.add_argument("-u", "--uninstall")
        .help("Uninstalls a package")
        .nargs(1);

    parser.add_argument("-r", "--root")
        .help("Sets the root where Birdy will install packages")
        .nargs(1);

    parser.add_argument("--fetch")
        .help("Fetch a package's file")
        .nargs(4);

    parser.add_argument("--info")
        .help("Get information about a package")
        .nargs(argparse::nargs_pattern::at_least_one);

    parser.add_argument("--server")
        .help("Change the Birdy server")
        .nargs(1);

    try
    {
        parser.parse_args(argc, argv);
        if (parser.present("--root"))
        {
            root = parser.get<std::string>("--root");
            if (root[root.size() - 1] != '/')
            {
                root += "/";
            }
        }

        if (parser.present("--server"))
        {
            auto serverArgs = parser.get<std::vector<std::string>>("--server");
            serverAddress = serverArgs[0];
        }

        if (parser.present("--install"))
        {
            auto installArgs = parser.get<std::vector<std::string>>("--install");
            if (installArgs.size() == 1)
            {
                install(installArgs[0]);
            }
            else
            {
                install(installArgs[0], installArgs[1]);
            }
        }

        if (parser.present("--uninstall"))
        {
            auto uninstallArgs = parser.get<std::vector<std::string>>("--uninstall");
            uninstall(uninstallArgs[0]);
        }

        if (parser.present("--fetch"))
        {
            auto fetchArgs = parser.get<std::vector<std::string>>("--fetch");
            fetchPackage(fetchArgs[0], fetchArgs[1], fetchArgs[2], fetchArgs[3]);
        }

        if (parser.present("--info"))
        {
            auto infoArgs = parser.get<std::vector<std::string>>("--info");
            if (infoArgs.size() == 1)
            {
                displayPackageInfo(infoArgs[0]);
            }
            else
            {
                displayPackageInfo(infoArgs[0], infoArgs[1]);
            }
        }
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << std::endl;
        std::cout << parser;
        return 1;
    }

    return 0;
}
