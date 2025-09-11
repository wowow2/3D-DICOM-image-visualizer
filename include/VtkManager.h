#pragma once

#include <vtkSmartPointer.h>
#include <vector>
#include "DicomManager.h" // Include DicomManager to get the DicomFrame definition

// Forward declarations to keep this header lightweight.
class vtkRenderer;                   // VTK class for managing the rendering process
class vtkGenericOpenGLRenderWindow;  // VTK render window for OpenGL rendering
class vtkImageActor;                 // VTK actor for displaying image data
class vtkMatrix4x4;                  // VTK class for 4x4 transformation matrices
class QVTKOpenGLNativeWidget;        // Qt widget that embeds VTK rendering
class vtkImageProperty;              // VTK class for controlling image appearance properties
class vtkActor;                      // VTK base class for objects in the rendered scene

class VtkManager {
public:
    // Constructor and Destructor
    VtkManager();
    ~VtkManager();

    // Connects VTK rendering pipeline to the Qt GUI widget. Called once on start up.
    void setup(QVTKOpenGLNativeWidget* widget);

    // Clears scence and builds a new one from the vector of Dicom frames
    void createScene(const std::vector<DicomFrame>& frames);
    
    // Resets the camera to frame all the actors in the scene.
    void resetCamera();

    // Sets the opacity value of slices
    void setSliceOpacity(double opacity);


private:
    // Creates transformation matrix from position and orientation data
    vtkSmartPointer<vtkMatrix4x4> createTransformMatrix(const DicomFrame& frame);

    // Core VTK rendering objects
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;

    // A single property object to control the appearance of all slices
    vtkSmartPointer<vtkImageProperty> m_imageProperty;

    // Create contour actors
    vtkSmartPointer<vtkActor> createContourActor(const DicomFrame& frame);

    // Track contour actors
    std::vector<vtkSmartPointer<vtkActor>> m_contourActors;

    // A list to keep track of the actors we've added to the scene
    std::vector<vtkSmartPointer<vtkImageActor>> m_sliceActors;
};