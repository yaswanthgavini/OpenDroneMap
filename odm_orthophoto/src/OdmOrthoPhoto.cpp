// C++
#include <math.h>
#include <sstream>

// This
#include "OdmOrthoPhoto.hpp"

std::ostream & operator<< (std::ostream &os, const WorldPoint &worldPoint)
{
    return os << worldPoint.eastInteger_ + worldPoint.eastFractional_ << " " << worldPoint.northInteger_ + worldPoint.northFractional_;
}

std::istream & operator>> (std::istream &is,  WorldPoint &worldPoint)
{
    is >> worldPoint.eastInteger_;
    // Check if east coordinate is given as rational.
    if('.' == is.peek())
    {
        is >> worldPoint.eastFractional_;
    }
    else
    {
        worldPoint.eastFractional_ = 0.0f;
    }
    
    is >> worldPoint.northInteger_;
    // Check if north coordinate is given as rational.
    if('.' == is.peek())
    {
        is >> worldPoint.northFractional_;
    }
    else
    {
        worldPoint.northFractional_ = 0.0f;
    }

    return is;
}

OdmOrthoPhoto::OdmOrthoPhoto()
    :log_(false)
{
    inputFile_ = "";
    inputGeoRefFile_ = "";
    outputFile_ = "ortho.jpg";
    logFile_    = "log.txt";

    resolution_ = 0.0f;

    boundaryDefined_ = false;
    boundaryPoint1_[0] = 0.0f; boundaryPoint1_[1] = 0.0f;
    boundaryPoint2_[0] = 0.0f; boundaryPoint2_[1] = 0.0f;
    boundaryPoint3_[0] = 0.0f; boundaryPoint3_[1] = 0.0f;
    boundaryPoint4_[0] = 0.0f; boundaryPoint4_[1] = 0.0f;
}

OdmOrthoPhoto::~OdmOrthoPhoto()
{
}

int OdmOrthoPhoto::run(int argc, char *argv[])
{
    try
    {
        parseArguments(argc, argv);
        createOrthoPhoto();
    }
    catch (const OdmOrthoPhotoException& e)
    {
        log_ << e.what() << "\n";
        log_.print(logFile_);
        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        log_ << "Error in OdmOrthoPhoto:\n";
        log_ << e.what() << "\n";
        log_.print(logFile_);
        return EXIT_FAILURE;
    }
    catch (...)
    {
        log_ << "Unknown error, terminating:\n";
        log_.print(logFile_);
        return EXIT_FAILURE;
    }
    
    log_.print(logFile_);
    
    return EXIT_SUCCESS;
}

