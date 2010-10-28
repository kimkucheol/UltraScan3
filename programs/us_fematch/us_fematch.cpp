//! \file us_fe_match.cpp

#include <QApplication>

#include "us_fematch.h"
#include "us_resids_bitmap.h"
#include "us_plot_control.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_analyte_gui.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_data_loader.h"
#include "us_util.h"
#include "us_investigator.h"
#include "us_lamm_astfvm.h"

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_FeMatch w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_FeMatch class constructor
US_FeMatch::US_FeMatch() : US_Widgets()
{
   setObjectName( "US_FeMatch" );
   def_local  = false;
   dbg_level  = US_Settings::us_debug();

   // set up the GUI
   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle(
      tr( "Compare Experimental Data to Sums of Finite Element Solutions" ) );

   mainLayout      = new QHBoxLayout( this );
   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   leftLayout      = new QVBoxLayout();
   rightLayout     = new QVBoxLayout();
   
   analysisLayout  = new QGridLayout();
   runInfoLayout   = new QGridLayout();
   parameterLayout = new QGridLayout();
   controlsLayout  = new QGridLayout();
   buttonLayout    = new QHBoxLayout();

   leftLayout->addLayout( analysisLayout  );
   leftLayout->addLayout( runInfoLayout   );
   leftLayout->addLayout( parameterLayout );
   leftLayout->addLayout( controlsLayout  );
   leftLayout->addStretch();
   leftLayout->addLayout( buttonLayout    );

   // Analysis buttons
   pb_load      = us_pushbutton( tr( "Load Data" ) );
   pb_details   = us_pushbutton( tr( "Run Details" ) );
   ck_edit      = new QCheckBox( tr( "Latest Data Edit" ) );
   pb_distrib   = us_pushbutton( tr( "s20,W Distribution" ) );
   pb_loadmodel = us_pushbutton( tr( "Load Model" ) );
   pb_simumodel = us_pushbutton( tr( "Simulate Model" ) );
   pb_view      = us_pushbutton( tr( "View Data Report" ) );
   pb_save      = us_pushbutton( tr( "Save Data" ) );
   ck_edit     ->setChecked( true );
   ck_edit     ->setFont( pb_load->font() );
   ck_edit     ->setPalette( US_GuiSettings::normalColor() );

   connect( pb_load,      SIGNAL( clicked() ),
            this,         SLOT(   load() ) );
   connect( pb_details,   SIGNAL( clicked() ),
            this,         SLOT(   details() ) );
   connect( pb_distrib,   SIGNAL( clicked() ),
            this,         SLOT(   distrib_type() ) );
   connect( pb_loadmodel, SIGNAL( clicked() ),
            this,         SLOT(   load_model()  ) );
   connect( pb_simumodel, SIGNAL( clicked() ),
            this,         SLOT(   simulate_model()  ) );
   connect( pb_view,      SIGNAL( clicked() ),
            this,         SLOT(   view_report() ) );
   connect( pb_save,      SIGNAL( clicked() ),
            this,         SLOT(   save_data() ) );

   pb_load     ->setEnabled( true );
   pb_details  ->setEnabled( false );
   ck_edit     ->setEnabled( true );
   pb_distrib  ->setEnabled( false );
   pb_loadmodel->setEnabled( false );
   pb_simumodel->setEnabled( false );
   pb_view     ->setEnabled( false );
   pb_save     ->setEnabled( false );

   analysisLayout->addWidget( pb_load,      0, 0 );
   analysisLayout->addWidget( pb_details,   0, 1 );
   analysisLayout->addWidget( ck_edit,      1, 0 );
   analysisLayout->addWidget( pb_distrib,   1, 1 );
   analysisLayout->addWidget( pb_loadmodel, 2, 0 );
   analysisLayout->addWidget( pb_simumodel, 2, 1 );
   analysisLayout->addWidget( pb_view,      3, 0 );
   analysisLayout->addWidget( pb_save,      3, 1 );

   // Run info
   QLabel* lb_info    = us_banner( tr( "Information for this Run" ) );
   QLabel* lb_triples = us_banner( tr( "Cell / Channel / Wavelength" ) );
   QLabel* lb_id      = us_label ( tr( "Run ID / Edit ID:" ) );
   QLabel* lb_temp    = us_label ( tr( "Average Temperature:" ) );

   le_id      = us_lineedit();
   le_id->setReadOnly( true );
   le_temp    = us_lineedit();
   le_temp->setReadOnly( true );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();

   te_desc    = us_textedit();
   te_desc->setMaximumHeight( fontHeight * 1 + 12 );  // Add for border
   te_desc->setReadOnly( true );

   lw_triples = us_listwidget();
   lw_triples->setMaximumHeight( fontHeight * 2 + 12 );


   runInfoLayout->addWidget( lb_info   , 0, 0, 1, 4 );
   runInfoLayout->addWidget( lb_id     , 1, 0, 1, 1 );
   runInfoLayout->addWidget( le_id     , 1, 1, 1, 3 );
   runInfoLayout->addWidget( lb_temp   , 2, 0, 1, 1 );
   runInfoLayout->addWidget( le_temp   , 2, 1, 1, 3 );
   runInfoLayout->addWidget( te_desc   , 3, 0, 2, 4 );
   runInfoLayout->addWidget( lb_triples, 5, 0, 1, 4 );
   runInfoLayout->addWidget( lw_triples, 6, 0, 5, 4 );

   // Parameters

   density      = DENS_20W;
   viscosity    = VISC_20W;
   compress     = 0.0;
   pb_density   = us_pushbutton( tr( "Density"   ) );
   le_density   = us_lineedit( QString::number( density,   'f', 6 ) );
   pb_viscosity = us_pushbutton( tr( "Viscosity" ) );
   le_viscosity = us_lineedit( QString::number( viscosity, 'f', 5 ) );
   pb_vbar      = us_pushbutton( tr( "Vbar"   ) );
   le_vbar      = us_lineedit( "0.7200" );
   pb_compress  = us_pushbutton( tr( "Compressibility" ) );
   le_compress  = us_lineedit( "0.0"     );
   pb_showmodel = us_pushbutton( tr( "Show Model #"   ) );
   lb_rmsd      = us_label     ( tr( "RMSD:"  ) );
   le_rmsd      = us_lineedit( "0.0" );
   le_variance  = us_lineedit( "0.0" );
   le_sedcoeff  = us_lineedit( "" );
   le_difcoeff  = us_lineedit( "" );
   le_partconc  = us_lineedit( "" );
   le_moweight  = us_lineedit( "" );
   ct_component = us_counter( 3, 1, 50, 1 );
   ct_component->setMinimumWidth( 170 );
   ct_component->setStep( 1.0 );
   QFontMetrics fme( pb_compress->font() );
   int pwid = fme.width( pb_compress->text() + 6 );
   int lwid = pwid * 3 / 4;
   pb_vbar    ->setMinimumWidth( pwid );
   le_vbar    ->setMinimumWidth( lwid );
   pb_compress->setMinimumWidth( pwid );
   le_compress->setMinimumWidth( lwid );
   connect( le_density,   SIGNAL( returnPressed() ),
            this,         SLOT(   buffer_text()   ) );
   connect( le_viscosity, SIGNAL( returnPressed() ),
            this,         SLOT(   buffer_text()   ) );
   connect( le_compress,  SIGNAL( returnPressed() ),
            this,         SLOT(   buffer_text()   ) );
   connect( le_vbar,      SIGNAL( returnPressed() ),
            this,         SLOT(   vbar_text()     ) );

   QLabel* lb_experiment   = us_banner( tr( "Experimental Parameters (at 20" ) 
      + DEGC + "):" );
   QLabel* lb_variance     = us_label ( tr( "Variance:" ) );
           lb_sedcoeff     = us_label ( tr( "Sedimentation Coefficient:" ) );
           lb_difcoeff     = us_label ( tr( "Diffusion Coefficient:" ) );
           lb_partconc     = us_label ( tr( "Partial Concentration:" ) );
           lb_moweight     = us_label ( tr( "Molecular Weight, f/f0:" ) );
           lb_component    = us_label ( tr( "Component:" ) );
           lb_simpoints    = us_label ( tr( "Simulation Points:" ) );
           lb_bldvolume    = us_label ( tr( "Band-loading Volume:" ) );
           lb_parameter    = us_label ( tr( "Parameter:" ) );

   cb_mesh      = us_comboBox();
   cb_mesh->addItem( tr( "Adaptive Space Time Mesh (ASTFEM)" )   );
   cb_mesh->addItem( tr( "Claverie Mesh" ) );
   cb_mesh->addItem( tr( "Moving Hat Mesh" ) );
   cb_mesh->addItem( tr( "File: \"$ULTRASCAN/mesh.dat\"" ) );
   cb_mesh->addItem( tr( "AST Finite Volume Method (ASTFVM)" )   );
   cb_grid      = us_comboBox();
   cb_grid->addItem( tr( "Moving Time Grid" )   );
   cb_grid->addItem( tr( "Constant Time Grid" ) );
   
   ct_simpoints = us_counter( 3, 0, 500, 1 );
   ct_bldvolume = us_counter( 3, 0, 1, 0.001 );
   ct_parameter = us_counter( 3, 1, 50, 1 );
   ct_modelnbr  = us_counter( 2, 1, 50, 1 );

   connect( pb_density,   SIGNAL( clicked() ),
            this,         SLOT( get_buffer() ) );
   connect( pb_viscosity, SIGNAL( clicked() ),
            this,         SLOT( get_buffer() ) );
   connect( pb_vbar,      SIGNAL( clicked() ),
            this,         SLOT( get_vbar() ) );
   connect( pb_compress,  SIGNAL( clicked() ),
            this,         SLOT( get_buffer() ) );
   connect( ct_component, SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( comp_number(  double ) ) );
   le_rmsd->setReadOnly( true );

   density   = DENS_20W;
   viscosity = VISC_20W;
   vbar      = TYPICAL_VBAR;
   compress  = 0.0;

   ct_simpoints->setValue( 200 );
   ct_bldvolume->setValue( 0.015 );
   ct_parameter->setValue( 0 );
   ct_modelnbr ->setValue( 0 );
   ct_simpoints->setMinimumWidth( 170 );

   ct_simpoints->setEnabled( true  );
   ct_bldvolume->setEnabled( true  );
   ct_parameter->setEnabled( true  );
   pb_showmodel->setEnabled( false );
   ct_modelnbr ->setEnabled( false );
   cb_mesh     ->setEnabled( true  );
   cb_grid     ->setEnabled( true  );

   parameterLayout->addWidget( lb_experiment   , 0, 0, 1, 4 );
   parameterLayout->addWidget( pb_density      , 1, 0, 1, 1 );
   parameterLayout->addWidget( le_density      , 1, 1, 1, 1 );
   parameterLayout->addWidget( pb_viscosity    , 1, 2, 1, 1 );
   parameterLayout->addWidget( le_viscosity    , 1, 3, 1, 1 );
   parameterLayout->addWidget( pb_vbar         , 2, 0, 1, 1 );
   parameterLayout->addWidget( le_vbar         , 2, 1, 1, 1 );
   parameterLayout->addWidget( pb_compress     , 2, 2, 1, 1 );
   parameterLayout->addWidget( le_compress     , 2, 3, 1, 1 );
   parameterLayout->addWidget( lb_variance     , 3, 0, 1, 1 );
   parameterLayout->addWidget( le_variance     , 3, 1, 1, 1 );
   parameterLayout->addWidget( lb_rmsd         , 3, 2, 1, 1 );
   parameterLayout->addWidget( le_rmsd         , 3, 3, 1, 1 );
   parameterLayout->addWidget( lb_sedcoeff     , 4, 0, 1, 2 );
   parameterLayout->addWidget( le_sedcoeff     , 4, 2, 1, 2 );
   parameterLayout->addWidget( lb_difcoeff     , 5, 0, 1, 2 );
   parameterLayout->addWidget( le_difcoeff     , 5, 2, 1, 2 );
   parameterLayout->addWidget( lb_partconc     , 6, 0, 1, 2 );
   parameterLayout->addWidget( le_partconc     , 6, 2, 1, 2 );
   parameterLayout->addWidget( lb_moweight     , 7, 0, 1, 2 );
   parameterLayout->addWidget( le_moweight     , 7, 2, 1, 2 );
   parameterLayout->addWidget( lb_component    , 8, 0, 1, 2 );
   parameterLayout->addWidget( ct_component    , 8, 2, 1, 2 );

   parameterLayout->addWidget( lb_simpoints    , 10, 0, 1, 2 );
   parameterLayout->addWidget( ct_simpoints    , 10, 2, 1, 2 );
   parameterLayout->addWidget( lb_bldvolume    , 11, 0, 1, 2 );
   parameterLayout->addWidget( ct_bldvolume    , 11, 2, 1, 2 );
   parameterLayout->addWidget( lb_parameter    , 12, 0, 1, 2 );
   parameterLayout->addWidget( ct_parameter    , 12, 2, 1, 2 );
   parameterLayout->addWidget( pb_showmodel    , 13, 0, 1, 2 );
   parameterLayout->addWidget( ct_modelnbr     , 13, 2, 1, 2 );
   parameterLayout->addWidget( cb_mesh         , 14, 0, 1, 4 );
   parameterLayout->addWidget( cb_grid         , 15, 0, 1, 4 );

   // Scan Controls

   QLabel* lb_scan    = us_banner( tr( "Scan Control"       ) ); 
   QLabel* lb_from    = us_label ( tr( "From:" ) );
   ct_from            = us_counter( 2, 0, 500, 1 );
   QLabel* lb_to      = us_label ( tr( "to:"   ) );
   ct_to              = us_counter( 2, 0, 500, 1 );
   pb_exclude         = us_pushbutton( tr( "Exclude Scan Range" ) );
   ct_from->setValue( 0 );
   ct_to  ->setValue( 0 );

   pb_exclude->setEnabled( false );

   connect( ct_from,    SIGNAL( valueChanged( double ) ),
            this,       SLOT  ( exclude_from( double ) ) );
   connect( ct_to,      SIGNAL( valueChanged( double ) ),
            this,       SLOT  ( exclude_to  ( double ) ) );
   connect( pb_exclude, SIGNAL( clicked() ),
            this,       SLOT  ( exclude() ) );

   controlsLayout->addWidget( lb_scan           , 0, 0, 1, 4 );
   controlsLayout->addWidget( lb_from           , 1, 0, 1, 1 );
   controlsLayout->addWidget( ct_from           , 1, 1, 1, 1 );
   controlsLayout->addWidget( lb_to             , 1, 2, 1, 1 );
   controlsLayout->addWidget( ct_to             , 1, 3, 1, 1 );
   controlsLayout->addWidget( pb_exclude        , 2, 0, 1, 4 );

   // Plots
   plotLayout1 = new US_Plot( data_plot1,
            tr( "Experimental Data" ),
            tr( "Radius (cm)" ),
            tr( "OD Difference" ) );

   gb_modelsim         = new QGroupBox( 
      tr( "Simulate data using parameters from"
          " model or from Monte Carlo statistics" ) );
   gb_modelsim->setFlat( true );
   QRadioButton* rb_curmod = new QRadioButton( tr( "Current Model" ) );
   QRadioButton* rb_mode   = new QRadioButton( tr( "Mode" ) );
   QRadioButton* rb_mean   = new QRadioButton( tr( "Mean" ) );
   QRadioButton* rb_median = new QRadioButton( tr( "Median" ) );
   gb_modelsim ->setFont( pb_load->font() );
   gb_modelsim ->setPalette( US_GuiSettings::normalColor() );
   QHBoxLayout* msbox = new QHBoxLayout();
   msbox->addWidget( rb_curmod );
   msbox->addWidget( rb_mode   );
   msbox->addWidget( rb_mean   );
   msbox->addWidget( rb_median );
   msbox->setSpacing       ( 0 );
   gb_modelsim->setLayout( msbox );
   rb_curmod->setChecked( true );

   plotLayout2 = new US_Plot( data_plot2,
            tr( "Velocity Data" ),
            tr( "Radius (cm)" ),
            tr( "Absorbance" ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot2->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( 560, 240 );
   data_plot2->setMinimumSize( 560, 240 );

   // Standard buttons
   pb_advanced = us_pushbutton( tr( "Advanced" ) );
   pb_reset    = us_pushbutton( tr( "Reset" ) );
   pb_help     = us_pushbutton( tr( "Help"  ) );
   pb_close    = us_pushbutton( tr( "Close" ) );

   buttonLayout->addWidget( pb_advanced );
   buttonLayout->addWidget( pb_reset    );
   buttonLayout->addWidget( pb_help     );
   buttonLayout->addWidget( pb_close    );

   connect( pb_reset,    SIGNAL( clicked() ),
            this,        SLOT(   reset()     ) );
   connect( pb_close,    SIGNAL( clicked() ),
            this,        SLOT(   close_all() ) );
   connect( pb_help,     SIGNAL( clicked() ),
            this,        SLOT(   help()      ) );
   connect( pb_advanced, SIGNAL( clicked() ),
            this,        SLOT(   advanced()  ) );

   rightLayout->addLayout( plotLayout1 );
   rightLayout->addWidget( gb_modelsim );
   rightLayout->addLayout( plotLayout2 );
   rightLayout->setStretchFactor( plotLayout1, 3 );
   rightLayout->setStretchFactor( plotLayout2, 2 );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );
   mainLayout->setStretchFactor( leftLayout, 2 );
   mainLayout->setStretchFactor( rightLayout, 5 );

   set_ra_visible( false );

   dataLoaded = false;
   buffLoaded = false;
   haveSim    = false;
   mfilter    = "";
   investig   = "USER";
   resids.clear();
   rbmapd     = 0;
   eplotcd    = 0;
   rbd_pos    = this->pos() + QPoint( 100, 100 );
   epd_pos    = this->pos() + QPoint( 200, 200 );

   ti_noise.count = 0;
   ri_noise.count = 0;

   sdata          = 0;
}

// public function to get pointer to edit data
US_DataIO2::EditedData*     US_FeMatch::fem_editdata() { return edata;     }

// public function to get pointer to sim data
US_DataIO2::RawData*        US_FeMatch::fem_simdata()  { return sdata;     }

// public function to get pointer to load model
US_Model*                   US_FeMatch::fem_model()    { return &model;    }

// public function to get pointer to TI noise
US_Noise*                   US_FeMatch::fem_ti_noise() { return &ti_noise; }

// public function to get pointer to RI noise
US_Noise*                   US_FeMatch::fem_ri_noise() { return &ri_noise; }

// public function to get pointer to resid bitmap diag
QPointer< US_ResidsBitmap > US_FeMatch::fem_resbmap()  { return rbmapd;    }

// load data
void US_FeMatch::load( void )
{
   QString     file;
   QStringList files;
   QStringList parts;
   lw_triples->  clear();
   dataList.     clear();
   rawList.      clear();
   excludedScans.clear();
   triples.      clear();

   dataLoaded = false;
   buffLoaded = false;
   dataLatest = ck_edit->isChecked();

   US_DataLoader* dialog =
      new US_DataLoader( true, dataLatest, def_local, dfilter, investig );

   if ( dialog->exec() == QDialog::Accepted )
   {
      dialog->settings(  def_local, investig, dfilter );
      dialog->load_edit( dataList,  rawList,  triples );
DbgLv(1) << "DLd:  local invest filter" << def_local << investig << dfilter;
DbgLv(1) << "DLd:   desc" << dialog->description();
DbgLv(1) << "DLd:    triples[0]" << triples.at(0);

      delete dialog;
   }

   else                     // load was aborted
      return;

   qApp->processEvents();

   for ( int ii = 0; ii < triples.size(); ii++ )
      lw_triples->addItem( triples.at( ii ) );

   edata     = &dataList[ 0 ];
   scanCount = edata->scanData.size();
   double avgTemp = average_temperature( edata );

   // set ID, description, and avg temperature text
   le_id  ->setText( edata->runID + " / " + edata->editID );
   te_desc->setText( edata->description );
   le_temp->setText( QString::number( avgTemp, 'f', 1 ) + " " + DEGC );

   lw_triples->setCurrentRow( 0 );

   dataLoaded = true;
   haveSim    = false;

   update( 0 );

   pb_details  ->setEnabled( true );
   pb_loadmodel->setEnabled( true );
   pb_exclude  ->setEnabled( true );
   mfilter     = QString( "=edit" );
   dfilter     = QString( "" );

   ct_from->disconnect();
   ct_from->setValue( 0 );

   connect( ct_from, SIGNAL( valueChanged( double ) ),
            this,    SLOT(   exclude_from( double ) ) );

   rbd_pos    = this->pos() + QPoint( 100, 100 );
   epd_pos    = this->pos() + QPoint( 200, 200 );

   // set up buffer values implied from experimental data
   QString bufid;
   QString bguid;
   QString bdesc;
   QString bdens = le_density  ->text();
   QString bvisc = le_viscosity->text();
   QString bcomp = le_compress ->text();
   bool    bufin = false;

   if ( def_local )
   {  // data from local disk:  get buffer vals (disk or db)
      bufin  = bufinfo_disk( edata, bufid, bguid, bdesc );
DbgLv(2) << "L:IL: bufin bdesc" << bufin << bdesc;
      bufin  = bufin ? bufin :
               bufinfo_db(   edata, bufid, bguid, bdesc );
DbgLv(2) << "L:ID: bufin bdesc" << bufin << bdesc;
      bufin  = bufvals_disk( bufid, bguid, bdesc, bdens, bvisc, bcomp );
DbgLv(2) << "L:VL: bufin bdens" << bufin << bdens;
      bufin  = bufin ? bufin :
               bufvals_db(   bufid, bguid, bdesc, bdens, bvisc, bcomp );
DbgLv(2) << "L:VD: bufin bdens" << bufin << bdens;
   }

   else
   {  // data from db:          get buffer vals (db or disk)
      bufin  = bufinfo_db(   edata, bufid, bguid, bdesc );
DbgLv(2) << "D:ID: bufin bdesc" << bufin << bdesc;
      bufin  = bufin ? bufin :
               bufinfo_disk( edata, bufid, bguid, bdesc );
DbgLv(2) << "D:IL: bufin bdesc" << bufin << bdesc;
      bufin  = bufvals_db(   bufid, bguid, bdesc, bdens, bvisc, bcomp );
DbgLv(2) << "D:VD: bufin bdens" << bufin << bdens;
      bufin  = bufin ? bufin :
               bufvals_disk( bufid, bguid, bdesc, bdens, bvisc, bcomp );
DbgLv(2) << "D:VL: bufin bdens" << bufin << bdens;
   }

   if ( bufin )
   {
      buffLoaded  = false;
      le_density  ->setText( bdens );
      le_viscosity->setText( bvisc );
      le_compress ->setText( bcomp );
      buffLoaded  = true;
      density     = bdens.toDouble();
      viscosity   = bvisc.toDouble();
      compress    = bcomp.toDouble();
   }

}

// details
void US_FeMatch::details( void )
{
   US_RunDetails2* dialog
      = new US_RunDetails2( rawList, runID, workingDir, triples );

   dialog->move( this->pos() + QPoint( 100, 100 ) );
   dialog->exec();
   qApp->processEvents();

   delete dialog;
//DEBUG: for now, use details button to toggle RA visibility
//bool visible=lb_simpoints->isVisible();
//qDebug() << "debug isRA" << !visible;
//set_ra_visible( !visible );
}

// update based on selected triples row
void US_FeMatch::update( int row )
{
   edata          = &dataList[ row ];
   scanCount      = edata->scanData.size();
   le_id->  setText( edata->runID + " / " + edata->editID );

   le_temp->setText( QString::number( average_temperature( edata ), 'f', 1 )
         + " " + DEGC );
   te_desc->setText( edata->description );

   ct_from->setMaxValue( scanCount - excludedScans.size() );
   ct_to  ->setMaxValue( scanCount - excludedScans.size() );
   ct_from->setStep( 1.0 );
   ct_to  ->setStep( 1.0 );

   data_plot();
}


// data plot
void US_FeMatch::data_plot( void )
{
   data_plot2->detachItems();

   if ( !dataLoaded )
      return;

   int row     = lw_triples->currentRow();
   edata       = &dataList[ row ];
   data_plot2->setTitle(
      tr( "Velocity Data for " ) + edata->runID );
   data_plot2->setAxisTitle( QwtPlot::yLeft,
      tr( "Absorbance at " ) + edata->wavelength + tr( " nm" ) );
   data_plot2->setAxisTitle( QwtPlot::xBottom,
      tr( "Radius (cm)" ) );

   data_plot2->clear();
   us_grid( data_plot2 );

   int     scan_nbr  = 0;
   int     from      = (int)ct_from->value();
   int     to        = (int)ct_to  ->value();

   int     points    = edata->scanData[ 0 ].readings.size();
   int     count     = points;

   if ( haveSim )
   {
      count     = sdata->scanData[ 0 ].readings.size();
DbgLv(1) << "R,V points count" << points << count;
      count     = points > count ? points : count;
   }

   double* r         = new double[ count ];
   double* v         = new double[ count ];

   QString       title; 
   QwtPlotCurve* c;
   QPen          pen_red(  Qt::red );
   QPen          pen_cyan( Qt::cyan );
   QPen          pen_plot( US_GuiSettings::plotCurve() );

   // Calculate basic parameters for other functions
   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.vbar      = le_vbar     ->text().toDouble();

   dscan           = &edata->scanData.last();
   int    point    = US_DataIO2::index( *dscan, dataList[ row ].x,
                        dataList[ row ].baseline );
   double baseline = 0.0;

   for ( int jj = point - 5; jj <= point + 5; jj++ )
      baseline        = dscan->readings[ jj ].value;

   baseline       /= 11.0;
   double avgTemp  = average_temperature( edata );
   solution.vbar20 = US_Math2::adjust_vbar( solution.vbar, avgTemp );
   US_Math2::data_correction( avgTemp, solution );

   // Draw curves
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) ) continue;

      scan_nbr++;
      bool highlight = ( scan_nbr >= from  &&  scan_nbr <= to );

      dscan          = &edata->scanData[ ii ];

      double lower_limit = baseline;
      double upper_limit = dscan->plateau;

      int jj    = 0;
      count     = 0;

      // Plot each scan in (up to) three segments: below, in, and above
      // the specified boundaries

      while (  jj < points  &&  dscan->readings[ jj ].value < lower_limit )
      {  // accumulate coordinates of below-baseline points
         r[ count   ] = edata->radius( jj );
         v[ count++ ] = edata->value( ii, jj++ );
      }

      if ( count > 1 )
      {  // plot portion of curve below baseline
         title = tr( "Curve " ) + QString::number( ii ) + tr( " below range" );
         c     = us_curve( data_plot2, title );

         if ( highlight )
            c->setPen( pen_red );
         else
            c->setPen( pen_cyan );
         
         c->setData( r, v, count );
      }

      count = 0;

      while ( jj < points && dscan->readings[ jj ].value < upper_limit )
      {  // accumulate coordinates of curve within baseline-to-plateau
         r[ count   ] = edata->radius( jj );
         v[ count++ ] = edata->value( ii, jj++ );
      }

      if ( count > 1 )
      {  // plot portion of curve within baseline-to-plateau
         title = tr( "Curve " ) + QString::number( ii ) + tr( " in range" );
         c     = us_curve( data_plot2, title );

         if ( highlight )
            c->setPen( pen_red );
         else
            c->setPen( pen_plot );
         
         c->setData( r, v, count );
      }

      count = 0;

      while ( jj < points )
      {  // accumulate coordinates of curve portion above plateau
         r[ count   ] = edata->radius( jj );
         v[ count++ ] = edata->value( ii, jj++ );
      }

      if ( count > 1 )
      {  // plot portion of curve above plateau
         title = tr( "Curve " ) + QString::number( ii ) + tr( " above range" );
         c     = us_curve( data_plot2, title );

         if ( highlight )
            c->setPen( pen_red );
         else
            c->setPen( pen_cyan );
        
         c->setData( r, v, count );
      }
   }

   // plot simulation
   if ( haveSim )
   {
      double rl = edata->radius( 0 );
      double vh = edata->value( scanCount - 1, points - 1 );
      rl       -= 0.05;
      vh       += ( vh - edata->value( 0, 0 ) ) * 0.05;
DbgLv(1) << "  RL" << rl << "  VH" << vh;
int nscan=scanCount;
int nconc=sdata->scanData[0].readings.size();
DbgLv(1) << "    sdata ns nc " << nscan << nconc;
DbgLv(1) << "      sdata->x0" << sdata->radius(0);
DbgLv(1) << "      sdata->xN" << sdata->radius(nconc-1);
DbgLv(1) << "      sdata->c00" << sdata->value(0,0);
DbgLv(1) << "      sdata->c0N" << sdata->value(0,nconc-1);
DbgLv(1) << "      sdata->cM0" << sdata->value(nscan-1,0);
DbgLv(1) << "      sdata->cMN" << sdata->value(nscan-1,nconc-1);

      for ( int ii = 0; ii < scanCount; ii++ )
      {
         if ( excludedScans.contains( ii ) ) continue;

         points    = sdata->scanData[ ii ].readings.size();
DbgLv(2) << "      II POINTS" << ii << points;
         count     = 0;
         int jj    = 0;
         double rr = 0.0;
         double vv = 0.0;

         while ( jj < points )
         {  // accumulate coordinates of simulation curve
            rr         = sdata->radius( jj );
            vv         = sdata->value( ii, jj++ );
DbgLv(3) << "       JJ rr vv" << jj << rr << vv;

            if ( rr > rl )
            {
               r[ count   ] = rr;
               v[ count++ ] = vv;
            }
         }
         title   = "SimCurve " + QString::number( ii );
         c       = us_curve( data_plot2, title );
         c->setPen( pen_red );
         c->setData( r, v, count );
DbgLv(1) << "Sim plot scan count" << ii << count
 << "  r0 v0 rN vN" << r[0] << v[0 ] << r[count-1] << v[count-1];
      }
   }

   data_plot2->replot();

   delete [] r;
   delete [] v;

   return;
}

