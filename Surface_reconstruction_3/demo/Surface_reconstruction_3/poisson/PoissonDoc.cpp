// PoissonDoc.cpp : implementation of the CPoissonDoc class
//

// This demo + Gyroviz
#include "stdafx.h"
#include "Poisson.h"
#include "DialogOptions.h"
#include "PoissonDoc.h"
#include "enriched_polyhedron.h"
#include "surface_reconstruction_read_pwc.h"
#include "surface_reconstruction_read_g23.h"
#include "remove_outliers_wrt_camera_cone_angle_3.h"
#include "orient_normals_wrt_cameras_3.h"

// CGAL
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/IO/File_scanner_OFF.h>
#include <CGAL/Timer.h>
#include <CGAL/Memory_sizer.h>

//Warning: crash when using #define CGAL_C2T3_USE_POLYHEDRON
#define CGAL_C2T3_USE_FILE_WRITER_OFF
#include <CGAL/IO/Complex_2_in_triangulation_3_file_writer.h>

// This package
#include <CGAL/IO/surface_reconstruction_read_off_point_cloud.h>
#include <CGAL/IO/surface_reconstruction_write_off_point_cloud.h>
#include <CGAL/IO/surface_reconstruction_read_pnb.h>
#include <CGAL/IO/surface_reconstruction_read_pwn.h>
#include <CGAL/IO/surface_reconstruction_write_pwn.h>
#include <CGAL/IO/surface_reconstruction_read_xyz.h>
#include <CGAL/IO/surface_reconstruction_write_xyz.h>
#include <CGAL/IO/surface_reconstruction_output.h>
#include <CGAL/remove_outliers_wrt_avg_knn_sq_distance_3.h>
#include <CGAL/estimate_normals_jet_fitting_3.h>
#include <CGAL/smooth_jet_fitting_3.h>
#include <CGAL/estimate_normals_pca_3.h>
#include <CGAL/average_spacing_3.h>
#include <CGAL/merge_epsilon_nearest_points_3.h>
#include <CGAL/random_simplification_points_3.h>
#include <CGAL/Peak_memory_sizer.h>
#include <CGAL/surface_reconstruction_assertions.h>

// STL
#include <iostream>
#include <fstream>
#include <cassert>
#include <math.h>
#ifndef M_PI
  #define M_PI       3.14159265358979323846
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPoissonDoc

IMPLEMENT_DYNCREATE(CPoissonDoc, CDocument)

BEGIN_MESSAGE_MAP(CPoissonDoc, CDocument)
    ON_COMMAND(ID_FILE_SAVE_SURFACE, OnFileSaveSurface)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_SURFACE, OnUpdateFileSaveSurface)
    ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
    ON_COMMAND(ID_EDIT_OPTIONS, OnEditOptions)
    ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
    ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
    ON_COMMAND(ID_EDIT_RESET_SELECTION, OnEditResetSelection)
    ON_UPDATE_COMMAND_UI(ID_EDIT_RESET_SELECTION, OnUpdateEditResetSelection)
    ON_COMMAND(ID_RECONSTRUCTION_DELAUNAYREFINEMENT, OnReconstructionDelaunayRefinement)
    ON_COMMAND(ID_RECONSTRUCTION_POISSON, OnReconstructionPoisson)
	ON_COMMAND(ID_RECONSTRUCTION_POISSON_NORMALIZED, OnReconstructionPoissonNormalized)
    ON_COMMAND(ID_ALGORITHMS_REFINEINSHELL, OnAlgorithmsRefineInShell)
    ON_COMMAND(ID_RECONSTRUCTION_POISSON_SURFACE_MESHING, OnReconstructionPoissonSurfaceMeshing)
    ON_UPDATE_COMMAND_UI(ID_RECONSTRUCTION_POISSON, OnUpdateReconstructionPoisson)
    ON_UPDATE_COMMAND_UI(ID_RECONSTRUCTION_POISSON_SURFACE_MESHING, OnUpdateReconstructionPoissonSurfaceMeshing)
    ON_COMMAND(ID_ALGORITHMS_MARCHINGTETCONTOURING, OnAlgorithmsMarchingTetContouring)
    ON_UPDATE_COMMAND_UI(ID_ALGORITHMS_MARCHINGTETCONTOURING, OnUpdateAlgorithmsMarchingTetContouring)
    ON_COMMAND(ID_ALGORITHMS_EXTRAPOLATENORMALS, OnAlgorithmsExtrapolatenormals)
    ON_COMMAND(ID_ALGORITHMS_POISSONSTATISTICS, OnAlgorithmsPoissonStatistics)
    ON_UPDATE_COMMAND_UI(ID_ALGORITHMS_POISSONSTATISTICS, OnUpdateAlgorithmsPoissonStatistics)
    ON_COMMAND(ID_ALGORITHMS_ESTIMATENORMALSBYPCA, OnAlgorithmsEstimateNormalsByPCA)
    ON_COMMAND(ID_ALGORITHMS_ESTIMATENORMALBYJETFITTING, OnAlgorithmsEstimateNormalsByJetFitting)
    ON_COMMAND(ID_ALGORITHMS_ORIENTNORMALSCAMERAS, OnAlgorithmsOrientNormalsWrtCameras)
    ON_COMMAND(ID_ALGORITHMS_ORIENTNORMALSMST, OnAlgorithmsOrientNormalsWithMST)
	ON_COMMAND(ID_ALGORITHMS_SMOOTHUSINGJETFITTING, OnAlgorithmsSmoothUsingJetFitting)
    ON_COMMAND(ID_MODE_POINT_SET, OnModePointSet)
    ON_UPDATE_COMMAND_UI(ID_MODE_POINT_SET, OnUpdateModePointSet)
    ON_COMMAND(ID_MODE_POISSON, OnModePoisson)
    ON_UPDATE_COMMAND_UI(ID_MODE_POISSON, OnUpdateModePoisson)
    ON_COMMAND(ID_CREATE_POISSON_TRIANGULATION, OnCreatePoissonTriangulation)
    ON_UPDATE_COMMAND_UI(ID_CREATE_POISSON_TRIANGULATION, OnUpdateCreatePoissonTriangulation)
    ON_UPDATE_COMMAND_UI(ID_ALGORITHMS_SMOOTHUSINGJETFITTING, OnUpdateAlgorithmsSmoothUsingJetFitting)
    ON_UPDATE_COMMAND_UI(ID_ALGORITHMS_ESTIMATENORMALSBYPCA, OnUpdateAlgorithmsEstimateNormalsByPCA)
    ON_UPDATE_COMMAND_UI(ID_ALGORITHMS_ESTIMATENORMALBYJETFITTING, OnUpdateAlgorithmsEstimateNormalByJetFitting)
    ON_UPDATE_COMMAND_UI(ID_ALGORITHMS_ORIENTNORMALSMST, OnUpdateAlgorithmsOrientNormalsWithMST)
    ON_UPDATE_COMMAND_UI(ID_ALGORITHMS_ORIENTNORMALSCAMERAS, OnUpdateAlgorithmsOrientNormalsWrtCameras)
    ON_UPDATE_COMMAND_UI(ID_RECONSTRUCTION_DELAUNAYREFINEMENT, OnUpdateReconstructionDelaunayRefinement)
    ON_UPDATE_COMMAND_UI(ID_ALGORITHMS_REFINEINSHELL, OnUpdateAlgorithmsRefineInShell)
    ON_UPDATE_COMMAND_UI(ID_ALGORITHMS_EXTRAPOLATENORMALS, OnUpdateAlgorithmsExtrapolateNormals)
    ON_COMMAND(ID_ALGORITHMS_OUTLIERS_REMOVAL_WRT_CAMERAS_CONE_ANGLE, OnAlgorithmsOutliersRemovalWrtCamerasConeAngle)
    ON_UPDATE_COMMAND_UI(ID_ALGORITHMS_OUTLIERS_REMOVAL_WRT_CAMERAS_CONE_ANGLE, OnUpdateAlgorithmsOutliersRemovalWrtCamerasConeAngle)
    ON_COMMAND(ID_ALGORITHMS_OUTLIERS_REMOVAL_WRT_AVG_KNN_SQ_DIST, OnOutliersRemovalWrtAvgKnnSqDist)
    ON_UPDATE_COMMAND_UI(ID_ALGORITHMS_OUTLIERS_REMOVAL_WRT_AVG_KNN_SQ_DIST, OnUpdateOutliersRemovalWrtAvgKnnSqDist)
    ON_COMMAND(ID_ANALYSIS_AVERAGE_SPACING, OnAnalysisAverageSpacing)
    ON_COMMAND(ID_ONE_STEP_POISSON_RECONSTRUCTION, OnOneStepPoissonReconstruction)
	ON_COMMAND(ID_RECONSTRUCTION_ONE_STEP_NORMALIZED, OnOneStepPoissonReconstructionWithNormalizedDivergence)
    ON_UPDATE_COMMAND_UI(ID_ONE_STEP_POISSON_RECONSTRUCTION, OnUpdateOneStepPoissonReconstruction)
    ON_UPDATE_COMMAND_UI(ID_ANALYSIS_AVERAGE_SPACING, OnUpdateAnalysisAverageSpacing)
    ON_COMMAND(ID_RECONSTRUCTION_APSS_RECONSTRUCTION, OnReconstructionApssReconstruction)
    ON_UPDATE_COMMAND_UI(ID_RECONSTRUCTION_APSS_RECONSTRUCTION, OnUpdateReconstructionApssReconstruction)
    ON_COMMAND(ID_MODE_APSS, OnModeAPSS)
    ON_UPDATE_COMMAND_UI(ID_MODE_APSS, OnUpdateModeAPSS)
	ON_COMMAND(ID_RECONSTRUCTION_SAVEAS, &CPoissonDoc::OnReconstructionSaveas)
	ON_COMMAND(ID_STEP_CALCULATEAVERAGESPACING, OnCalculateAverageSpacing)
	ON_COMMAND(ID_STEP_EXTRAPOLATENORMALSUSINGGAUSSIANKERNELS,OnExtrapolateNormalsUsingGaussianKernel)
    ON_COMMAND(ID_POINT_CLOUD_SIMPLIFICATION_BY_CLUSTERING, &CPoissonDoc::OnPointCloudSimplificationByClustering)
    ON_UPDATE_COMMAND_UI(ID_POINT_CLOUD_SIMPLIFICATION_BY_CLUSTERING, &CPoissonDoc::OnUpdatePointCloudSimplificationByClustering)
    ON_COMMAND(ID_POINT_CLOUD_SIMPLIFICATION_RANDOM, &CPoissonDoc::OnPointCloudSimplificationRandom)
    ON_UPDATE_COMMAND_UI(ID_POINT_CLOUD_SIMPLIFICATION_RANDOM, &CPoissonDoc::OnUpdatePointCloudSimplificationRandom)