void OdmOrthoPhoto::parseArguments(int argc, char *argv[])
{
    logFile_ = std::string(argv[0]) + "_log.txt";
    log_ << logFile_ << "\n\n";
    
    // If no arguments were passed, print help.
    if (argc == 1)
    {
        printHelp();
    }
    
    log_ << "Arguments given\n";
    for(int argIndex = 1; argIndex < argc; ++argIndex)
    {
        log_ << argv[argIndex] << '\n';
    }
    
    log_ << '\n';
    for(int argIndex = 1; argIndex < argc; ++argIndex)
    {
        // The argument to be parsed.
        std::string argument = std::string(argv[argIndex]);
        
        if(argument == "-help")
        {
            printHelp();
        }
        else if(argument == "-resolution")
        {
            ++argIndex;
            if (argIndex >= argc)
            {
                throw OdmOrthoPhotoException("Argument '" + argument + "' expects 1 more input following it, but no more inputs were provided.");
            }
            std::stringstream ss(argv[argIndex]);
            ss >> resolution_;
            log_ << "Resolution count was set to: " << resolution_ << "pixels/meter\n";
        }
        else if(argument == "-boundary")
        {
            if(argIndex+8 >= argc)
            {
                throw OdmOrthoPhotoException("Argument '" + argument + "' expects 8 more input following it, but no more inputs were provided.");
            }

            std::stringstream ss;
            ss << argv[argIndex+1] << " " << argv[argIndex+2] << " " << argv[argIndex+3] << " " << argv[argIndex+4] << " " << argv[argIndex+5] << " " << argv[argIndex+6] << " " << argv[argIndex+7] << " " << argv[argIndex+8];
            ss >> worldPoint1_ >> worldPoint2_ >> worldPoint3_ >> worldPoint4_;
            boundaryDefined_ = true;

            argIndex += 8;

            log_ << "Boundary point 1 was set to: " << worldPoint1_ << '\n';
            log_ << "Boundary point 2 was set to: " << worldPoint2_ << '\n';
            log_ << "Boundary point 3 was set to: " << worldPoint3_ << '\n';
            log_ << "Boundary point 4 was set to: " << worldPoint4_ << '\n';
        }
        else if(argument == "-boundaryMinMax")
        {
            if(argIndex+4 >= argc)
            {
                throw OdmOrthoPhotoException("Argument '" + argument + "' expects 4 more input following it, but no more inputs were provided.");
            }

            std::stringstream ss;
            ss << argv[argIndex+1] << " " << argv[argIndex+2] << " " << argv[argIndex+3] << " " << argv[argIndex+4];
            ss >> worldPoint1_ >> worldPoint3_;
            boundaryDefined_ = true;

            // Set the other world points as the other two corners.
            worldPoint2_.eastFractional_  = worldPoint1_.eastFractional_;
            worldPoint2_.eastInteger_     = worldPoint1_.eastInteger_;
            worldPoint2_.northFractional_ = worldPoint3_.northFractional_;
            worldPoint2_.northInteger_    = worldPoint3_.northInteger_;
            
            worldPoint4_.eastFractional_  = worldPoint3_.eastFractional_;
            worldPoint4_.eastInteger_     = worldPoint3_.eastInteger_;
            worldPoint4_.northFractional_ = worldPoint1_.northFractional_;
            worldPoint4_.northInteger_    = worldPoint1_.northInteger_;

            argIndex += 4;

            log_ << "Boundary point 1 was set to: " << worldPoint1_ << '\n';
            log_ << "Boundary point 2 was set to: " << worldPoint2_ << '\n';
            log_ << "Boundary point 3 was set to: " << worldPoint3_ << '\n';
            log_ << "Boundary point 4 was set to: " << worldPoint4_ << '\n';
        }
        else if(argument == "-verbose")
        {
            log_.setIsPrintingInCout(true);
        }
        else if (argument == "-logFile")
        {
            ++argIndex;
            if (argIndex >= argc)
            {
                throw OdmOrthoPhotoException("Missing argument for '" + argument + "'.");
            }
            logFile_ = std::string(argv[argIndex]);
            std::ofstream testFile(logFile_.c_str());
            if (!testFile.is_open())
            {
                throw OdmOrthoPhotoException("Argument '" + argument + "' has a bad value.");
            }
            log_ << "Log file path was set to: " << logFile_ << "\n";
        }
        else if(argument == "-inputFile")
        {
            argIndex++;
            if (argIndex >= argc)
            {
                throw OdmOrthoPhotoException("Argument '" + argument + "' expects 1 more input following it, but no more inputs were provided.");
            }
            inputFile_ = std::string(argv[argIndex]);
            log_ << "Reading textured mesh from: " << inputFile_ << "\n";
        }
        else if(argument == "-inputGeoRefFile")
        {
            argIndex++;
            if (argIndex >= argc)
            {
                throw OdmOrthoPhotoException("Argument '" + argument + "' expects 1 more input following it, but no more inputs were provided.");
            }
            inputGeoRefFile_ = std::string(argv[argIndex]);
            log_ << "Reading georef from: " << inputGeoRefFile_ << "\n";
        }
        else if(argument == "-outputFile")
        {
            argIndex++;
            if (argIndex >= argc)
            {
                throw OdmOrthoPhotoException("Argument '" + argument + "' expects 1 more input following it, but no more inputs were provided.");
            }
            outputFile_ = std::string(argv[argIndex]);
            log_ << "Writing output to: " << outputFile_ << "\n";
        }
        else
        {
            printHelp();
            throw OdmOrthoPhotoException("Unrecognised argument '" + argument + "'");
        }
    }
    log_ << "\n";
}

void OdmOrthoPhoto::printHelp()
{
    log_.setIsPrintingInCout(true);

    log_ << "OpenDroneMapOrthoPhoto.exe\n\n";

    log_ << "Purpose\n";
    log_ << "Create an orthograpical photo from an oriented textured mesh.\n\n";

    log_ << "Usage:\n";
    log_ << "The program requires a path to an input OBJ mesh file and a resolution, as pixels/m. All other input parameters are optional.\n\n";

    log_ << "The following flags are available\n";
    log_ << "Call the program with flag \"-help\", or without parameters to print this message, or check any generated log file.\n";
    log_ << "Call the program with flag \"-verbose\", to print log messages in the standard output stream as well as in the log file.\n\n";

    log_ << "Parameters are specified as: \"-<argument name> <argument>\", (without <>), and the following parameters are configureable:n";
    log_ << "\"-inputFile <path>\" (mandatory)\n";
    log_ << "\"Input obj file that must contain a textured mesh.\n\n";

    log_ << "\"-inputGeoRefFile <path>\" (optional, if specified boundary points are assumed to be givan as world coordinates. If not specified, the boundary points are assumed to be local coordinates)\n";
    log_ << "\"Input geograpical reference system file that describes the world position of the model's origin.\n\n";

    log_ << "\"-outputFile <path>\" (optional, default: ortho.jpg)\n";
    log_ << "\"Target file in which the orthophoto is saved.\n\n";

    log_ << "\"-resolution <pixels/m>\" (mandatory)\n";
    log_ << "\"The number of pixels used per meter.\n\n";

    log_ << "\"-boundary <Point1x Point1y Point2x Point2y Point3x Point3y Point4x Point4y>\" (optional, if not specified the entire model will be rendered)\n";
    log_ << "\"Describes the area which should be covered in the ortho photo. The area will be a bounding box containing all four points. The points should be given in the same georeference system as the model.\n\n";

    log_ << "\"-boundaryMinMax <MinX MinY MaxX MaxY>\" (optional, if not specified the entire model will be rendered.)\n";
    log_ << "\"Describes the area which should be covered in the ortho photo. The area will be a bounding box with corners at MinX, MinY and MaxX, MaxY. The points should be given in the same georeference system as the model.\n\n";

    log_.setIsPrintingInCout(false);
}

