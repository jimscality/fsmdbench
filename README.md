File system meta data benchmark

This benchmark focuses on file system operations instead of throughput.
In fact, there is no observable throughput in the benchmark.

The benchmark creates a tree of directories and a number of files in these directories.
For example, if the levels of the tree is 2 and directories per level is 2 and files in
a directory is 3, the benchmark generates a tree like

target directory |
                 |- d0_c0_i0 |- d1_c0_i0
                 |           |- d1_c0_i1
                 |           |- f1_c0_i0 
                 |           |- f1_c0_i1 
                 |           |- f1_c0_i2 
                 |- d0_c0_i1 |- d1_c0_i0
                 |           |- d1_c0_i1
                 |           |- f1_c0_i0 
                 |           |- f1_c0_i1 
                 |           |- f1_c0_i2 
                 |- f0_c0_i0 
                 |- f0_c0_i1 
                 |- f0_c0_i2 

The benchmark supports multiple clients run from different mountpoints on the same target directory.
If the number of clients is 2, the tree is like

target directory |
                 |- d0_c0_i0 |- d1_c0_i0
                 |           |- d1_c0_i1
                 |           |- d1_c1_i0
                 |           |- d1_c1_i1
                 |           |- f1_c0_i0 
                 |           |- f1_c0_i1 
                 |           |- f1_c0_i2 
                 |           |- f1_c1_i0 
                 |           |- f1_c1_i1 
                 |           |- f1_c1_i2 
                 |- d0_c0_i1 |- d1_c0_i0
                 |           |- d1_c0_i1
                 |           |- d1_c1_i0
                 |           |- d1_c1_i1
                 |           |- f1_c0_i0 
                 |           |- f1_c0_i1 
                 |           |- f1_c0_i2 
                 |           |- f1_c1_i0 
                 |           |- f1_c1_i1 
                 |           |- f1_c1_i2 
                 |- d0_c1_i0 |- d1_c0_i0
                 |           |- d1_c0_i1
                 |           |- d1_c1_i0
                 |           |- d1_c1_i1
                 |           |- f1_c0_i0 
                 |           |- f1_c0_i1 
                 |           |- f1_c0_i2 
                 |           |- f1_c1_i0 
                 |           |- f1_c1_i1 
                 |           |- f1_c1_i2 
                 |- d0_c1_i1 |- d1_c0_i0
                 |           |- d1_c0_i1
                 |           |- d1_c1_i0
                 |           |- d1_c1_i1
                 |           |- f1_c0_i0 
                 |           |- f1_c0_i1 
                 |           |- f1_c0_i2 
                 |           |- f1_c1_i0 
                 |           |- f1_c1_i1 
                 |           |- f1_c1_i2 
                 |- f0_c0_i0 
                 |- f0_c0_i1 
                 |- f0_c0_i2 
                 |- f0_c1_i0 
                 |- f0_c1_i1 
                 |- f0_c1_i2 

Note that file system operations from different clients are in the same parent directories.
The program ensures that clients from different servers operate collaboratively so that
parent directories always exist.

Usage examples

mkdir -p /tmp/bench
fsmdbench -t /tmp/bench

fsmdbench -t /tmp/bench -c 3 -a 192.168.1.10,192.168.1.11 -l 10 -d 5 -f 10