// save the enhanced data
void US_FeMatch::save_data( void )
{ 
   write_res();

   write_cofs();
}

// update density
void US_FeMatch::update_density(  double new_dens )
{
   density    = new_dens;
}

// update viscosity
void US_FeMatch::update_viscosity( double new_visc )
{
   viscosity  = new_visc;
}

// open dialog and get buffer information
void US_FeMatch::get_buffer( void )
{
   int idPers  = investig.section( ":", 0, 0 ).toInt();
   US_Buffer buff;

   US_BufferGui* bdiag = new US_BufferGui( idPers, true, buff, def_local );
   connect( bdiag, SIGNAL( valueChanged(  US_Buffer ) ),
            this,  SLOT  ( update_buffer( US_Buffer ) ) );
   bdiag->exec();
   qApp->processEvents();
}

// slot to update parameters after buffer dialog
void US_FeMatch::update_buffer( US_Buffer buffer )
{
   bool changed = true;

   // if we still have experiment buffer, allow user to abort changes
   if ( buffLoaded )
      changed = verify_buffer();

   // if experiment buffer is to be overridden, proceed with it
   if ( changed )
   {
      density    = buffer.density;
      viscosity  = buffer.viscosity;
      compress   = buffer.compressibility;

      buffLoaded = false;
      le_density  ->setText( QString::number( density,   'f', 6 ) );
      le_viscosity->setText( QString::number( viscosity, 'f', 5 ) );
      le_compress ->setText( QString::number( compress,  'e', 3 ) );
      qApp->processEvents();
   }
}