void OdmOrthoPhoto::createOrthoPhoto()
{
    if(inputFile_.empty())
    {
        throw OdmOrthoPhotoException("Failed to create ortho photo, no texture mesh given.");
    }

    if(boundaryDefined_)
    {
        if(inputGeoRefFile_.empty())
        {
            // Points are assumed to be given in as local points.
            adjustBoundsForLocal();
        }
        else
        {
            // Points are assumed to be given in as world points.
            adjustBoundsForGeoRef();
        }
    }
    else if(!inputGeoRefFile_.empty())
    {
        // No boundary points specified, but georeference system file was given.
        log_ << "Warning:\n";
        log_ << "\tSpecified -inputGeoRefFile, but no boundary points. The georeference system will be ignored.\n";
    }

    log_ << "Reading mesh file...\n";
    // The textureds mesh.
    pcl::TextureMesh mesh;
    pcl::io::loadOBJFile(inputFile_, mesh);
    log_ << ".. mesh file read.\n\n";

    // Does the model have more than one material?
    multiMaterial_ = 1 < mesh.tex_materials.size();

    if(multiMaterial_)
    {
        // Need to check relationship between texture coordinates and faces.
        if(!isModelOk(mesh))
        {
            throw OdmOrthoPhotoException("Could not generate ortho photo: The given mesh has multiple textures, but the number of texture coordinates is NOT equal to 3 times the number of faces.");
        }
    }

    if(!boundaryDefined_)
    {
        // Determine boundary from model.
        adjustBoundsForEntireModel(mesh);
    }

    // The minimum and maximum boundary values.
    float xMax, xMin, yMax, yMin;
    xMin = std::min(std::min(boundaryPoint1_[0], boundaryPoint2_[0]), std::min(boundaryPoint3_[0], boundaryPoint4_[0]));
    xMax = std::max(std::max(boundaryPoint1_[0], boundaryPoint2_[0]), std::max(boundaryPoint3_[0], boundaryPoint4_[0]));
    yMin = std::min(std::min(boundaryPoint1_[1], boundaryPoint2_[1]), std::min(boundaryPoint3_[1], boundaryPoint4_[1]));
    yMax = std::max(std::max(boundaryPoint1_[1], boundaryPoint2_[1]), std::max(boundaryPoint3_[1], boundaryPoint4_[1]));

    log_ << "Ortho photo bounds x : " << xMin << " -> " << xMax << '\n';
    log_ << "Ortho photo bounds y : " << yMin << " -> " << yMax << '\n';

    // The size of the area.
    float xDiff = xMax - xMin;
    float yDiff = yMax - yMin;
    log_ << "Ortho photo area : " << xDiff*yDiff << "m2\n";

    // The resolution neccesary to fit the area with the given resolution.
    int rowRes = static_cast<int>(std::ceil(resolution_*yDiff));
    int colRes = static_cast<int>(std::ceil(resolution_*xDiff));
    log_ << "Ortho photo resolution, width x height : " << colRes << "x" << rowRes << '\n';

    // Check size of photo.
    if(0 >= rowRes*colRes)
    {
        if(0 >= rowRes)
        {
            log_ << "Warning: ortho photo has zero area, height = " << rowRes << ". Forcing height = 1.\n";
            rowRes = 1;
        }
        if(0 >= colRes)
        {
            log_ << "Warning: ortho photo has zero area, width = " << colRes << ". Forcing width = 1.\n";
            colRes = 1;
        }
        log_ << "New ortho photo resolution, width x height : " << colRes << "x" << rowRes << '\n';
    }

    // Init ortho photo
    photo_ = cv::Mat::zeros(rowRes, colRes, CV_8UC3) + cv::Scalar(255, 255, 255);
    depth_ = cv::Mat::zeros(rowRes, colRes, CV_32F) - std::numeric_limits<float>::infinity();

    // Contains the vertices of the mesh.
    pcl::PointCloud<pcl::PointXYZ>::Ptr meshCloud (new pcl::PointCloud<pcl::PointXYZ>);
    pcl::fromPCLPointCloud2 (mesh.cloud, *meshCloud);

    // Creates a transformation which aligns the area for the ortho photo.
    Eigen::Transform<float, 3, Eigen::Affine> transform = getROITransform(xMin, -yMax);

    log_ << "Translating and scaling mesh...\n";

    // Move the mesh into position.
    pcl::transformPointCloud(*meshCloud, *meshCloud, transform);
    log_ << ".. mesh translated and scaled.\n\n";

    // Flatten texture coordiantes.
    std::vector<Eigen::Vector2f> uvs;
    for(size_t t = 0; t < mesh.tex_coordinates.size(); ++t)
    {
        uvs.insert(uvs.end(), mesh.tex_coordinates[t].begin(), mesh.tex_coordinates[t].end());
    }

    // The current material texture
    cv::Mat texture;

    // Used to keep track of the global face index.
    size_t faceOff = 0;

    log_ << "Rendering the ortho photo...\n";

    // Iterate over each part of the mesh (one per material).
    for(size_t t = 0; t < mesh.tex_materials.size(); ++t)
    {
        // The material of the current submesh.
        pcl::TexMaterial material = mesh.tex_materials[t];
        texture = cv::imread(material.tex_file);
        
        // Check for missing files.
        if(texture.empty())
        {
            log_ << "Material texture could not be read:\n";
            log_ << material.tex_file << '\n';
            log_ << "Could not be read as image, does the file exist?\n";
            continue; // Skip to next material.
        }

        // The faces of the current submesh.
        std::vector<pcl::Vertices> faces = mesh.tex_polygons[t];

        // Iterate over each face...
        for(size_t faceIndex = 0; faceIndex < faces.size(); ++faceIndex)
        {
            // The current polygon.
            pcl::Vertices polygon = faces[faceIndex];

            // ... and draw it into the ortho photo.
            drawTexturedTriangle(texture, polygon, meshCloud, uvs, faceIndex+faceOff);
        }
        faceOff += faces.size();
        log_ << "Material " << t << " rendered.\n";
    }
    log_ << "...ortho photo rendered\n";

    log_ << '\n';
    log_ << "Writing ortho photo to " << outputFile_ << "\n";
    cv::imwrite(outputFile_, photo_);
    log_ << "Orthophoto generation done.\n";
}

