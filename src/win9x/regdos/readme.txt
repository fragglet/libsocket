Registry Code Readme
====================

    The registry access code is from The RegDos Group's Regdos library. Alfons
Hoogervorst <alfons@hoogervorst.demon.nl> is the maintainer of this. I think
RegDos can be obtained from http://www.hoogervorst.demon.nl.

    In incorporating the RegDos code into libsocket, I made a couple of
changes - these are described in the source code itself.

    The include files have been moved into ..\..\..\..\include\regdos. This
change required modification of the #include statements.

    Rich Dawe
    <rd5718@bristol.ac.uk>

1998-12-13: I've now pruned the code considerably. Hopefully this will lead
to a smaller library! It should also be easier to read the code. The pruning
means that the RegDos code will now only work with DJGPP. Also, there is now
only an include file in ..\..\..\..\include\lsck, called "registry.h".
