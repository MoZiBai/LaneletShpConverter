#include "converter.h"
#include <iostream>

Converter::Converter() {
    initGDAL();
}

Converter::~Converter() {
    cleanup();
}

void Converter::initGDAL() {
    GDALAllRegister();
}

void Converter::cleanup() {
}


/* test1
bool Converter::shpToLanelet2(const std::string& shpFile, const std::string& outputFile) {

    std::string extension = outputFile.substr(outputFile.find_last_of(".") + 1);
    if (extension != "osm") {
        std::cerr << "Error: Output file must have .osm extension" << std::endl;
        return false;
    }


    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(shpFile.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (!poDS) {
        std::cerr << "Failed to open shapefile." << std::endl;
        return false;
    }

    OGRLayer* poLayer = poDS->GetLayer(0);
    if (!poLayer) {
        std::cerr << "Failed to get layer from shapefile" << std::endl;
        GDALClose(poDS);
        return false;
    }


    OGRSpatialReference* sourceSRS = poLayer->GetSpatialRef();
    if (!sourceSRS) {
        std::cerr << "Warning: Source shapefile does not have spatial reference information." << std::endl;
        std::cerr << "Assuming WGS84..." << std::endl;
        sourceSRS = new OGRSpatialReference();
        sourceSRS->SetWellKnownGeogCS("WGS84");
    }

    OGREnvelope extent;
    if (poLayer->GetExtent(&extent) != OGRERR_NONE) {
        std::cerr << "Warning: Could not determine layer extent." << std::endl;
        return false;
    }

    double originLat = (extent.MaxY + extent.MinY) / 2.0;
    double originLon = (extent.MaxX + extent.MinX) / 2.0;

    // ����Lanelet2��ͼ
    lanelet::LaneletMapPtr map = std::make_shared<lanelet::LaneletMap>();

    try {
        // ����ԭ���UTMͶӰ
        lanelet::GPSPoint gps_point;
        gps_point.lat = originLat;
        gps_point.lon = originLon;
        lanelet::Origin origin(gps_point);
        lanelet::projection::UtmProjector projector(origin);

        poLayer->ResetReading();
        OGRFeature* poFeature;
        while ((poFeature = poLayer->GetNextFeature()) != NULL) {
            OGRGeometry* poGeometry = poFeature->GetGeometryRef();
            if (poGeometry != NULL && wkbFlatten(poGeometry->getGeometryType()) == wkbLineString) {
                OGRLineString* poLine = (OGRLineString*)poGeometry;

                std::vector<lanelet::Point3d> points;
                for (int i = 0; i < poLine->getNumPoints(); i++) {

                    double lon = poLine->getX(i);
                    double lat = poLine->getY(i);
                    double z = poLine->getZ(i);

                    lanelet::GPSPoint gps_pt;
                    gps_pt.lat = lat;
                    gps_pt.lon = lon;
                    lanelet::BasicPoint3d utm_pt = projector.forward(gps_pt);

                    // ����Lanelet2��
                    points.push_back(lanelet::Point3d(
                        lanelet::utils::getId(),
                        utm_pt.x(),    // UTM��������
                        utm_pt.y(),    // UTM��������
                        z              // ����ԭʼ�߳�
                    ));
                }

                lanelet::LineString3d ls(lanelet::utils::getId(), points);
                map->add(ls);
            }
            OGRFeature::DestroyFeature(poFeature);
        }

        // ����Lanelet2��ͼ
        lanelet::write(outputFile, *map, projector);

    }
    catch (const lanelet::UnsupportedExtensionError& e) {
        std::cerr << "Error saving file: " << e.what() << std::endl;
        GDALClose(poDS);
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        GDALClose(poDS);
        return false;
    }

    GDALClose(poDS);
    return true;
}
*/


