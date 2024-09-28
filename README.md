# FlyingDisk for Linux

A linux version of the modern, cross-platform package manager.

## Installation

**From Source:**

1. Clone the repository: `git clone https://github.com/flyingdiskapp/flyingdisk-linux`
2. Navigate to the directory: `cd flyingdisk`
3. Build the project: `make`
4. Install the binary: `sudo make install`

## Usage

``flyingdisk [options] [command] [package] [version]``


**Options:**

* `-r, --root <path>`: Sets the root directory for package installations (default is "/")
* `--server <url>`: Changes the FlyingDisk server URL (default is "https://fdrepo.natesworks.com")

**Commands:**

* `install <package> [version]`: Installs a package. If no version is specified, the latest version is installed.
* `uninstall <package>`: Uninstalls a package.
* `fetch <package> <version> <file> <output_file>`: Fetches a specific file from a package.
* `info <package> [version]`: Displays information about a package. If no version is specified, the latest version is displayed.

**Error Codes:**

* **0:** Success
* **1:** General error (e.g., failed to fetch package information, failed to extract archive)
* **2:** Package not found or not installed
* **3:** Package does not support the current platform

## Examples

**Install the latest version of "mypackage":**

``flyingdisk install mypackage``


**Install version 1.2.3 of "mypackage":**

``flyingdisk install mypackage 1.2.3``

**Uninstall "mypackage":**

``flyingdisk uninstall mypackage``

**Fetch the "data.txt" file from version 1.0.0 of "mypackage" and save it to "mydata.txt":**

``flyingdisk fetch mypackage 1.0.0 data.txt mydata.txt``

**Display information about "mypackage":**

``flyingdisk info mypackage``

# Contributing

Contributions are welcome! Please open an issue or submit a pull request.

## License

This project is licensed under the MIT License.