# FileSystem
Design and Implementation of a Simulated File System
## Design Content
1. Design a file system for 10 users. Each user can save up to 10 files, and multiple files can be opened by a user simultaneously during runtime.
2. The program adopts a two-level file directory system. It includes a Master File Directory (MFD) and User File Directories (UFD). Additionally, file pointers are set for open files.
3. For the sake of simplicity in implementation, file read and write operations are simplified. When executing read and write commands, only the read and write pointers are modified, and actual read and write operations are not performed.
4. The implemented basic functionalities mainly include: Change Directory (CD), Create Directory (MD), Display Directory (DIR), Remove Directory (RD), Open All Files (openall), Open a Single File (open), Create a File (create), Delete a File (delete), Write to a File (write), Read from a File (read), Modify File Protection Code (change), and Exit (exit).