/* test2
bool Converter::shpToLanelet2(const std::string& shpFile, const std::string& outputFile) {
    GDALDataset* poDS = nullptr;
    try {
        poDS = (GDALDataset*)GDALOpenEx(shpFile.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
        if (!poDS) {
            std::cerr << "Failed to open shapefile." << std::endl;
            return false;
        }

        OGRLayer* poLayer = poDS->GetLayer(0);
        if (!poLayer) {
            std::cerr << "Failed to get layer from shapefile." << std::endl;
            GDALClose(poDS);
            return false;
        }

        // ��鼸������
        OGRFeature* poFeature = poLayer->GetNextFeature();
        if (!poFeature) {
            std::cerr << "No features found in shapefile." << std::endl;
            GDALClose(poDS);
            return false;
        }
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        OGRwkbGeometryType geomType = wkbFlatten(poGeometry->getGeometryType());
        OGRFeature::DestroyFeature(poFeature);
        poLayer->ResetReading();

        // ��ȡ�ռ�ο�ϵͳ
        OGRSpatialReference* poSRS = poLayer->GetSpatialRef();
        if (!poSRS) {
            std::cerr << "Error: No coordinate system found in shapefile." << std::endl;
            std::cerr << "Please set the coordinate system before conversion." << std::endl;
            GDALClose(poDS);
            return false;
        }

        bool isGeographic = poSRS->IsGeographic();
        bool isProjected = poSRS->IsProjected();
        const char* projName = poSRS->GetAttrValue("PROJECTION");

        std::cout << "Detected coordinate system: " << (isGeographic ? "Geographic" :
            (isProjected ? "Projected" : "Unknown")) << std::endl;
        if (isProjected && projName) {
            std::cout << "Projection type: " << projName << std::endl;
        }

        // ��ȡ���ݷ�Χ
        OGREnvelope extent;
        if (poLayer->GetExtent(&extent) != OGRERR_NONE) {
            std::cerr << "Failed to get layer extent." << std::endl;
            GDALClose(poDS);
            return false;
        }

        double centerLat = (extent.MaxY + extent.MinY) / 2.0;
        double centerLon = (extent.MaxX + extent.MinX) / 2.0;

        std::cout << "Data extent:" << std::endl;
        std::cout << "Latitude range: " << extent.MinY << " to " << extent.MaxY << std::endl;
        std::cout << "Longitude range: " << extent.MinX << " to " << extent.MaxX << std::endl;
        std::cout << "Using center point as origin: " << centerLat << ", " << centerLon << std::endl;

        lanelet::LaneletMapPtr map = std::make_shared<lanelet::LaneletMap>();
        lanelet::Origin origin({ centerLat, centerLon });
        lanelet::projection::UtmProjector projector(origin);

        switch (geomType) {
        case wkbPoint: {
            std::cout << "Processing point features..." << std::endl;
            while ((poFeature = poLayer->GetNextFeature()) != NULL) {
                OGRPoint* poPoint = (OGRPoint*)poFeature->GetGeometryRef();
                if (!poPoint) continue;

                double lat = poPoint->getY();
                double lon = poPoint->getX();
                double z = poPoint->getZ();

                if (isGeographic) {
                    lanelet::GPSPoint gps_pt;
                    gps_pt.lat = lat;
                    gps_pt.lon = lon;
                    lanelet::BasicPoint3d utm_pt = projector.forward(gps_pt);
                    map->add(lanelet::Point3d(lanelet::utils::getId(), utm_pt.x(), utm_pt.y(), z));
                }
                else {
                    map->add(lanelet::Point3d(lanelet::utils::getId(), lon, lat, z));
                }
                OGRFeature::DestroyFeature(poFeature);
            }
            break;
        }
        case wkbLineString: {
            std::cout << "Processing line features..." << std::endl;
            while ((poFeature = poLayer->GetNextFeature()) != NULL) {
                OGRLineString* poLine = (OGRLineString*)poFeature->GetGeometryRef();
                if (!poLine) continue;

                std::vector<lanelet::Point3d> points;
                for (int i = 0; i < poLine->getNumPoints(); i++) {
                    double lat = poLine->getY(i);
                    double lon = poLine->getX(i);
                    double z = poLine->getZ(i);

                    if (isGeographic) {
                        lanelet::GPSPoint gps_pt;
                        gps_pt.lat = lat;
                        gps_pt.lon = lon;
                        lanelet::BasicPoint3d utm_pt = projector.forward(gps_pt);
                        points.push_back(lanelet::Point3d(
                            lanelet::utils::getId(),
                            utm_pt.x(),
                            utm_pt.y(),
                            z
                        ));
                    }
                    else {
                        points.push_back(lanelet::Point3d(
                            lanelet::utils::getId(),
                            lon,
                            lat,
                            z
                        ));
                    }
                    map->add(points.back());
                }
                lanelet::LineString3d ls(lanelet::utils::getId(), points);
                map->add(ls);
                OGRFeature::DestroyFeature(poFeature);
            }
            break;
        }
        default: {
            std::cerr << "Unsupported geometry type." << std::endl;
            GDALClose(poDS);
            return false;
        }
        }

        std::cout << "Saving lanelet2 map..." << std::endl;
        lanelet::write(outputFile, *map, projector);

        GDALClose(poDS);
        std::cout << "Conversion completed successfully." << std::endl;
        return true;

    }
    catch (const lanelet::UnsupportedExtensionError& e) {
        std::cerr << "Error saving file: " << e.what() << std::endl;
        if (poDS) GDALClose(poDS);
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        if (poDS) GDALClose(poDS);
        return false;
    }
}
*/
/*bool Converter::lanelet2ToShp(const std::string& laneletFile, const std::string& outputShp) {
    try {
        // ��ȡ�û�����Ĵ���ԭ��
        double approxLat, approxLon;
        std::cout << "Please enter an approximate origin point near your data area." << std::endl;
        std::cout << "Latitude (-90 to 90): ";
        std::cin >> approxLat;
        std::cout << "Longitude (-180 to 180): ";
        std::cin >> approxLon;

        // ��֤����
        if (approxLat < -90 || approxLat > 90 || approxLon < -180 || approxLon > 180) {
            std::cerr << "Invalid coordinates. Latitude must be between -90 and 90, longitude between -180 and 180." << std::endl;
            return false;
        }

        std::cout << "\nPhase 1: Loading map with approximate origin..." << std::endl;

        lanelet::Origin approxOrigin({ approxLat, approxLon });
        lanelet::projection::UtmProjector projector(approxOrigin);

        lanelet::ErrorMessages errors;
        lanelet::LaneletMapPtr map = lanelet::load(laneletFile, projector, &errors);

        if (!map) {
            std::cerr << "Failed to load lanelet2 file" << std::endl;
            return false;
        }

        if (!errors.empty()) {
            std::cout << "\nWarnings during map loading (these can usually be ignored):" << std::endl;
            for (const auto& err : errors) {
                std::cout << "  - " << err << std::endl;
            }
        }

        // �ռ����в��빹���ߵĵ��ID
        std::set<lanelet::Id> usedPointIds;
        for (const auto& element : map->lineStringLayer) {
            for (const auto& pt : element) {
                usedPointIds.insert(pt.id());
            }
        }

        // ����Ƿ��й�����
        bool hasIsolatedPoints = false;
        for (const auto& point : map->pointLayer) {
            if (usedPointIds.find(point.id()) == usedPointIds.end()) {
                hasIsolatedPoints = true;
                break;
            }
        }

        // �����ߵ�shapefile
        std::string lineShpPath = outputShp;
        GDALDataset* poLineDS = createShapefile(lineShpPath, wkbLineString);
        if (!poLineDS) {
            std::cerr << "Failed to create line shapefile" << std::endl;
            return false;
        }
        OGRLayer* poLineLayer = poLineDS->GetLayer(0);

        // ����й����㣬�������shapefile
        GDALDataset* poPointDS = nullptr;
        OGRLayer* poPointLayer = nullptr;
        std::string pointShpPath;

        if (hasIsolatedPoints) {
            // ������ļ�·���й����shapefile��·��
            pointShpPath = outputShp.substr(0, outputShp.find_last_of(".")) + "_points.shp";
            poPointDS = createShapefile(pointShpPath, wkbPoint);
            if (!poPointDS) {
                std::cerr << "Failed to create point shapefile" << std::endl;
                GDALClose(poLineDS);
                return false;
            }
            poPointLayer = poPointDS->GetLayer(0);
        }

        std::cout << "Processing linestrings..." << std::endl;
        int processedLines = 0;

        // ������ͼ��
        for (const auto& element : map->lineStringLayer) {
            OGRFeature* poFeature = OGRFeature::CreateFeature(poLineLayer->GetLayerDefn());
            if (!poFeature) {
                std::cerr << "Failed to create line feature" << std::endl;
                continue;
            }

            OGRLineString line;
            const auto& points = element.basicLineString();
            for (const auto& point : points) {
                lanelet::GPSPoint gps = projector.reverse({ point.x(), point.y(), point.z() });
                line.addPoint(gps.lon, gps.lat, point.z());
            }

            poFeature->SetGeometry(&line);

            if (poLineLayer->CreateFeature(poFeature) != OGRERR_NONE) {
                std::cerr << "Failed to create feature in line shapefile" << std::endl;
                OGRFeature::DestroyFeature(poFeature);
                continue;
            }

            OGRFeature::DestroyFeature(poFeature);
            processedLines++;
        }

        int processedPoints = 0;
        // ����й����㣬�����ͼ��
        if (hasIsolatedPoints) {
            std::cout << "Processing isolated points..." << std::endl;
            for (const auto& point : map->pointLayer) {
                if (usedPointIds.find(point.id()) == usedPointIds.end()) {
                    OGRFeature* poFeature = OGRFeature::CreateFeature(poPointLayer->GetLayerDefn());
                    if (!poFeature) {
                        std::cerr << "Failed to create point feature" << std::endl;
                        continue;
                    }

                    OGRPoint ogrPoint;
                    const auto& basicPoint = point.basicPoint();
                    lanelet::GPSPoint gps = projector.reverse({ basicPoint.x(), basicPoint.y(), basicPoint.z() });
                    ogrPoint.setX(gps.lon);
                    ogrPoint.setY(gps.lat);
                    ogrPoint.setZ(basicPoint.z());

                    poFeature->SetGeometry(&ogrPoint);

                    if (poPointLayer->CreateFeature(poFeature) != OGRERR_NONE) {
                        std::cerr << "Failed to create feature in point shapefile" << std::endl;
                        OGRFeature::DestroyFeature(poFeature);
                        continue;
                    }

                    OGRFeature::DestroyFeature(poFeature);
                    processedPoints++;
                }
            }
        }

        std::cout << "Successfully processed " << processedLines << " linestrings" << std::endl;
        if (hasIsolatedPoints) {
            std::cout << "Successfully processed " << processedPoints << " isolated points" << std::endl;
            std::cout << "Created " << lineShpPath << " and " << pointShpPath << std::endl;
        }
        else {
            std::cout << "No isolated points found" << std::endl;
            std::cout << "Created " << lineShpPath << std::endl;
        }

        GDALClose(poLineDS);
        if (poPointDS) {
            GDALClose(poPointDS);
        }

        return true;

    }
    catch (const lanelet::UnsupportedExtensionError& e) {
        std::cerr << "Error saving file: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}
*/

