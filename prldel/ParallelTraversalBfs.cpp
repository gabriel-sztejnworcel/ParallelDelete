
#include "ParallelTraversalBfs.h"

#include <cassert>
#include <thread>

ParallelTraversalBfs::~ParallelTraversalBfs()
{
    if (evt_ != nullptr)
	{
		CloseHandle(evt_);
	}
}

TraverseResult ParallelTraversalBfs::Traverse(const std::wstring& root, int nworkers)
{
    dirQueue_.clear();
    files_.clear();
    dirsByLevel_.clear();

    DWORD attributes = GetFileAttributesW(root.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
	{
        // TODO: Error handling
        return TraverseResult{ std::move(files_), std::move(dirsByLevel_) };
    }

    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        this->AddFileResult(root, attributes);
        return TraverseResult{ std::move(files_), std::move(dirsByLevel_) };
    }

    if (evt_ == nullptr)
    {
        evt_ = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        assert(evt_ != nullptr);
    }

    std::vector<std::thread> workers;
    for (int i = 0; i < nworkers; ++i)
    {
        workers.push_back(std::thread(&ParallelTraversalBfs::Worker, this));
    }

    this->EnqueueDir(root, 0);
    this->AddDirResult(root, attributes, 0);

    for (auto& worker : workers)
    {
        worker.join();
    }

    return TraverseResult{ std::move(files_), std::move(dirsByLevel_) };
}

void ParallelTraversalBfs::Worker()
{
    while (true)
    {
        DirQueueEntry dirQueueEntry{ L"", -1 };

        {
            std::lock_guard<std::mutex> lock(dirQueueMtx_);
            if (dirQueue_.empty() && activeWorkers_ == 0)
            {
                break;
            }

            if (!dirQueue_.empty())
            {
                dirQueueEntry = dirQueue_.back();
                dirQueue_.pop_back();
                ++activeWorkers_;
            }
        }

        if (!dirQueueEntry.path.empty())
        {
            this->ProcessDir(dirQueueEntry.path, dirQueueEntry.level);
            --activeWorkers_;
        }
        else
        {
            WaitForSingleObject(evt_, INFINITE);
        }
    }
}

void ParallelTraversalBfs::ProcessDir(const std::wstring& path, int level)
{
    std::wstring searchPath = path + L"\\*";
    WIN32_FIND_DATAW findFileData;

    HANDLE findHandle = FindFirstFileW(searchPath.c_str(), &findFileData);
    assert(findHandle != INVALID_HANDLE_VALUE);

    do {
        const std::wstring fileOrDir = findFileData.cFileName;
        if (fileOrDir != L"." && fileOrDir != L"..")
        {
            std::wstring fullPath = path + L"\\" + fileOrDir;

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                this->AddDirResult(fullPath, findFileData.dwFileAttributes, level + 1);
                this->EnqueueDir(fullPath, level + 1);
            }
            else
            {
                this->AddFileResult(fullPath, findFileData.dwFileAttributes);
            }
        }
    } while (FindNextFileW(findHandle, &findFileData) != 0);

    FindClose(findHandle);
    SetEvent(evt_);
}

void ParallelTraversalBfs::AddFileResult(const std::wstring& path, DWORD attributes)
{
    std::lock_guard<std::mutex> lock(filesMtx_);
    files_.push_back(FileEntry{ path, attributes });
}

void ParallelTraversalBfs::AddDirResult(const std::wstring& path, DWORD attributes, int level)
{
	std::lock_guard<std::mutex> lock(dirsByLevelMtx_);

    if (dirsByLevel_.find(level) == dirsByLevel_.end())
    {
        dirsByLevel_[level] = std::vector<FileEntry>();
    }

	dirsByLevel_[level].push_back(FileEntry{ path, attributes });
}

void ParallelTraversalBfs::EnqueueDir(const std::wstring& path, int level)
{
    std::lock_guard<std::mutex> lock(dirQueueMtx_);
    dirQueue_.push_back(DirQueueEntry{ path, level });
}
