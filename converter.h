#pragma once

#include <string>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_io/Io.h>
#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_io/io_handlers/OsmHandler.h>
#include <lanelet2_projection/UTM.h>
#include <memory>
#include <lanelet2_io/Projection.h>

using namespace lanelet;

class Converter {
public:
    Converter();
    ~Converter();

    bool shpToLanelet2(const std::string& shpFile, const std::string& outputFile);
    bool lanelet2ToShp(const std::string& laneletFile, const std::string& outputShp);

private:
    void initGDAL();
    GDALDataset* createShapefile(const std::string& filename, OGRwkbGeometryType geomType, lanelet::GPSPoint CenterGPS);
    double centerLat;
    double centerLon;
    int determineUTMZone(double longitude);
    void addFields(OGRLayer* layer);
    void cleanup();
};