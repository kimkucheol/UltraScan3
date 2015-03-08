//! \file us_vhwc_pltctl.cpp

#include "us_vhw_combine.h"
#include "us_vhwc_pltctl.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"

#include <qwt_legend.h>

// Constructor:  enhanced spectra plot control widget
US_VhwCPlotControl::US_VhwCPlotControl( QWidget* p, QVector< QVector3D >* d,
      bool a_envel )
   : US_WidgetsDialog( 0, 0 )
{
qDebug() << "PCtrl: IN";
   wparent        = p;
   xyzdat         = d;
   envel          = a_envel;
qDebug() << "PCtrl: xyzdat count" << xyzdat->count();

   setObjectName( "US_VhwCPlotControl" );
   setAttribute( Qt::WA_DeleteOnClose, true );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // Lay out the GUI
   setWindowTitle( tr( "3-D Plotting Controls" ) );

   mainLayout      = new QVBoxLayout( this );
   controlsLayout  = new QGridLayout( );
   buttonsLayout   = new QHBoxLayout( );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   mainLayout->addLayout( controlsLayout );
   mainLayout->addLayout( buttonsLayout  );

   QLabel*      lb_zscalefac = us_label(  tr( "Z Scale Factor:"   ) );
   QLabel*      lb_rxscale   = us_label(  tr( "Relative X Scale:" ) );
   QLabel*      lb_ryscale   = us_label(  tr( "Relative Y Scale:" ) );
   QLabel*      lb_peaksmoo  = us_label(  tr( "Peak Smoothing"    ) );
   QLabel*      lb_peakwid   = us_label(  tr( "Peak Width:"       ) );
   QLabel*      lb_gridres   = us_label(  tr( "Grid Resolution:"  ) );

   QPushButton* pb_plot3d    = us_pushbutton( tr( "3D Plot" ) );
   QPushButton* pb_help      = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close     = us_pushbutton( tr( "Close" ) );

   QLayout*     lo_yrevrs    = us_checkbox( tr( "Reverse Y"    ), ck_yrevrs,
                                            true );
   QLayout*     lo_contour   = us_checkbox( tr( "Contour Plot" ), ck_contour,
                                            false );

                ct_peaksmoo  = us_counter( 3,    1,  200,    1 );
                ct_peakwid   = us_counter( 3, 0.01, 10.0, 0.01 );
                ct_gridres   = us_counter( 3,   50,  600,   10 );
                ct_zscalefac = us_counter( 3, 0.01, 1000, 0.01 );
                ct_rxscale   = us_counter( 3, 0.01, 1000, 0.01 );
                ct_ryscale   = us_counter( 3, 0.01, 1000, 0.01 );

   int row = 0;
   controlsLayout->addWidget( lb_zscalefac, row,   0, 1, 2 );
   controlsLayout->addWidget( ct_zscalefac, row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_rxscale,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_rxscale,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_ryscale,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_ryscale,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_peaksmoo,  row,   0, 1, 2 );
   controlsLayout->addWidget( ct_peaksmoo,  row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_peakwid,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_peakwid,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_gridres,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_gridres,   row++, 2, 1, 2 );
   controlsLayout->addLayout( lo_contour,   row,   0, 1, 2 );
   controlsLayout->addLayout( lo_yrevrs,    row++, 2, 1, 2 );

   buttonsLayout->addWidget( pb_plot3d );
   buttonsLayout->addWidget( pb_help   );
   buttonsLayout->addWidget( pb_close  );

   zscale   = 1.0;
   rxscale  = 1.0;
   ryscale  = 1.0;
   pksmooth = 80.0;
   pkwidth  = 0.1;
   gridres  = 150;
   ct_zscalefac->setValue( zscale   );
   ct_rxscale  ->setValue( rxscale  );
   ct_ryscale  ->setValue( ryscale  );
   ct_peaksmoo ->setValue( pksmooth );
   ct_peakwid  ->setValue( pkwidth  );
   ct_gridres  ->setValue( gridres  );
   ct_zscalefac->setStep(  0.01 );
   ct_rxscale  ->setStep(  0.01 );
   ct_ryscale  ->setStep(  0.01 );
   ct_peaksmoo ->setStep(     1 );
   ct_peakwid  ->setStep(  0.01 );
   ct_gridres  ->setStep(    10 );

   connect( pb_plot3d, SIGNAL( clicked()   ),
            this,      SLOT  ( plot3_btn() ) );
   connect( pb_help,   SIGNAL( clicked()   ),
            this,      SLOT  ( help()      ) );
   connect( pb_close,  SIGNAL( clicked()   ),
            this,      SLOT  ( close_all() ) );

   plot3d_w = 0;

   ct_zscalefac->setMinimumWidth( lb_zscalefac->width() );
   adjustSize();
qDebug() << "PCtrl:  w size" << size();
}

