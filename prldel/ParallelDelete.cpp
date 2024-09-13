
#include "ParallelDelete.h"

DeleteResult ParallelDelete::Delete(const TraverseResult& traverseResult, int nworkers, bool suppressErrors)
{
    suppressErrors_ = suppressErrors;

    this->DeleteFilesInParallel(traverseResult.files, nworkers);

    for (auto it = traverseResult.dirsByLevel.rbegin(); it != traverseResult.dirsByLevel.rend(); ++it)
    {
        this->DeleteFilesInParallel(it->second, nworkers);
    }

    return DeleteResult{ filesDeleted_, dirsDeleted_, filesFailed_, dirFailed_ };
}

void ParallelDelete::DeleteFilesInParallel(const std::vector<FileEntry>& files, int nworkers)
{
    size_t totalFiles = files.size();

    if (nworkers <= 0 || totalFiles <= 0)
    {
        return;
    }

    if (nworkers > totalFiles)
    {
        nworkers = (int)totalFiles;
    }

    size_t filesPerThread = (totalFiles + nworkers - 1) / nworkers; // Ceiling division
    std::vector<std::thread> threads;

    for (int i = 0; i < nworkers; ++i)
    {
        if (i * filesPerThread >= totalFiles)
        {
            break;
        }

        auto startIter = files.begin() + i * filesPerThread;
        auto endIter = files.end();
        if ((i + 1) * filesPerThread <= totalFiles)
        {
            endIter = (i == nworkers - 1) ? files.end() : startIter + filesPerThread;
        }

        if (startIter < files.end())
        {
            std::vector<FileEntry> subList(startIter, endIter);
            threads.push_back(std::thread([subList, this, nworkers]()
            {
                this->DeleteFiles(subList, nworkers);
            }));
        }
    }

    for (auto& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

void ParallelDelete::DeleteFiles(const std::vector<FileEntry>& files, int nworkers)
{
    for (const FileEntry& fileEntry : files)
    {
        BOOL result = FALSE;

        if (fileEntry.attributes & FILE_ATTRIBUTE_READONLY)
        {
            DWORD attributes = fileEntry.attributes;
            attributes &= ~FILE_ATTRIBUTE_READONLY;
            SetFileAttributesW(fileEntry.path.c_str(), attributes);
        }

        if (fileEntry.attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (RemoveDirectoryW(fileEntry.path.c_str()))
            {
                ++dirsDeleted_;
            }
            else
            {
                if (!suppressErrors_)
                {
                    wprintf(L"Failed to delete directory (%s, %d)\n", fileEntry.path.c_str(), GetLastError());
                }

                ++dirFailed_;
            }
        }
        else
        {
            if (DeleteFileW(fileEntry.path.c_str()))
            {
                ++filesDeleted_;
            }
            else
            {
                if (!suppressErrors_)
                {
                    wprintf(L"Failed to delete file (%s, %d)\n", fileEntry.path.c_str(), GetLastError());
                }

                ++filesFailed_;
            }
        }
    }
}
