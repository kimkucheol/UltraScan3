//! \file us_resplot.cpp

#include "us_resplot.h"
#include "us_fematch.h"
#include "us_settings.h"
#include "us_gui_settings.h"

#include <qwt_legend.h>

// constructor:  residuals plot widget
US_ResidPlot::US_ResidPlot( QWidget* p = 0 )
   : US_WidgetsDialog( p, 0 )
{
   // lay out the GUI
   setWindowTitle( tr( "Finite Element Data/Residuals Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );

   mainLayout      = new QHBoxLayout( this );
   leftLayout      = new QVBoxLayout();
   rightLayout     = new QVBoxLayout();
   datctrlsLayout  = new QGridLayout();
   resctrlsLayout  = new QGridLayout();
   buttonsLayout   = new QVBoxLayout();

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   mainLayout->addLayout( leftLayout );
   mainLayout->addLayout( rightLayout  );

   QLabel* lb_datctrls    = us_banner( tr( "FE Analysis Data Viewer" ) );
   QLabel* lb_resctrls    = us_banner( tr( "FE Analysis Residuals Viewer" ) );
   QLabel* lb_vari        = us_label(  tr( "Variance:" ) );
   QLabel* lb_rmsd        = us_label(  tr( "RMSD:" ) );

   QPushButton* pb_write  = us_pushbutton( tr( "Write Ti/Ri Noise" ) );
   QPushButton* pb_close  = us_pushbutton( tr( "Close" ) );

   ck_plteda = new QCheckBox( tr( "Plot Experimental Data" ) );
   ck_subtin = new QCheckBox( tr( "Subtract Time Invariant Noise" ) );
   ck_subrin = new QCheckBox( tr( "Subtract Radially Invariant Noise" ) );
   ck_pltsda = new QCheckBox( tr( "Plot Simulated/Modeled Data" ) );
   ck_addtin = new QCheckBox( tr( "Add Time Invariant Noise" ) );
   ck_addrin = new QCheckBox( tr( "Add Radially Invariant Noise" ) );
   ck_pltres = new QCheckBox( tr( "Plot Residuals" ) );
   ck_plttin = new QCheckBox( tr( "Plot Time Invariant Noise" ) );
   ck_pltrin = new QCheckBox( tr( "Plot Radially Invariant Noise" ) );
   ck_pltran = new QCheckBox( tr( "Plot Random Noise" ) );
   ck_shorbm = new QCheckBox( tr( "Show Residuals Bitmap" ) );

   le_vari   = us_lineedit();
   le_rmsd   = us_lineedit();
   le_vari->setReadOnly( true );
   le_rmsd->setReadOnly( true );

   datctrlsLayout->addWidget( lb_datctrls, 0, 0, 1, 8 );
   datctrlsLayout->addWidget( ck_plteda,   1, 0, 1, 8 );
   datctrlsLayout->addWidget( ck_subtin,   2, 1, 1, 7 );
   datctrlsLayout->addWidget( ck_subrin,   3, 1, 1, 7 );
   datctrlsLayout->addWidget( ck_pltsda,   4, 0, 1, 8 );
   datctrlsLayout->addWidget( ck_addtin,   5, 1, 1, 7 );
   datctrlsLayout->addWidget( ck_addrin,   6, 1, 1, 7 );

   resctrlsLayout->addWidget( lb_resctrls, 0, 0, 1, 8 );
   resctrlsLayout->addWidget( ck_pltres,   1, 0, 1, 8 );
   resctrlsLayout->addWidget( ck_plttin,   2, 0, 1, 8 );
   resctrlsLayout->addWidget( ck_pltrin,   3, 0, 1, 8 );
   resctrlsLayout->addWidget( ck_pltran,   4, 0, 1, 8 );
   resctrlsLayout->addWidget( ck_shorbm,   5, 0, 1, 8 );
   resctrlsLayout->addWidget( lb_vari,     6, 0, 1, 3 );
   resctrlsLayout->addWidget( le_vari,     6, 3, 1, 5 );
   resctrlsLayout->addWidget( lb_rmsd,     7, 0, 1, 3 );
   resctrlsLayout->addWidget( le_rmsd,     7, 3, 1, 5 );

   buttonsLayout->addWidget( pb_write );
   buttonsLayout->addWidget( pb_close );

   ck_plteda->setChecked( true  );
   ck_subtin->setChecked( false );

   ck_plteda->setFont( pb_close->font() );
   ck_subtin->setFont( pb_close->font() );
   ck_subrin->setFont( pb_close->font() );
   ck_pltsda->setFont( pb_close->font() );
   ck_addtin->setFont( pb_close->font() );
   ck_addrin->setFont( pb_close->font() );
   ck_pltres->setFont( pb_close->font() );
   ck_plttin->setFont( pb_close->font() );
   ck_pltrin->setFont( pb_close->font() );
   ck_pltran->setFont( pb_close->font() );
   ck_shorbm->setFont( pb_close->font() );

   ck_plteda->setPalette( US_GuiSettings::normalColor() );
   ck_subtin->setPalette( US_GuiSettings::normalColor() );
   ck_subrin->setPalette( US_GuiSettings::normalColor() );
   ck_pltsda->setPalette( US_GuiSettings::normalColor() );
   ck_addtin->setPalette( US_GuiSettings::normalColor() );
   ck_addrin->setPalette( US_GuiSettings::normalColor() );
   ck_pltres->setPalette( US_GuiSettings::normalColor() );
   ck_plttin->setPalette( US_GuiSettings::normalColor() );
   ck_pltrin->setPalette( US_GuiSettings::normalColor() );
   ck_pltran->setPalette( US_GuiSettings::normalColor() );
   ck_shorbm->setPalette( US_GuiSettings::normalColor() );

   plotLayout1 = new US_Plot( data_plot1,
         tr( "Experimental Data" ),
         tr( "Radius (cm)" ),
         tr( "Absorbance" ) );
   plotLayout2 = new US_Plot( data_plot2,
         tr( "Residuals" ),
         tr( "Radius (cm)" ),
         tr( "OD Difference" ) );
   data_plot1->setCanvasBackground( Qt::black );
   data_plot2->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( 600, 240 );
   data_plot2->setMinimumSize( 600, 240 );

   leftLayout ->addLayout( datctrlsLayout );
   leftLayout ->addLayout( resctrlsLayout );
   leftLayout ->addStretch();
   leftLayout ->addLayout( buttonsLayout  );
   rightLayout->addLayout( plotLayout1    );
   rightLayout->addLayout( plotLayout2    );

   connect( ck_plteda, SIGNAL( toggled( bool ) ),
            this,      SLOT( pedaCheck( bool ) ) );
   connect( ck_subtin, SIGNAL( toggled( bool ) ),
            this,      SLOT( stinCheck( bool ) ) );
   connect( ck_subrin, SIGNAL( toggled( bool ) ),
            this,      SLOT( srinCheck( bool ) ) );
   connect( ck_pltsda, SIGNAL( toggled( bool ) ),
            this,      SLOT( psdaCheck( bool ) ) );
   connect( ck_addtin, SIGNAL( toggled( bool ) ),
            this,      SLOT( atinCheck( bool ) ) );
   connect( ck_addrin, SIGNAL( toggled( bool ) ),
            this,      SLOT( arinCheck( bool ) ) );
   connect( ck_pltres, SIGNAL( toggled( bool ) ),
            this,      SLOT( presCheck( bool ) ) );
   connect( ck_plttin, SIGNAL( toggled( bool ) ),
            this,      SLOT( ptinCheck( bool ) ) );
   connect( ck_pltrin, SIGNAL( toggled( bool ) ),
            this,      SLOT( prinCheck( bool ) ) );
   connect( ck_pltran, SIGNAL( toggled( bool ) ),
            this,      SLOT( pranCheck( bool ) ) );
   connect( ck_shorbm, SIGNAL( toggled( bool ) ),
            this,      SLOT( srbmCheck( bool ) ) );

   connect( pb_write,  SIGNAL( clicked()   ),
            this,      SLOT( write_noise() ) );
   connect( pb_close,  SIGNAL( clicked()   ),
            this,      SLOT( close_all()   ) );

   // get data pointers from parent of parent

   have_ed   = false;
   have_sd   = false;
   have_ti   = false;
   have_ri   = false;
   have_bm   = false;
   skip_plot = true;

   if ( p )
   {
      if ( p->parent() )
      {
qDebug() << "RP: grandparent" << p->parent()->objectName();
         US_FeMatch* fem = (US_FeMatch*)p->parent();
         edata           = fem->fem_editdata();
         sdata           = fem->fem_simdata();
         ti_noise        = fem->fem_ti_noise();
         ri_noise        = fem->fem_ri_noise();
         resbmap         = fem->fem_resbmap();
         have_ed         = ( edata != 0 );
         have_sd         = ( sdata != 0 );
         have_ti         = ( ti_noise != 0  &&  ti_noise->count > 0 );
         have_ri         = ( ri_noise != 0  &&  ri_noise->count > 0 );
         have_bm         = ( resbmap != 0 );
qDebug() << "RP:edata  " << have_ed;
qDebug() << "RP:sdata  " << have_sd;
qDebug() << "RP:ti_noise count" << (have_ti ? ti_noise->count : 0);
qDebug() << "RP:ri_noise count" << (have_ri ? ri_noise->count : 0);
qDebug() << "RP:resbmap" << have_bm;
      }
      else
      {
         qDebug() << "*ERROR* unable to get RP grandparent";
      }
   }

   else
   {
      qDebug() << "*ERROR* unable to get RP parent";
   }

   ck_subtin->setEnabled( have_ti );
   ck_subrin->setEnabled( have_ri );
   ck_addtin->setEnabled( false );
   ck_addrin->setEnabled( false );
   ck_plttin->setEnabled( have_ti );
   ck_pltrin->setEnabled( have_ri );
   ck_pltrin->setEnabled( have_ri );


   ck_plteda->setChecked( true );
   skip_plot = false;

   plot_data();

   setVisible( true );

}

// plot-experimental-data box [un]checked
void US_ResidPlot::pedaCheck( bool chkd )
{
   if ( chkd )
   {  // box is being checked:  sub boxes enabled if data present
      ck_subtin->setEnabled( have_ti );
      ck_subrin->setEnabled( have_ri );
   }

   else
   {  // box is being unchecked:  sub boxes disabled
      ck_subtin->setEnabled( false );
      ck_subrin->setEnabled( false );
   }

   plot_data();
}

// subtract-ti-noise box [un]checked
void US_ResidPlot::stinCheck( bool )
{
   plot_data();
}

// subtract-ri-noise box [un]checked
void US_ResidPlot::srinCheck( bool )
{
   plot_data();
}

// plot-simulation-data box [un]checked
void US_ResidPlot::psdaCheck( bool chkd )
{
   if ( chkd )
   {  // box is being checked:  sub boxes enabled if data present
      ck_addtin->setEnabled( have_ti );
      ck_addrin->setEnabled( have_ri );
   }

   else
   {  // box is being unchecked:  sub boxes disabled
      ck_addtin->setEnabled( false );
      ck_addrin->setEnabled( false );
   }

   plot_data();
}

// add-ti-noise box [un]checked
void US_ResidPlot::atinCheck( bool )
{
   plot_data();
}
// add-ri-noise box [un]checked
void US_ResidPlot::arinCheck( bool )
{
   plot_data();
}
// plot-residuals  box [un]checked
void US_ResidPlot::presCheck( bool chkd )
{
   skip_plot = true;

   if ( chkd )
   {  // alternate residual plots unchecked
      ck_plttin->setChecked( false );
      ck_pltrin->setChecked( false );
      ck_pltran->setChecked( false );
   }

   skip_plot = false;

   plot_data();
}
// plot-ti-noise box [un]checked
void US_ResidPlot::ptinCheck( bool chkd )
{
   skip_plot = true;

   if ( chkd )
   {  // alternate residual plots unchecked
      ck_pltres->setChecked( false );
      ck_pltrin->setChecked( false );
      ck_pltran->setChecked( false );
   }

   skip_plot = false;

   plot_data();
}
// plot-ri-noise box [un]checked
void US_ResidPlot::prinCheck( bool chkd )
{
   skip_plot = true;

   if ( chkd )
   {  // alternate residual plots unchecked
      ck_pltres->setChecked( false );
      ck_plttin->setChecked( false );
      ck_pltran->setChecked( false );
   }

   skip_plot = false;

   plot_data();
}
// plot-random-noise box [un]checked
void US_ResidPlot::pranCheck( bool chkd )
{
   skip_plot = true;

   if ( chkd )
   {  // alternate residual plots unchecked
      ck_pltres->setChecked( false );
      ck_plttin->setChecked( false );
      ck_pltrin->setChecked( false );
   }

   skip_plot = false;

   plot_data();
}
// show-residual-bitmap box [un]checked
void US_ResidPlot::srbmCheck( bool chkd )
{
   if ( chkd )
   {  // bitmap checked:  replot to possibly build new map

      if ( have_bm )
      {  // if bitmap exists already, detect if closed
         connect( resbmap, SIGNAL( destroyed()   ),
                  this,    SLOT( resids_closed() ) );
      }

      plot_data();
   }
}

// write-noise button clicked
void US_ResidPlot::write_noise()
{
}

// close button clicked
void US_ResidPlot::close_all()
{
   close();
}

// plot the data
void US_ResidPlot::plot_data()
{
   if ( skip_plot )  // avoid redundant successive calls
      return;

   plot_edata();     // plot experimental

   plot_rdata();     // plot residuals
}

// plot the experimental data
void US_ResidPlot::plot_edata()
{
double ptfac=0.82;
   data_plot1->detachItems();
   data_plot1->clear();

   bool   do_plteda = have_ed  &&  ck_plteda->isChecked();
   bool   do_pltsda = have_sd  &&  ck_pltsda->isChecked();
   bool   do_addtin = have_ti  &&  ck_addtin->isChecked();
   bool   do_subtin = have_ti  &&  ck_subtin->isChecked();
   bool   do_addrin = have_ri  &&  ck_addrin->isChecked();
   bool   do_subrin = have_ri  &&  ck_subrin->isChecked();

   int    points    = 0;
   int    count     = 0;
   int    ii;
   int    jj;
   double tinoi     = 0.0;
   double rinoi     = 0.0;

   if ( !do_plteda  &&  !do_pltsda )
   {  // no real experimental plot specified:  re-do plot and return
      data_plot1->replot();
      return;
   }

   us_grid( data_plot1 );

   if ( do_plteda )
   {  // get title and values count for experimental data
      data_plot1->setAxisTitle( QwtPlot::yLeft,
         tr( "Absorbance at " ) + edata->wavelength + tr( " nm" ) );
      points   = edata->scanData[ 0 ].readings.size();
   }

   if ( do_pltsda )
   {  // get title and values count for simulation data
      if ( have_ed )
         data_plot1->setAxisTitle( QwtPlot::yLeft,
            tr( "Absorbance at " ) + edata->wavelength + tr( " nm" ) );
      else
         data_plot1->setAxisTitle( QwtPlot::yLeft, tr( "Absorbance" ) );

      count    = sdata->scanData[ 0 ].readings.size();
   }

   count    = ( points > count ) ? points : count;  // maximum readings count

   double* rr  = new double[ count ];
   double* vv  = new double[ count ];

   QString       title;
   QwtPlotCurve* curv;
   QPen          pen_red(  Qt::red );
   QPen          pen_plot( US_GuiSettings::plotCurve() );

   if ( do_plteda )
   {  // plot experimental curves
      points   = edata->scanData[ 0 ].readings.size();
      count    = edata->scanData.size();
points=qRound(points*ptfac);

      for ( jj = 0; jj < points; jj++ )
      {  // get radii (x) just once
         rr[ jj ] = edata->radius( jj );
      }

      for ( ii = 0; ii < count; ii++ )
      {  // get readings (y) for each scan
         rinoi    = ( do_subrin ? ri_noise->values[ ii ] : 0.0 );

         for ( jj = 0; jj < points; jj++ )
         {  // each y is reading, optionally minus some noise
            tinoi    = ( do_subtin ? ti_noise->values[ jj ] : 0.0 );
            vv[ jj ] = edata->value( ii, jj ) - rinoi - tinoi;
         }

         title   = tr( "Curve " ) + QString::number( ii );
         curv    = us_curve( data_plot1, title );

         curv->setPen( pen_plot );
         curv->setData( rr, vv, points );
      }
   }

   if ( do_pltsda )
   {  // plot simulation curves
      points   = sdata->scanData[ 0 ].readings.size();
      count    = sdata->scanData.size();
points=qRound(points*ptfac);

      for ( jj = 0; jj < points; jj++ )
      {  // get radii (x) just once
         rr[ jj ] = sdata->radius( jj );
      }

      for ( ii = 0; ii < count; ii++ )
      {  // get readings (y) for each scan
         rinoi    = ( do_addrin ? ri_noise->values[ ii ] : 0.0 );

         for ( jj = 0; jj < points; jj++ )
         {  // each y is reading, optionally minus some noise
            tinoi    = ( do_addtin ? ti_noise->values[ jj ] : 0.0 );
            vv[ jj ] = sdata->value( ii, jj ) + rinoi + tinoi;
         }

         title   = tr( "S-Curve " ) + QString::number( ii );
         curv    = us_curve( data_plot1, title );

         curv->setPen( pen_red );
         curv->setData( rr, vv, points );
      }
   }

   data_plot1->replot();

   delete [] rr;
   delete [] vv;
}

// plot the residual data
void US_ResidPlot::plot_rdata()
{
double ptfac=0.82;
   data_plot2->detachItems();
   data_plot2->clear();

   bool   do_pltres = have_ed  &&  ck_pltres->isChecked()  &&  have_sd;
   bool   do_plttin = have_ti  &&  ck_plttin->isChecked();
   bool   do_pltrin = have_ri  &&  ck_pltrin->isChecked();
   bool   do_pltran = have_ed  &&  ck_pltran->isChecked()  &&  have_sd;
   bool   do_shorbm = have_ed  &&  ck_shorbm->isChecked()  &&  have_sd;
   bool   do_addtin = have_ti  &&  ck_addtin->isChecked();
   bool   do_subtin = have_ti  &&  ck_subtin->isChecked();
   bool   do_addrin = have_ri  &&  ck_addrin->isChecked();
   bool   do_subrin = have_ri  &&  ck_subrin->isChecked();

   int    points    = 0;
   int    count     = 0;
   int    ii;
   int    jj;
   double tinoi     = 0.0;
   double rinoi     = 0.0;
   double evalu     = 0.0;
   double svalu     = 0.0;
   double rmsd      = 0.0;

   if ( !do_pltres  &&  !do_plttin  && !do_pltrin  &&
        !do_pltran  &&  !do_shorbm )
   {  // if no plots specified, just clean up plot and return
      data_plot2->replot();
      return;
   }

   us_grid( data_plot2 );
   data_plot2->setAxisTitle( QwtPlot::xBottom, tr( "Radius (cm)" ) );

   if ( have_ed )
   {
      points   = edata->scanData[ 0 ].readings.size();
   }

   if ( have_sd )
   {
      count    = sdata->scanData[ 0 ].readings.size();
   }

   count    = ( points > count ) ? points : count;

   double* rr  = new double[ count ];
   double* vv  = new double[ count ];

   QString       title;
   QwtPlotCurve* curv;
   QPen          pen_plot( Qt::green );


   if ( do_pltres )
   {  // plot residuals
      points   = sdata->scanData[ 0 ].readings.size();
      count    = sdata->scanData.size();
points=qRound(points*ptfac);

      for ( jj = 0; jj < points; jj++ )
      {  // get radii (x) just once
         rr[ jj ] = sdata->radius( jj );
      }

      for ( ii = 0; ii < count; ii++ )
      {  // get readings (y) for each scan

         for ( jj = 0; jj < points; jj++ )
         {  // each residual is e-value minus s-value
            evalu    = edata->value( ii, jj );
            svalu    = sdata->value( ii, jj );
            evalu   -= svalu;
            vv[ jj ] = evalu;
            rmsd    += ( evalu * evalu );
         }

         title   = tr( "resids " ) + QString::number( ii );
         curv    = us_curve( data_plot2, title );

         curv->setPen(   pen_plot );
         curv->setStyle( QwtPlotCurve::Dots );
         curv->setData(   rr, vv, points );
      }

      // display variance and RMSD
      rmsd   /= (double)( count * points );
      le_vari->setText( QString::number( rmsd ) );
      rmsd    = sqrt( rmsd );
      le_rmsd->setText( QString::number( rmsd ) );
   }

   else if ( do_plttin )
   {  // plot time-invariant noise
      points   = edata->scanData[ 0 ].readings.size();
points=qRound(points*ptfac);

      for ( jj = 0; jj < points; jj++ )
      {  // accumulate radii and noise values
         rr[ jj ] = edata->radius( jj );
         vv[ jj ] = ti_noise->values[ jj ];
      }

      title   = tr( "ti_noise" );
      curv    = us_curve( data_plot2, title );

      curv->setPen(  pen_plot );
      curv->setData( rr, vv, points );
   }

   else if ( do_pltrin )
   {  // plot radially-invariant noise
      count    = edata->scanData.size();

      for ( ii = 0; ii < count; ii++ )
      {  // accumulate scan numbers and noise values
         rr[ ii ] = (double)( ii + 1 );
         vv[ ii ] = ri_noise->values[ ii ];
      }

      title   = tr( "ri_noise" );
      curv    = us_curve( data_plot2, title );

      data_plot2->setAxisTitle( QwtPlot::xBottom, tr( "Scan Number" ) );
      curv->setPen(  pen_plot );
      curv->setData( rr, vv, count );
   }

   else if ( do_pltran )
   {  // plot random noise
      points   = sdata->scanData[ 0 ].readings.size();
      count    = sdata->scanData.size();
points=qRound(points*ptfac);

      for ( jj = 0; jj < points; jj++ )
      {  // get radii (x) just once
         rr[ jj ] = sdata->radius( jj );
      }

      for ( ii = 0; ii < count; ii++ )
      {  // get random noise (y) for each scan
         rinoi    = ( do_addrin ? ri_noise->values[ ii ] : 0.0 );
         rinoi   += ( do_subrin ? ri_noise->values[ ii ] : 0.0 );

         for ( jj = 0; jj < points; jj++ )
         {  // each random value is e-value minus s-value with optional noise
            evalu    = edata->value( ii, jj );
            svalu    = sdata->value( ii, jj );
            tinoi    = ( do_addtin ? ti_noise->values[ jj ] : 0.0 );
            tinoi   += ( do_subtin ? ti_noise->values[ jj ] : 0.0 );
            evalu   -= svalu;
            vv[ jj ] = evalu - rinoi - tinoi;
            rmsd    += ( evalu * evalu );
         }

         title   = tr( "random noise " ) + QString::number( ii );
         curv    = us_curve( data_plot2, title );

         curv->setPen(   pen_plot );
         curv->setStyle( QwtPlotCurve::Dots );
         curv->setData(   rr, vv, points );
      }

      // display variance and RMSD
      rmsd   /= (double)( count * points );
      le_vari->setText( QString::number( rmsd ) );
      rmsd    = sqrt( rmsd );
      le_rmsd->setText( QString::number( rmsd ) );
   }


   if ( do_shorbm )
   {  // show residuals bitmap (if not already shown)

      if ( resbmap )
      {  // already have resbmap:  just raise existing window
         resbmap->raise();
         resbmap->activateWindow();
      }

      else
      {  // need to create new resbmap
         QVector< QVector< double > > resids;
         QVector< double >            resscan;

         points   = sdata->scanData[ 0 ].readings.size();
         count    = sdata->scanData.size();
points=qRound(points*ptfac);
         resids .resize( count );
         resscan.resize( points );

         for ( ii = 0; ii < count; ii++ )
         {  // build a vector for each scan

            for ( jj = 0; jj < points; jj++ )
            {  // build residual values within a scan
               evalu         = edata->value( ii, jj );
               svalu         = sdata->value( ii, jj );
               evalu        -= svalu;
               resscan[ jj ] = evalu;
               rmsd         += ( evalu * evalu );
            }

            resids[ ii ] = resscan;
         }

         rmsd   /= (double)( count * points );
         le_vari->setText( QString::number( rmsd ) );
         rmsd    = sqrt( rmsd );
         le_rmsd->setText( QString::number( rmsd ) );

         // pop up a little dialog with residuals bitmap
         resbmap = new US_ResidsBitmap( resids );
         connect( resbmap, SIGNAL( destroyed() ),
                  this,    SLOT(   resids_closed() ) );
         resbmap->move( this->pos() + QPoint( 100, 100 ) );
         resbmap->show();
         resbmap->raise();
         qApp->processEvents();
      }
   }

   // display curves we have created; then clean up

   data_plot2->replot();

   delete [] rr;
   delete [] vv;
}

// react to residual bitmap having been closed
void US_ResidPlot::resids_closed()
{
qDebug() << "Resids BitMap Closed!!!";
   resbmap = 0;
   have_bm = false;
   ck_shorbm->setChecked( false );
}