END_MESSAGE_MAP()


// CPoissonDoc construction/destruction

CPoissonDoc::CPoissonDoc()
: m_poisson_function(NULL),
  m_poisson_dt(NULL),
  m_apss_function(NULL),
  m_surface_mesher_c2t3(m_surface_mesher_dt)
{
  m_edit_mode = NO_EDIT_MODE; // No points yet

  m_triangulation_refined = false; // Need to apply Delaunay refinement
  m_poisson_solved = false; // Need to solve Poisson equation

  // Surface mesher options
  m_sm_angle = 20.0; // theorical guaranty if angle >= 30, but slower
  m_sm_radius = 0.1; // as suggested by LR
  m_sm_error_bound = 1e-3;

  // Poisson options
  m_sm_distance_poisson = 0.002; // AG: was 0.005 (upper bound of distance to surface/Poisson)
  m_dr_shell_size = 0.01; // 3 Delaunay refinements options
  m_dr_sizing = 0.5 * m_dr_shell_size;
  m_dr_max_vertices = (unsigned int)5e6;
  m_contouring_value = 0.0; // Poisson contouring value; should be 0 (TEST)
  m_lambda = 0.1;

  // APSS options
  m_sm_distance_apss = 0.005; // Upper bound of distance to surface (APSS).
                              // Note: 1.5 * Poisson's distance gives roughly the same number of triangles.

  // K-nearest neighbours options
  m_number_of_neighbours = 7;

  // Outliers removal
  m_min_cameras_cone_angle = 0.5; // min angle of camera's cone (degrees)
  m_threshold_percent_avg_knn_sq_dst = 5.0; // percentage of outliers to remove

  // Point set simplification
  m_clustering_step = 0.004; // Grid's step for simplification by clustering
  m_random_simplification_percentage = 50.0; // percentage of random points to remove
}

CPoissonDoc::~CPoissonDoc()
{
  // Clean up current mode
  CloseMode();
}

// CPoissonDoc diagnostics

#ifdef _DEBUG
void CPoissonDoc::AssertValid() const
{
  CDocument::AssertValid();
}

void CPoissonDoc::Dump(CDumpContext& dc) const
{
  CDocument::Dump(dc);
}
#endif //_DEBUG