void OdmOrthoPhoto::adjustBoundsForGeoRef()
{
    log_ << "Adjusting bounds for world coordinates\n";

    // A stream of the georef system.
    std::ifstream geoRefStream(inputGeoRefFile_.c_str());

    // The system name
    std::string system;
    // The false easting and northing
    int falseEasting, falseNorthing;

    // Parse file
    std::getline(geoRefStream, system);
    if(!(geoRefStream >> falseEasting))
    {
            throw OdmOrthoPhotoException("Could not extract geographical reference system from \n" + inputGeoRefFile_ + "\nCould not extract false easting.");
    }
    if(!(geoRefStream >> falseNorthing))
    {
            throw OdmOrthoPhotoException("Could not extract geographical reference system from \n" + inputGeoRefFile_ + "\nCould not extract false northing.");
    }

    log_ << "Georeference system\n";
    log_ << system << "\n";
    log_ << "False easting " << falseEasting << "\n";
    log_ << "False northing " << falseNorthing << "\n";

    // Adjust boundary points.
    boundaryPoint1_[0] = static_cast<float>(worldPoint1_.eastInteger_  - falseEasting)  + worldPoint1_.eastFractional_;
    boundaryPoint1_[1] = static_cast<float>(worldPoint1_.northInteger_ - falseNorthing) + worldPoint1_.northFractional_;
    boundaryPoint2_[0] = static_cast<float>(worldPoint2_.eastInteger_  - falseEasting)  + worldPoint2_.eastFractional_;
    boundaryPoint2_[1] = static_cast<float>(worldPoint2_.northInteger_ - falseNorthing) + worldPoint2_.northFractional_;
    boundaryPoint3_[0] = static_cast<float>(worldPoint3_.eastInteger_  - falseEasting)  + worldPoint3_.eastFractional_;
    boundaryPoint3_[1] = static_cast<float>(worldPoint3_.northInteger_ - falseNorthing) + worldPoint3_.northFractional_;
    boundaryPoint4_[0] = static_cast<float>(worldPoint4_.eastInteger_  - falseEasting)  + worldPoint4_.eastFractional_;
    boundaryPoint4_[1] = static_cast<float>(worldPoint4_.northInteger_ - falseNorthing) + worldPoint4_.northFractional_;

    log_ << "Local boundary points:\n";
    log_ << "Point 1: " << boundaryPoint1_[0] << " " << boundaryPoint1_[1] << "\n";
    log_ << "Point 2: " << boundaryPoint2_[0] << " " << boundaryPoint2_[1] << "\n";
    log_ << "Point 3: " << boundaryPoint3_[0] << " " << boundaryPoint3_[1] << "\n";
    log_ << "Point 4: " << boundaryPoint4_[0] << " " << boundaryPoint4_[1] << "\n";
}