// open dialog and get vbar information
void US_FeMatch::get_vbar( void )
{
   int idPers  = investig.section( ":", 0, 0 ).toInt();
   QString aguid = "";

   US_AnalyteGui* vdiag = new US_AnalyteGui( idPers, true, aguid, !def_local );
   connect( vdiag,  SIGNAL( valueChanged( US_Analyte ) ),
             this,  SLOT  ( update_vbar ( US_Analyte ) ) );
   vdiag->exec();
   qApp->processEvents();
}

void US_FeMatch::view_report( )
{
   QString mtext;
   int     row    = lw_triples->currentRow();
   edata          = &dataList[ row ];

   // generate the report file
   write_res();

   // open it
   QString filename = US_Settings::resultDir() + "/" + edata->runID + "."
      + text_model( model, 0 ) + "_res." + edata->cell + wave_index( row );
   QFile   res_f( filename );
   QString fileexts = tr( "Result files (*_res*);;" )
      + tr( "RunID files (" ) + edata->runID + "*);;"
      + tr( "All files (*)" );

   if ( res_f.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QTextStream ts( &res_f );

      while ( !ts.atEnd() )
         mtext.append( ts.readLine() + "\n" );

      res_f.close();
   }

   else
   {
      mtext.append( "*ERROR* Unable to open file " + filename );
   }

   // display the report dialog
   US_Editor* editd = new US_Editor( US_Editor::LOAD, true, fileexts );
   editd->setWindowTitle( tr( "Results:  FE Match Model Simulation" ) );
   editd->move( this->pos() + QPoint( 100, 100 ) );
   editd->resize( 600, 500 );
   editd->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
   editd->e->setText( mtext );
   editd->show();
}

// update vbar
void US_FeMatch::update_vbar( US_Analyte analyte )
{
   bool changed = true;

   if ( buffLoaded )
      changed   = verify_vbar();

   if ( changed )
   {
      vbar       = analyte.vbar20;
      buffLoaded = false;
      le_vbar->setText( QString::number( vbar, 'f', 5 ) );
      qApp->processEvents();
   }
}

void US_FeMatch::exclude_from( double from )
{
   double to = ct_to->value();

   if ( to < from )
   {
      ct_to->disconnect();
      ct_to->setValue( from );

      connect( ct_to,   SIGNAL( valueChanged( double ) ),
               this,    SLOT  ( exclude_to  ( double ) ) );
   }

   data_plot();
}

void US_FeMatch::exclude_to( double to )
{
   double from = ct_from->value();

   if ( from > to )
   {
      ct_from->disconnect();
      ct_from->setValue( to );

      connect( ct_from, SIGNAL( valueChanged( double ) ),
               this,    SLOT  ( exclude_from( double ) ) );
   }

   data_plot();
}

void US_FeMatch::exclude( void )
{
   double from          = ct_from->value();
   double to            = ct_to  ->value();
   int    displayedScan = 1;
          edata         = &dataList[ lw_triples->currentRow() ];
   int    totalScans    = edata->scanData.size();

   for ( int ii = 0; ii < totalScans; ii++ )
   {
      if ( excludedScans.contains( ii ) )
         continue;

      if ( displayedScan >= from  &&  displayedScan <= to )
         excludedScans << ii;

      displayedScan++;
   }

   ct_to->setValue( 0 );   // resets both counters and replots

   ct_from->setMaxValue( totalScans - excludedScans.size() );
   ct_to  ->setMaxValue( totalScans - excludedScans.size() );
}

void US_FeMatch::set_ra_visible( bool visible )
{
   lb_sedcoeff ->setVisible( true );
   adjustSize(); 
   QSize pl1siz = data_plot1->size();
   QSize pl2siz = data_plot2->size();
   lb_simpoints->setVisible( visible );  // visibility of RA experimental pars
   ct_simpoints->setVisible( visible );
   lb_bldvolume->setVisible( visible );
   ct_bldvolume->setVisible( visible );
   lb_parameter->setVisible( visible );
   ct_parameter->setVisible( visible );
   pb_showmodel->setVisible( visible );
   ct_modelnbr ->setVisible( visible );
   cb_mesh     ->setVisible( visible );
   cb_grid     ->setVisible( visible );
   lb_sedcoeff ->setVisible( visible );
   le_sedcoeff ->setVisible( visible );
   lb_difcoeff ->setVisible( visible );
   le_difcoeff ->setVisible( visible );
   lb_partconc ->setVisible( visible );
   le_partconc ->setVisible( visible );
   lb_moweight ->setVisible( visible );
   le_moweight ->setVisible( visible );
   lb_component->setVisible( visible );
   ct_component->setVisible( visible );

   gb_modelsim ->setVisible( visible );  // visibility model simulate group box

   data_plot1->resize( pl1siz );
   data_plot2->resize( pl2siz );
   adjustSize(); 
}

