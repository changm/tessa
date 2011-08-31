========================================================================
    STATIC LIBRARY : TessaVisitors Project Overview
========================================================================

This defines the TessaVisitor interface. It also holds other random TESSA visitors
that aren't either:

1) An optimization pass
2) A LIR generation Pass

For example, the TESSA interpreter, and consistency checkers should go in here.
Visitors should be limited to do only ONE thing per pass.