void OdmOrthoPhoto::adjustBoundsForLocal()
{
    log_ << "Adjusting bounds for local coordinates\n";

    // Set boundary points from world points.
    boundaryPoint1_[0] = static_cast<float>(worldPoint1_.eastInteger_ )  + worldPoint1_.eastFractional_;
    boundaryPoint1_[1] = static_cast<float>(worldPoint1_.northInteger_) + worldPoint1_.northFractional_;
    boundaryPoint2_[0] = static_cast<float>(worldPoint2_.eastInteger_ )  + worldPoint2_.eastFractional_;
    boundaryPoint2_[1] = static_cast<float>(worldPoint2_.northInteger_) + worldPoint2_.northFractional_;
    boundaryPoint3_[0] = static_cast<float>(worldPoint3_.eastInteger_ )  + worldPoint3_.eastFractional_;
    boundaryPoint3_[1] = static_cast<float>(worldPoint3_.northInteger_) + worldPoint3_.northFractional_;
    boundaryPoint4_[0] = static_cast<float>(worldPoint4_.eastInteger_ )  + worldPoint4_.eastFractional_;
    boundaryPoint4_[1] = static_cast<float>(worldPoint4_.northInteger_) + worldPoint4_.northFractional_;

    log_ << "Local boundary points:\n";
    log_ << "Point 1: " << boundaryPoint1_[0] << " " << boundaryPoint1_[1] << "\n";
    log_ << "Point 2: " << boundaryPoint2_[0] << " " << boundaryPoint2_[1] << "\n";
    log_ << "Point 3: " << boundaryPoint3_[0] << " " << boundaryPoint3_[1] << "\n";
    log_ << "Point 4: " << boundaryPoint4_[0] << " " << boundaryPoint4_[1] << "\n";
    log_ << "\n";
}

void OdmOrthoPhoto::adjustBoundsForEntireModel(const pcl::TextureMesh &mesh)
{
    log_ << "Set boundary to contain entire model.\n";

    // The boundary of the model.
    float xMin, xMax, yMin, yMax;

    xMin = std::numeric_limits<float>::infinity();
    xMax = -std::numeric_limits<float>::infinity();
    yMin = std::numeric_limits<float>::infinity();
    yMax = -std::numeric_limits<float>::infinity();

    // Contains the vertices of the mesh.
    pcl::PointCloud<pcl::PointXYZ>::Ptr meshCloud (new pcl::PointCloud<pcl::PointXYZ>);
    pcl::fromPCLPointCloud2 (mesh.cloud, *meshCloud);

    for(size_t t = 0; t < mesh.tex_materials.size(); ++t)
    {
        // The faces of the current submesh.
        std::vector<pcl::Vertices> faces = mesh.tex_polygons[t];

        // Iterate over each face...
        for(size_t faceIndex = 0; faceIndex < faces.size(); ++faceIndex)
        {
            // The current polygon.
            pcl::Vertices polygon = faces[faceIndex];

            // The index to the vertices of the polygon.
            size_t v1i = polygon.vertices[0];
            size_t v2i = polygon.vertices[1];
            size_t v3i = polygon.vertices[2];

            // The polygon's points.
            pcl::PointXYZ v1 = meshCloud->points[v1i];
            pcl::PointXYZ v2 = meshCloud->points[v2i];
            pcl::PointXYZ v3 = meshCloud->points[v3i];

            xMin = std::min(std::min(xMin, v1.x), std::min(v2.x, v3.x));
            xMax = std::max(std::max(xMax, v1.x), std::max(v2.x, v3.x));
            yMin = std::min(std::min(yMin, v1.y), std::min(v2.y, v3.y));
            yMax = std::max(std::max(yMax, v1.y), std::max(v2.y, v3.y));
        }
    }

    // Create dummy boundary points.
    boundaryPoint1_[0] = xMin; boundaryPoint1_[1] = yMin;
    boundaryPoint2_[0] = xMin; boundaryPoint2_[1] = yMax;
    boundaryPoint3_[0] = xMax; boundaryPoint3_[1] = yMax;
    boundaryPoint4_[0] = xMax; boundaryPoint4_[1] = yMin;

    log_ << "Local boundary points:\n";
    log_ << "Point 1: " << boundaryPoint1_[0] << " " << boundaryPoint1_[1] << "\n";
    log_ << "Point 2: " << boundaryPoint2_[0] << " " << boundaryPoint2_[1] << "\n";
    log_ << "Point 3: " << boundaryPoint3_[0] << " " << boundaryPoint3_[1] << "\n";
    log_ << "Point 4: " << boundaryPoint4_[0] << " " << boundaryPoint4_[1] << "\n";
    log_ << "\n";
}