// respond to click of current type of distribution plot
void US_FeMatch::distrib_type( )
{
   const char* dptyp[] = 
   {
      "s20,w distribution",
      "MW distribution",
      "D20,w distribution",
      "f_f0 vs s20,w",
      "f_f0 vs MW",
      "D20,w vs s20,w",
      "D20,w vs MW",
      "Residuals"
   };
   const int ndptyp = sizeof( dptyp ) / sizeof( dptyp[0] );

   QString curtxt = pb_distrib->text();
   int     itype  = 0;

   for ( int ii = 0; ii < ndptyp; ii++ )
   { // identify text of current push button
      if ( curtxt == QString( dptyp[ ii ] ) )
      { // found:  save index and break
         itype   = ii;
         break;
      }
   }

   // get pointer to data for use by plot routines
   edata   = &dataList[ lw_triples->currentRow() ];

   // set push button text to next type
   int ii  = itype + 1;
   ii      = ( ii == ndptyp ) ? 0 : ii;
   pb_distrib->setText( QString( dptyp[ ii ] ) );
 
   switch( itype )
   {
      case 0:     // s20,w distribution
      case 1:     // MW distribution
      case 2:     // D20,w distribution
         distrib_plot_stick( itype );  // bar (1-d) plot
         break;
      case 3:     // f_f0 vs s20,w
      case 4:     // f_f0 vs MW
      case 5:     // D20,w vs s20,w
      case 6:     // D20,w vs MW
         distrib_plot_2d(    itype );  // 2-d plot
         break;
      case 7:     // Residuals
         distrib_plot_resids();        // residuals plot
         break;
   }
}

// do stick type distribution plot
void US_FeMatch::distrib_plot_stick( int type )
{
   QString pltitle = tr( "Run " ) + edata->runID + tr( ": Cell " )
      + edata->cell + " (" + edata->wavelength + " nm)";
   QString xatitle;
   QString yatitle = tr( "Relative Concentration" );

   if ( type == 0 )
   {
      pltitle = pltitle + tr( "\ns20,W Distribution" );
      xatitle = tr( "Corrected Sedimentation Coefficient" );
   }

   else if ( type == 1 )
   {
      pltitle = pltitle + tr( "\nMW Distribution" );
      xatitle = tr( "Molecular Weight (Dalton)" );
   }

   else if ( type == 2 )
   {
      pltitle = pltitle + tr( "\nD20,W Distribution" );
      xatitle = tr( "D20,W (cm^2/sec)" );
   }

   data_plot1->detachItems();

   data_plot1->setTitle(                       pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   data_plot1->clear();
   QwtPlotGrid*  data_grid = us_grid( data_plot1 );
   QwtPlotCurve* data_curv = us_curve( data_plot1, "distro" );

   int     dsize  = model.components.size();
   double* xx     = new double[ dsize ];
   double* yy     = new double[ dsize ];
   double  xmin   = 1.0e30;
   double  xmax   = -1.0e30;
   double  ymin   = 1.0e30;
   double  ymax   = -1.0e30;
   double  xval;
   double  yval;
   double  rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xval     = ( type == 0 ) ? model.components[ jj ].s :
               ( ( type == 1 ) ? model.components[ jj ].mw :
                                 model.components[ jj ].D );
      yval     = model.components[ jj ].signal_concentration;
      xx[ jj ] = xval;
      yy[ jj ] = yval;
      xmin     = min( xval, xmin );
      xmax     = max( xval, xmax );
      ymin     = min( yval, ymin );
      ymax     = max( yval, ymax );
   }

   rdif   = ( xmax - xmin ) / 20.0;
   xmin  -= rdif;
   xmax  += rdif;
   rdif   = ( ymax - ymin ) / 20.0;
   ymin  -= rdif;
   ymax  += rdif;
   xmin   = max( xmin, 0.0 );
   ymin   = max( ymin, 0.0 );

   data_grid->enableYMin( true );
   data_grid->enableY(    true );
   data_grid->setMajPen(
      QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );

   data_curv->setData(  xx, yy, dsize );
   data_curv->setPen(   QPen( Qt::yellow, 3, Qt::SolidLine ) );
   data_curv->setStyle( QwtPlotCurve::Sticks );

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot1->setAxisScale( QwtPlot::yLeft,   ymin, ymax );

   data_plot1->replot();

   delete [] xx;
   delete [] yy;
}

// do 2d type distribution plot
void US_FeMatch::distrib_plot_2d( int type )
{
   QString pltitle = tr( "Run " ) + edata->runID + tr( ": Cell " )
      + edata->cell + " (" + edata->wavelength + " nm)";
   QString yatitle;
   QString xatitle;

   if ( type == 3 )
   {
      pltitle = pltitle + tr( "\nf/f0 vs Sed. Coeff." );
      yatitle = tr( "Frictional Ratio f/f0" );
      xatitle = tr( "Sedimentation Coefficient s20,W" );
   }

   else if ( type == 4 )
   {
      pltitle = pltitle + tr( "\nf/f0 vs Mol. Weight" );
      yatitle = tr( "Frictional Ratio f/f0" );
      xatitle = tr( "Molecular Weight" );
   }

   else if ( type == 5 )
   {
      pltitle = pltitle + tr( "\nDiff. Coeff. vs Sed. Coeff." );
      yatitle = tr( "Diffusion Coefficent D20,W" );
      xatitle = tr( "Sedimentation Coefficient s20,W" );
   }

   else if ( type == 6 )
   {
      pltitle = pltitle + tr( "\nDiff. Coeff. vs Molecular Weight" );
      yatitle = tr( "Diffusion Coefficent D20,W" );
      xatitle = tr( "Molecular Weight" );
   }

   data_plot1->setTitle(                       pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   data_plot1->clear();
   data_plot1->detachItems();

   QwtPlotGrid*  data_grid = us_grid( data_plot1 );
   QwtPlotCurve* data_curv = us_curve( data_plot1, "distro" );
   QwtSymbol     symbol;

   int     dsize  = model.components.size();
   double* xx     = new double[ dsize ];
   double* yy     = new double[ dsize ];
   double  xmin   = 1.0e30;
   double  xmax   = -1.0e30;
   double  ymin   = 1.0e30;
   double  ymax   = -1.0e30;
   double  xval;
   double  yval;
   double  rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xval     = ( ( type & 1 ) == 1 ) ? model.components[ jj ].s :
                                         model.components[ jj ].mw;
      yval     = ( type < 5          ) ? model.components[ jj ].f_f0 :
                                         model.components[ jj ].D;
      xx[ jj ] = xval;
      yy[ jj ] = yval;
      xmin     = min( xval, xmin );
      xmax     = max( xval, xmax );
      ymin     = min( yval, ymin );
      ymax     = max( yval, ymax );
   }

   rdif   = ( xmax - xmin ) / 20.0;
   xmin  -= rdif;
   xmax  += rdif;
   rdif   = ( ymax - ymin ) / 20.0;
   ymin  -= rdif;
   ymax  += rdif;
   xmin   = max( xmin, 0.0 );
   ymin   = max( ymin, 0.0 );

   data_grid->enableYMin( true );
   data_grid->enableY(    true );
   data_grid->setMajPen(
      QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );

   symbol.setStyle( QwtSymbol::Ellipse );
   symbol.setPen(   QPen(   Qt::red    ) );
   symbol.setBrush( QBrush( Qt::yellow ) );
   if ( dsize > 100 )
      symbol.setSize(  5 );
   else if ( dsize > 50 )
      symbol.setSize(  8 );
   else if ( dsize > 20 )
      symbol.setSize( 10 );
   else
      symbol.setSize( 12 );

   data_curv->setStyle(  QwtPlotCurve::NoCurve );
   data_curv->setSymbol( symbol );
   data_curv->setData(   xx, yy, dsize );

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot1->setAxisScale( QwtPlot::yLeft,   ymin, ymax );

   data_plot1->replot();

   delete [] xx;
   delete [] yy;
}

// do residuals type distribution plot
void US_FeMatch::distrib_plot_resids( )
{
   QString pltitle = tr( "Run " ) + edata->runID + tr( ": Cell " )
      + edata->cell + " (" + edata->wavelength + " nm)" + tr( "\nResiduals" );
   QString yatitle = tr( "OD Difference" );
   QString xatitle = tr( "Radius (cm)" );

   data_plot1->setTitle(     pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   data_plot1->clear();
   data_plot1->detachItems();

   QwtPlotGrid*  data_grid = us_grid( data_plot1 );
   QwtPlotCurve* data_curv;
   QwtPlotCurve* line_curv = us_curve( data_plot1, "resids zline" );

   int     dsize  = edata->scanData[ 0 ].readings.size();
   double* xx     = new double[ dsize ];
   double* yy     = new double[ dsize ];
   double  zx[ 2 ];
   double  zy[ 2 ];
   double  xmin   = 1.0e30;
   double  xmax   = -1.0e30;
   double  ymin   = 1.0e30;
   double  ymax   = -1.0e30;
   double  xval;
   double  yval;
   double  rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   { // accumulate x (radius) values and min,max
      xval     = edata->radius( jj );
      xmin     = min( xval, xmin );
      xmax     = max( xval, xmax );
      xx[ jj ] = xval;
   }

   rdif   = ( xmax - xmin ) / 20.0;  // expand grid range slightly
   xmin  -= rdif;
   xmax  += rdif;
   xmin   = max( xmin, 0.0 );

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // accumulate min,max y (residual) values
      for ( int jj = 0; jj < dsize; jj++ )
      {
         yval     = resids[ ii ][ jj ];
         yval     = min( yval, 0.1 );
         yval     = max( yval, -0.1 );
         ymin     = min( yval, ymin );
         ymax     = max( yval, ymax );
      }
   }

   rdif   = ( ymax - ymin ) / 20.0;
   ymin  -= rdif;
   ymax  += rdif;

   data_grid->enableYMin( true );
   data_grid->enableY(    true );
   data_grid->setMajPen(
      QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot1->setAxisScale( QwtPlot::yLeft,   ymin, ymax );

   // draw the red zero line
   zx[ 0 ] = xmin;
   zx[ 1 ] = xmax;
   zy[ 0 ] = 0.0;
   zy[ 1 ] = 0.0;
   line_curv->setPen( QPen( Qt::red ) );
   line_curv->setData( zx, zy, 2 );

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // draw residual dots a scan at a time

      for ( int jj = 0; jj < dsize; jj++ )
      {  // get residuals for this scan
         yval     = resids[ ii ][ jj ];
         yval     = min( yval, 0.1 );
         yval     = max( yval, -0.1 );
         yy[ jj ] = yval;
      }

      // plot the residual scatter for this scan
      data_curv = us_curve( data_plot1, "resids " +  QString::number( ii ) );
      data_curv->setPen(    QPen( Qt::yellow ) );
      data_curv->setStyle(  QwtPlotCurve::Dots );
      data_curv->setData(   xx, yy, dsize );
   }

   data_plot1->replot();

   delete [] xx;
   delete [] yy;
}

// toggle advanced/basic display components
void US_FeMatch::advanced( )
{
   bool visible = !lb_simpoints->isVisible();

   if ( pb_advanced->text() == "Advanced" )
   {
      pb_advanced->setText( "Basic" );
      visible  = true;
   }

   else
   {
      pb_advanced->setText( "Advanced" );
      visible  = false;
   }

   set_ra_visible( visible );
}

// reset excluded scan range
void US_FeMatch::reset( )
{
   if ( !dataLoaded )
      return;

   excludedScans.clear();

   ct_from->disconnect();
   ct_to  ->disconnect();
   ct_from->setValue( 0 );
   ct_to  ->setValue( 0 );

   connect( ct_from, SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( exclude_from( double ) ) );
   connect( ct_to,   SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( exclude_to  ( double ) ) );

   data_plot();
}

// load model data and detect if RA
void US_FeMatch::load_model( )
{
   bool     visible = lb_simpoints->isVisible();  // current RA visibility
   bool     isRA;                                 // is model RA?
   QString  mdesc;

   // load model
   US_ModelLoader* dialog = new US_ModelLoader( false, def_local,
      mfilter, investig );
   dialog->move( this->pos() + QPoint( 200, 200 ) );
   dialog->set_edit_guid( dataList[ 0 ].editGUID );

   if ( dialog->exec() == QDialog::Accepted )
   {
      mfilter       = dialog->search_filter();     // next search filter
      investig      = dialog->investigator_text(); // next investigator
      mdesc         = dialog->description( 0 );

      dialog->load_model( model, 0 );              // load selected model

      if ( mdesc.section( mdesc.left( 1 ), 2, 2 ).isEmpty() )
         def_local     = false;  // empty filename:      default to db next
      else
         def_local     = true;   // non-empty filename:  default to local next

      delete dialog;
   }

   else
      return;                     // Cancel:  bail out now

   qApp->processEvents();
   model_loaded = model;   // save model exactly as loaded

   int ncomp    = model.components.size();       // components count
   int nassoc   = model.associations.size();     // associations count
   isRA         = ( nassoc > 1 );                // RA if #assocs > 1

   ct_component->setMaxValue( (double)ncomp );

   // set values for component 1
   component_values( 0 );

   if ( ( isRA && !visible )  ||  ( !isRA && visible ) )
   {  // new RA visibility state out of sync:  change it
      set_ra_visible( isRA );  // change visible components based on RA

      adjustSize();            // adjust overall size to visible components
   }

   pb_simumodel->setEnabled( true );

   // see if there are any noise files to load
   load_noise();

}

