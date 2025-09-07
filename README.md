<h1 align="center">  CS 525: PROGRAMMING ASSIGNMENT I: STORAGE MANAGER  GROUP:27
</h1>

# Storage Manager Module Description:
A module has been created to simplify the retrieval of blocks from a disk file into memory and the transfer of blocks from memory to a disk file. This storage manager operates based on fixed-size pages, referred to as PAGE SIZE. Alongside essential read and write operations, it provides functionalities for creating, opening, and closing files. When a file is opened, the storage manager retains crucial information, including the total number of pages, the current page position for both reading and writing, the file name, and either a POSIX file descriptor or a FILE pointer. Additionally, the test_assign1_1.c file has been enhanced with custom test cases to validate the execution of additional predefined methods that were not previously utilized.

### Programming Language Used : c


# Contributors

Sumanth Kotha - A20550647 - 25% Contributed

SaiKartheek Goli - A20546631 - 25% Contributed

Bachu Pavan Venkata Sai - A20551759 - 25% Contributed

Meghana Mahamkali - A20564182 - 25% Contributed


## Steps to Run Code

Step 1: Go to the "storemanager" folder in the Step 1 location. 

Step 2: Type "make clean" to delete old, compiled files.

Step 3: Type "make" to run the Makefile using the make command.

Step 4: Then, run “./test_storemanager" to execute all test cases.

Step 5: Run "make clean" to clean up the executable files created for the previous instance and repeat steps 3 and 4 to rerun the complete assignment again.

<p>
 
</p>
<br>

## File Related Methods:

### Section 1: Manipulating Page File
-----
In this section we will be focusing on the operations like creating, opening, closing and destroying page files,

●	void initStorageManager ()
- Purpose: to Initialize the Storage Manager.

- Description: Sets the global file pointer `fgrp27` to `NULL`. This function is tasked with initializing the global variable utilized by the Storage Manager.


●	RC createPageFile (char *_fileName)
- Purpose: This function generates a new page file using the provided `_fileName`. Its objective is to create fresh page files and initialize them with empty pages.

- Description: 
  - Attempts to generate the file using `fopen` with write mode (`"w+"`).
  - If the file creation fails (fgrp27 is `NULL`), and returns `RC_FILE_NOT_FOUND`.
  - If the file is created successfully, it fills the page with null bytes, writes it to the file using `calloc`, then checks for the pages in file if its failed returns RC_WRITE_FAILED.
  - Else close the file using `fclose` and returns `RC_OK`.


●	RC openPageFile (char *fileName, SM_FileHandle *fileHandle)

- Purpose: Opens an existing page file and initializes the file handle.

- Description: 
  - Attempts to open the file using `fopen` with read mode (`"r"`).
  - Check the filepointer if it is NULL, returns `RC_FILE_NOT_FOUND`.
  - If the file is found, it initializes the provided `fileHandle` structure with file information.
  - It computes the total number of pages within the file by navigating to the end of the file and dividing the file size by the constant `PAGE_SIZE`, which is set to 4096.
  - Changes the file pointer position to the start of the file and sets the current page position to 0 and returns `RC_OK`.


●	RC closePageFile (SM_FileHandle *fileHandle)

- Purpose: Closes an open page file associated with the provided `SM_FileHandle` structure.

- Description: 
  - Internally calls closeFilePointer() to close the file pointer if it is initialized.


●	RC destroyPageFile (char *fileName)

- Purpose: Deletes an existing page file with the specified `fileName`.

- Description: 
  - Check if the file exists before attempting to open it
  - If file is accessed returns RC_FILE_NOT_FOUND
  - Else destroys file using remove() and returns RC_OK.

### Section 2: Reading Files
-----
In this section we will be focusing on the operations like readBlock, getBlockPos , readFirstBlock, readLastBlock,readPreviousBlock, readCurrentBlock and readNextBlock, however the code description has been included in each transition unit.

●	RC readBlock(int pageNumber, SM_FileHandle *fileHandle, SM_PageHandle pageMemory)

Purpose: Reads the specified page number and stores it into `memPage`.