// File >> Open implementation
BOOL CPoissonDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
  CGAL::Timer task_timer; task_timer.start();

  if (!CDocument::OnOpenDocument(lpszPathName))
    return FALSE;

  status_message("Load point set %s...",lpszPathName);

  // get extension
  CString file = lpszPathName;
  CString extension = lpszPathName;
  extension = extension.Right(4);
  extension.MakeLower();

  // set current path
  CString path = lpszPathName;
  path = path.Left(path.ReverseFind('\\'));
  SetCurrentDirectory(path);

  // if .off extension
  if(extension.CompareNoCase(".off") == 0)
  {
    // Is this OFF file a mesh or a point cloud?
    std::ifstream header_stream(lpszPathName);
    CGAL::File_scanner_OFF header(header_stream, true /* verbose */);
    if(!header_stream || header.size_of_vertices() == 0)
    {
      prompt_message("Unable to read file");
      return FALSE;
    }
    bool is_mesh = (header.size_of_facets() > 0);
    header_stream.close();

    // Read OFF file as a mesh and compute normals from connectivity
    if (is_mesh)
    {
      // read file in polyhedron
      typedef Enriched_polyhedron<Kernel> Polyhedron;
      Polyhedron input_mesh;
      std::ifstream file_stream(lpszPathName);
      CGAL::scan_OFF(file_stream, input_mesh, true /* verbose */);
      if(!file_stream || !input_mesh.is_valid() || input_mesh.empty())
      {
        prompt_message("Unable to read file");
        return FALSE;
      }

      // Compute normals using mesh connectivity
      input_mesh.compute_normals();

      // Copy points to m_points
      Polyhedron::Vertex_iterator v;
      for(v = input_mesh.vertices_begin();
          v != input_mesh.vertices_end();
          v++)
      {
        const Point& p = v->point();
        const Vector& n = v->normal();
        m_points.push_back(Point_with_normal(p,n));
      }
    }
    else // Read OFF file as a point cloud
    {
      if( ! CGAL::surface_reconstruction_read_off_point_cloud(lpszPathName,
                                                              std::back_inserter(m_points)) )
      {
        prompt_message("Unable to read file");
        return FALSE;
      }
    }
  }
  // if .pwn extension
  else if(extension.CompareNoCase(".pwn") == 0)
  {
    if( ! CGAL::surface_reconstruction_read_pwn(lpszPathName,
                                                std::back_inserter(m_points)) )
    {
      prompt_message("Unable to read file");
      return FALSE;
    }
  }
  // if .xyz extension
  else if(extension.CompareNoCase(".xyz") == 0)
  {
    if( ! CGAL::surface_reconstruction_read_xyz(lpszPathName,
                                                std::back_inserter(m_points)) )
    {
      prompt_message("Unable to read file");
      return FALSE;
    }
    //
  }
  // if .pnb extension
  else if(extension.CompareNoCase(".pnb") == 0)
  {
    if( ! CGAL::surface_reconstruction_read_pnb(lpszPathName,
                                                std::back_inserter(m_points)) )
    {
      prompt_message("Unable to read file");
      return FALSE;
    }
  }
  // if Gyroviz .pwc extension
  else if (extension.CompareNoCase(".pwc") == 0)
  {
    std::vector<Point> cameras; // temporary container of cameras to read
    if( ! surface_reconstruction_read_pwc(lpszPathName,
                                          std::back_inserter(m_points),
                                          std::back_inserter(cameras)) )
    {
      prompt_message("Unable to read file");
      return FALSE;
    }

    // Set options for Gyroviz file
    m_number_of_neighbours = 50;
  }
  // if Gyroviz .g23 extension
  else if (extension.CompareNoCase(".g23") == 0)
  {
    std::string movie_file_name;
    std::vector<Point> cameras; // temporary container of cameras to read
    if( ! surface_reconstruction_read_g23(lpszPathName,
                                          std::back_inserter(m_points),
                                          std::back_inserter(cameras),
                                          &movie_file_name) )
    {
      prompt_message("Unable to read file");
      return FALSE;
    }

    // Set options for Gyroviz file
    m_number_of_neighbours = 50;
  }
  // if unknown extension
  else
  {
    prompt_message("File format not supported");
    return FALSE;
  }

  m_points.invalidate_bounding_box();
  m_edit_mode = POINT_SET;

  status_message("Load point set...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  return TRUE;
}

// Save input point set as...  callback
void CPoissonDoc::OnFileSaveAs()
{
  // file filters
  CString szFilter;
  szFilter  = "Points With Normals Files (*.pwn)|*.pwn|";
  szFilter += "XYZ Files (*.xyz)|*.xyz|";
  szFilter += "OFF Files (*.off)|*.off|";
  szFilter += "All Files (*.*)|*.*||";

  // create the Save As dialog
  CFileDialog dlgExport(false, "pwn", NULL,
                        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, AfxGetMainWnd());

  // dialog title
  dlgExport.m_ofn.lpstrTitle = "Save reconstructed surface to file";

  // show the dialog
  if (dlgExport.DoModal() == IDOK)
  {
    // get extension
    CString file = dlgExport.m_ofn.lpstrFile;
    CString extension = dlgExport.m_ofn.lpstrFile;
    extension = extension.Right(4);
    extension.MakeLower();

    // set current path
    CString path = dlgExport.m_ofn.lpstrFile;
    path = path.Left(path.ReverseFind('\\'));
    SetCurrentDirectory(path);

    // if .pwn extension
    if(extension.CompareNoCase(".pwn") == 0)
    {
      if( ! CGAL::surface_reconstruction_write_pwn(dlgExport.m_ofn.lpstrFile,
                                                   m_points.begin(), m_points.end()) )
      {
        prompt_message("Unable to save file");
        return;
      }
    }
    // if .xyz extension
    else if(extension.CompareNoCase(".xyz") == 0)
    {
      if( ! CGAL::surface_reconstruction_write_xyz(dlgExport.m_ofn.lpstrFile,
                                                   m_points.begin(), m_points.end()) )
      {
        prompt_message("Unable to save file");
        return;
      }
    }
    // if .off extension
    else if(extension.CompareNoCase(".off") == 0)
    {
      if( ! CGAL::surface_reconstruction_write_off_point_cloud(dlgExport.m_ofn.lpstrFile,
                                                               m_points.begin(), m_points.end()) )
      {
        prompt_message("Unable to save file");
        return;
      }
    }
    else
    {
      prompt_message("File format not supported");
      return;
    }
  }
}

// Save m_points[] only if it is the form visible on screen
void CPoissonDoc::OnUpdateFileSaveAs(CCmdUI *pCmdUI)
{
  assert(m_points.begin() != m_points.end());
  pCmdUI->Enable(m_edit_mode == POINT_SET);
}

// Save reconstructed surface as...  callback
void CPoissonDoc::OnFileSaveSurface()
{
  // file filters
  static char szFilter[] = "OFF Files (*.off)|*.off; *.off|All Files (*.*)|*.*||";

  // create the Save As dialog
  CFileDialog dlgExport(false, "off", NULL,
                        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, AfxGetMainWnd());

  // dialog title
  dlgExport.m_ofn.lpstrTitle = "Save reconstructed surface to file";

  // show the dialog
  if (dlgExport.DoModal() == IDOK)
  {
    // get extension
    CString file = dlgExport.m_ofn.lpstrFile;
    CString extension = dlgExport.m_ofn.lpstrFile;
    extension = extension.Right(4);
    extension.MakeLower();

    // set current path
    CString path = dlgExport.m_ofn.lpstrFile;
    path = path.Left(path.ReverseFind('\\'));
    SetCurrentDirectory(path);

    // if .off extension
    if(extension.CompareNoCase(".off") == 0)
    {
      std::ofstream out((char *)dlgExport.m_ofn.lpstrFile);
      if( !out )
      {
        prompt_message("Unable to save file");
        return;
      }

      CGAL::output_surface_facets_to_off(out, m_surface_mesher_c2t3);
    }
    else
    {
      prompt_message("File format not supported");
      return;
    }
  }
}