std::string processAttributeKey(const std::string& key) {
    if (key == "Type") {
        return "type";
    }
    return key;
}

bool Converter::shpToLanelet2(const std::string& shpFile, const std::string& outputFile) {
    GDALDataset* poDS = nullptr;
    try {
        poDS = (GDALDataset*)GDALOpenEx(shpFile.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
        if (!poDS) {
            std::cerr << "Failed to open shapefile." << std::endl;
            return false;
        }

        OGRLayer* poLayer = poDS->GetLayer(0);
        if (!poLayer) {
            std::cerr << "Failed to get layer from shapefile." << std::endl;
            GDALClose(poDS);
            return false;
        }

        // ����һ��Ҫ�صļ�������
        OGRFeature* poFeature = poLayer->GetNextFeature();
        if (!poFeature) {
            std::cerr << "No features found in shapefile." << std::endl;
            GDALClose(poDS);
            return false;
        }
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        OGRwkbGeometryType geomType = wkbFlatten(poGeometry->getGeometryType());
        OGRFeature::DestroyFeature(poFeature);
        poLayer->ResetReading();

        // ����ϵ���
        OGRSpatialReference* poSRS = poLayer->GetSpatialRef();
        if (!poSRS) {
            std::cerr << "Error: No coordinate system found in shapefile." << std::endl;
            std::cerr << "Please set the coordinate system before conversion." << std::endl;
            GDALClose(poDS);
            return false;
        }

        bool isGeographic = poSRS->IsGeographic();
        bool isProjected = poSRS->IsProjected();
        const char* projName = poSRS->GetAttrValue("PROJECTION");

        std::cout << "Detected coordinate system: " << (isGeographic ? "Geographic" :
            (isProjected ? "Projected" : "Unknown")) << std::endl;
        if (isProjected && projName) {
            std::cout << "Projection type: " << projName << std::endl;
        }

        // ���ݷ�Χ
        OGREnvelope extent;
        if (poLayer->GetExtent(&extent) != OGRERR_NONE) {
            std::cerr << "Failed to get layer extent." << std::endl;
            GDALClose(poDS);
            return false;
        }

        double centerY = (extent.MaxY + extent.MinY) / 2.0;
        double centerX = (extent.MaxX + extent.MinX) / 2.0;
        double centerLat = centerY, centerLon = centerX;

        // �����ͶӰ���꣬ת�����ĵ㵽��γ��
        if (isProjected) {
            OGRSpatialReference srsWGS84;
            srsWGS84.SetWellKnownGeogCS("WGS84");

            OGRCoordinateTransformation* poCT = OGRCreateCoordinateTransformation(poSRS, &srsWGS84);

            if (poCT) {
                double lon = centerX;
                double lat = centerY;

                if (poCT->Transform(1, &lon, &lat)) {
                    centerLat = lat;
                    centerLon = lon;
                }

                OCTDestroyCoordinateTransformation(poCT);
            }
        }
        std::cout << "Data extent:" << std::endl;
        if (isGeographic) {
            std::cout << "Latitude range: " << extent.MinY << " to " << extent.MaxY << std::endl;
            std::cout << "Longitude range: " << extent.MinX << " to " << extent.MaxX << std::endl;
        }
        else {
            std::cout << "X range: " << extent.MinX << " to " << extent.MaxX << std::endl;
            std::cout << "Y range: " << extent.MinY << " to " << extent.MaxY << std::endl;
        }
        std::cout << "Using center point as origin: " << centerLat << ", " << centerLon << std::endl;
        lanelet::LaneletMapPtr map = std::make_shared<lanelet::LaneletMap>();
        lanelet::Origin origin({ centerLat, centerLon });
        lanelet::projection::UtmProjector projector(origin);

        switch (geomType) {
        case wkbPoint: {
            std::cout << "Processing point features..." << std::endl;
            poLayer->ResetReading();
            while ((poFeature = poLayer->GetNextFeature()) != NULL) {
                OGRPoint* poPoint = (OGRPoint*)poFeature->GetGeometryRef();
                if (!poPoint) continue;

                double lat = poPoint->getY();
                double lon = poPoint->getX();
                double z = poPoint->getZ();

                if (isGeographic) {
                    lanelet::GPSPoint gps_pt;
                    gps_pt.lat = lat;
                    gps_pt.lon = lon;
                    lanelet::BasicPoint3d utm_pt = projector.forward(gps_pt);

                    // �����㲢��ӷǿ�����
                    lanelet::Point3d point(lanelet::utils::getId(), utm_pt.x(), utm_pt.y(), z);
                    OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();
                    for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
                        OGRFieldDefn* poFieldDefn = poFDefn->GetFieldDefn(i);
                        if (!poFeature->IsFieldSet(i)) continue;

                        std::string originalKey = poFieldDefn->GetNameRef();
                        std::string key = processAttributeKey(originalKey);
                        std::string value = poFeature->GetFieldAsString(i);

                        // ֻ��ӷǿյ�����
                        if (!value.empty() && value != "null" && value != "NULL") {
                            point.setAttribute(key, value);
                        }
                    }
                    map->add(point);
                }
                else {
                    // �����Ѿ���ͶӰ��������
                    lanelet::Point3d point(lanelet::utils::getId(), lon, lat, z);
                    OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();
                    for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
                        OGRFieldDefn* poFieldDefn = poFDefn->GetFieldDefn(i);
                        if (!poFeature->IsFieldSet(i)) continue;

                        std::string originalKey = poFieldDefn->GetNameRef();
                        std::string key = processAttributeKey(originalKey);
                        std::string value = poFeature->GetFieldAsString(i);

                        // ֻ��ӷǿյ�����
                        if (!value.empty() && value != "null" && value != "NULL") {
                            point.setAttribute(key, value);
                        }
                    }
                    map->add(point);
                }
                OGRFeature::DestroyFeature(poFeature);
            }
            break;
        }
        case wkbLineString: {
            std::cout << "Processing line features..." << std::endl;
            poLayer->ResetReading();
            while ((poFeature = poLayer->GetNextFeature()) != NULL) {
                OGRLineString* poLine = (OGRLineString*)poFeature->GetGeometryRef();
                if (!poLine) continue;

                std::vector<lanelet::Point3d> points;
                for (int i = 0; i < poLine->getNumPoints(); i++) {
                    double lat = poLine->getY(i);
                    double lon = poLine->getX(i);
                    double z = poLine->getZ(i);

                    if (isGeographic) {
                        lanelet::GPSPoint gps_pt;
                        gps_pt.lat = lat;
                        gps_pt.lon = lon;
                        lanelet::BasicPoint3d utm_pt = projector.forward(gps_pt);
                        points.push_back(lanelet::Point3d(
                            lanelet::utils::getId(),
                            utm_pt.x(),
                            utm_pt.y(),
                            z
                        ));
                    }
                    else {
                        points.push_back(lanelet::Point3d(
                            lanelet::utils::getId(),
                            lon,
                            lat,
                            z
                        ));
                    }
                    map->add(points.back());
                }

                // ���� LineString ����ӷǿ�����
                lanelet::LineString3d ls(lanelet::utils::getId(), points);
                OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();
                for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
                    OGRFieldDefn* poFieldDefn = poFDefn->GetFieldDefn(i);
                    if (!poFeature->IsFieldSet(i)) continue;

                    std::string originalKey = poFieldDefn->GetNameRef();
                    std::string key = processAttributeKey(originalKey);
                    std::string value = poFeature->GetFieldAsString(i);

                    // ֻ��ӷǿյ�����
                    if (!value.empty() && value != "null" && value != "NULL") {
                        ls.setAttribute(key, value);
                    }
                }
                map->add(ls);
                OGRFeature::DestroyFeature(poFeature);
            }
            break;
        }
        default: {
            std::cerr << "Unsupported geometry type." << std::endl;
            GDALClose(poDS);
            return false;
        }
        }

        std::cout << "Saving lanelet2 map..." << std::endl;
        lanelet::write(outputFile, *map, projector);

        GDALClose(poDS);
        std::cout << "Conversion completed successfully." << std::endl;
        return true;
    }
    catch (const lanelet::UnsupportedExtensionError& e) {
        std::cerr << "Error saving file: " << e.what() << std::endl;
        if (poDS) GDALClose(poDS);
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        if (poDS) GDALClose(poDS);
        return false;
    }
}



