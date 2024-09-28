#ifndef ARCHIVE_MANAGER_H
#define ARCHIVE_MANAGER_H

#include <string>
#include <vector>

// Archive management functions
void extractArchive(const std::string &archivePath, const std::string &outputDir, std::vector<std::string> &extractedFiles);
int copyData(struct archive *ar, struct archive *aw);

#endif // ARCHIVE_MANAGER_H
