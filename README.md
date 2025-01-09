## ShpToLanelet2
Supports shp file conversion in wgs84 geographical coordinate system and shp file conversion in projected coordinate system.
## Lanelet2ToShp
You need to manually enter the approximate origin.After reading the osm file, the program will automatically calculate the center point based on the node latitude and longitude as the origin and load the osm data again.
The transformed shp is the UTM projection coordinate system.
## Dependencies
lanelet2、boost、eigen、geographiclib、pugixml、gdal.
Some dependencies are required for compiling lanelet2.
## Conclusion
The data structure of shp is not suitable for conversion with lanelet2. A shp file can only have one data structure, such as dots, lines, and surfaces. At the same time, it is difficult for shp to express the relation of the lanelet2 data structure.
