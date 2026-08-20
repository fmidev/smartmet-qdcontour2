# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What is qdcontour2

A command-line tool that reads a control file (conf script) and generates weather map images by contouring QueryData and rendering shapefiles. Successor to `smartmet-qdcontour`, built on the `imagine2` (Cairo-based) rendering library. Produces PNG, JPEG, PDF, PS, EPS, and SVG output.

## Build commands

```bash
make                  # Build the qdcontour2 binary
make test             # Run all visual regression tests
make format           # Run clang-format on all source
make clean            # Clean build artifacts
make rpm              # Build RPM package
make install          # Install binary to $(bindir)
```

Build uses the shared SmartMet build config via `makefile.inc`. Key dependencies: `newbase`, `imagine2`, `gis`, `trax`, `macgyver`, cairomm, GEOS, GDAL.

## Running a single test

Tests are visual regression tests that compare rendered PNG output against reference images using `smartpngdiff`:

```bash
# Run one named test (test name = conf file basename in test/conf/)
make -C test _check TEST=trivial
make -C test _check TEST=contourfill

# The test runs: qdcontour2 -f conf/<TEST>.conf
# Then compares results/<TEST>*.png against results_ok/<TEST>*.png
```

The test Makefile uses `../qdcontour2` (locally built binary) if present, otherwise the system-installed `qdcontour2`.

## Source layout

- `main/qdcontour2.cpp` — single-file main program (~5800 lines). Parses conf scripts and orchestrates all rendering. This is where most command handling lives.
- `include/` — headers, `source/` — implementation files for supporting classes.
- `include/Globals.h` — the `Globals` struct holds all runtime state (a static global instance). Configuration commands in the conf script set fields on this struct.
- `test/conf/` — test configuration scripts. `test/results_ok/` — reference images. `test/data/` — test QueryData/shapefiles.
- `docs/qdcontour.txt` — full Doxygen-format command reference for the conf scripting language.

## Architecture

The tool is driven by a **conf script** (plain text command file). The main loop in `qdcontour2.cpp` reads the script line by line, interprets commands that set global state (`Globals` struct), and executes rendering when it hits a `draw` command.

Key classes:
- **`ContourSpec`** — rendering specification for one parameter (contour fills/lines/patterns/symbols/labels/fonts)
- **`ContourCalculator`** — computes and caches contour paths (isolines/isobands) from grid data using the Trax library
- **`LazyQueryData`** — lazy-loading wrapper around `NFmiQueryData` (FMI's native gridded data format)
- **`ShapeSpec`** — specification for rendering ESRI shapefiles as map backgrounds

The rendering pipeline: conf script → set globals/specs → `draw contours` command → for each timestep: load data, compute contours via `ContourCalculator`, render shapes/contours/arrows/labels onto a Cairo surface, write image file.

## CI

CircleCI builds and tests RPMs on RHEL 8 and RHEL 10 using `fmidev/smartmet-cibase` Docker images. The `ci-build` tool handles dependency installation, RPM building, and test execution.
