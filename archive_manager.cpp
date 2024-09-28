#include <archive.h>
#include <archive_entry.h>
#include <iostream>
#include "archive_manager.h"
#include "utils.h"

void extractArchive(const std::string &archivePath, const std::string &outputDir, std::vector<std::string> &extractedFiles)
{
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_compression_all(a);
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME);
    archive_write_disk_set_standard_lookup(ext);
    if ((r = archive_read_open_filename(a, archivePath.c_str(), 10240)))
    {
        std::cerr << "Failed to open archive: " << archive_error_string(a) << std::endl;
        return;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
    {
        const char *currentFile = archive_entry_pathname(entry);
        std::string fullOutputPath = outputDir;
        if (!fullOutputPath.empty() && fullOutputPath[fullOutputPath.size() - 1] != '/')
        {
            fullOutputPath += "/";
        }
        fullOutputPath += currentFile;
        archive_entry_set_pathname(entry, fullOutputPath.c_str());

        if (archive_entry_filetype(entry) == AE_IFREG)
        {
            extractedFiles.push_back(fullOutputPath);
        }

        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK)
        {
            std::cerr << "archive_write_header() failed: " << archive_error_string(ext) << std::endl;
        }
        else
        {
            copyData(a, ext);
            r = archive_write_finish_entry(ext);
            if (r != ARCHIVE_OK)
            {
                std::cerr << "archive_write_finish_entry() failed: " << archive_error_string(ext) << std::endl;
            }
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
}

int copyData(struct archive *ar, struct archive *aw)
{
    const void *buff;
    size_t size;
    la_int64_t offset;

    for (;;)
    {
        int r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r != ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK)
        {
            std::cerr << "archive_write_data_block() failed: " << archive_error_string(aw) << std::endl;
            return (r);
        }
    }
}