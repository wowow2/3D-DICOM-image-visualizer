#pragma once

#include <string>
#include <vector>
#include <map>

// Represents a frame in a DICOM series, including the contour
struct DicomFrame {
    std::string filePath;
    std::string contourFilePath; 
    int instanceNumber = 0; // frame number
    
    // DICOM tags, pre-sized with default values for safety
    std::vector<double> imagePosition{0.0, 0.0, 0.0};
    std::vector<double> imageOrientation{1.0, 0.0, 0.0, 0.0, 1.0, 0.0};
    std::vector<double> pixelSpacing{1.0, 1.0};
    
    // Image dims
    int rows = 0;
    int cols = 0;

    // This allows us to sort a vector of DicomFrames by their time step.
    bool operator<(const DicomFrame& other) const {
        return instanceNumber < other.instanceNumber;
    }
};

// alias for a vector of DicomFrames, representing a single time series.
using DicomSeries = std::vector<DicomFrame>;

// Manages all DICOM file discovery, parsing, and data organization.
class DicomManager {
public:
    DicomManager();
    ~DicomManager();

    // Scans paitient directory , finds all subfolders and returns a vector of all subfolders
    std::vector<std::string> discoverSeries(const std::string& patientPath);

    // Loads the selected series, each file is parsed and if any dicom series are read we return True
    bool loadSelectedSeries(const std::string& patientPath, const std::vector<std::string>& seriesNames);

    // Gathers slices from a specific frame from a series
    std::vector<DicomFrame> getFramesForTimepoint(int timeIndex) const;
    
    // return length of longest time series
    int getNumberOfFrames() const;
    
    // clear previous data
    void clear();

private:
    // Stores all loaded series data, keyed by the full path to the series folder.
    std::map<std::string, DicomSeries> m_seriesMap;
};