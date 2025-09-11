#include "DicomManager.h"

#include <iostream>
#include <filesystem>
#include <algorithm>

// DCMTK Headers
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcostrmz.h"

namespace fs = std::filesystem;

// Constructor and Destructor
DicomManager::DicomManager() {}
DicomManager::~DicomManager() {}

//
void DicomManager::clear() {
    m_seriesMap.clear();
}

// Discovers all potential DICOM series in a patient directory
std::vector<std::string> DicomManager::discoverSeries(const std::string& patientPath) {
    std::vector<std::string> seriesNames;
    if (!fs::exists(patientPath) || !fs::is_directory(patientPath)) {
        std::cerr << "Error: Patient path is not a valid directory: " << patientPath << std::endl;
        return seriesNames; // Return empty list
    }

    // Find all sub-directories and assume they are series
    for (const auto& entry : fs::directory_iterator(patientPath)) {
        if (entry.is_directory()) {
            seriesNames.push_back(entry.path().filename().string());
        }
    }
    std::sort(seriesNames.begin(), seriesNames.end()); // Sort alphabetically
    return seriesNames;
}

// Loads DICOM data from selected series directories
bool DicomManager::loadSelectedSeries(const std::string& patientPath, const std::vector<std::string>& seriesNames) {
    clear(); // Clear previous data before loading new data

    for (const auto& name : seriesNames) {
        // Reconstruct the full path to the series directory
        fs::path seriesPath = fs::path(patientPath) / name;
        if (!fs::is_directory(seriesPath)) continue;

        DicomSeries currentSeries;
        
        // Loop through the files in this series directory
        for (const auto& fileEntry : fs::directory_iterator(seriesPath.string())) {
            if (!fileEntry.is_regular_file() || fileEntry.path().extension() != ".dcm") {
                continue; // Skip non-DICOM files
            }

            std::string filePath = fileEntry.path().string();
            DcmFileFormat fileformat;

            if (fileformat.loadFile(filePath.c_str()).good()) {
                DcmDataset *dataset = fileformat.getDataset();
                DicomFrame frame; // Initialize dicom frame
                frame.filePath = filePath; // Set filepath

                // Extract essential tags
                OFString value;
                bool success = true;

                if (dataset->findAndGetOFStringArray(DCM_ImagePositionPatient, value).good()) {
                    // (X, Y, Z) coordinates of paitient origin
                    sscanf(value.c_str(), "%lf\\%lf\\%lf", &frame.imagePosition[0], &frame.imagePosition[1], &frame.imagePosition[2]);
                } else { success = false; }
                
                if (dataset->findAndGetOFStringArray(DCM_ImageOrientationPatient, value).good()) {
                    // First three are left to right cosines, and next 3 are up and down cosines
                    sscanf(value.c_str(), "%lf\\%lf\\%lf\\%lf\\%lf\\%lf", &frame.imageOrientation[0], &frame.imageOrientation[1], &frame.imageOrientation[2], &frame.imageOrientation[3], &frame.imageOrientation[4], &frame.imageOrientation[5]);
                } else { success = false; }

                if (dataset->findAndGetOFStringArray(DCM_PixelSpacing, value).good()) {
                    // Gets pixel spacing values as (y,x)
                    sscanf(value.c_str(), "%lf\\%lf", &frame.pixelSpacing[0], &frame.pixelSpacing[1]);
                } else { success = false; }
                
                // Temporal position in series
                Sint32 instNum = 0;
                if (!dataset->findAndGetSint32(DCM_InstanceNumber, instNum).good()) { success = false; }
                frame.instanceNumber = instNum;
                
                // Extract Image Dimensions
                Uint16 rows, cols;
                if (!dataset->findAndGetUint16(DCM_Rows, rows).good()) { success = false; }
                if (!dataset->findAndGetUint16(DCM_Columns, cols).good()) { success = false; }
                frame.rows = rows;
                frame.cols = cols;

                // Only add the frame if all essential tags were found
                if (success) {
                    // Find corresponding contour file
                    fs::path dcmPath(filePath);
                    std::string contourName = dcmPath.stem().string() + "_cont.npy";
                    fs::path contourPath = dcmPath.parent_path() / contourName;
                    if (fs::exists(contourPath)) {
                        frame.contourFilePath = contourPath.string();
                    }
                    currentSeries.push_back(frame);
                }
            }
        }

        // Store series if any valid frames were found
        if (!currentSeries.empty()) {
            std::sort(currentSeries.begin(), currentSeries.end());
            m_seriesMap[seriesPath.string()] = currentSeries;
        }
    }

    return !m_seriesMap.empty();
}

// Retrieves frames from all series at a specific time index
std::vector<DicomFrame> DicomManager::getFramesForTimepoint(int timeIndex) const {
    std::vector<DicomFrame> frames;

    // Extract frame from a specific timeFrame
    for (const auto& pair : m_seriesMap) {
        const DicomSeries& series = pair.second;
        if (timeIndex >= 0 && timeIndex < series.size()) {
            frames.push_back(series[timeIndex]);
        }
    }
    // Sort the final list of slices by their Z position to ensure correct stacking for SA view
    std::sort(frames.begin(), frames.end(), [](const DicomFrame& a, const DicomFrame& b) {
        return a.imagePosition[2] < b.imagePosition[2];
    });
    return frames;
}

// Finds the maximum number of frames across all loaded series
int DicomManager::getNumberOfFrames() const {
    size_t maxFrames = 0; // Use size_t for size comparison
    if (m_seriesMap.empty()) {
        return 0;
    }
    for (const auto& pair : m_seriesMap) {
        if (pair.second.size() > maxFrames) {
            maxFrames = pair.second.size();
        }
    }
    return static_cast<int>(maxFrames);
}