std::string sanitizeAttributeValue(const std::string& value) {
    std::string sanitized;
    std::copy_if(value.begin(), value.end(), std::back_inserter(sanitized),
        [](char c) {
            return std::isprint(static_cast<unsigned char>(c)) ||
                std::isspace(static_cast<unsigned char>(c));
        });

    sanitized.erase(0, sanitized.find_first_not_of(" \t\n\r"));
    sanitized.erase(sanitized.find_last_not_of(" \t\n\r") + 1);

    return sanitized;
}

std::set<std::string> collectIsolatedPointAttributeKeys(lanelet::LaneletMapPtr map) {
    std::set<std::string> isolatedPointAttributeKeys;

    std::set<lanelet::Id> usedPointIds;
    for (const auto& element : map->lineStringLayer) {
        for (const auto& pt : element) {
            usedPointIds.insert(pt.id());
        }
    }

    for (const auto& point : map->pointLayer) {
        if (usedPointIds.find(point.id()) == usedPointIds.end()) {
            const auto& attributes = point.attributes();
            for (const auto& attr : attributes) {
                isolatedPointAttributeKeys.insert(attr.first);
            }
        }
    }

    return isolatedPointAttributeKeys;
}

// �ռ���Ҫ�ص����Լ�
std::set<std::string> collectLineStringAttributeKeys(lanelet::LaneletMapPtr map) {
    std::set<std::string> lineStringAttributeKeys;

    for (const auto& element : map->lineStringLayer) {
        const auto& attributes = element.attributes();
        for (const auto& attr : attributes) {
            lineStringAttributeKeys.insert(attr.first);
        }
    }

    return lineStringAttributeKeys;
}

