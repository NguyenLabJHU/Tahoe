*version
1.0
*title
2 x 2 x 2 elements - node at center perturbed to break symmetry
*dimensions
27  # number of nodes
3   # number of spatial dimensions
1   # number of element sets
# [ID] [nel] [nen]
1   8   8
6   # number of node sets
# [ID] [nnd]
1   9
2   9
3   9
4   9
5   9
6   9
0   # number of side sets
# end dimensions
*nodesets
*set
9   # number of nodes
1  2  3  
4  5  6
7  8  9
*set
9   # number of nodes
19 20 21
22 23 24
25 26 27
*set
9   # number of nodes
 1  2  3
10 11 12
19 20 21
*set
9   # number of nodes
 3  6  9
12 15 18
21 24 27
*set
9   # number of nodes
 7  8  9
16 17 18
25 26 27
*set
9   # number of nodes
 1  4  7
10 13 16
19 22 25
# end node sets
*sidesets
*elements
*set
cube.1.elem
# end elements
*nodes
cube.1a.node
