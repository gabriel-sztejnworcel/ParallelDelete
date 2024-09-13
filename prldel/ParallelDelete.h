#pragma once

#include "ParallelTraversalBfs.h"

struct DeleteResult
{
    int filesDeleted = 0;
    int dirsDeleted = 0;
    int filesFailed = 0;
    int dirsFailed = 0;
};

class ParallelDelete
{
public:
    DeleteResult Delete(const TraverseResult& traverseResult, int nworkers, bool suppressErrors);

private:
    void DeleteFilesInParallel(const std::vector<FileEntry>& files, int nworkers);
    void DeleteFiles(const std::vector<FileEntry>& files, int nworkers);

    bool suppressErrors_ = false;
    std::atomic<int> filesDeleted_ = 0;
    std::atomic<int> dirsDeleted_ = 0;
    std::atomic<int> filesFailed_ = 0;
    std::atomic<int> dirFailed_ = 0;
};