// Return caller of plot_control
QWidget* US_VhwCPlotControl::caller( void )
{
   return wparent;
}

// Public slot to force (re-)plot of currently specified 3-D plot
void US_VhwCPlotControl::do_3dplot()
{
   plot3_btn();
}

// Public slot to return a pointer to the 3D plot data widget
QGLWidget* US_VhwCPlotControl::data_3dplot( void )
{
   QGLWidget* widgP = (QGLWidget*)0;
   widgP            = ( plot3d_w != 0 ) ? plot3d_w->dataWidgetP() : widgP;

   return widgP;
}

// Public slot to return a pointer to the 3D plot main widget
US_Plot3Dxyz* US_VhwCPlotControl::widget_3dplot( void )
{
   return plot3d_w;
}

// 3D Plot button clicked
void US_VhwCPlotControl::plot3_btn()
{
qDebug() << "PCtrl:  plot3_btn";
   QString wtitle  = tr( "Multiwavelength 3-D vHW Viewer" );
   QString ptitle  = tr( "MWL 3-D Plot, vanHolde-Weischet Distributions" );
   QString xatitle = tr( "Lambda(nm)" );
   QString yatitle = tr( "Sed.C.(*e+13)" );
   QString zatitle = envel ? tr( "Concen." ) : tr( "B.Frac." );
   zscale          = ct_zscalefac->value();
   rxscale         = ct_rxscale  ->value();
   ryscale         = ct_ryscale  ->value();
   ryscale         = ( ck_yrevrs->isChecked() ) ? -ryscale : ryscale;
qDebug() << "PCtrl:  plot3_btn: scales" << rxscale << ryscale << zscale;
   bool contour    = ck_contour->isChecked();
   pksmooth        = contour ? 0.0 : ct_peaksmoo ->value();
   pkwidth         = contour ? 0.0 : ct_peakwid  ->value();
qDebug() << "PCtrl:  plot3_btn: smoo.." << pksmooth << pkwidth << gridres;

   int nidpt       = xyzdat->count();
qDebug() << "PCtrl:  plot3_btn: nidpt" << nidpt;
   QList< double > xvals;

   for ( int ii = 0; ii < nidpt; ii++ )
   {  // Accumulate unique X values so that we can count rows
      double xval     = xyzdat->at( ii ).x();
      if ( ! xvals.contains( xval ) )
         xvals << xval;
   }

   int nrow        = xvals.count();  // Row count is number of unique X's
   int ncol        = nidpt / nrow;   // Column count is Total/Rows
   gridres         = contour ? (int)ct_gridres  ->value() : ncol;
qDebug() << "PCtrl:  ncol nrow" << ncol << nrow;

   if ( plot3d_w == 0 )
   {  // If no 3-D plot window, bring it up now
      plot3d_w     = new US_Plot3Dxyz( this, xyzdat );
      // Position so a corner is near a control dialog corner
      int cx       = x();
      int cy       = y();
      int ncx      = width();
      int ncy      = height();
      int nhdx     = qApp->desktop()->width()  / 2;
      int nhdy     = qApp->desktop()->height() / 2;
      int nwx      = plot3d_w->width();
      int nwy      = plot3d_w->height();
      int wx       = ( cx < nhdx ) ? ( cx + ncx + 20 ) : ( cx - nwx - 20 );
      int wy       = ( cy < nhdy ) ? ( cy + ncy + 20 ) : ( cy - nwy - 20 );
      plot3d_w->move( wx, wy );
   }

   // Set plot window parameters; do initial plot; and make it visible
#if 0
   zscale         *= 8000.0;
   rxscale        *= 100.0;
   ryscale        *= 100.0;
#endif
#if 1
   zscale         *= 240.0;
   rxscale        *= 10.0;
   ryscale        *= 2.0;
#endif
   plot3d_w->setTitles    ( wtitle, ptitle, xatitle, yatitle, zatitle );
   plot3d_w->setParameters( ncol, nrow, rxscale, ryscale, zscale,
                            pksmooth, pkwidth );
   plot3d_w->replot       ( );

   plot3d_w->setVisible( true );
}

// Close button clicked
void US_VhwCPlotControl::close_all()
{
   if ( plot3d_w != 0 )      // Close 3-D plot window if need be
      plot3d_w->close();

   emit has_closed();        // Send a signal of the closing

   close();                  // And go away
}

