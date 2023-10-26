# ece1387_a2



## design notes
structure

circuit has blocks
circuit has nets
nets have references to blocks
blocks have references to nets

do we need to access it both ways?

maybe its easiest to just have the circuit keep unordered sets of both cells and nets
and then blocks and nets just keep a list of labels
i'm not sure we need to traverse the graph in such a way that we transit through types eg net->cell->net->cell
we're not really finding shortest paths at this point - just spreading things out
so we can probably get by with this simpler approach