Eigen::Transform<float, 3, Eigen::Affine> OdmOrthoPhoto::getROITransform(float xMin, float yMin) const
{
    // The tranform used to move the chosen area into the ortho photo.
    Eigen::Transform<float, 3, Eigen::Affine> transform;

    transform(0, 0) = resolution_;     // x Scaling.
    transform(1, 0) = 0.0f;
    transform(2, 0) = 0.0f;
    transform(3, 0) = 0.0f;

    transform(0, 1) = 0.0f;
    transform(1, 1) = -resolution_;     // y Scaling, mirrored for easier rendering.
    transform(2, 1) = 0.0f;
    transform(3, 1) = 0.0f;

    transform(0, 2) = 0.0f;
    transform(1, 2) = 0.0f;
    transform(2, 2) = 1.0f;
    transform(3, 2) = 0.0f;

    transform(0, 3) = -xMin*resolution_;    // x Translation
    transform(1, 3) = -yMin*resolution_;    // y Translation
    transform(2, 3) = 0.0f;
    transform(3, 3) = 1.0f;

    return transform;
}

void OdmOrthoPhoto::drawTexturedTriangle(const cv::Mat &texture, const pcl::Vertices &polygon, const pcl::PointCloud<pcl::PointXYZ>::Ptr &meshCloud, const std::vector<Eigen::Vector2f> &uvs, size_t faceIndex)
{
    // The index to the vertices of the polygon.
    size_t v1i = polygon.vertices[0];
    size_t v2i = polygon.vertices[1];
    size_t v3i = polygon.vertices[2];

    // The polygon's points.
    pcl::PointXYZ v1 = meshCloud->points[v1i];
    pcl::PointXYZ v2 = meshCloud->points[v2i];
    pcl::PointXYZ v3 = meshCloud->points[v3i];

    if(isSliverPolygon(v1, v2, v3))
    {
        log_ << "Warning: Sliver polygon found at face index " << faceIndex << '\n';
        return;
    }

    // The face data. Position v*{x,y,z}. Texture coordinate v*{u,v}. * is the vertex number in the polygon.
    float v1x, v1y, v1z, v1u, v1v;
    float v2x, v2y, v2z, v2u, v2v;
    float v3x, v3y, v3z, v3u, v3v;

    // Barycentric coordinates of the currently rendered point.
    float l1, l2, l3;

    // The size of the photo, as float.
    float fRows, fCols;
    fRows = static_cast<float>(texture.rows);
    fCols = static_cast<float>(texture.cols);

    // Get vertex position.
    v1x = v1.x; v1y = v1.y; v1z = v1.z;
    v2x = v2.x; v2y = v2.y; v2z = v2.z;
    v3x = v3.x; v3y = v3.y; v3z = v3.z;

    // Get texture coorinates. (Special cases for PCL when using multiple materials vs one material)
    if(multiMaterial_)
    {
        v1u = uvs[3*faceIndex][0]; v1v = uvs[3*faceIndex][1];
        v2u = uvs[3*faceIndex+1][0]; v2v = uvs[3*faceIndex+1][1];
        v3u = uvs[3*faceIndex+2][0]; v3v = uvs[3*faceIndex+2][1];
    }
    else
    {
        v1u = uvs[v1i][0]; v1v = uvs[v1i][1];
        v2u = uvs[v2i][0]; v2v = uvs[v2i][1];
        v3u = uvs[v3i][0]; v3v = uvs[v3i][1];
    }

    // Check bounding box overlap.
    int xMin = static_cast<int>(std::min(std::min(v1x, v2x), v3x));
    if(xMin > photo_.cols)
    {
        return; // Completly outside to the right.
    }
    int xMax = static_cast<int>(std::max(std::max(v1x, v2x), v3x));
    if(xMax < 0)
    {
        return; // Completly outside to the left.
    }
    int yMin = static_cast<int>(std::min(std::min(v1y, v2y), v3y));
    if(yMin > photo_.rows)
    {
        return; // Completly outside to the top.
    }
    int yMax = static_cast<int>(std::max(std::max(v1y, v2y), v3y));
    if(yMax < 0)
    {
        return; // Completly outside to the bottom.
    }

    // Top point row and column positions
    float topR, topC;
    // Middle point row and column positions
    float midR, midC;
    // Bottom point row and column positions
    float botR, botC;

    // Find top, middle and bottom points.
    if(v1y < v2y)
    {
        if(v1y < v3y)
        {
            if(v2y < v3y)
            {
                // 1 -> 2 -> 3
                topR = v1y; topC = v1x;
                midR = v2y; midC = v2x;
                botR = v3y; botC = v3x;
            }
            else
            {
                // 1 -> 3 -> 2
                topR = v1y; topC = v1x;
                midR = v3y; midC = v3x;
                botR = v2y; botC = v2x;
            }
        }
        else
        {
            // 3 -> 1 -> 2
            topR = v3y; topC = v3x;
            midR = v1y; midC = v1x;
            botR = v2y; botC = v2x;
        }        
    }
    else // v2y <= v1y
    {
        if(v2y < v3y)
        {
            if(v1y < v3y)
            {
                // 2 -> 1 -> 3
                topR = v2y; topC = v2x;
                midR = v1y; midC = v1x;
                botR = v3y; botC = v3x;
            }
            else
            {
                // 2 -> 3 -> 1
                topR = v2y; topC = v2x;
                midR = v3y; midC = v3x;
                botR = v1y; botC = v1x;
            }
        }
        else
        {
            // 3 -> 2 -> 1
            topR = v3y; topC = v3x;
            midR = v2y; midC = v2x;
            botR = v1y; botC = v1x;
        }
    }

    // General appreviations:
    // ---------------------
    // tm : Top(to)Middle.
    // mb : Middle(to)Bottom.
    // tb : Top(to)Bottom.
    // c  : column.
    // r  : row.
    // dr : DeltaRow, step value per row.

    // The step along column for every step along r. Top to middle.
    float ctmdr;
    // The step along column for every step along r. Top to bottom.
    float ctbdr;
    // The step along column for every step along r. Middle to bottom.
    float cmbdr;

    ctbdr = (botC-topC)/(botR-topR);

    // The current column position, from top to middle.
    float ctm = topC;
    // The current column position, from top to bottom.
    float ctb = topC;

    // Check for vertical line between middle and top.
    if(FLT_EPSILON < midR-topR)
    {
        ctmdr = (midC-topC)/(midR-topR);

        // The first pixel row for the bottom part of the triangle.
        int rqStart = std::max(static_cast<int>(std::floor(topR+0.5f)), 0);
        // The last pixel row for the top part of the triangle.
        int rqEnd = std::min(static_cast<int>(std::floor(midR+0.5f)), photo_.rows);

        // Travers along row from top to middle.
        for(int rq = rqStart; rq < rqEnd; ++rq)
        {
            // Set the current column positions.
            ctm = topC + ctmdr*(static_cast<float>(rq)+0.5f-topR);
            ctb = topC + ctbdr*(static_cast<float>(rq)+0.5f-topR);

            // The first pixel column for the current row.
            int cqStart = std::max(static_cast<int>(std::floor(0.5f+std::min(ctm, ctb))), 0);
            // The last pixel column for the current row.
            int cqEnd = std::min(static_cast<int>(std::floor(0.5f+std::max(ctm, ctb))), photo_.cols);

            for(int cq = cqStart; cq < cqEnd; ++cq)
            {
                // Get barycentric coordinates for the current point.
                getBarycentricCoordiantes(v1, v2, v3, static_cast<float>(cq)+0.5f, static_cast<float>(rq)+0.5f, l1, l2, l3);
                
                if(0.f > l1 || 0.f > l2 || 0.f > l3)
                {
                    //continue;
                }
                
                // The z value for the point.
                float z = v1z*l1+v2z*l2+v3z*l3;

                // Check depth
                float depthValue = depth_.at<float>(rq, cq);
                if(z < depthValue)
                {
                    // Current is behind another, don't draw.
                    continue;
                }

                // The uv values of the point.
                float u, v;
                u = v1u*l1+v2u*l2+v3u*l3;
                v = v1v*l1+v2v*l2+v3v*l3;
                
                renderPixel(rq, cq, u*fCols, (1.0f-v)*fRows, texture);
                
                // Update depth buffer.
                depth_.at<float>(rq, cq) = z;
            }
        }
    }

    if(FLT_EPSILON < botR-midR)
    {
        cmbdr = (botC-midC)/(botR-midR);

        // The current column position, from middle to bottom.
        float cmb = midC;

        // The first pixel row for the bottom part of the triangle.
        int rqStart = std::max(static_cast<int>(std::floor(midR+0.5f)), 0);
        // The last pixel row for the bottom part of the triangle.
        int rqEnd = std::min(static_cast<int>(std::floor(botR+0.5f)), photo_.rows);

        // Travers along row from middle to bottom.
        for(int rq = rqStart; rq < rqEnd; ++rq)
        {
            // Set the current column positions.
            ctb = topC + ctbdr*(static_cast<float>(rq)+0.5f-topR);
            cmb = midC + cmbdr*(static_cast<float>(rq)+0.5f-midR);

            // The first pixel column for the current row.
            int cqStart = std::max(static_cast<int>(std::floor(0.5f+std::min(cmb, ctb))), 0);
            // The last pixel column for the current row.
            int cqEnd = std::min(static_cast<int>(std::floor(0.5f+std::max(cmb, ctb))), photo_.cols);

            for(int cq = cqStart; cq < cqEnd; ++cq)
            {
                // Get barycentric coordinates for the current point.
                getBarycentricCoordiantes(v1, v2, v3, static_cast<float>(cq)+0.5f, static_cast<float>(rq)+0.5f, l1, l2, l3);
                
                if(0.f > l1 || 0.f > l2 || 0.f > l3)
                {
                    //continue;
                }

                // The z value for the point.
                float z = v1z*l1+v2z*l2+v3z*l3;

                // Check depth
                float depthValue = depth_.at<float>(rq, cq);
                if(z < depthValue)
                {
                    // Current is behind another, don't draw.
                    continue;
                }

                // The uv values of the point.
                float u, v;
                u = v1u*l1+v2u*l2+v3u*l3;
                v = v1v*l1+v2v*l2+v3v*l3;

                renderPixel(rq, cq, u*fCols, (1.0f-v)*fRows, texture);

                // Update depth buffer.
                depth_.at<float>(rq, cq) = z;
            }
        }
    }
}