void US_FeMatch::adjust_model()
{
   model              = model_loaded;

   // build model component correction factors
   double avgTemp     = average_temperature( edata );

   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.vbar      = le_vbar     ->text().toDouble();
   solution.vbar20    = US_Math2::adjust_vbar( solution.vbar, avgTemp );

   US_Math2::data_correction( avgTemp, solution );

   double scorrec  = 1.0 / solution.correction;
   double dcorrec  = ( ( K0 + avgTemp ) * VISC_20W )
                     / ( K20 * solution.viscosity );

   // fill out components values and adjust s,D based on buffer

   for ( int jj = 0; jj < model.components.size(); jj++ )
   {
      US_Model::SimulationComponent* sc = &model.components[ jj ];

      double s20w = fabs( sc->s );
      double D20w = sc->D;
      sc->mw      = 0.0;
      sc->f       = 0.0;
      sc->f_f0    = 0.0;

      model.calc_coefficients( *sc );

      sc->s      *= scorrec;
      sc->D      *= dcorrec;
DbgLv(1) << " cx" << jj+1
 << "s20w s" << s20w << sc->s << "  D20w D" << D20w << sc->D;

      if ( sc->extinction > 0.0 )
         sc->molar_concentration = sc->signal_concentration / sc->extinction;
   }
}

// load noise record(s) if there are any and user so chooses
void US_FeMatch::load_noise( )
{
   QStringList mieGUIDs;  // list of GUIDs of models-in-edit
   QStringList nimGUIDs;  // list of GUIDs:type:index of noises-in-models
   QStringList nieGUIDs;  // list of GUIDS:type:index of noises-in-edit
   QStringList tmpGUIDs;  // temporary noises-in-model list
   QString     editGUID  = dataList[ 0 ].editGUID; // loaded edit GUID
   QString     modelGUID = model.modelGUID;        // loaded model GUID
   QString     lmodlGUID;                          // list model GUID
   QString     lnoisGUID;                          // list noise GUID
   QString     modelIndx;                          // "0001" style model index
DbgLv(1) << "editGUID  " << editGUID;
DbgLv(1) << "modelGUID " << modelGUID;

   // get a list of models tied to the loaded edit
   int nemods  = models_in_edit(  def_local, editGUID, mieGUIDs );

   if ( nemods == 0 )
      return;                 // go no further if no models in edit

   // get a list of noises tied to the loaded model
   int nmnois  = noises_in_model( def_local, modelGUID, nimGUIDs );
   int kk      = 0;           // start of noise-in-edit search

   // move the loaded model to the head of the models-in-edit list
   if ( nmnois > 0 )
   {  // if loaded model has noise, make sure it's in noise-in-edit list

      if ( nemods > 1  &&  mieGUIDs.removeOne( modelGUID ) )
      {  // make sure loaded model goes to head of model-in-edit list
         mieGUIDs.insert( 0, modelGUID );
      }

      kk        = 1;          // skip 1st model for noise-in-edit search
      nieGUIDs << nimGUIDs;   // initialize noise-in-edit list
   }

   int nenois  = nmnois;      // initial noise-in-edit count is noises in model

   for ( int ii = kk; ii < nemods; ii++ )
   {  // search through models in edit
      lmodlGUID  = mieGUIDs[ ii ];                    // this model's GUID
      modelIndx  = QString().sprintf( "%4.4d", ii );  // models-in-edit index

      // find the noises tied to this model
      int kenois = noises_in_model( def_local, lmodlGUID, tmpGUIDs );

      if ( kenois > 0 )
      {  // if we have 1 or 2 noises, add to noise-in-edit list
         nenois    += kenois;
         // adjust entry to have the right model-in-edit index
         lnoisGUID  = tmpGUIDs.at( 0 ).section( ":", 0, 1 )
            + ":" + modelIndx;
         nieGUIDs << lnoisGUID;
         if ( kenois > 1 )
         {  // add a second noise to the list
            lnoisGUID  = tmpGUIDs.at( 1 ).section( ":", 0, 1 )
               + ":" + modelIndx;
            nieGUIDs << lnoisGUID;
         }
      }
   }
DbgLv(1) << "nemods nmnois nenois" << nemods << nmnois << nenois;
for (int jj=0;jj<nenois;jj++)
 DbgLv(1) << " jj nieG" << jj << nieGUIDs.at(jj);

   if ( nenois > 0 )
   {  // There is/are noise(s):  ask user if she wants to load
      QMessageBox msgBox;
      QString     msg;

      if ( nenois > 1 )
         msg = tr( "There are noise files. Do you want to load them?" );

      else
         msg = tr( "There is a noise file. Do you want to load it?" );

      msgBox.setWindowTitle( tr( "Edit/Model Associated Noise" ) );
      msgBox.setText( msg );
      msgBox.setStandardButtons( QMessageBox::No | QMessageBox::Yes );
      msgBox.setDefaultButton( QMessageBox::Yes );

      if ( msgBox.exec() == QMessageBox::Yes )
      {
         US_Passwd pw;
         US_DB2    db( pw.getPasswd() );
         US_DB2*   dbP = NULL;

         if ( !def_local )
             dbP          = &db;

         if ( nenois > 1 )
         {
            US_NoiseLoader* nldiag = new US_NoiseLoader( dbP,
               mieGUIDs, nieGUIDs, ti_noise, ri_noise );
            nldiag->move( this->pos() + QPoint( 200, 200 ) );
            nldiag->exec();
            qApp->processEvents();

            delete nldiag;
         }

         else
         {
            lnoisGUID     = nieGUIDs.at( 0 );
            QString typen = lnoisGUID.section( ":", 1, 1 );
            lnoisGUID     = lnoisGUID.section( ":", 0, 0 );

            if ( typen == "ti" )
               ti_noise.load( !def_local, lnoisGUID, dbP );

            else
               ri_noise.load( !def_local, lnoisGUID, dbP );
         }
      }
   }
}

// do model simulation
void US_FeMatch::simulate_model( )
{
   int    row     = lw_triples->currentRow();
   US_SimulationParameters simparams;
   US_DataIO2::RawData*    rdata   = &rawList[  row ];
   US_DataIO2::EditedData* edata   = &dataList[ row ];
   US_DataIO2::Reading     reading;
   int    nscan   = rdata->scanData.size();
   int    nconc   = edata->x.size();
   double radlo   = edata->radius( 0 );
   double radhi   = edata->radius( nconc - 1 );
   double rmsd    = 0.0;
DbgLv(1) << " nscan nconc" << nscan << nconc;
DbgLv(1) << " radlo radhi" << radlo << radhi;
DbgLv(1) << " baseline plateau" << edata->baseline << edata->plateau;

   adjust_model();

   sdata          = new US_DataIO2::RawData();

   // initialize simulation parameters using edited data information
   simparams.initFromData( NULL, *edata );
DbgLv(1) << " initFrDat serial type coeffs" << simparams.rotorSerial
   << simparams.rotorType      << simparams.rotorcoeffs[0]
   << simparams.rotorcoeffs[1] << simparams.rotorcoeffs[2]
   << simparams.rotorcoeffs[3] << simparams.rotorcoeffs[4];

   simparams.meshType          = US_SimulationParameters::ASTFEM;
   simparams.gridType          = US_SimulationParameters::MOVING;
   simparams.radial_resolution = ( radhi - radlo ) / (double)( nconc - 1 );

   QString mtyp = cb_mesh->currentText();
   QString gtyp = cb_grid->currentText();

   if ( mtyp.contains( "Claverie" ) )
      simparams.meshType = US_SimulationParameters::CLAVERIE;
   else if ( mtyp.contains( "Moving Hat" ) )
      simparams.meshType = US_SimulationParameters::MOVING_HAT;
   else if ( mtyp.contains( "File:"      ) )
      simparams.meshType = US_SimulationParameters::USER;
   else if ( mtyp.contains( "Space Volu" ) )
      simparams.meshType = US_SimulationParameters::ASTFVM;

   if ( gtyp.contains( "Constant" ) )
      simparams.gridType = US_SimulationParameters::FIXED;

   simparams.band_firstScanIsConcentration   = false;
DbgLv(1) << "  duration_hours  " << simparams.speed_step[0].duration_hours;
DbgLv(1) << "  duration_minutes" << simparams.speed_step[0].duration_minutes;
DbgLv(1) << "  delay_hours  " << simparams.speed_step[0].delay_hours;
DbgLv(1) << "  delay_minutes" << simparams.speed_step[0].delay_minutes;

   // make a simulation copy of the experimental data without actual readings

   US_AstfemMath::initSimData( *sdata, *edata,
         model.components[ 0 ].signal_concentration );

   sdata->cell        = rdata->cell;
   sdata->channel     = rdata->channel;
   sdata->description = rdata->description;
DbgLv(1) << "  sdata->description" << sdata->description;
DbgLv(1) << "   sdata->x0" << sdata->radius(0);
DbgLv(1) << "   sdata->xN" << sdata->radius(nconc-1);
DbgLv(1) << "   rdata->c0" << rdata->value(0,0);
DbgLv(1) << "   rdata->cN" << rdata->value(0,nconc-1);
DbgLv(1) << "   edata->c0" << edata->value(0,0);
DbgLv(1) << "   edata->cN" << edata->value(0,nconc-1);
DbgLv(1) << "   sdata->c00" << sdata->value(0,0);
DbgLv(1) << "   sdata->c0N" << sdata->value(0,nconc-1);
DbgLv(1) << "   sdata->cM0" << sdata->value(nscan-1,0);
DbgLv(1) << "   sdata->cMN" << sdata->value(nscan-1,nconc-1);
DbgLv(1) << " afrsa init";
if ( dbg_level > 1 )
 simparams.save_simparms( US_Settings::appBaseDir() + "/etc/sp_fematch.xml" );

   compress  = le_compress->text().toDouble();

   if ( model.components[ 0 ].sigma == 0.0  &&
        model.components[ 0 ].delta == 0.0  &&
        model.coSedSolute           <  0.0  &&
        compress                    == 0.0 )
   {
      US_Astfem_RSA* astfem_rsa = new US_Astfem_RSA( model, simparams );
   
DbgLv(1) << " afrsa calc";
//astfem_rsa->setTimeCorrection( true );

      astfem_rsa->calculate( *sdata );
   }

   else
   {
      US_LammAstfvm *astfvm     = new US_LammAstfvm( model, simparams );

      astfvm->calculate(     *sdata );
   }

nscan = sdata->scanData.size();
nconc = sdata->x.size();
DbgLv(1) << " afrsa done M N" << nscan << nconc;
DbgLv(1) << "   sdata->x0" << sdata->radius(0);
DbgLv(1) << "   sdata->xN" << sdata->radius(nconc-1);
DbgLv(1) << "   sdata->c00" << sdata->value(0,0);
DbgLv(1) << "   sdata->c0N" << sdata->value(0,nconc-1);
DbgLv(1) << "   sdata->cM0" << sdata->value(nscan-1,0);
DbgLv(1) << "   sdata->cMN" << sdata->value(nscan-1,nconc-1);

   rmsd        = US_AstfemMath::variance( *sdata, *edata );
   le_variance->setText( QString::number( rmsd ) );
   rmsd        = sqrt( rmsd );
   le_rmsd    ->setText( QString::number( rmsd ) );

   haveSim     = true;
   pb_distrib->setEnabled( true );
   pb_view   ->setEnabled( true );
   pb_save   ->setEnabled( true );

   calc_residuals();             // calculate residuals

   distrib_plot_resids();        // plot residuals

   data_plot();                  // re-plot data+simulation

   if ( rbmapd )
   {
      rbd_pos  = rbmapd->pos();
      rbmapd->close();
   }

   rbmapd = new US_ResidsBitmap( resids );
   rbmapd->move( rbd_pos );
   rbmapd->show();

   if ( eplotcd )
   {
      epd_pos  = eplotcd->pos();
      eplotcd->close();
   }

   eplotcd = new US_PlotControl( this, &model );
   eplotcd->move( epd_pos );
   eplotcd->show();
}

