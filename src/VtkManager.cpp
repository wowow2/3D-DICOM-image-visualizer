#include "VtkManager.h"
// VTK Includes
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageActor.h>
#include <vtkImageSliceMapper.h>
#include <vtkMatrix4x4.h>
#include <vtkCamera.h>
#include <vtkImageProperty.h>
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkImageFlip.h>

// Math Library
#include <eigen3/Eigen/Dense>
#include <algorithm>

// Read numpy files
#include "cnpy.h"


// Constructs a VtkManager and initializes VTK components
VtkManager::VtkManager() {
    // Create a renderer for managing the scene
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    
    // Create a render window for displaying the rendered scene
    m_renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    // Create and configure image property for controlling slice appearance
    m_imageProperty = vtkSmartPointer<vtkImageProperty>::New();
    m_imageProperty->SetOpacity(0.7);
    m_imageProperty->SetColorWindow(1000);
    m_imageProperty->SetColorLevel(500);
    m_imageProperty->SetInterpolationTypeToLinear();
}

VtkManager::~VtkManager() {}

// Connects the VTK rendering pipeline to the Qt widget
void VtkManager::setup(QVTKOpenGLNativeWidget* widget) {
    widget->setRenderWindow(m_renderWindow);
    m_renderWindow->AddRenderer(m_renderer);
    m_renderer->SetBackground(0.1, 0.2, 0.4);
}

// Resets the camera to frame all objects in the scene and applies a zoom
void VtkManager::resetCamera() {
    m_renderer->ResetCamera();
    m_renderer->GetActiveCamera()->Zoom(1.5);
    m_renderWindow->Render();
}

// Creates a transformation matrix from DICOM metadata
vtkSmartPointer<vtkMatrix4x4> VtkManager::createTransformMatrix(const DicomFrame& frame) {
    // Extract orientation vectors from DICOM metadata
    Eigen::Vector3d row_vec(frame.imageOrientation[0], frame.imageOrientation[1], frame.imageOrientation[2]);
    Eigen::Vector3d col_vec(frame.imageOrientation[3], frame.imageOrientation[4], frame.imageOrientation[5]);

    // Extract paitient's position vector
    Eigen::Vector3d pos_vec(frame.imagePosition[0], frame.imagePosition[1], frame.imagePosition[2]);

    // Calculate normal vector, while not needed in this case its there for completeness
    Eigen::Vector3d normal_vec = row_vec.cross(col_vec);

    // Construct 4x4 transformation matrix
    Eigen::Matrix4d transform_eigen = Eigen::Matrix4d::Identity();
    transform_eigen.block<3,1>(0, 0) = row_vec;
    transform_eigen.block<3,1>(0, 1) = col_vec;
    transform_eigen.block<3,1>(0, 2) = normal_vec;
    transform_eigen.block<3,1>(0, 3) = pos_vec;

    // Convert Eigen matrix to VTK matrix
    auto vtk_matrix = vtkSmartPointer<vtkMatrix4x4>::New();
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            vtk_matrix->SetElement(i, j, transform_eigen(i, j));
        }
    }
    return vtk_matrix;
}