void OdmOrthoPhoto::renderPixel(int row, int col, float s, float t, const cv::Mat &texture)
{
    // The colors of the texture pixels. tl : top left, tr : top right, bl : bottom left, br : bottom right.
    cv::Vec3b tl, tr, bl, br;
    
    // The offset of the texture coordinate from its pixel positions.
    float leftF, topF;
    // The position of the top left pixel.
    int left, top;
    // The distance to the left and right pixel from the texture coordinate.
    float dl, dt;
    // The distance to the top and bottom pixel from the texture coordinate.
    float dr, db;
    
    dl = modff(s, &leftF);
    dr = 1.0f - dl;
    dt = modff(t, &topF);
    db = 1.0f - dt;
    
    left = static_cast<int>(leftF);
    top = static_cast<int>(topF);
    
    tl = texture.at<cv::Vec3b>(top, left);
    tr = texture.at<cv::Vec3b>(top, left+1);
    bl = texture.at<cv::Vec3b>(top+1, left);
    br = texture.at<cv::Vec3b>(top+1, left+1);
    
    // The interpolated color values.
    float r = 0.0f, g = 0.0f, b = 0.0f;
    
    // Red
    r += static_cast<float>(tl[2]) * dr * db;
    r += static_cast<float>(tr[2]) * dl * db;
    r += static_cast<float>(bl[2]) * dr * dt;
    r += static_cast<float>(br[2]) * dl * dt;
    
    // Green
    g += static_cast<float>(tl[1]) * dr * db;
    g += static_cast<float>(tr[1]) * dl * db;
    g += static_cast<float>(bl[1]) * dr * dt;
    g += static_cast<float>(br[1]) * dl * dt;
    
    // Blue
    b += static_cast<float>(tl[0]) * dr * db;
    b += static_cast<float>(tr[0]) * dl * db;
    b += static_cast<float>(bl[0]) * dr * dt;
    b += static_cast<float>(br[0]) * dl * dt;
    
    photo_.at<cv::Vec3b>(row,col) = cv::Vec3b(static_cast<unsigned char>(b), static_cast<unsigned char>(g), static_cast<unsigned char>(r));
}

