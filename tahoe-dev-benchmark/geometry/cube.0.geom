*version
1.0
*title
1 x 1 x 1 element cube patch
*dimensions
8   # number of nodes
3   # number of spatial dimensions
1   # number of element sets
# [ID] [nel] [nen]
1   1   8
7   # number of node sets
# [ID] [nnd]
1   4
2   4
3   4
4   4
5   4
6   4
7   8
0   # number of side sets
# end dimensions
*nodesets
*set
4   # number of nodes; x3 = 0
1  2  3  4
*set
4   # number of nodes; x3 = 1
5  6  7  8
*set
4   # number of nodes; x2 = 0
1  2  5  6
*set
4   # number of nodes; x1 = 0
1  4  5  8
*set
4   # number of nodes; x2 = 1
3  4  7  8
*set
4   # number of nodes; x1 = 1
2  3  6  7
*set
8   # number of nodes; all nodes
1  2  3  4  5  6  7  8 
# end node sets
*sidesets
*elements
*set
1
8
1
1 2 3 4 5 6 7 8
# end elements
*nodes
8
3
1  0.0  0.0  0.0
2  1.0  0.0  0.0
3  1.0  1.0  0.0
4  0.0  1.0  0.0

5  0.0  0.0  1.0
6  1.0  0.0  1.0
7  1.0  1.0  1.0
8  0.0  1.0  1.0
