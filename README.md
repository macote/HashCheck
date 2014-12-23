HashCheck for Windows
=====================

HashCheck is a Windows application that creates and verifies file checksums.


How to build
------------

This project was created with:
- Eclipse Luna with CDT
- MinGW (GCC 4.8.1)


Usage
-----

Launch the executable to create a checksum file if it does not already exists.

Launch the executable to verify file integrity against a checksum file if it exists.
 

License
-------

Hashing code found on the Internet is in the public domain:

MD5: Colin Plumb (1993) / John Walker (2003)

SHA1: Steve Reid (199?) / ... / Ralph Giles (2002)

FileStream structure and code was inspired from Microsoft C# implementation.


FAQ
---

### Why MinGW?

I wanted to have an executable having the least runtime dependencies possible.

### Why FileStream?

MinGW's fstream can't open WCHAR filenames and I was not interested in changing fstream's implementation.

