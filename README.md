**CS 525: Programming Assignment I — Storage Manager 
Contributer : Deepesh Kumar Appar Senthilkumar 
A20644067**


This module and assignment implements a paged file storage manager designed to facilitate block-level file management typical in database systems. It simplifies reading and writing fixed-size pages (blocks) from/to disk files into memory buffers and vice versa, enabling efficient data storage and retrieval operations.

The storage manager operates using a fixed constant page size, maintaining essential metadata for each open file, including the total number of pages, the current page position, the file name, and internal file pointers to manage access.

**This module provides and explains:**

-File creation, opening, closing, and deletion (destroy) functions.

-Reading and writing  pages and sequential blocks.

-Appending empty pages to over capacity files.

-Managing and ensuring minimum file capacity.

-Custom error handling for robust operation.

-This design abstracts low-level file operations providing a consistent interface for page-based data management.

**Programming Language**
Implemented entirely in C, leveraging standard libraries such as <stdio.h>, <stdlib.h>, and system headers for file handling and memory management.

System Requirements & Dependencies
C compiler (e.g., GCC) capable of compiling code.

Standard POSIX-compatible OS for file operations (Linux, macOS, Unix).

Installing MinGW-w64 and mingw32-make on Windows : 

Download Installer

Go to the official MinGW-w64 site or SourceForge:
https://sourceforge.net/projects/mingw-w64/

Click "Download" to get the installer (mingw-w64-install.exe).

Run Installer

Double-click the installer and follow prompts.

When asked:

Select architecture: x86_64 (for 64-bit Windows).

Threads: posix

Exception: seh

Build revision: Latest

Choose installation directory (e.g., C:\mingw-w64).

Complete the installation.

Add MinGW-w64 to PATH

Open Control Panel → System → Advanced → Environment Variables.

Under "System Variables", find and edit Path.

Add the path to the bin folder, e.g.,
C:\mingw-w64\mingw64\bin

Click "OK" to save.

Verify Installation

Open Command Prompt and check:

gcc --version
mingw32-make --version
Both commands should show installed versions.



Compilation and Execution Instructions
On Windows (using MinGW and mingw32-make)
Open the Command Prompt and navigate to your project directory (e.g., cd path\to\storemanager).

Clean previous executables and object files:
Run:


mingw32-make clean
This command removes the compiled executable (test_storemanager.exe) if it exists.

It uses Windows-specific del for cleaning, as configured in your Makefile.

Build the project:
Run:


mingw32-make -B
The -B flag forces a rebuild of all targets, ensuring every file is freshly compiled.

This compiles test_assign1_1.c, storage_mgr.c, and dberror.c, and produces the test_storemanager.exe executable.

Run the test executable:
After a successful build, execute:


test_storemanager.exe
This runs all the bundled test cases to verify your storage manager’s behavior.

Repeat cleaning and rebuilding as needed:
You can use mingw32-make clean before building again to ensure a fresh start.

Additional Notes
Use only the Windows Command Prompt or a MinGW shell.

If you see errors like 'rm' is not recognized as an internal or external command, it means you’re on Windows—your Makefile already accommodates this by using del instead of Unix rm.

Always use mingw32-make instead of make when using MinGW on Windows.

You can place this expanded section directly in your README under "Compilation and Execution Instructions" for clear guidance on building and running your storage manager using MinGW and Windows Command Prompt.

i need to give them similar process in mac also so provide me that too

i need to give them similar process in mac also so provide me that too
Here’s a step-by-step process for compiling, cleaning, and running your project on a Mac (or any UNIX-like system with make):

Compilation and Execution Instructions : 

On Mac OS (using Terminal and Make)
Open Terminal

Use Spotlight (Cmd + Space, then type "Terminal") or open the Terminal app from Applications > Utilities.

Navigate to your project directory

text
cd /path/to/storemanager
Clean previous builds
Run:

mingw32-make clean
This command will remove any previous executable (like test_storemanager), ensuring a clean build environment.

Build the project
Run:

mingw32-make -B

This generates the test_storemanager executable. After this execute dir command to verify whether the given exe file is created.

Run the test executable
Run:

test_storemanager.exe
This will execute all tests validating your storage manager’s functionality.

Repeat cleaning and rebuilding as needed

Use make clean before make -B any time you need a fresh compilation.

IN OS : 
 
Open Terminal

Use Spotlight (Cmd + Space, then type "Terminal") or open the Terminal app from Applications > Utilities.

2. cd /path/to/storemanager

3. make clean

4../test_storemanager.exe

Storage Manager – Functionalities Overview
1. Setup and File Handling

initStorageManager(void)
Prepares the storage manager for use. It resets internal states and ensures file pointers start clean before any operation.

createPageFile(char *fileName)
Creates a brand-new page file on disk. The file always starts with a single empty page filled with zero bytes.
The function returns success or error codes depending on whether the file could be created.

openPageFile(char *fileName, SM_FileHandle *fHandle)
Opens an existing page file for both reading and writing. It also fills the file handle with metadata such as:

the file’s name,

the number of pages (computed from file size / PAGE_SIZE),

and sets the current page position to zero.
If the file does not exist, it gracefully reports the error.

closePageFile(SM_FileHandle *fHandle)
Safely closes the file, resets internal structures, and ensures all resources are properly released.

destroyPageFile(char *fileName)
Permanently removes a page file from disk, but only after confirming that the file exists.

2. Reading from Files

readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
Loads the content of the specified page into memory. It checks page boundaries and file validity before performing the read.

getBlockPos(SM_FileHandle *fHandle)
Returns the current page position, which helps track where you are in the file.

Convenience Read Functions:

readFirstBlock → reads the first page (page 0).

readPreviousBlock → steps back one page; fails if you’re already at the start.

readCurrentBlock → re-reads the current page.

readNextBlock → moves forward one page.

readLastBlock → jumps directly to the last page.

All these functions rely on the core readBlock logic and add appropriate error handling for edge cases.

3. Writing to Files

writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
Writes data from memory into the specified page of the file. It ensures the page exists and updates the file metadata accordingly.

writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
Similar to writeBlock, but always writes to the page where the file handle is currently pointing.

appendEmptyBlock(SM_FileHandle *fHandle)
Expands the file by adding one more blank page filled with zeros. Updates the total page count in the file handle.

ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)
Makes sure the file has at least the requested number of pages. If not, it appends enough empty blocks to reach that size.

4. Helper Utilities

Memory allocation for new pages is done safely with calloc.

File size and page counts are determined using system calls.

Page indices are validated before read/write operations.

File seeking uses page numbers as references to jump precisely within the file.

File position is updated after every successful read or write.

Error Handling

Uses custom return codes like RC_FILE_NOT_FOUND, RC_WRITE_FAILED, and RC_READ_NON_EXISTING_PAGE.

Every function checks for invalid pointers, bad page numbers, or broken file access before proceeding.

This consistent error handling makes debugging and integration easier.

Memory Management

Allocates memory only when necessary (e.g., for blank pages).

Frees allocated memory to avoid leaks.

Design Principles: 

Clear separation between public APIs and internal helpers.

Minimal use of global variables to reduce side effects.

Readable coding style with modular functions.

Designed to align with database systems coursework while simulating realistic DBMS storage management.