void createFieldsForAllKeys(OGRLayer* poLayer, const std::set<std::string>& globalAttributeKeys) {
    for (const auto& key : globalAttributeKeys) {
        if (poLayer->GetLayerDefn()->GetFieldIndex(key.c_str()) < 0) {
            OGRFieldDefn oField(key.c_str(), OFTString);
            oField.SetWidth(254);  
            oField.SetNullable(true);  

            if (poLayer->CreateField(&oField) != OGRERR_NONE) {
                std::cerr << "Failed to create field: " << key << std::endl;
            }
        }
    }
}

void processElementAttributes(OGRFeature* poFeature,
    const std::set<std::string>& globalAttributeKeys,
    const lanelet::AttributeMap& attributes) {

    for (const auto& key : globalAttributeKeys) {
        // ��ȡ�ضϺ���ֶ���
        std::string truncatedKey = key.substr(0, 10);
        int fieldIndex = poFeature->GetFieldIndex(truncatedKey.c_str());
        if (fieldIndex < 0) continue;

        auto it = attributes.find(key);
        if (it != attributes.end()) {
            try {
                std::string value = it->second.value();
                std::string safeValue = sanitizeAttributeValue(value);

                if (safeValue.empty() || safeValue == "nullpos" || safeValue == "nullopt") {
                    poFeature->SetFieldNull(fieldIndex);
                }
                else {
                    if (safeValue.length() > 254) {
                        safeValue = safeValue.substr(0, 254);
                    }
                    poFeature->SetField(fieldIndex, safeValue.c_str());
                }
            }
            catch (...) {
                poFeature->SetFieldNull(fieldIndex);
            }
        }
        else {
            poFeature->SetFieldNull(fieldIndex);
        }
    }
}