// pare down files list by including only the last-edit versions
QStringList US_FeMatch::last_edit_files( QStringList files )
{
   QStringList ofiles;
   QStringList part;
   QString     file;
   QString     test;
   QString     pfile;
   QString     ptest;
   int         nfi   = files.size();

   // if only one in list, we need do no more
   if ( nfi < 2 )
   {
      return files;
   }

   // make sure files list is in ascending alphabetical order
   files.sort();

   // get first file name and its non-editID parts
   file    = files[ 0 ];
   part    = file.split( "." );
   test    = part[ 0 ] + part[ 3 ] + part[ 4 ] + part[ 5 ];

   // skip all but last of any duplicates (differ only in editID)
   for ( int ii = 1; ii < nfi; ii++ )
   {
      pfile   = file;
      ptest   = test;
      file    = files[ ii ];
      part    = file.split( "." );
      test    = part[ 0 ] + part[ 3 ] + part[ 4 ] + part[ 5 ];

      if ( QString::compare( test, ptest ) != 0 )
      {  // differs by more than just edit, so output previous
         ofiles.append( pfile );
      }
   }

   // output the final
   ofiles.append( file );

   return ofiles;
}

// set values for component at index
void US_FeMatch::component_values( int index )
{
   le_sedcoeff->setText( QString::number( model.components[ index ].s ) );
   le_difcoeff->setText( QString::number( model.components[ index ].D ) );
   le_partconc->setText(
      QString::number( model.components[ index ].signal_concentration ) );
   le_moweight->setText(
      QString( "%1 kD,  %2" ).arg( model.components[ index ].mw / 1000.0 )
      .arg( model.components[ index ].f_f0 ) );
}

// component number changed
void US_FeMatch::comp_number( double cnbr )
{
   component_values( (int)cnbr - 1 );
}

// interpolate an sdata y (readings) value for a given x (radius)
double US_FeMatch::interp_sval( double xv, double* sx, double* sy, int ssize )
{
   for ( int jj = 1; jj < ssize; jj++ )
   {
      if ( xv == sx[ jj ] )
      {
         return sy[ jj ];
      }

      if ( xv < sx[ jj ] )
      {  // given x lower than array x: interpolate between point and previous
         double dx = sx[ jj ] - sx[ jj - 1 ];
         double dy = sy[ jj ] - sy[ jj - 1 ];
         return ( sy[ jj ] + ( xv - sx[ jj - 1 ] ) * dy / dx );
      }
   }

   // given x position not found:  interpolate using last two points
   int    jj = ssize - 1;
   double dx = sx[ jj ] - sx[ jj - 1 ];
   double dy = sy[ jj ] - sy[ jj - 1 ];
   return ( sy[ jj ] + ( xv - sx[ jj - 1 ] ) * dy / dx );
}

// write the results text file
void US_FeMatch::write_res()
{
   int     row      = lw_triples->currentRow();
   edata            = &dataList[ row ];
   QString filename = US_Settings::resultDir() + "/" + edata->runID + "."
      + text_model( model, 0 ) + "_res." + edata->cell + wave_index( row );
   QFile   res_f( filename );

   if ( !res_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return;
   }

   dscan            = &edata->scanData[ 0 ];
   int     vcount   = dscan->readings.size();
   int     scount   = edata->scanData.size();
   int     ccount   = model.components.size();
   QString t20d     = QString( "20" ) + DEGC;
   QString stars    = QString( "*" ).repeated( 60 );
   double  tcorrec  = US_Math2::time_correction( dataList );
   double  baseline = calc_baseline( lw_triples->currentRow() );

   QTextStream ts( &res_f );

   ts << stars << "\n";
   ts << "*" << text_model( model, 58 ) << "*\n";
   ts << stars << "\n\n\n";
   ts << tr( "Data Report for Run \"" ) << edata->runID
      << tr( "\",\n Cell " ) << edata->cell
      << tr( ", Channel " ) << edata->channel
      << tr( ", Wavelength " ) << edata->wavelength
      << tr( ", Edited Dataset " ) << edata->editID << "\n\n";

   ts << tr( "Detailed Run Information:\n\n" );
   ts << tr( "Cell Description:        " ) << edata->description << "\n";
   ts << tr( "Raw Data Directory:      " ) << workingDir << "\n";
   ts << tr( "Rotor Speed:             " ) << dscan->rpm << " rpm\n";
   ts << tr( "Average Temperature:     " ) << le_temp->text() << "\n";
   ts << tr( "Temperature Variation:   Within Tolerance\n" );
   ts << tr( "Time Correction:         " ) << text_time( tcorrec, 1 ) << "\n";
   ts << tr( "Run Duration:            " )
      << text_time( edata->scanData[ scount - 1 ].seconds, 2 ) << "\n";
   ts << tr( "Wavelength:              " ) << edata->wavelength << " nm\n";
   ts << tr( "Baseline Absorbance:     " ) << baseline << " OD\n";
   ts << tr( "Meniscus Position:       " ) << edata->meniscus << " cm\n";
   ts << tr( "Edited Data starts at:   " ) << edata->radius( 0 ) << " cm\n";
   ts << tr( "Edited Data stops at:    " )
      << edata->radius( vcount - 1 ) << " cm\n\n\n";

   ts << tr( "Hydrodynamic Settings:\n\n" );
   ts << tr( "Viscosity correction:    " ) << solution.viscosity << "\n";
   ts << tr( "Viscosity (absolute):    " ) << solution.viscosity_tb << "\n";
   ts << tr( "Density correction:      " ) << solution.density << " g/ccm\n";
   ts << tr( "Density (absolute):      " )
      << solution.density_tb << " g/ccm\n";
   ts << tr( "Vbar:                    " ) << solution.vbar << " ccm/g\n";
   ts << tr( "Vbar corrected for " ) << t20d << ": "
      << solution.vbar20 << " ccm/g\n";
   ts << tr( "Buoyancy (Water, " ) << t20d << "):  "
      << solution.buoyancyw << "\n";
   ts << tr( "Buoyancy (absolute):     " ) << solution.buoyancyb << "\n";
   ts << tr( "Correction Factor:       " ) << solution.correction << "\n\n\n";

   ts << tr( "Data Analysis Settings:\n\n" );
   ts << tr( "Number of Components:    " ) << ccount << "\n";
   ts << tr( "Residual RMS Deviation:  " ) << le_rmsd->text() << "\n\n";

   double sum_mw   = 0.0;
   double sum_s    = 0.0;
   double sum_D    = 0.0;
   double sum_c    = 0.0;
   double ctime    = 0.0;

   for ( int jj = 0; jj < ccount; jj++ )
   {
      double conc;
      conc     = model.components[ jj ].signal_concentration;
      sum_c   += conc;
      sum_mw  += model.components[ jj ].mw * conc;
      sum_s   += model.components[ jj ].s  * conc;
      sum_D   += model.components[ jj ].D  * conc;
   }

   ts << tr( "Weight Averages:\n\n" );
   ts << tr( "Weight Average s20,W:    " )
      << QString().sprintf( "%6.4e\n", (sum_s  / sum_c ) );
   ts << tr( "Weight Average D20,W:    " )
      << QString().sprintf( "%6.4e\n", (sum_D  / sum_c ) );
   ts << tr( "W.A. Molecular Weight:   " )
      << QString().sprintf( "%6.4e\n", (sum_mw / sum_c ) );
   ts << tr( "Total Concentration:     " )
      << QString().sprintf( "%6.4e\n", sum_c ) << "\n\n";

   ts << tr( "Distribution Information:\n\n" );
   ts << tr( "Molecular Weight    " )
      << tr( "S 20,W         " )
      << tr( "D 20,W         " )
      << tr( "Concentration\n" );

   for ( int jj = 0; jj < ccount; jj++ )
   {
      double conc;
      double perc;
      conc     = model.components[ jj ].signal_concentration;
      perc     = 100.0 * conc / sum_c;
      ts << QString().sprintf( " %12.5e  %14.5e %14.5e %14.5e  (%5.2f",
         model.components[ jj ].mw, model.components[ jj ].s,
         model.components[ jj ].D,  conc, perc ) << " %)\n";
   }

   ts << tr( "\n\nScan Information:\n\n" );
   ts << tr( "Scan" )
      << tr( "     Corrected Time" )
      << tr( "  Plateau Concentration" )
      << tr( "  (Ed,Sim Omega_s_t)\n" );

   for ( int ii = 0; ii < scount; ii++ )
   {
      dscan     = &edata->scanData[ ii ];
      ctime     = dscan->seconds - tcorrec;
      ts << QString().sprintf( "%4i:", ( ii + 1 ) );
      ts << "   " << text_time( ctime, 0 );
      ts << QString().sprintf( "%14.6f OD  (%9.3e, %9.3e)\n",
            dscan->plateau, dscan->omega2t, sdata->scanData[ ii ].omega2t );
   }

   ts << "\n";

   res_f.close();
}

// write the results text file
void US_FeMatch::write_cofs()
{
   int    row      = lw_triples->currentRow();
   edata           = &dataList[ row ];
   int    ccount   = model.components.size();
   double avgTemp  = average_temperature( edata ) + K0;
   double scorrec  = 1.0 / solution.correction;
   double dcorrec  = ( avgTemp * VISC_20W ) / ( K20 * solution.viscosity );

   QString filename = US_Settings::resultDir() + "/" + edata->runID + "."
      + text_model( model, 0 ) + "_dis." + edata->cell + wave_index( row );
   QFile   res_f( filename );

   if ( !res_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return;
   }

   QTextStream ts( &res_f );

   ts << tr( "S_apparent" )
      << tr( "  S_20,W    " )
      << tr( "  D_apparent" )
      << tr( "  D_20,W    " )
      << tr( "  MW        " )
      << tr( "  Frequency " )
      << tr( "  f/f0(20,W)\n" );

   for ( int jj = 0; jj < ccount; jj++ )
   {
      ts << QString().sprintf(
         "%10.4e %11.4e %11.4e %11.4e %11.4e %11.4e %11.4e",
         model.components[ jj ].s,  model.components[ jj ].s / scorrec,
         model.components[ jj ].D,  model.components[ jj ].D / dcorrec,
         model.components[ jj ].mw, model.components[ jj ].signal_concentration,
         model.components[ jj ].f_f0 )
         << "\n";
   }

   res_f.close();
}

// format a wavelength index number string
QString US_FeMatch::wave_index( int row )
{
   QString cwaveln = dataList[ row ].wavelength;
   QStringList wavelns;

   wavelns << dataList[ 0 ].wavelength;  // start list of wavelengths

   for ( int jj = 1; jj < dataList.size(); jj++ )
   {  // add to list of unique wavelength strings
      QString dwaveln = dataList[ jj ].wavelength;

      if ( !wavelns.contains( dwaveln ) )
         wavelns << dwaveln;
   }

   wavelns.sort();                       // sort wavelengths

   // return string representation of index of current wavelength
   return QString::number( wavelns.indexOf( cwaveln ) + 1 );
}

// text of minutes,seconds or hours,minutes for a given total seconds value
QString US_FeMatch::text_time( double seconds, int type )
{
   int mins = (int)( seconds / 60.0 );
   int secs = (int)( seconds - (double)mins * 60.0 );

   if ( type == 0 )
   {  // fixed-field mins,secs text
      QString tmin = QString().sprintf( "%4d", mins );
      QString tsec = QString().sprintf( "%3d", secs );
      return tr( "%1 min %2 sec" ).arg( tmin ).arg( tsec );
   }

   else if ( type == 1 )
   {  // minutes,seconds text
      return tr( "%1 minute(s) %2 second(s)" ).arg( mins ).arg( secs );
   }

   else
   {  // hours,minutes text
      int hrs   = (int)( seconds / 3600.0 );
      mins      = qRound( ( seconds - (double)hrs * 3600.0 ) / 60.0 );
      return tr( "%1 hour(s) %2 minute(s)" ).arg( hrs ).arg( mins );
   }
}

// calculate average baseline absorbance
double US_FeMatch::calc_baseline( int row )
{
   US_DataIO2::EditedData* dd = &dataList[ row ];
   const US_DataIO2::Scan* ss = &dd->scanData.last();
   int                     nn = US_DataIO2::index( *ss, dd->x, dd->baseline );
   double                  bl = 0.0;

   for ( int jj = nn - 5; jj < nn + 6; jj++ )
      bl += ss->readings[ jj ].value;

   return ( bl / 11.0 );
}

