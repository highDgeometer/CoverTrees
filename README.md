# covertrees

Implementation of covertree algorithm.
See
http://hunch.net/~jl/projects/cover_tree/cover_tree.html
for other implementations, papers, descriptions, experiments


To compile under LINUX (tested on CentOS 7)

g++ -Ofast -std=gnu++0x -DFLOAT main.cpp ThreadsWithCounter.C IDLList.C IDLListNode.C Vector.C Cover.C Point.C CoverNode.C EnlargeData.C Timer.C Distances.C TimeUtils.cpp -lpthread -o covertree
and for compiling for DOUBLE type data

g++ -Ofast -std=gnu++0x -DDOUBLE main.cpp ThreadsWithCounter.C IDLList.C IDLListNode.C Vector.C Cover.C Point.C CoverNode.C EnlargeData.C Timer.C Distances.C TimeUtils.cpp -lpthread -o covertree

