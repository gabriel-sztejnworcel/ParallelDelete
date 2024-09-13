#pragma once

#include <Windows.h>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include <map>

struct FileEntry
{
	std::wstring path;
    DWORD attributes;
};

struct DirQueueEntry
{
    std::wstring path;
    int level;
};

struct TraverseResult
{
	std::vector<FileEntry> files;
    std::map<int, std::vector<FileEntry>> dirsByLevel;
};

class ParallelTraversalBfs
{
public:
    virtual ~ParallelTraversalBfs();
    TraverseResult Traverse(const std::wstring& root, int nworkers);

private:
    void Worker();
    void ProcessDir(const std::wstring& path, int level);

    void AddFileResult(const std::wstring& path, DWORD attributes);
    void AddDirResult(const std::wstring& path, DWORD attributes, int level);
    void EnqueueDir(const std::wstring& path, int level);

    std::vector<DirQueueEntry> dirQueue_;
    std::mutex dirQueueMtx_;

    std::vector<FileEntry> files_;
    std::mutex filesMtx_;

    std::map<int, std::vector<FileEntry>> dirsByLevel_;
    std::mutex dirsByLevelMtx_;

    HANDLE evt_ = nullptr;
    std::atomic<int> activeWorkers_ = 0;
};