// model type text string
QString US_FeMatch::text_model( US_Model model, int width )
{
   QString title;

   switch ( (int)model.type )
   {
      case (int)US_Model::TWODSA:
         title = ( width == 0 ) ? "sa2d" :
            tr( "2-Dimensional Spectrum Analysis" );
         break;

      case (int)US_Model::TWODSA_MW:
         title = ( width == 0 ) ? "sa2d-mw" :
            tr( "2-Dimensional Spectrum Analysis" );
         break;

      case (int)US_Model::GA:
      case (int)US_Model::GA_RA:
         title = ( width == 0 ) ? "ga" :
            tr( "Genetic Algorithm Analysis" );
         break;

      case (int)US_Model::GA_MW:
         title = ( width == 0 ) ? "ga-mw" :
            tr( "Genetic Algorithm Analysis" );
         break;

      case (int)US_Model::COFS:
         title = ( width == 0 ) ? "cofs" :
            tr( "C(s) Analysis" );
         break;

      case (int)US_Model::FE:
         title = ( width == 0 ) ? "fe" :
            tr( "Finite Element Analysis" );
         break;

      case (int)US_Model::GLOBAL:
         title = ( width == 0 ) ? "global" :
            tr( "Global Algorithm Analysis" );
         break;

      case (int)US_Model::ONEDSA:
         title = ( width == 0 ) ? "sa1d" :
            tr( "1-Dimensional Spectrum Analysis" );
         break;

      case (int)US_Model::MANUAL:
      default:
         title = ( width == 0 ) ? "sa2d" :
            tr( "2-Dimensional Spectrum Analysis" );
         break;
   }

   if ( width == 0 )
   {  // short title (file node):  add any "ra" or "mc"

      if ( model.associations.size() > 1 )
         title = title + "-ra";

      if ( model.iterations > 1 )
         title = title + "-mc";

   }

   else if ( width > title.length() )
   {  // long title centered:  center it in fixed-length string
      int lent = title.length();
      int lenl = ( width - lent ) / 2;
      int lenr = width - lent - lenl;
      title    = QString( " " ).repeated( lenl ) + title
               + QString( " " ).repeated( lenr );
   }

   return title;
}


// calculate residual absorbance values (data - sim)
void US_FeMatch::calc_residuals()
{
   int     dsize  = edata->scanData[ 0 ].readings.size();
   int     ssize  = sdata->scanData[ 0 ].readings.size();
   double* xx     = new double[ dsize ];
   double* sx     = new double[ ssize ];
   double* sy     = new double[ ssize ];
   double  yval;
   double  sval;
   //double  rl     = edata->radius( 0 );
   //double  vh     = edata->value( scanCount - 1, dsize - 1 );
   double  rmsd   = 0.0;

   QVector< double > resscan;

   resids.clear();
   resscan.resize( dsize );

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xx[ jj ] = edata->radius( jj );
   }

   for ( int jj = 0; jj < ssize; jj++ )
   {
      sx[ jj ] = sdata->radius( jj );
   }

   for ( int ii = 0; ii < scanCount; ii++ )
   {

      for ( int jj = 0; jj < ssize; jj++ )
      {
         sy[ jj ] = sdata->value( ii, jj );
      }

      for ( int jj = 0; jj < dsize; jj++ )
      {
         sval          = interp_sval( xx[ jj ], sx, sy, ssize );
         yval          = edata->value( ii, jj ) - sval;

         //if ( xx[ jj ] < rl )
         //   yval          = 0.0;

         rmsd         += sq( yval );
         resscan[ jj ] = yval;
      }

      resids.append( resscan );
   }

   rmsd  /= (double)( scanCount * dsize );
   le_variance->setText( QString::number( rmsd ) );
   rmsd   = sqrt( rmsd );
   le_rmsd    ->setText( QString::number( rmsd ) );

   delete [] xx;
   delete [] sx;
   delete [] sy;
}

// calculate average temperature across scans
double US_FeMatch::average_temperature( US_DataIO2::EditedData* d )
{
   int    sCount   = d->scanData.size();
   double sumTemp  = 0.0;

   for ( int ii = 0; ii < sCount; ii++ )
      sumTemp += d->scanData[ ii ].temperature;

   return ( sumTemp / (double)sCount );
}

// slot to make sure all windows and dialogs get closed
void US_FeMatch::close_all()
{
   if ( rbmapd )
      rbmapd->close();

   if ( eplotcd )
      eplotcd->close();

   close();
}

// build a list of models(GUIDs) for a given edit(GUID)
int US_FeMatch::models_in_edit( bool ondisk, QString eGUID, QStringList& mGUIDs )
{
   QString xmGUID;
   QString xeGUID;

   mGUIDs.clear();

   if ( ondisk )
   {  // Models from local disk files
      QDir    dir;
      QString path = US_Settings::dataDir() + "/models";

      if ( !dir.exists( path ) )
         dir.mkpath( path );

      dir          = QDir( path );

      QStringList filter( "M*.xml" );
      QStringList f_names = dir.entryList( filter, QDir::Files, QDir::Name );

      QXmlStreamAttributes attr;

      for ( int ii = 0; ii < f_names.size(); ii++ )
      {
         QString fname( path + "/" + f_names[ ii ] );
         QFile   m_file( fname );

         if ( !m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QXmlStreamReader xml( &m_file );


         while ( ! xml.atEnd() )
         {  // Search XML elements until we find "model"
            xml.readNext();

            if ( xml.isStartElement()  &&  xml.name() == "model" )
            {  // test for desired editGUID
               attr    = xml.attributes();
               xeGUID  = attr.value( "editGUID"    ).toString();
               xmGUID  = attr.value( "modelGUID"   ).toString();

               if ( xeGUID == eGUID )
                  mGUIDs << xmGUID;
            }
         }

         m_file.close();
      }
   }

   else
   {
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         return 0;
      }

      QStringList query;
      QString     invID  = investig.section( ":", 0, 0 );

      query.clear();

      query << "get_model_desc" << invID;
      db.query( query );

      while ( db.next() )
      {  // accumulate from db desc entries matching editGUID;
         xmGUID  = db.value( 1 ).toString();
         xeGUID  = db.value( 3 ).toString();
DbgLv(2) << "MIE(db): xm/xe/e GUID" << xmGUID << xeGUID << eGUID;

         if ( xeGUID == eGUID )
            mGUIDs << xmGUID;
      }
   }

   return mGUIDs.size();
}

