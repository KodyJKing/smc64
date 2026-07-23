---
name: decomp
description: Conventions for writing hand-authored C++ decompilations of engine functions. Use when the user asks to decompile, hand-decompile, or clean up a Ghidra-decompiled function into `engine/decomp/impl/`, or when adding an entry to `decomp_defs.hpp`.
---

# Decomp

Conventions for producing hand-authored C++ decompilations of engine
functions, distinct from Ghidra's raw decompiler output.

## Purpose

Unless otherwise stated, the goal of a decomp is reverse engineering: improve
the clarity of a piece of code and verify our understanding of it. At times
the goal will instead be to extend engine functionality.

## Scope Selection

When asked to decompile a function, do not naively decompile everything it
touches. Only pull in dependencies that are:

- Pertinent to the user's research interest.
- Large enough that they aren't already clear from context.
- Messy enough in Ghidra's output that a hand-written version would improve
  clarity.

## File Layout

- Decomp implementations are declared in `decomp_defs.hpp` (an X-Macro).
- Each declared function is implemented in `engine/decomp/impl/<function_name>.hpp`.
- Further conventions (one file per function, exact Ghidra naming, struct
  promotion rules) are detailed in `engine/decomp/impl/README.md` — consult it
  before adding new files or types.