void OdmOrthoPhoto::getBarycentricCoordiantes(pcl::PointXYZ v1, pcl::PointXYZ v2, pcl::PointXYZ v3, float x, float y, float &l1, float &l2, float &l3) const
{
    // Diff along y.
    float y2y3 = v2.y-v3.y;
    float y1y3 = v1.y-v3.y;
    float y3y1 = v3.y-v1.y;
    float yy3  =  y  -v3.y;
    
    // Diff along x.
    float x3x2 = v3.x-v2.x;
    float x1x3 = v1.x-v3.x;
    float xx3  =  x  -v3.x;
    
    // Normalization factor.
    float norm = (y2y3*x1x3 + x3x2*y1y3);
    
    l1 = (y2y3*(xx3) + x3x2*(yy3)) / norm;
    l2 = (y3y1*(xx3) + x1x3*(yy3)) / norm;
    l3 = 1 - l1 - l2;
}

bool OdmOrthoPhoto::isSliverPolygon(pcl::PointXYZ v1, pcl::PointXYZ v2, pcl::PointXYZ v3) const
{
    // Calculations are made using doubles, to minize rounding errors.
    Eigen::Vector3d a = Eigen::Vector3d(static_cast<double>(v1.x), static_cast<double>(v1.y), static_cast<double>(v1.z));
    Eigen::Vector3d b = Eigen::Vector3d(static_cast<double>(v2.x), static_cast<double>(v2.y), static_cast<double>(v2.z));
    Eigen::Vector3d c = Eigen::Vector3d(static_cast<double>(v3.x), static_cast<double>(v3.y), static_cast<double>(v3.z));
    Eigen::Vector3d dummyVec = (a-b).cross(c-b);

    // Area smaller than, or equal to, floating-point epsilon.
    return std::numeric_limits<float>::epsilon() >= static_cast<float>(std::sqrt(dummyVec.dot(dummyVec))/2.0);
}

bool OdmOrthoPhoto::isModelOk(const pcl::TextureMesh &mesh)
{
    // The number of texture coordinates in the model.
    size_t nTextureCoordinates = 0;
    // The number of faces in the model.
    size_t nFaces = 0;
    
    for(size_t t = 0; t < mesh.tex_coordinates.size(); ++t)
    {
        nTextureCoordinates += mesh.tex_coordinates[t].size();
    }
    for(size_t t = 0; t < mesh.tex_polygons.size(); ++t)
    {
        nFaces += mesh.tex_polygons[t].size();
    }
    
    log_ << "Number of faces in the model " << nFaces << '\n';
    
    return 3*nFaces == nTextureCoordinates;
}