// Enable "Save reconstructed surface as..." if surface is computed
void CPoissonDoc::OnUpdateFileSaveSurface(CCmdUI *pCmdUI)
{
  pCmdUI->Enable((m_edit_mode == POISSON || m_edit_mode == APSS)
                 && m_surface_mesher_dt.number_of_vertices() > 0);
}

// Update the number of vertices and tetrahedra in the status bar
// and write them to cerr.
void CPoissonDoc::update_status()
{
  CStatusBar* pStatus =
    (CStatusBar*)AfxGetApp()->m_pMainWnd->GetDescendantWindow(AFX_IDW_STATUS_BAR);
  ASSERT(pStatus != NULL);

  if (m_edit_mode == POINT_SET || m_edit_mode == APSS)
  {
    CString points;
    points.Format("%d points",m_points.size());
    CString selected_points;
    selected_points.Format("%d selected",m_points.nb_selected_points());

    // write message to cerr
    std::cerr << "=> " << points << " (" << selected_points << "), "
                       << (CGAL::Memory_sizer().virtual_size()>>20) << " Mb allocated, "
                       << "largest free block=" << (CGAL::Peak_memory_sizer().largest_free_block()>>20) << " Mb, "
                       << "#blocks over 100 Mb=" << CGAL::Peak_memory_sizer().count_free_memory_blocks(100*1048576)
                       << std::endl;

    // Update status bar
    pStatus->SetPaneText(1,points);
    pStatus->SetPaneText(2,selected_points);
    pStatus->UpdateWindow();
  }
  else if (m_edit_mode == POISSON)
  {
    assert(m_poisson_dt != NULL);
    assert(m_poisson_function != NULL);

    CString vertices;
    vertices.Format("%d vertices",m_poisson_dt->number_of_vertices());
    CString tets;
    tets.Format("%d tets",m_poisson_dt->number_of_cells());

    // write message to cerr
    std::cerr << "=> " << vertices << ", " << tets << ", "
                       << (CGAL::Memory_sizer().virtual_size()>>20) << " Mb allocated, "
                       << "largest free block=" << (CGAL::Peak_memory_sizer().largest_free_block()>>20) << " Mb, "
                       << "#blocks over 100 Mb=" << CGAL::Peak_memory_sizer().count_free_memory_blocks(100*1048576)
                       << std::endl;

    // Update status bar
    pStatus->SetPaneText(1,vertices);
    pStatus->SetPaneText(2,tets);
    pStatus->UpdateWindow();
  }
}

// Write user message in status bar and cerr
void CPoissonDoc::status_message(char* fmt,...)
{
  // format message in 'buffer'
  char buffer[256];
  va_list argptr;
  va_start(argptr,fmt);
  vsprintf(buffer,fmt,argptr);
  va_end(argptr);

  // write message to cerr
  std::cerr << buffer << std::endl;

  // write message in status bar
  CStatusBar* pStatus =
    (CStatusBar*)AfxGetApp()->m_pMainWnd->GetDescendantWindow(AFX_IDW_STATUS_BAR);
  ASSERT(pStatus != NULL);
  pStatus->SetPaneText(0,buffer);
  pStatus->UpdateWindow();
}

// Write user message in message box and cerr
void CPoissonDoc::prompt_message(char* fmt,...)
{
  // format message in 'buffer'
  char buffer[256];
  va_list argptr;
  va_start(argptr,fmt);
  vsprintf(buffer,fmt,argptr);
  va_end(argptr);

  // write message to cerr
  std::cerr << buffer << std::endl;

  // write message in message box
  AfxMessageBox(buffer);
}

// Display Options dialog
void CPoissonDoc::OnEditOptions()
{
  CDialogOptions dlg;
  dlg.m_sm_angle = m_sm_angle;
  dlg.m_sm_radius = m_sm_radius;
  dlg.m_sm_distance_poisson = m_sm_distance_poisson;
  dlg.m_sm_error_bound = m_sm_error_bound;
  dlg.m_dr_sizing = m_dr_sizing;
  dlg.m_dr_shell_size = m_dr_shell_size;
  dlg.m_dr_max_vertices = m_dr_max_vertices;
  dlg.m_contouring_value = m_contouring_value;
  dlg.m_lambda = m_lambda;
  dlg.m_sm_distance_apss = m_sm_distance_apss;
  dlg.m_number_of_neighbours = m_number_of_neighbours;
  dlg.m_min_cameras_cone_angle = m_min_cameras_cone_angle;
  dlg.m_threshold_percent_avg_knn_sq_dst = m_threshold_percent_avg_knn_sq_dst;
  dlg.m_clustering_step = m_clustering_step;
  dlg.m_random_simplification_percentage = m_random_simplification_percentage;

  if(dlg.DoModal() == IDOK)
  {
    m_sm_angle = dlg.m_sm_angle;
    m_sm_radius = dlg.m_sm_radius;
    m_sm_distance_poisson = dlg.m_sm_distance_poisson;
    m_sm_error_bound = dlg.m_sm_error_bound;
    m_dr_sizing = dlg.m_dr_sizing;
    m_dr_shell_size = dlg.m_dr_shell_size;
    m_dr_max_vertices = dlg.m_dr_max_vertices;
    m_contouring_value = dlg.m_contouring_value;
    m_lambda = dlg.m_lambda;
    m_sm_distance_apss = dlg.m_sm_distance_apss;
    m_number_of_neighbours = dlg.m_number_of_neighbours;
    m_min_cameras_cone_angle = dlg.m_min_cameras_cone_angle;
    m_threshold_percent_avg_knn_sq_dst = dlg.m_threshold_percent_avg_knn_sq_dst;
    m_clustering_step = dlg.m_clustering_step;
    m_random_simplification_percentage = dlg.m_random_simplification_percentage;

    UpdateAllViews(NULL);
    EndWaitCursor();
  }
}