bool Converter::lanelet2ToShp(const std::string& laneletFile, const std::string& outputShp) {
    try {
        // �������ԭ������
        double approxLat, approxLon;
        std::cout << "Please enter an approximate origin point near your data area." << std::endl;
        std::cout << "Latitude (-90 to 90): ";
        std::cin >> approxLat;
        std::cout << "Longitude (-180 to 180): ";
        std::cin >> approxLon;

        if (approxLat < -90 || approxLat > 90 || approxLon < -180 || approxLon > 180) {
            std::cerr << "Invalid coordinates. Latitude must be between -90 and 90, longitude between -180 and 180." << std::endl;
            return false;
        }

        std::cout << "\nPhase 1: Loading map with approximate origin..." << std::endl;

        lanelet::Origin approxOrigin({ approxLat, approxLon });
        lanelet::projection::UtmProjector projector(approxOrigin);

        lanelet::ErrorMessages errors;
        lanelet::LaneletMapPtr map = lanelet::load(laneletFile, projector, &errors);

        // �������е�����귶Χ
        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();

        // ֱ�ӱ�����ͼ������е�
        for (const auto& point : map->pointLayer) {
            const auto& basicPoint = point.basicPoint();
            minX = std::min(minX, basicPoint.x());
            maxX = std::max(maxX, basicPoint.x());
            minY = std::min(minY, basicPoint.y());
            maxY = std::max(maxY, basicPoint.y());
        }

        // �����µ����ĵ�
        double centerX = (minX + maxX) / 2.0;
        double centerY = (minY + maxY) / 2.0;
        // ʹ���µ����ĵ����´���ͶӰ��
        lanelet::GPSPoint newCenterGPS = projector.reverse({ centerX, centerY, 0.0 });
        lanelet::Origin newOrigin({ newCenterGPS.lat, newCenterGPS.lon });
        lanelet::projection::UtmProjector newProjector(newOrigin);

        // ���¼��ص�ͼ
        map = lanelet::load(laneletFile, newProjector, &errors);

        if (!map) {
            std::cerr << "Failed to load lanelet2 file" << std::endl;
            return false;
        }
        if (!errors.empty()) {
            std::cout << "\nWarnings during map loading (these can usually be ignored):" << std::endl;
            for (const auto& err : errors) {
                std::cout << "  - " << err << std::endl;
            }
        }

        std::set<std::string> lineStringAttributeKeys = collectLineStringAttributeKeys(map);
        std::set<std::string> isolatedPointAttributeKeys = collectIsolatedPointAttributeKeys(map);

        // ������Ҫ��
        std::string lineShpPath = outputShp;
        GDALDataset* poLineDS = createShapefile(lineShpPath, wkbLineString,newCenterGPS);
        OGRLayer* poLineLayer = poLineDS->GetLayer(0);

        createFieldsForAllKeys(poLineLayer, lineStringAttributeKeys);

        for (const auto& element : map->lineStringLayer) {
            OGRFeature* poFeature = OGRFeature::CreateFeature(poLineLayer->GetLayerDefn());

            OGRLineString line;
            const auto& points = element.basicLineString();
            for (const auto& point : points) {
                double zValue = 0.0;
                for (const auto& p : element) {
                    if (p.basicPoint() == point) {
                        auto eleTag = p.attributes().find("ele");
                        if (eleTag != p.attributes().end()) {
                            try {
                                zValue = std::stod(eleTag->second.value());
                            }
                            catch (...) {
                                std::cout << "Warning: Could not parse 'ele' tag for Z value" << std::endl;
                            }
                            break;
                        }
                    }
                }
                lanelet::GPSPoint gps = newProjector.reverse({ point.x(), point.y(), zValue });
                OGRSpatialReference sourceSRS, targetSRS;
                int utmZone = determineUTMZone(newCenterGPS.lon);
                bool isNorthHemisphere = newCenterGPS.lat >= 0;
                targetSRS.SetUTM(utmZone, isNorthHemisphere);
                sourceSRS.SetWellKnownGeogCS("WGS84");

                OGRCoordinateTransformation* poCT = OGRCreateCoordinateTransformation(&sourceSRS, &targetSRS);

                double lon = gps.lon;
                double lat = gps.lat;
                if (poCT) {
                    poCT->Transform(1, &lat, &lon);
                    OCTDestroyCoordinateTransformation(poCT);
                }

                line.addPoint(lat, lon, zValue);
                //line.addPoint(gps.lon, gps.lat, zValue);

            }
            poFeature->SetGeometry(&line);
            processElementAttributes(poFeature, lineStringAttributeKeys, element.attributes());
            if (poLineLayer->CreateFeature(poFeature) != OGRERR_NONE) {
                std::cerr << "Failed to create feature in line shapefile" << std::endl;
                OGRFeature::DestroyFeature(poFeature);
                continue;
            }

            OGRFeature::DestroyFeature(poFeature);
        }

        // �жϹ�����
        std::set<lanelet::Id> usedPointIds;
        for (const auto& element : map->lineStringLayer) {
            for (const auto& pt : element) {
                usedPointIds.insert(pt.id());
            }
        }

        GDALDataset* poPointDS = nullptr;
        OGRLayer* poPointLayer = nullptr;
        std::string pointShpPath;

        bool hasIsolatedPoints = false;
        for (const auto& point : map->pointLayer) {
            if (usedPointIds.find(point.id()) == usedPointIds.end()) {
                hasIsolatedPoints = true;
                break;
            }
        }

        // ���������
        if (hasIsolatedPoints) {
            pointShpPath = outputShp.substr(0, outputShp.find_last_of(".")) + "_points.shp";
            poPointDS = createShapefile(pointShpPath, wkbPoint,newCenterGPS);
            poPointLayer = poPointDS->GetLayer(0);

            createFieldsForAllKeys(poPointLayer, isolatedPointAttributeKeys);
            for (const auto& point : map->pointLayer) {
                if (usedPointIds.find(point.id()) == usedPointIds.end()) {
                    OGRFeature* poFeature = OGRFeature::CreateFeature(poPointLayer->GetLayerDefn());

                    OGRPoint ogrPoint;
                    const auto& basicPoint = point.basicPoint();
                    double zValue = basicPoint.z();
                    auto eleTag = point.attributes().find("ele");
                    if (eleTag != point.attributes().end()) {
                        try {
                            zValue = std::stod(eleTag->second.value());
                        }
                        catch (...) {
                            std::cout << "Warning: Could not parse 'ele' tag for Z value" << std::endl;
                        }
                    }
                    lanelet::GPSPoint gps = newProjector.reverse({ basicPoint.x(), basicPoint.y(), zValue });
                    OGRSpatialReference sourceSRS, targetSRS;
                    int utmZone = determineUTMZone(newCenterGPS.lon);
                    bool isNorthHemisphere = newCenterGPS.lat >= 0;
                    targetSRS.SetUTM(utmZone, isNorthHemisphere);
                    sourceSRS.SetWellKnownGeogCS("WGS84");

                    OGRCoordinateTransformation* poCT = OGRCreateCoordinateTransformation(&sourceSRS, &targetSRS);
                    double lon = gps.lon;
                    double lat = gps.lat;
                    if (poCT) {
                        poCT->Transform(1, &lat, &lon);
                        OCTDestroyCoordinateTransformation(poCT);
                    }

                    
                    ogrPoint.setX(lat);
                    ogrPoint.setY(lon);
                    ogrPoint.setZ(zValue);
                    poFeature->SetGeometry(&ogrPoint);

                    processElementAttributes(poFeature, isolatedPointAttributeKeys, point.attributes());

                    if (poPointLayer->CreateFeature(poFeature) != OGRERR_NONE) {
                        std::cerr << "Failed to create feature in point shapefile" << std::endl;
                        OGRFeature::DestroyFeature(poFeature);
                        continue;
                    }

                    OGRFeature::DestroyFeature(poFeature);
                }
            }
        }
        GDALClose(poLineDS);
        if (poPointDS) {
            GDALClose(poPointDS);
        }

        std::cout << "Successfully converted Lanelet2 to Shapefile" << std::endl;
        if (hasIsolatedPoints) {
            std::cout << "Created " << lineShpPath << " and " << pointShpPath << std::endl;
        }
        else {
            std::cout << "Created " << lineShpPath << std::endl;
        }

        return true;
    }
    catch (const lanelet::UnsupportedExtensionError& e) {
        std::cerr << "Error saving file: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

/*GDALDataset* Converter::createShapefile(const std::string& filename, OGRwkbGeometryType geomType) {
    const char* pszDriverName = "ESRI Shapefile";
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
    if (poDriver == NULL) {
        return NULL;
    }

    OGRwkbGeometryType geomTypeWithZ = wkbSetZ(geomType);

    GDALDataset* poDS = poDriver->Create(filename.c_str(), 0, 0, 0, GDT_Unknown, NULL);
    if (poDS == NULL) {
        return NULL;
    }

    OGRSpatialReference oSRS;
    oSRS.SetWellKnownGeogCS("WGS84");
    const char* layerName = (geomType == wkbPoint) ? "points" : "lines";

    OGRLayer* poLayer = poDS->CreateLayer(layerName, &oSRS, geomTypeWithZ, NULL);
    if (poLayer == NULL) {
        GDALClose(poDS);
        return NULL;
    }

    addFields(poLayer);
    return poDS;
}
*/
GDALDataset* Converter::createShapefile(const std::string& filename, OGRwkbGeometryType geomType,lanelet::GPSPoint CenterGPS) {
    const char* pszDriverName = "ESRI Shapefile";
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
    if (poDriver == NULL) {
        return NULL;
    }

    GDALDataset* poDS = poDriver->Create(filename.c_str(), 0, 0, 0, GDT_Unknown, NULL);
    if (poDS == NULL) {
        return NULL;
    }

    // ���� UTM �ռ�ο�ϵͳ
    OGRSpatialReference oSRS;
    int utmZone = determineUTMZone(CenterGPS.lon); 
    bool isNorthHemisphere = CenterGPS.lat >= 0;
    std::cout << utmZone << std::endl;
    // ���� UTM ͶӰ
    oSRS.SetUTM(utmZone, isNorthHemisphere);
    oSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    oSRS.SetWellKnownGeogCS("WGS84");

    const char* layerName = (geomType == wkbPoint) ? "points" : "lines";
    OGRLayer* poLayer = poDS->CreateLayer(layerName, &oSRS, geomType, NULL);
    if (poLayer == NULL) {
        GDALClose(poDS);
        return NULL;
    }

    addFields(poLayer);
    return poDS;
}


int Converter::determineUTMZone(double longitude) {
    return static_cast<int>((longitude + 180.0) / 6.0) + 1;
}

void Converter::addFields(OGRLayer* layer) {
    OGRFieldDefn oField("ID", OFTInteger);
    layer->CreateField(&oField);

    OGRFieldDefn oField2("Type", OFTString);
    oField2.SetWidth(32);
    layer->CreateField(&oField2);
}