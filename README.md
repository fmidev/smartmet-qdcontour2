# smartmet-qdcontour2

Part of [SmartMet Server](https://github.com/fmidev/smartmet-server). See the [SmartMet Server documentation](https://github.com/fmidev/smartmet-server) for an overview of the ecosystem.

## Overview

`qdcontour2` is an updated QueryData contouring and map rendering tool, succeeding [smartmet-qdcontour](https://github.com/fmidev/smartmet-qdcontour). It reads a control file and generates weather map images by rendering contours from QueryData and map features from shapefiles.

Note: For server-side rendering, the [WMS plugin](https://github.com/fmidev/smartmet-plugin-wms) is the preferred approach.

## Features

- Contour rendering from QueryData (isolines, isobands)
- Shapefile-based map background rendering
- Weather symbol rendering
- Configurable fonts, colors, and styling
- Raster image output (PNG, JPEG)

## Documentation

Detailed configuration reference is available in [docs/qdcontour.txt](docs/qdcontour.txt).

## Dependencies

- [smartmet-library-newbase](https://github.com/fmidev/smartmet-library-newbase) — QueryData format
- [smartmet-library-gis](https://github.com/fmidev/smartmet-library-gis) — GIS operations
- [smartmet-library-imagine2](https://github.com/fmidev/smartmet-library-imagine2) — image rendering
- [smartmet-library-giza](https://github.com/fmidev/smartmet-library-giza) — color mapping

## License

MIT — see [LICENSE](LICENSE)

## Contributing

Bug reports and pull requests are welcome on [GitHub](../../issues).
