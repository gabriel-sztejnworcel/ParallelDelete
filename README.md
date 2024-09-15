# ParallelDelete
#### Delete Windows directories in parallel for slow SMB connections
Deleting a complex directory structure in Windows can take a very long time, especially over a slow SMB connection. This tool provides the ability to delete directories in parallel using multiple worker threads, which could be almost 50 times faster. For example, deleting a standard user profile over SMB with latency of 60ms took 16 minutes using built-in Windows commands, while using this tool it only took 20 seconds.
### How does it work?
The process is divided to 2 steps:
1. Directory traveral: Perform a parallel BFS traversal and build a list of all the subdirectories in each level of the tree, and a list of non-directory files.
2. Delete: First, delete the files from the file list in parallel by dividing the list to n workers and deleting each sublist in parallel. Then delete directories in each level in the same way, in reverse order (last level first).
### Usage
```
Usage: ParallelDelete.exe --path <directory_path> --num-workers <number_of_worker_threads>
```