// build a list of noise(GUIDs) for a given model(GUID)
int US_FeMatch::noises_in_model( bool ondisk, QString mGUID,
      QStringList& nGUIDs )
{
   QString xnGUID;
   QString xmGUID;
   QString xntype;

   nGUIDs.clear();
   if ( ondisk )
   {  // Noises from local disk files
      QDir    dir;
      QString path = US_Settings::dataDir() + "/noises";

      if ( !dir.exists( path ) )
         dir.mkpath( path );

      dir          = QDir( path );

      QStringList filter( "N*.xml" );
      QStringList f_names = dir.entryList( filter, QDir::Files, QDir::Name );

      QXmlStreamAttributes attr;

      for ( int ii = 0; ii < f_names.size(); ii++ )
      {
         QString fname( path + "/" + f_names[ ii ] );
         QFile   m_file( fname );

         if ( !m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QXmlStreamReader xml( &m_file );


         while ( ! xml.atEnd() )
         {  // Search XML elements until we find "noise"
            xml.readNext();

            if ( xml.isStartElement()  &&  xml.name() == "noise" )
            {  // test for desired editGUID
               attr    = xml.attributes();
               xmGUID  = attr.value( "modelGUID"   ).toString();
               xnGUID  = attr.value( "noiseGUID"   ).toString();
               xntype  = attr.value( "type"        ).toString();

               if ( xmGUID == mGUID )
                  nGUIDs << xnGUID + ":" + xntype + ":0000";
            }
         }

         m_file.close();
      }
   }

   else
   {
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
         return 0;

      QStringList query;
      QString     invID  = investig.section( ":", 0, 0 );
      QString     xnoiID;
      QString     xmodID;
      QString     modlID;

      query.clear();
      query << "get_modelID" << mGUID;
      db.query( query );
      db.next();
      modlID  = db.value( 0 ).toString();

      query.clear();
      query << "get_noise_desc" << invID;
      db.query( query );

      while ( db.next() )
      {  // accumulate from db desc entries matching editGUID;
         xnoiID  = db.value( 0 ).toString();
         xnGUID  = db.value( 1 ).toString();
         xmodID  = db.value( 3 ).toString();
         xntype  = db.value( 4 ).toString();
         xntype  = xntype.contains( "ri_nois", Qt::CaseInsensitive ) ?
                   "ri" : "ti";

//DbgLv(2) << "NIM(db): xm/xe/e ID" << xnoiID << xmodID << modlID;
         if ( xmodID == modlID )
            nGUIDs << xnGUID + ":" + xntype + ":0000";
      }
   }

   return nGUIDs.size();
}

// get buffer info from DB: ID, GUID, description
bool US_FeMatch::bufinfo_db( US_DataIO2::EditedData* edata,
      QString& bufId, QString& bufGuid, QString& bufDesc )
{
   bool bufinfo = false;

   QStringList query;
   QString rawGUID  = edata->dataGUID;

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   query << "get_rawDataID_from_GUID" << rawGUID;
   db.query( query );
   if ( db.lastErrno() != US_DB2::OK )
   {
      qDebug() << "***Unable to get raw Data ID from GUID" << rawGUID
         << " lastErrno" << db.lastErrno();
      return bufinfo;
   }
   db.next();
   QString rawID    = db.value( 0 ).toString();
   QString soluID   = db.value( 2 ).toString();
QString expID=db.value(1).toString();
DbgLv(2) << "BInfD: rawGUID rawID expID soluID"
 << rawGUID << rawID << expID << soluID;

   query.clear();
   query << "get_solutionBuffer" << soluID;
   db.query( query );
   if ( db.lastErrno() != US_DB2::OK )
   {
      qDebug() << "***Unable to get solutionBuffer from soluID" << soluID
         << " lastErrno" << db.lastErrno();
      query.clear();
      query << "get_solutionIDs" << expID;
      db.query( query );
      db.next();
      soluID = db.value( 0 ).toString();
      query.clear();
      query << "get_solutionBuffer" << soluID;
      db.query( query );
      if ( db.lastErrno() != US_DB2::OK )
      {
         qDebug() << "***Unable to get solutionBuffer from soluID" << soluID
            << " lastErrno" << db.lastErrno();
      }
      else
         qDebug() << "+++ Got solutionBuffer from soluID" << soluID;
      //return bufinfo;
   }
   db.next();
   QString id       = db.value( 0 ).toString();
   QString guid     = db.value( 1 ).toString();
   QString desc     = db.value( 2 ).toString();
DbgLv(2) << "BInfD: id guid desc" << id << guid << desc;

   if ( !id.isEmpty() )
   {
      bufId         = id;
      bufGuid       = guid.isEmpty() ? bufGuid : guid;
      bufDesc       = desc.isEmpty() ? bufDesc : desc;
      bufinfo       = true;
   }
 
//*DEBUG
QString invID  = investig.section( ":", 0, 0 );
query.clear();
query << "get_experiment_desc" << invID;
db.query( query );
QStringList expIDs;
while ( db.next() ) expIDs << db.value(0).toString();
DbgLv(2) << "esbDbg:invID nexpIDs" << invID << expIDs.size();
for ( int ii=0; ii<expIDs.size(); ii++ )
{
 DbgLv(2) << "esbDbg: ii expID" << ii << expIDs[ii];
 QStringList solIDs;
 query.clear();
 query << "get_solutionIDs" << expIDs[ii];
 db.query( query );
 while ( db.next() ) solIDs << db.value(0).toString();
 for ( int jj=0; jj<solIDs.size(); jj++ )
 {
  query.clear();
  query << "get_solution" << solIDs[jj];
  db.query( query );
  db.next();
  DbgLv(2) << "esbDbg:   jj solID sGUID temp desc notes" << jj << solIDs[jj]
   << db.value(0).toString() << db.value(2).toString()
   << db.value(1).toString() << db.value(3).toString();
  query.clear();
  query << "get_solutionBuffer" << solIDs[jj];
  db.query( query );
  db.next();
  DbgLv(2) << "esbDbg      bId bDesc bGuid" << db.value(0).toString()
   << db.value(2).toString() << db.value(1).toString();
 }
}
//*DEBUG

   return bufinfo;
}

// get buffer info from local disk: ID, GUID, description
bool US_FeMatch::bufinfo_disk( US_DataIO2::EditedData* edata,
   QString& bufId, QString& bufGuid, QString& bufDesc )
{
   bool    bufinfo  = false;
   QString soluGUID = "";

   QString exppath = US_Settings::resultDir() + "/" + edata->runID + "/"
      + edata->runID + "." + edata->dataType + ".xml";

   QFile filei( exppath );
   if ( !filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
      return bufinfo;

DbgLv(2) << "BInfL: runID dType" << edata->runID << edata->dataType;
   QXmlStreamReader xml( &filei );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         QXmlStreamAttributes ats = xml.attributes();

         if ( xml.name() == "buffer" )
         {
            QString id    = ats.value( "id"   ).toString();
            QString guid  = ats.value( "guid" ).toString();
            QString desc  = ats.value( "desc" ).toString();
       
            if ( ! id.isEmpty()  ||  ! guid.isEmpty() )
            {
               bufId         = id  .isEmpty() ? bufId   : id;
               bufGuid       = guid.isEmpty() ? bufGuid : guid;
               bufDesc       = desc.isEmpty() ? bufDesc : desc;
               bufinfo       = true;
               bufId         = bufId.isEmpty() ? "N/A"  : bufId;
            }
            break;
         }

         else if ( xml.name() == "solution" )
         {
            soluGUID      = ats.value( "guid" ).toString();
DbgLv(2) << "BInfL:   soluGUID" << soluGUID;
         }
      }
   }

   filei.close();

   if ( ! bufinfo  &&  ! soluGUID.isEmpty() )
   {  // no buffer info yet, but solution GUID found:  get buffer from solution
      QString spath = US_Settings::dataDir() + "/solutions";
      QDir    f( spath );
      spath         = spath + "/";
      QStringList filter( "S*.xml" );
      QStringList names = f.entryList( filter, QDir::Files, QDir::Name );
      QString fname;
      QString bdens;
      QString bvisc;
      QString bcomp;

      for ( int ii = 0; ii < names.size(); ii++ )
      {
         fname      = spath + names[ ii ];
         QFile filei( fname );

         if ( !filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QXmlStreamReader xml( &filei );

         while ( ! xml.atEnd() )
         {
            xml.readNext();

            if ( xml.isStartElement() )
            {
               QXmlStreamAttributes ats = xml.attributes();

               if (  xml.name() == "solution" )
               {
                  QString sguid = ats.value( "guid" ).toString();
                  if ( sguid != soluGUID )
                     break;
               }

               else if (  xml.name() == "buffer" )
               {
                  QString bid   = ats.value( "id"   ).toString();
                  QString bguid = ats.value( "guid" ).toString();
                  QString bdesc = ats.value( "desc" ).toString();
       
                  if ( ! bid.isEmpty()  ||  ! bguid.isEmpty() )
                  {
                     bufId         = bid  .isEmpty() ? bufId   : bid;
                     bufId         = bufId.isEmpty() ? "N/A"   : bufId;
                     bufGuid       = bguid.isEmpty() ? bufGuid : bguid;
                     bufDesc       = bdesc.isEmpty() ? bufDesc : bdesc;
                     bufinfo       = true;
                  }
                  break;
               }
            }
            if ( bufinfo )
               break;
         }
         if ( bufinfo )
            break;
      }
   }

   return bufinfo;
}

// get buffer values from DB: density, viscosity, compressiblity
bool US_FeMatch::bufvals_db( QString& bufId, QString& bufGuid, QString& bufDesc,
      QString& dens, QString& visc, QString& comp )
{
   bool bufvals = false;

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   QStringList query;
   int idBuf     = bufId.isEmpty() ? -1    : bufId.toInt();
   bufId         = ( idBuf < 1  )  ? "N/A" : bufId;

   if ( bufId == "N/A"  &&  ! bufGuid.isEmpty() )
   {
      query.clear();
      query << "get_bufferID" << bufGuid;
      db.query( query );
      if ( db.lastErrno() != US_DB2::OK )
         qDebug() << "***Unable to get bufferID from GUID" << bufGuid
            << " lastErrno" << db.lastErrno();
      db.next();
      bufId         = db.value( 0 ).toString();
      bufId         = bufId.isEmpty() ? "N/A" : bufId;
   }

   if ( bufId != "N/A" )
   {
      query.clear();
      query << "get_buffer_info" << bufId;
      db.query( query );
      if ( db.lastErrno() != US_DB2::OK )
      {
         qDebug() << "***Unable to get buffer info from bufID" << bufId
            << " lastErrno" << db.lastErrno();
         return bufvals;
      }
      db.next();
      QString ddens = db.value( 5 ).toString();
      QString dvisc = db.value( 4 ).toString();
      QString dcomp = db.value( 2 ).toString();
      dens          = ddens.isEmpty() ? dens : ddens;
      visc          = dvisc.isEmpty() ? visc : dvisc;
      comp          = dcomp.isEmpty() ? comp : dcomp;
      bufvals       = true;
   }

   else
   {
      QString invID  = investig.section( ":", 0, 0 );
      query.clear();
      query << "get_buffer_desc" << invID;
      db.query( query );
      if ( db.lastErrno() != US_DB2::OK )
      {
         qDebug() << "***Unable to get buffer desc from invID" << invID
            << " lastErrno" << db.lastErrno();
         return bufvals;
      }

      while ( db.next() )
      {
         QString desc = db.value( 1 ).toString();
         
         if ( desc == bufDesc )
         {
            bufId         = db.value( 0 ).toString();
            break;
         }
      }

      if ( ! bufId.isEmpty() )
      {
         query.clear();
         query << "get_buffer_info" << bufId;
         db.query( query );
         if ( db.lastErrno() != US_DB2::OK )
         {
            qDebug() << "***Unable to get buffer info from bufID" << bufId
               << " lastErrno" << db.lastErrno();
            return bufvals;
         }
         db.next();
         QString ddens = db.value( 5 ).toString();
         QString dvisc = db.value( 4 ).toString();
         QString dcomp = db.value( 2 ).toString();
         dens          = ddens.isEmpty() ? dens : ddens;
         visc          = dvisc.isEmpty() ? visc : dvisc;
         comp          = dcomp.isEmpty() ? comp : dcomp;
         bufvals       = true;
      }
   }

   return bufvals;
}

// get buffer values from local disk: density, viscosity, compressiblity
bool US_FeMatch::bufvals_disk( QString& bufId, QString& bufGuid,
      QString& bufDesc, QString& dens, QString& visc, QString& comp )
{
   bool bufvals  = false;
   bool dfound   = false;
   QString bpath = US_Settings::dataDir() + "/buffers";
   QDir    f( bpath );
   bpath         = bpath + "/";
   QStringList filter( "B*.xml" );
   QStringList names = f.entryList( filter, QDir::Files, QDir::Name );
   QString fname;
   QString bdens;
   QString bvisc;
   QString bcomp;

   for ( int ii = 0; ii < names.size(); ii++ )
   {
      fname      = bpath + names[ ii ];
      QFile filei( fname );

      if ( !filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
         continue;

DbgLv(2) << "  bvL: ii fname" << ii << names[ii];
      QXmlStreamReader xml( &filei );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "buffer" )
         {
            QXmlStreamAttributes ats = xml.attributes();
            QString bid   = ats.value( "id"          ).toString();
            QString bguid = ats.value( "guid"        ).toString();
            QString bdesc = ats.value( "description" ).toString();

            if ( bguid == bufGuid  ||  bid == bufId )
            {
               bdens    = ats.value( "density"         ).toString();
               bvisc    = ats.value( "viscosity"       ).toString();
               bcomp    = ats.value( "compressibility" ).toString();
               dens     = bdens.isEmpty() ? dens : bdens;
               visc     = bvisc.isEmpty() ? visc : bvisc;
               comp     = bcomp.isEmpty() ? comp : bcomp;
               bufvals  = true;
DbgLv(2) << "  bvL:   i/g I/G dens" << bid << bguid << bufId << bufGuid << dens;
            }

            else if ( bdesc == bufDesc )
            {
               bdens    = ats.value( "density"         ).toString();
               bvisc    = ats.value( "viscosity"       ).toString();
               bcomp    = ats.value( "compressibility" ).toString();
               dfound   = true;
            }

            break;
         }
      }

      if ( bufvals )
         break;
   }

DbgLv(2) << "  bvL:    bufvals dfound" << bufvals << dfound;
   if ( ! bufvals  &&  dfound )
   {
      dens     = bdens.isEmpty() ? dens : bdens;
      visc     = bvisc.isEmpty() ? visc : bvisc;
      comp     = bcomp.isEmpty() ? comp : bcomp;
      bufvals  = true;
   }

   return bufvals;
}

// use dialogs to alert user to change in experiment buffer
bool US_FeMatch::verify_buffer( )
{
   bool changed = true;

   if ( buffLoaded )
   {  // only need verify buffer change while experiment values are loaded
      if ( QMessageBox::No == QMessageBox::warning( this,
               tr( "Warning" ),
               tr( "Attention:\n"
                   "You are attempting to override buffer parameters\n"
                   "that have been set from the experimental data!\n\n"
                   "Do you really want to override them?" ),
               QMessageBox::Yes, QMessageBox::No ) )
      {  // "No":  retain loaded values, mark unchanged
         QMessageBox::information( this,
            tr( "Buffer Retained" ),
            tr( "Buffer parameters from the experiment will be retained" ) );
         changed    = false;
      }

      else
      {  // "Yes":  change values,  mark experiment values no longer used
         QMessageBox::information( this,
            tr( "Buffer Overridden" ),
            tr( "Buffer parameters from the experiment will be overridden" ) );
         buffLoaded = false;
      }
   }

   return changed;
}

// slot to respond to text box change to buffer parameter
void US_FeMatch::buffer_text( )
{
   if ( buffLoaded )
   {  // only need verify desire to change while experiment values are loaded
      bool changed = verify_buffer();

      if ( changed )
      {  // "Yes" to change: pick up values as entered and turn off loaded flag
         buffLoaded   = false;
         density      = le_density  ->text().toDouble();
         viscosity    = le_viscosity->text().toDouble();
         compress     = le_compress ->text().toDouble();
      }

      else
      {  // "No" to change:  restore text and insure loaded flag still on
         buffLoaded   = false;
         le_density  ->setText( QString::number( density,   'f', 6 ) );
         le_viscosity->setText( QString::number( viscosity, 'f', 5 ) );
         le_compress ->setText( QString::number( compress,  'e', 3 ) );
         qApp->processEvents();
         buffLoaded   = true;
      }
   }
}

// use dialogs to alert user to change in experiment solution common vbar
bool US_FeMatch::verify_vbar( )
{
   bool changed = true;

   if ( buffLoaded )
   {  // only need verify vbar change while experiment values are loaded
      if ( QMessageBox::No == QMessageBox::warning( this,
               tr( "Warning" ),
               tr( "Attention:\n"
                   "You are attempting to override the vbar parameter\n"
                   "that has been set from the experimental data!\n\n"
                   "Do you really want to override it?" ),
               QMessageBox::Yes, QMessageBox::No ) )
      {  // "No":  retain loaded value, mark unchanged
         QMessageBox::information( this,
            tr( "Vbar Retained" ),
            tr( "Vbar parameter from the experiment will be retained" ) );
         changed    = false;
      }

      else
      {  // "Yes":  change value,  mark experiment values no longer used
         QMessageBox::information( this,
            tr( "Vbar Overridden" ),
            tr( "Vbar parameter from the experiment will be overridden" ) );
         buffLoaded = false;
      }
   }

   qApp->processEvents();
   return changed;
}

// slot to respond to text box change to vbar parameter
void US_FeMatch::vbar_text( )
{
   if ( buffLoaded )
   {  // only need verify desire to change while experiment values are loaded
      bool changed = verify_vbar();
      buffLoaded   = false;

      if ( changed )
      {  // "Yes" to change: pick up values as entered and turn off loaded flag
         vbar         = le_vbar->text().toDouble();
      }

      else
      {  // "No" to change:  restore text and insure loaded flag still on
         le_vbar->setText( QString::number( vbar, 'f', 5 ) );
         qApp->processEvents();
         buffLoaded   = true;
      }
   }
}

