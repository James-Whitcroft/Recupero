# Recupero
Ext2 file recovery program

# Compiling
To compile cd into src and run make

# Running
The user can run Recupero in 2 ways

./recuper /path/to/image/or/device

This will give the user an interactive prompt
    or
./recupero \<command\> /path/to/image/or/device

This will run the command right away, useful for scripting

# Limitations
Due to time constrictions and time wasting problems we have limited tested functionality
to ext2 file systems with block size 1024 and an inode size of 128. 

Another restriction is that of program can only currently recover file's direct block
only! This limits file size of recoverable files too (block size * 12). 

# Directories
The Doc/ directory contains all of our documentation.
the src/ directory contains all of our source code.
the test/ directory contains all of our automated tests that we didn't get too.
the xmpl/ directory contains all of our experimental code for playing around in.
