//! \file us_convert_gui.h
#ifndef US_CONVERT_GUI_H
#define US_CONVERT_GUI_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_convert.h"
#include "us_experiment.h"
#include "us_dataIO2.h"
#include "us_solution.h"
#include "us_selectbox.h"
#include "us_mwl_data.h"

class US_ConvertGui : public US_Widgets
{
  Q_OBJECT

  public:

      //! \brief   Some status codes to keep track of where data has been saved to
      enum aucStatus
      {
         NOT_SAVED,         //!< The file has not been saved
         EDITING,           //!< Data being edited; certain opers. not permitted
         HD_ONLY,           //!< The file has been saved to the HD
         BOTH               //!< The file has been saved to both HD and DB
      };

      //! \brief  Generic constructor for the US_ConvertGui() program.
      US_ConvertGui();

      US_Disk_DB_Controls* disk_controls;     //!< Radiobuttons for disk/db choice
      bool            save_diskDB;            //!< To keep track of changes

      QList< double > subsets;                //!< A list of subset boundaries

  signals:

  public slots:

  private:

      enum { SPLIT, REFERENCE, NONE } step;

      aucStatus      saveStatus;
      US_Help        showHelp;
      US_PlotPicker* picker;

      QString       runType;
      QString       oldRunType;

      QLabel*       lb_description;

      QLineEdit*    le_investigator;
      QString       runID;
      QLineEdit*    le_runID;
      QLineEdit*    le_runID2;
      QLineEdit*    le_dir;
      QLineEdit*    le_description;
      QLineEdit*    le_solutionDesc;

      QLabel*       lb_triple;
      QListWidget*  lw_triple;                   // cell, channel, wavelength
      QListWidget*  lw_todoinfo;                 // to do list

      QLabel*       lb_scan;
      QLabel*       lb_from;
      QLabel*       lb_to;
      QwtCounter*   ct_from;
      QwtCounter*   ct_to;

      QwtCounter*   ct_tolerance;

      QLineEdit*    le_lambraw;
      QLabel*       lb_lambplot;
      QComboBox*    cmb_lambplot;
      QPushButton*  pb_lambprev;
      QPushButton*  pb_lambnext;

      QPushButton*  pb_editRuninfo;
      QPushButton*  pb_import;
      QPushButton*  pb_loadUS3;
      QPushButton*  pb_impmwl;
      QPushButton*  pb_details;
      QPushButton*  pb_applyAll;
      QPushButton*  pb_solution;
      QPushButton*  pb_exclude;
      QPushButton*  pb_include;
      QPushButton*  pb_define;
      QPushButton*  pb_process;
      QPushButton*  pb_reference;
      QPushButton*  pb_intensity;
      QPushButton*  pb_cancelref;
      QPushButton*  pb_dropScan;
      QPushButton*  pb_saveUS3;

      US_SelectBox*  cb_centerpiece;

      QList< US_DataIO2::BeckmanRawScan > legacyData; // legacy data from file
      QVector< US_DataIO2::RawData >  allData;      // All ccw-separated data
      QString       currentDir;
      QString       saveDescription;

      QVector< US_Convert::Excludes > allExcludes;  // All excludes for triples

      US_MwlData    mwl_data;

      QwtPlot*      data_plot;
      QwtPlotGrid*  grid;

      double        reference_start;           // Boundary of reference scans
      double        reference_end;
      bool          referenceDefined;          // True if RI averages done
      int           Pseudo_reference_triple;   // Number of reference triple
      bool          isPseudo;                  // Is RI data pseudo-absorbance?
      bool          toleranceChanged;          // Has the tolerance changed?
      double        scanTolerance;             // Remember scan tolerance value
      int           countSubsets;              // Number of subsets maximum = 4
      bool          isMwl;                     // Is Multi-Wavelength?
      int           tripDatax;                 // Triple data index
      int           tripListx;                 // Triple list index
      int           nlambda;                   // Lambda (wavelength) count

      bool show_plot_progress;
      US_Experiment      ExpData; 
      QList< US_Convert::TripleInfo > triples;

      void reset           ( void );
      void enableRunIDControl( bool );
      void enableScanControls( void );
      void enableSaveBtn   ( void );
      void getExpInfo      ( void );
      void setTripleInfo   ( void );
      void checkTemperature( void );
      int  findTripleIndex ( void );
      void focus           ( int, int );
      void init_excludes   ( void );
      void start_reference   ( const QwtDoublePoint& );
      void process_reference ( const QwtDoublePoint& );
      void PseudoCalcAvg     ( void );
      bool read            ( void );
      bool read            ( QString dir );
      bool convert         ( void );
      void initTriples     ( void );
      bool centerpieceInfo ( void );
      bool centerpieceInfoDB( void );
      bool centerpieceInfoDisk( void );
      void plot_current    ( void );
      void plot_titles     ( void );
      void plot_all        ( void );
      void replot          ( void );
      void set_colors      ( const QList< int >& );
      void draw_vline      ( double );
      void db_error        ( const QString& );
      void triple_index    ( int = -1, int = -1 );

  private slots:
      //! \brief Select the current investigator
      void sel_investigator( void );

      /*! \brief Assign the selected investigator as current
          \param invID  The ID of the selected investigator
      */
      void assign_investigator( int );

      void import            ( QString dir = "" );
      void reimport          ( void );
      void importMWL         ( void );
      void enableControls    ( void );
      void runIDChanged      ( void );
      void toleranceValueChanged( double );     // signal to notify of change
      void lambdaPlotChanged ( int );
      void lambdaPrevClicked ( void );
      void lambdaNextClicked ( void );
      void editRuninfo       ( void );
      void loadUS3           ( QString dir = "" );
      void loadUS3Disk       ( void );
      void loadUS3Disk       ( QString );
      void loadUS3DB         ( void );
      void updateExpInfo     ( US_Experiment& );
      void cancelExpInfo     ( void );
      void getSolutionInfo   ( void );
      void updateSolutionInfo( US_Solution );
      void cancelSolutionInfo( void );
      void tripleApplyAll    ( void );
      void runDetails        ( void );
      void changeDescription ( void );
      void changeTriple      ( QListWidgetItem* );
      void getCenterpieceIndex( int );
      void focus_from        ( double );
      void focus_to          ( double );
      void exclude_scans     ( void );
      void include           ( void );
      void define_subsets    ( void );
      void cClick            ( const QwtDoublePoint& );
      void process_subsets   ( void );
      void define_reference  ( void );
      void show_intensity    ( void );
      void cancel_reference  ( void );
      void drop_reference    ( void );
      void saveUS3           ( void );
      int  saveUS3Disk       ( void );
      void saveUS3DB         ( void );
      void saveReportsToDB   ( void );
      void resetAll          ( void );
      void source_changed    ( bool );
      void update_disk_db    ( bool );
      void show_mwl_control  ( bool );
      void mwl_connect       ( bool );
      void help              ( void )
        { showHelp.show_help( "convert.html" ); };
};
#endif
