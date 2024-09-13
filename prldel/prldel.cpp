// TODO:
// - Error handling
// - Tests/Benchmark

#include <iostream>
#include <thread>
#include <cassert>

#include "ParallelTraversalBfs.h"
#include "ParallelDelete.h"
#include "Timer.h"

void PrintUsage()
{
    wprintf(L"Usage: prldel.exe --path <directory_path> --num-workers <number_of_worker_threads>\n");
}

int wmain(int argc, wchar_t* argv[])
{
    std::wstring path;
    int nworkers = 0;
    bool suppressErrors = false;

    for (int i = 1; i < argc; ++i)
    {
        std::wstring arg = argv[i];

        if (arg == L"--path" && i + 1 < argc)
        {
            path = argv[++i];
        }
        else if (arg == L"--num-workers" && i + 1 < argc)
        {
            nworkers = std::stoi(argv[++i]);
        }
        else if (arg == L"--suppress-errors")
        {
            suppressErrors = true;
        }
        else
        {
            PrintUsage();
            return 1;
        }
    }

    if (path.empty())
    {
        wprintf(L"Error: --path argument is required.\n");
        PrintUsage();
        return 1;
    }

    if (nworkers <= 0)
    {
        wprintf(L"Error: --num-workers argument is required.\n");
        PrintUsage();
        return 1;
    }

    wprintf(L"\n");
    
    ParallelTraversalBfs parallelTraversalBfs;
    Timer timer;

    auto traverseResult =
        parallelTraversalBfs.Traverse(path, nworkers);

    ParallelDelete parallelDelete;
    auto deleteResult = parallelDelete.Delete(traverseResult, nworkers, suppressErrors);

    wprintf(L"---\nFile(s) deleted: %d, ", deleteResult.filesDeleted);
    wprintf(L"dir(s) deleted: %d\n", deleteResult.dirsDeleted);
    wprintf(L"File(s) failed: %d, ", deleteResult.filesFailed);
    wprintf(L"dir(s) failed: %d\n\n", deleteResult.dirsFailed);

    return 0;
}