// Creates a contour actor from numpy contour data
vtkSmartPointer<vtkActor> VtkManager::createContourActor(const DicomFrame& frame) {
    // Check if contour file exists
    if (frame.contourFilePath.empty()) {
        return nullptr;
    }   

    // Validate numpy array shape (should be 2xN)
    cnpy::NpyArray arr = cnpy::npy_load(frame.contourFilePath);
    if (arr.shape.size() != 2 || arr.shape[0] != 2) {
        std::cerr << "Warning: Contour file " << frame.contourFilePath 
                  << " has incorrect shape. Expected (2, N).\n";
        return nullptr;
    }

    // Get pointer to contour data and number of points
    double* data = arr.data<double>();
    size_t num_points = arr.shape[1];

    // Need at least 2 points to form a contour
    if (num_points < 2) {
        return nullptr;
    }

    // Get transformation matrix for this frame
    vtkSmartPointer<vtkMatrix4x4> transform = createTransformMatrix(frame);

    // Create VTK data structures for storing contour geometry
    auto points = vtkSmartPointer<vtkPoints>::New();
    auto lines = vtkSmartPointer<vtkCellArray>::New();

    // Extract x and y coordinates from numpy array
    double* x_coords = data;
    double* y_coords = data + num_points;

    // Create a polyline connecting all contour points
    lines->InsertNextCell(num_points + 1);

    // Process each contour point
    for (size_t i = 0; i < num_points; ++i) {
        // Get pixel coordinates
        double pixel_x = x_coords[i];
        double pixel_y = y_coords[i];   
        
        // Convert pixel coordinates to millimeter coordinates
        double mm_x = pixel_x * frame.pixelSpacing[1];
        double mm_y = pixel_y * frame.pixelSpacing[0];

        // Transform from local coordinates to world coordinates
        double p_local[4] = {mm_x, mm_y, 0, 1.0};
        double p_world[4];
        transform->MultiplyPoint(p_local, p_world);
        
        // Add point to geometry
        points->InsertNextPoint(p_world[0], p_world[1], p_world[2]);
        lines->InsertCellPoint(i);
    }

    // Close the contour by connecting last point to first
    lines->InsertCellPoint(0);

    // Create polyData object to hold geometry
    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);
    polydata->SetLines(lines);

    // Create a mapper that takes the polydata as input and prepares it for rendering
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polydata);

    // Create actor to represent contour in the scene
    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    
    // Set contour appearance properties
    actor->GetProperty()->SetColor(1.0, 1.0, 0.0); // Yellow
    actor->GetProperty()->SetLineWidth(2.0);
    return actor;
}

// Creates a new scene from a set of DICOM frames
void VtkManager::createScene(const std::vector<DicomFrame>& frames) {
    // Clear previous scene
    m_renderer->RemoveAllViewProps();
    m_sliceActors.clear();
    m_contourActors.clear();

    // Process each DICOM frame
    for (const auto& frame : frames) {
        // Read DICOM image
        auto reader = vtkSmartPointer<vtkDICOMImageReader>::New();
        reader->SetFileName(frame.filePath.c_str());
        reader->Update();

        // Flip image vertically
        auto flipY = vtkSmartPointer<vtkImageFlip>::New();
        flipY->SetFilteredAxis(1); // axis 1 = Y
        flipY->SetInputConnection(reader->GetOutputPort());
        flipY->Update();
        
        // Create transformation matrix from DICOM metadata
        vtkSmartPointer<vtkMatrix4x4> transform = createTransformMatrix(frame);

        // Create image actor and mapper
        auto imageActor = vtkSmartPointer<vtkImageActor>::New();
        auto mapper = vtkSmartPointer<vtkImageSliceMapper>::New();

        // Connect mapper to image data
        mapper->SetInputConnection(flipY->GetOutputPort());

        // Configure image actor
        imageActor->SetMapper(mapper);
        imageActor->SetUserMatrix(transform);
        imageActor->SetScale(frame.pixelSpacing[1], frame.pixelSpacing[0], 1.0);
        imageActor->SetProperty(m_imageProperty);
        
        // Store and add to renderer
        m_sliceActors.push_back(imageActor);
        m_renderer->AddViewProp(imageActor);

        // Create and add contour if available
        vtkSmartPointer<vtkActor> contourActor = createContourActor(frame);
        if (contourActor) {
            m_contourActors.push_back(contourActor);
            m_renderer->AddViewProp(contourActor);
        }
    }
}

// Sets the opacity of all image slices
void VtkManager::setSliceOpacity(double opacity) {
    if (m_imageProperty) {
        // Clamp opacity to valid range [0, 1]
        double clamped_opacity = std::max(0.0, std::min(1.0, opacity));
        m_imageProperty->SetOpacity(clamped_opacity);

        // Trigger re-rendering if we have a valid render window
        if (m_renderWindow && m_renderWindow->GetRenderers()->GetNumberOfItems() > 0) {
            m_renderWindow->Render();
        }
    }
}