// Compute normals direction by Principal Component Analysis
void CPoissonDoc::OnAlgorithmsEstimateNormalsByPCA()
{
  BeginWaitCursor();
  status_message("Estimate Normals Direction by PCA...");
  CGAL::Timer task_timer; task_timer.start();

  CGAL::estimate_normals_pca_3(m_points.begin(), m_points.end(),
                               m_points.normals_begin(),
                               m_number_of_neighbours);

  status_message("Estimate Normals Direction by PCA...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdateAlgorithmsEstimateNormalsByPCA(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POINT_SET);
}

// Compute normals direction by Jet Fitting
void CPoissonDoc::OnAlgorithmsEstimateNormalsByJetFitting()
{
  BeginWaitCursor();
  status_message("Estimate Normals Direction by Jet Fitting...");
  CGAL::Timer task_timer; task_timer.start();

  CGAL::estimate_normals_jet_fitting_3(m_points.begin(), m_points.end(),
                                       m_points.normals_begin(),
                                       m_number_of_neighbours);

  status_message("Estimate Normals Direction by Jet Fitting...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdateAlgorithmsEstimateNormalByJetFitting(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POINT_SET);
}

// Orient the normals using a minimum spanning tree.
void CPoissonDoc::OnAlgorithmsOrientNormalsWithMST()
{
  BeginWaitCursor();
  status_message("Orient Normals with MST...");
  CGAL::Timer task_timer; task_timer.start();

  CGAL::orient_normals_minimum_spanning_tree_3(m_points.begin(), m_points.end(),
                                               get(boost::vertex_index, m_points),
                                               get(CGAL::vertex_point, m_points),
                                               get(boost::vertex_normal, m_points),
                                               m_number_of_neighbours);

  // Select non-oriented normals
  m_points.select(m_points.begin(), m_points.end(), false);
  for (int i=0; i<m_points.size(); i++)
    if ( ! m_points[i].normal().is_oriented() )
      m_points.select(&m_points[i]);

  status_message("Orient Normals with MST...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdateAlgorithmsOrientNormalsWithMST(CCmdUI *pCmdUI)
{
  assert(m_points.begin() != m_points.end());
  bool points_have_normals = (m_points.begin()->normal() != CGAL::NULL_VECTOR);
  pCmdUI->Enable(m_edit_mode == POINT_SET && points_have_normals);
}

// Specific to Gyroviz: orient the normals w.r.t. the position of cameras
// that reconstructed the points by photogrammetry.
void CPoissonDoc::OnAlgorithmsOrientNormalsWrtCameras()
{
  BeginWaitCursor();
  status_message("Orient Normals wrt Cameras...");
  CGAL::Timer task_timer; task_timer.start();

  // Copy normals to select swapped ones below
  std::vector<Normal> normals_copy(m_points.normals_begin(), m_points.normals_end());

  orient_normals_wrt_cameras_3(m_points.begin(), m_points.end(),
                               get(CGAL::vertex_point, m_points),
                               get(boost::vertex_normal, m_points),
                               get(boost::vertex_cameras, m_points));

  // Select swapped normals
  m_points.select(m_points.begin(), m_points.end(), false);
  for (int i=0; i<m_points.size(); i++)
    if (m_points[i].normal() * normals_copy[i] < 0)
      m_points.select(&m_points[i]);

  status_message("Orient Normals wrt Cameras...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdateAlgorithmsOrientNormalsWrtCameras(CCmdUI *pCmdUI)
{
  assert(m_points.begin() != m_points.end());
  bool points_have_normals = (m_points.begin()->normal() != CGAL::NULL_VECTOR);
  bool points_have_cameras = (m_points.begin()->cameras_begin() != m_points.begin()->cameras_end());
  pCmdUI->Enable(m_edit_mode == POINT_SET && points_have_normals && points_have_cameras);
}

// Reconstruction >> Poisson >> Create Poisson Triangulation callback:
// Create m_poisson_dt from m_points[] and enter in Poisson edit mode.
// m_points is not editable in this mode.
void CPoissonDoc::OnCreatePoissonTriangulation()
{
  BeginWaitCursor();
  status_message("Create Poisson Triangulation...");
  CGAL::Timer task_timer; task_timer.start();

  // Clean up previous mode
  CloseMode();

  // Copy points to m_poisson_dt
  assert(m_poisson_dt == NULL);
  assert(m_poisson_function == NULL);
  m_poisson_dt = new Dt3;
  m_poisson_function = new Poisson_implicit_function(*m_poisson_dt, m_points.begin(), m_points.end());

  m_edit_mode = POISSON;
  m_triangulation_refined = false; // Need to apply Delaunay refinement
  m_poisson_solved = false; // Need to solve Poisson equation

  status_message("Create Poisson Triangulation...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

// Enable Reconstruction >> Poisson menu items if normals are computed and oriented.
void CPoissonDoc::OnUpdateCreatePoissonTriangulation(CCmdUI *pCmdUI)
{
  assert(m_points.begin() != m_points.end());
  bool points_have_normals = (m_points.begin()->normal() != CGAL::NULL_VECTOR);
  bool normals_are_oriented = m_points.begin()->normal().is_oriented();
  pCmdUI->Enable((m_edit_mode == POINT_SET || m_edit_mode == POISSON)
                 && points_have_normals && normals_are_oriented);
}

// Uniform Delaunay refinement
void CPoissonDoc::OnReconstructionDelaunayRefinement()
{
  assert(m_poisson_dt != NULL);
  assert(m_poisson_function != NULL);

  BeginWaitCursor();
  status_message("Delaunay refinement...");
  CGAL::Timer task_timer; task_timer.start();

  const double radius_edge_ratio_bound = 2.5;
  const unsigned int max_vertices = (unsigned int)1e7; // max 10M vertices
  const double enlarge_ratio = 1.5;
  const double size = sqrt(m_poisson_function->bounding_sphere().squared_radius()); // get triangulation's radius
  const double cell_radius_bound = size/5.; // large
  unsigned int nb_vertices_added = m_poisson_function->delaunay_refinement(radius_edge_ratio_bound,cell_radius_bound,max_vertices,enlarge_ratio);

  m_triangulation_refined = true;

  status_message("Delaunay refinement...done (%.2lf s, %d vertices inserted)",
                 task_timer.time(), nb_vertices_added);
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdateReconstructionDelaunayRefinement(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POISSON);
}

// Delaunay refinement in a surface's shell
void CPoissonDoc::OnAlgorithmsRefineInShell()
{
  assert(m_poisson_dt != NULL);
  assert(m_poisson_function != NULL);

  BeginWaitCursor();
  status_message("Delaunay refinement in surface shell...");
  CGAL::Timer task_timer; task_timer.start();

  const unsigned int max_vertices = (unsigned int)1e7; // max 10M vertices
  const double enlarge_ratio = 1.5;
  unsigned int nb_vertices_added = m_poisson_function->delaunay_refinement_shell(m_dr_shell_size,m_dr_sizing,m_dr_max_vertices);
  m_triangulation_refined = true;

  status_message("Delaunay refinement in surface shell...done (%.2lf s, %d vertices inserted)",
                 task_timer.time(), nb_vertices_added);
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdateAlgorithmsRefineInShell(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POISSON);
}

// Extrapolate the normals field:
// compute null normals by averaging neighbour normals.
void CPoissonDoc::OnAlgorithmsExtrapolatenormals()
{
  assert(m_poisson_dt != NULL);
  assert(m_poisson_function != NULL);

  BeginWaitCursor();
  status_message("Extrapolate the normals field...");
  CGAL::Timer task_timer; task_timer.start();

  m_poisson_function->extrapolate_normals();

  status_message("Extrapolate the normals field...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdateAlgorithmsExtrapolateNormals(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POISSON && m_triangulation_refined);
}

// Solve Poisson equation callback
void CPoissonDoc::OnReconstructionPoisson()
{
  assert(m_poisson_dt != NULL);
  assert(m_poisson_function != NULL);

  BeginWaitCursor();
  status_message("Solve Poisson equation...");
  CGAL::Timer task_timer; task_timer.start();

  // Solve Poisson equation such that:
  // - (*m_poisson_function)() = 0 on the input points,
  // - (*m_poisson_function)() < 0 inside the surface.
  double duration_assembly, duration_factorization, duration_solve;
  m_poisson_solved = m_poisson_function->solve_poisson(m_lambda,
                                                       &duration_assembly,
                                                       &duration_factorization,
                                                       &duration_solve);
  m_poisson_function->set_contouring_value(m_poisson_function->median_value_at_input_vertices());
  m_contouring_value = 0.0;

  if (!m_poisson_solved)
      status_message("Solve Poisson equation...solver failed");
  else
      status_message("Solve Poisson equation...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnReconstructionPoissonNormalized()
{
  assert(m_poisson_dt != NULL);
  assert(m_poisson_function != NULL);

  BeginWaitCursor();
  status_message("Solve Poisson equation with normalized divergence...");
  CGAL::Timer task_timer; task_timer.start();

  // Solve Poisson equation such that:
  // - (*m_poisson_function)() = 0 on the input points,
  // - (*m_poisson_function)() < 0 inside the surface.
  double duration_assembly, duration_factorization, duration_solve;
  m_poisson_solved = m_poisson_function->solve_poisson(m_lambda,
                                                       &duration_assembly,
                                                       &duration_factorization,
                                                       &duration_solve,
                                                       true /* normalized*/);
  m_poisson_function->set_contouring_value(m_poisson_function->median_value_at_input_vertices());
  m_contouring_value = 0.0;

  if (!m_poisson_solved)
      status_message("Solve Poisson equation...solver failed");
  else
      status_message("Solve Poisson equation...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

// Enable "Solve Poisson equation" if Delaunay refinement is applied
void CPoissonDoc::OnUpdateReconstructionPoisson(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POISSON && m_triangulation_refined);
}

// Reconstruction >> Poisson >> Surface Meshing callback
void CPoissonDoc::OnReconstructionPoissonSurfaceMeshing()
{
    assert(m_poisson_dt != NULL);
    assert(m_poisson_function != NULL);

    BeginWaitCursor();
    status_message("Surface meshing...");
    CGAL::Timer task_timer; task_timer.start();

    // Clear previous call
    m_surface_mesher_dt.clear();
    m_surface_mesher_c2t3.clear();
    m_surface.clear();

    // Apply contouring value defined in Options dialog
    m_poisson_function->set_contouring_value(m_contouring_value);

    // Get inner point
    Point inner_point = m_poisson_function->get_inner_point();
    FT inner_point_value = (*m_poisson_function)(inner_point);
    if(inner_point_value >= 0.0)
    {
      status_message("Error: unable to seed (%lf at inner_point)",inner_point_value);
      return;
    }

    // Get implicit surface's radius
    Sphere bounding_sphere = m_poisson_function->bounding_sphere();
    FT size = sqrt(bounding_sphere.squared_radius());

    // defining the surface
    typedef CGAL::Implicit_surface_3<Kernel, Poisson_implicit_function&> Surface_3;
    Point sm_sphere_center = inner_point; // bounding sphere centered at inner_point
    FT    sm_sphere_radius = 2 * size;
    sm_sphere_radius *= 1.1; // <= the Surface Mesher fails if the sphere does not contain the surface
    Surface_3 surface(*m_poisson_function,
                      Sphere(sm_sphere_center,sm_sphere_radius*sm_sphere_radius),
                      m_sm_error_bound*size/sm_sphere_radius); // dichotomy stops when segment < m_sm_error_bound*size

    // defining meshing criteria
    CGAL::Surface_mesh_default_criteria_3<STr> criteria(m_sm_angle,  // lower bound of facets angles (degrees)
                                                        m_sm_radius*size,  // upper bound of Delaunay balls radii
                                                        m_sm_distance_poisson*size); // upper bound of distance to surface

    CGAL_TRACE_STREAM << "  make_surface_mesh(dichotomy error="<<m_sm_error_bound<<" * point set radius,\n"
                      << "                    sphere center=("<<sm_sphere_center << "),\n"
                      << "                    sphere radius="<<sm_sphere_radius/size<<" * p.s.r.,\n"
                      << "                    angle="<<m_sm_angle << " degrees,\n"
                      << "                    radius="<<m_sm_radius<<" * p.s.r.,\n"
                      << "                    distance="<<m_sm_distance_poisson<<" * p.s.r.,\n"
                      << "                    Manifold_tag)\n";

    // meshing surface
    CGAL::make_surface_mesh(m_surface_mesher_c2t3, surface, criteria, CGAL::Manifold_tag());

    // get output surface
    std::list<Triangle> triangles;
    CGAL::output_surface_facets<C2t3,Triangle>(triangles,m_surface_mesher_c2t3);
    m_surface.insert(m_surface.end(), triangles.begin(), triangles.end());

    // Reset contouring value
    m_poisson_function->set_contouring_value(0.0);

    // Print status
    status_message("Surface meshing...done (%d output vertices, %.2lf s)",
                   m_surface_mesher_dt.number_of_vertices(), task_timer.time());
    update_status();
    UpdateAllViews(NULL);
    EndWaitCursor();
}

// Enable Reconstruction >> Poisson >> Surface Meshing if Poisson equation is solved
void CPoissonDoc::OnUpdateReconstructionPoissonSurfaceMeshing(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POISSON && m_poisson_solved);
}

// Marching Tet Contouring callback
void CPoissonDoc::OnAlgorithmsMarchingTetContouring()
{
  assert(m_poisson_dt != NULL);
  assert(m_poisson_function != NULL);

  BeginWaitCursor();
  status_message("Marching tet contouring (%3.1lf%%)...", m_contouring_value);
  CGAL::Timer task_timer; task_timer.start();

  m_contour.clear(); // clear previous call

  std::list<Triangle> triangles;
  int nb = m_poisson_dt->marching_tet(std::back_inserter(triangles), m_contouring_value);
  m_contour.insert(m_contour.end(), triangles.begin(), triangles.end());

  status_message("Marching tet contouring...done (%d triangles, %.2lf s)",
                 nb, task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

// Enable "Marching Tet Contouring" if Poisson equation is solved
void CPoissonDoc::OnUpdateAlgorithmsMarchingTetContouring(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POISSON && m_poisson_solved);
}

void CPoissonDoc::OnAlgorithmsPoissonStatistics()
{
  assert(m_poisson_dt != NULL);
  assert(m_poisson_function != NULL);

  BeginWaitCursor();

  // write message in message box
  prompt_message(
    "Poisson implicit function:\n- Median value at input vertices = %lf\n- Average value at input vertices = %lf\n- Min value at input vertices = %lf\n- Max value at input vertices = %lf\n- Median value at convex hull = %lf\n- Average value at convex hull = %lf\n- Min value = %lf",
    m_poisson_function->median_value_at_input_vertices(),
    m_poisson_function->average_value_at_input_vertices(),
    m_poisson_function->min_value_at_input_vertices(),
    m_poisson_function->max_value_at_input_vertices(),
    m_poisson_function->median_value_at_convex_hull(),
    m_poisson_function->average_value_at_convex_hull(),
    (*m_poisson_function)(m_poisson_function->get_inner_point()));

  EndWaitCursor();
}

// Enable "Poisson Statistics" if Poisson equation is solved
void CPoissonDoc::OnUpdateAlgorithmsPoissonStatistics(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POISSON && m_poisson_solved);
}

// Smooth point set using jet fitting + projection
void CPoissonDoc::OnAlgorithmsSmoothUsingJetFitting()
{
  BeginWaitCursor();
  status_message("Smooth Point Set...");
  CGAL::Timer task_timer; task_timer.start();

  // Smooth points in m_points[]
  CGAL::smooth_jet_fitting_3(m_points.begin(), m_points.end(),
                             m_number_of_neighbours);
  m_points.invalidate_bounding_box();

  status_message("Smooth Point Set...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdateAlgorithmsSmoothUsingJetFitting(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POINT_SET);
}

// Clean up current mode
void CPoissonDoc::CloseMode()
{
  // Nothing to do if m_edit_mode == POINT_SET

  // If m_edit_mode == POISSON
  delete m_poisson_function; m_poisson_function = NULL;
  delete m_poisson_dt; m_poisson_dt = NULL;

  // If m_edit_mode == APSS
  delete m_apss_function; m_apss_function = NULL;

  m_edit_mode = NO_EDIT_MODE;
}

// Edit >> Mode >> Point set callback
void CPoissonDoc::OnModePointSet()
{
  // No need to convert Poisson triangulation back to point set (yet)

  // Clean up previous mode
  CloseMode();

  m_edit_mode = POINT_SET;

  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdateModePointSet(CCmdUI *pCmdUI)
{
  // Edit >> Mode >> Point set is always enabled
  pCmdUI->SetCheck(m_edit_mode == POINT_SET);
}

// "Edit >> Mode >> Poisson" is an alias to
// "Reconstruction >> Poisson >> Create Poisson Triangulation".
void CPoissonDoc::OnModePoisson()
{
  OnCreatePoissonTriangulation();
}

void CPoissonDoc::OnUpdateModePoisson(CCmdUI *pCmdUI)
{
  OnUpdateCreatePoissonTriangulation(pCmdUI);
  pCmdUI->SetCheck(m_edit_mode == POISSON);
}

// Reconstruction >> Poisson reconstruction callback: call in 1 step:
// - Create Poisson Delaunay Triangulation
// - Delaunay refinement
// - Solve Poisson Equation
// - Surface Meshing
void CPoissonDoc::OnOneStepPoissonReconstruction()
{
  BeginWaitCursor();
  status_message("1-step Poisson reconstruction...");
  CGAL::Timer task_timer; task_timer.start();

  OnCreatePoissonTriangulation();
  OnReconstructionDelaunayRefinement();
  if (m_triangulation_refined)
    OnReconstructionPoisson();
  if (m_poisson_solved)
    OnReconstructionPoissonSurfaceMeshing();

  status_message("1-step Poisson reconstruction...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdateOneStepPoissonReconstruction(CCmdUI *pCmdUI)
{
  OnUpdateCreatePoissonTriangulation(pCmdUI);
}

void CPoissonDoc::OnOneStepPoissonReconstructionWithNormalizedDivergence()
{
  BeginWaitCursor();
  status_message("1-step Poisson reconstruction with normalized divergence...");
  CGAL::Timer task_timer; task_timer.start();

  OnCreatePoissonTriangulation();
  //OnCalculateAverageSpacing();
  OnReconstructionDelaunayRefinement();
  //OnExtrapolateNormalsUsingGaussianKernel(); // TO TEST
  OnReconstructionPoissonNormalized();
  OnReconstructionPoissonSurfaceMeshing();

  status_message("1-step Poisson reconstruction with normalized divergence...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

// Remove vertices / cameras cone's angle is low
void CPoissonDoc::OnAlgorithmsOutliersRemovalWrtCamerasConeAngle()
{
  BeginWaitCursor();
  status_message("Remove outliers / cameras cone's angle is low...");
  CGAL::Timer task_timer; task_timer.start();

  // Select points to delete
  Point_set::iterator first_iterator_to_remove =
    remove_outliers_wrt_camera_cone_angle_3(
            m_points.begin(), m_points.end(),
            m_min_cameras_cone_angle*M_PI/180.0);
  m_points.select(first_iterator_to_remove, m_points.end(), true);

  status_message("Remove outliers / cameras cone's angle is low...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdateAlgorithmsOutliersRemovalWrtCamerasConeAngle(CCmdUI *pCmdUI)
{
  assert(m_points.begin() != m_points.end());
  bool points_have_cameras = (m_points.begin()->cameras_begin() != m_points.begin()->cameras_end());
  pCmdUI->Enable(m_edit_mode == POINT_SET && points_have_cameras);
}

// Remove outliers:
// - compute average squared distance to the K nearest neighbors,
// - remove threshold_percent worst points.
void CPoissonDoc::OnOutliersRemovalWrtAvgKnnSqDist()
{
  BeginWaitCursor();
  status_message("Remove outliers wrt average squared distance to knn...");
  CGAL::Timer task_timer; task_timer.start();

  // Select points to delete
  Point_set::iterator first_iterator_to_remove =
    CGAL::remove_outliers_wrt_avg_knn_sq_distance_3(
            m_points.begin(), m_points.end(),
            m_number_of_neighbours,
            m_threshold_percent_avg_knn_sq_dst);
  m_points.select(first_iterator_to_remove, m_points.end(), true);

  status_message("Remove outliers wrt average squared distance to knn...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdateOutliersRemovalWrtAvgKnnSqDist(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POINT_SET);
}

void CPoissonDoc::OnAnalysisAverageSpacing()
{
  BeginWaitCursor();

  double value = CGAL::average_spacing_3(m_points.begin(),
                                         m_points.end(),
                                         m_number_of_neighbours,
                                         Kernel());
  // write message in message box
  prompt_message("Average spacing: %lf", value);

  EndWaitCursor();
}

void CPoissonDoc::OnUpdateAnalysisAverageSpacing(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POINT_SET);
}

// "Reconstruction >> APSS reconstruction" callback
void CPoissonDoc::OnReconstructionApssReconstruction()
{
    BeginWaitCursor();
    status_message("APSS reconstruction...");
    CGAL::Timer task_timer; task_timer.start();

    // Clean up previous mode
    CloseMode();

    // Clear previous call
    m_surface_mesher_dt.clear();
    m_surface_mesher_c2t3.clear();
    m_surface.clear();

    // Create implicit function
    CGAL_TRACE_STREAM << "  APSS_implicit_function(knn="<<m_number_of_neighbours << ")\n";
    m_apss_function = new APSS_implicit_function(m_points.begin(), m_points.end(),
                                                 m_number_of_neighbours);

    // Get inner point
    Point inner_point = m_apss_function->get_inner_point();
    FT inner_point_value = (*m_apss_function)(inner_point);
    if(inner_point_value >= 0.0)
    {
      status_message("Error: unable to seed (%lf at inner_point)",inner_point_value);
      return;
    }

    // Get implicit surface's radius
    Sphere bounding_sphere = m_apss_function->bounding_sphere();
    FT size = sqrt(bounding_sphere.squared_radius());

    // defining the surface
    typedef CGAL::Implicit_surface_3<Kernel, APSS_implicit_function&> Surface_3;
    Point sm_sphere_center = inner_point; // bounding sphere centered at inner_point
    FT    sm_sphere_radius = 2 * size;
    sm_sphere_radius *= 1.1; // <= the Surface Mesher fails if the sphere does not contain the surface
    Surface_3 surface(*m_apss_function,
                      Sphere(sm_sphere_center,sm_sphere_radius*sm_sphere_radius),
                      m_sm_error_bound*size/sm_sphere_radius); // dichotomy stops when segment < m_sm_error_bound*size

    // defining meshing criteria
    CGAL::Surface_mesh_default_criteria_3<STr> criteria(m_sm_angle,  // lower bound of facets angles (degrees)
                                                        m_sm_radius*size,  // upper bound of Delaunay balls radii
                                                        m_sm_distance_apss*size); // upper bound of distance to surface

    CGAL_TRACE_STREAM << "  make_surface_mesh(dichotomy error="<<m_sm_error_bound<<" * point set radius,\n"
                      << "                    sphere center=("<<sm_sphere_center << "),\n"
                      << "                    sphere radius="<<sm_sphere_radius/size<<" * p.s.r.,\n"
                      << "                    angle="<<m_sm_angle << " degrees,\n"
                      << "                    radius="<<m_sm_radius<<" * p.s.r.,\n"
                      << "                    distance="<<m_sm_distance_apss<<" * p.s.r.,\n"
                      << "                    Manifold_tag)\n";

    // meshing surface
    CGAL::make_surface_mesh(m_surface_mesher_c2t3, surface, criteria, CGAL::Manifold_tag());

    // get output surface
    std::list<Triangle> triangles;
    CGAL::output_surface_facets<C2t3,Triangle>(triangles,m_surface_mesher_c2t3);
    m_surface.insert(m_surface.end(), triangles.begin(), triangles.end());

    // Record new mode
    m_edit_mode = APSS;

    // Print status
    status_message("APSS reconstruction...done (%d vertices, %.2lf s)",
                   m_surface_mesher_dt.number_of_vertices(), task_timer.time());
    update_status();
    UpdateAllViews(NULL);
    EndWaitCursor();
}

// Enable "Reconstruction >> APSS reconstruction" if normals are computed and oriented.
void CPoissonDoc::OnUpdateReconstructionApssReconstruction(CCmdUI *pCmdUI)
{
  assert(m_points.begin() != m_points.end());
  bool points_have_normals = (m_points.begin()->normal() != CGAL::NULL_VECTOR);
  bool normals_are_oriented = m_points.begin()->normal().is_oriented();
  pCmdUI->Enable((m_edit_mode == POINT_SET || m_edit_mode == APSS)
                 && points_have_normals && normals_are_oriented);
}

// "Edit >> Mode >> APSS" is an alias to "Reconstruction >> APSS reconstruction".
void CPoissonDoc::OnModeAPSS()
{
  OnReconstructionApssReconstruction();
}

// "Edit >> Mode >> APSS" is an alias to "Reconstruction >> APSS reconstruction".
void CPoissonDoc::OnUpdateModeAPSS(CCmdUI *pCmdUI)
{
  OnUpdateReconstructionApssReconstruction(pCmdUI);
  pCmdUI->SetCheck(m_edit_mode == APSS);
}


void CPoissonDoc::OnReconstructionSaveas()
{
	m_poisson_function->SaveAsMeshFile();
}

void CPoissonDoc::OnCalculateAverageSpacing()
{
  BeginWaitCursor();
  status_message("Calculating average spacing");
  CGAL::Timer task_timer; task_timer.start();

  Point_set output;
  m_poisson_function->average_spacing_avg_knn_sq_distance_3();
  status_message("Average spacing calculated...took %f seconds", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnExtrapolateNormalsUsingGaussianKernel()
{
  BeginWaitCursor();
  status_message("Extrapolating using gaussian kernels");
  CGAL::Timer task_timer; task_timer.start();

  int a = m_poisson_function->extrapolate_normals_using_gaussian_kernel();
  status_message("Extrapolation of normals done... %d normals inserted",a);

  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnEditDelete()
{
    BeginWaitCursor();
    status_message("Delete selected points");

    m_points.delete_selection();

    update_status();
    UpdateAllViews(NULL);
    EndWaitCursor();
}

void CPoissonDoc::OnUpdateEditDelete(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POINT_SET);
}

void CPoissonDoc::OnEditResetSelection()
{
    BeginWaitCursor();
    status_message("Reset selection");

    m_points.select(m_points.begin(), m_points.end(), false);

    update_status();
    UpdateAllViews(NULL);
    EndWaitCursor();
}

void CPoissonDoc::OnUpdateEditResetSelection(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POINT_SET);
}

void CPoissonDoc::OnPointCloudSimplificationByClustering()
{
  BeginWaitCursor();
  status_message("Point cloud simplification by clustering...");
  CGAL::Timer task_timer; task_timer.start();

  // Get point set's radius
  Sphere bounding_sphere = m_points.bounding_sphere();
  FT size = sqrt(bounding_sphere.squared_radius());

  // Select points to delete
  Point_set::iterator first_iterator_to_remove =
    CGAL::merge_epsilon_nearest_points_3(
            m_points.begin(), m_points.end(),
            m_clustering_step*size);
  m_points.select(first_iterator_to_remove, m_points.end(), true);

  status_message("Point cloud simplification by clustering...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdatePointCloudSimplificationByClustering(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POINT_SET);
}

void CPoissonDoc::OnPointCloudSimplificationRandom()
{
  BeginWaitCursor();
  status_message("Random point cloud simplification...");
  CGAL::Timer task_timer; task_timer.start();

  // Select points to delete
  Point_set::iterator first_iterator_to_remove =
    CGAL::random_simplification_points_3(
            m_points.begin(), m_points.end(),
            m_random_simplification_percentage);
  m_points.select(first_iterator_to_remove, m_points.end(), true);

  status_message("Random point cloud simplification...done (%.2lf s)", task_timer.time());
  update_status();
  UpdateAllViews(NULL);
  EndWaitCursor();
}

void CPoissonDoc::OnUpdatePointCloudSimplificationRandom(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(m_edit_mode == POINT_SET);
}