Description:
- First we will be validating the `pageNumber`, in not valid pageNumber then set return code to `RC_FILE_NOT_FOUND` if `mgmtInfo` is equal to `NULL` else `RC_READ_NON_EXISTING_PAGE`.
- Attempts to generate the file using `fopen` with read mode (`"r"`).
- Checks for the file pointer if it is NULL returns RC_FILE_NOT_FOUND.
- Once the above two validations are false, we use fseek () move the cursor to the beginning of the page and then move it to pageNumber * PAGE_SIZE which would give us the starting point for the page. For the given page number. 


●	int getBlockPos(SM_FileHandle *fileHandle)

Purpose: Return Current page

Description: This function returns the current page position of the file from the fileHandle->curPagePos.


●	RC readFirstBlock(SM_FileHandle *fileHandle, SM_PageHandle memoryPage)

Purpose: This method reads the first page in the file.

Description: This sets page number as `0` and returns `readBlock(pageNumber, file_Handle, memPage)`.


● RC readPreviousBlock(SM_FileHandle *fileHandle, SM_PageHandle memoryPage)

Purpose: This method reads the previous page in the file.

Description: Checks for the file handle current position if it's 0 then returns RC_READ_AT_BEGINNING, else Calculating current page number by dividing page size by current page position and returns `readBlock(currentPageNumber, fHandle, memPage)`.


● RC readCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle memoryPage)

Purpose: This method reads the current page in the file.

Description: Checks for fHandle->curPagePos / PAGE_SIZE if it's not `0` returns readBlock(fHandle->curPagePos / PAGE_SIZE, fHandle, memPage) else returns `RC_ERROR`.


● RC readNextBlock(SM_FileHandle *fileHandle, SM_PageHandle memoryPage)

Purpose: This method reads the next page in the file.

Description: Calculates the current page number and adds +1 to that and returns `readBlock(*currentPageNumberPtr, fHandle, memPage)`.


● RC readLastBlock(SM_FileHandle *fileHandle, SM_PageHandle memoryPage)

Purpose: This method reads the last page in the file.

Description: This sets page number as file_Handle->totalNumPages - 1 and returns `readBlock(file_Handle->totalNumPages - 1, file_Handle, memPage)`.


### Section 3: Writing Files
----
In this section we will be focusing on the operations like writeBlock(), appendEmptyBlock(), ensureCapacity()

● RC writeBlock(int pageNumber, SM_FileHandle *fileHandle, SM_PageHandle memoryPage)

Purpose: Writes to a specified block in the file.

Description: 
- Checks the validity of the page in question calls `checkValidPageNum()` internally.
- If valid, opens the file in read-write mode
- Checks for file pointer if it's `NULL` returns `RC_FILE_NOT_FOUND`.
- Else uses a loop to write one byte at a time using pointers,
- Closes the file using `fclose()` and returns `RC_OK`.

●  RC writeCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle memoryPage)

Purpose: Writes to the current block.

Description: 
- This will set page number as current page(file_Handle->curPagePos).
- Uses a loop to write one byte at a time using pointers,starting from the current position
- Update the file position and total number of pages after successful writing and returns `RC_OK`.


●  RC appendEmptyBlock(SM_FileHandle *fileHandle)

Purpose: Appends an empty block at the end of the file.

Description: 
- Allocates memory for an empty block using `calloc`, setting its size to `PAGE_SIZE` times the size of a character.
- Utilizes a `for` loop to write null bytes (0) one at a time to the file, effectively appending an empty block.
- Checks if writing a null byte fails (returns `EOF`), in which case it frees the allocated memory and returns an error code (`RC_WRITE_FAILED`).
- Increments the `totalNumPages` attribute of the file handle to reflect the addition of the appended empty block.
- Frees the allocated memory for the empty block and returns `RC_OK`.


●  RC ensureCapacity(int numberOfPages, SM_FileHandle *fileHandle)

Purpose: Ensures that the file has at least `numberOfPages` available..

Description: 
- First will check if the file is present, If the file is found,
- Checks if the numberOfPages is greater than `currentNumPages`(current total number of pages),
- If true, adds empty pages until numberOfPages equals `totalNumPages` and calls ensureCapacity() recursively untill file has enough capacity
- Returns `RC_OK` once the method is successful.


## Memory Leak Check:
After Running the code we need to check for the possible memory leaks, There are `no memory` leaks in the code post execution.

### Below are commands used to check.

* MAC OS:  leaks -atExit -- ./test_storemanager

* LINUX OS: valgrind --leak-check=full --track-origins=yes ./test_storemanager
