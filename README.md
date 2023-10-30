# ece1387_a2



## design notes
structure

* circuit has blocks
* circuit has nets
* nets have references to blocks
* blocks have references to nets

* do we need to access it both ways?

* maybe its easiest to just have the circuit keep unordered sets of both cells and nets, 
and then blocks and nets just keep a list of labels

* i'm not sure we need to traverse the graph in such a way that we transit through types eg net->cell->net->cell
* we're not really finding shortest paths at this point - just spreading things out.
so we can probably get by with this simpler approach


next steps (oct 27 2023)
- easier: calculate the clique weight per net (ie. to figure out wi,j) 
- less easy: extrapolate the relation of w_i,j and build the correct matrix

Px = wij(xi-xj)^2

2wij(xi-xj)


so C is the same as b for some reason

the elements of Q are as such:

qi,i (diagonal) is the sum of all weights connected to that node
qi,j is -wi,j

Ci = wiz\* xz if xi connects to fixed cell xz


so, things to do next,
-function to sum all weights connecting to a block
-populate q(x)
-function to determine if a given cell connects to a fixed cell
-populate C

