//! \file us_ri_noise.cpp

#include "us_ri_noise.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"

US_RiNoise::US_RiNoise( const US_DataIO2::RawData& raw, 
                        const QList< int >&        Includes,
                        int&                       initial_order, 
                        QList< double >&           r )
  : US_WidgetsDialog( 0, 0 ), 
    data( raw ), includes( Includes ), order( initial_order ), residuals( r )
{
   setWindowTitle( tr( "Determine Radial Invariant Noise" ) );
   setPalette( US_GuiSettings::frameColor() );

   QHBoxLayout* main = new QHBoxLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   if ( order < 4 ) order = 4;

   QVBoxLayout* info  = new QVBoxLayout();
   
   te_details = us_textedit();
   te_details->setReadOnly( true );
   info->addWidget( te_details );

   QHBoxLayout* spin = new QHBoxLayout;

   QLabel* lb_spin = us_label( tr( "Fit Order:" ) );
   spin->addWidget( lb_spin );

   ct_order = us_counter( 1, 4.0, 9.0, (double)order );
   ct_order->setStep( 1.0 );
   connect( ct_order, SIGNAL( valueChanged ( double ) ),
                      SLOT  ( draw_fit     ( double ) ) );
   spin->addWidget( ct_order );
   info->addLayout( spin );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( reject() ) );
   buttons->addWidget( pb_cancel );

   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );
   buttons->addWidget( pb_accept );

   info->addLayout( buttons );

   main->addLayout( info );

   QBoxLayout* plot = new US_Plot( data_plot,
      tr( "RI Noise Fit" ),
      tr( "Scan Time (seconds)" ), tr( "Absorbance" ) );
   
   data_plot->setMinimumSize( 600, 400 );
   us_grid( data_plot );

   main->addLayout( plot );

   draw_fit( order );
}

void US_RiNoise::draw_fit( double new_order )
{
   order = (int)new_order;

   int scan_count = data.scanData.size();

   double* coeffs              = new double[ order ];
   double* absorbance_integral = new double[ scan_count ];
   double* fit                 = new double[ scan_count ];
   double* scan_time           = new double[ scan_count ];;

   // Calculate the integral of each scan which is needed for the least-squares
   // polynomial fit to correct for radially invariant baseline noise. We also
   // keep track of the total integral at each point.

   int scan = 0;

   for ( int i = 0; i < scan_count; i++ )
   {
      if ( ! includes.contains( i ) ) continue;

      absorbance_integral[ scan ] = 0.0;

      // For now, all radii are spaces equally at 0.001 cm
      const double delta_r = 0.001;

      const US_DataIO2::Scan* s = &data.scanData[ i ];
      int value_count           = s->readings.size();
      
      // Integrate using trapezoid rule
      for ( int j = 1; j < value_count; j++ )
      {
         double avg = 
            ( s->readings[ j ].value + s->readings[ j - 1 ].value ) / 2.0;
         
         absorbance_integral[ scan ] += avg * delta_r;
      }

      scan++;
   }

   scan = 0;

   for ( int i = 0; i < scan_count; i++ )
   {
      if ( ! includes.contains( i ) ) continue;
      scan_time[ scan++ ] =  data.scanData[ i ].seconds;
   }

   US_Matrix::lsfit( coeffs, scan_time, absorbance_integral, scan, order );

   residuals.clear();
   scan = 0;

   for ( int i = 0; i < scan_count; i++ )
   {
      if ( includes.contains( i ) ) 
      {
         fit[ scan ] = 0;
         
         for ( int j = 0; j < order; j++ )
            fit[ scan ] +=  coeffs[ j ] * pow( data.scanData[ i ].seconds, j );

         residuals << absorbance_integral[ scan ] - fit[ scan ];
         scan++;
      }
      else 
         residuals << 0.0;  // Fill in holes for deleted scans
   }

   // Write the coefficients
   te_details->clear();
   QString s = tr( "Coefficients for %1th order polynomial fit\n" ).arg( order );
   te_details->append( s );

   for ( int i = 0; i < order; i++ )
   {
      QString coef = QString::number( coeffs[ i ], 'e', 6 );
      if ( coef.at( 0 ) != '-' ) coef.prepend( " " );
      s = tr( "Coefficient %1: " ).arg( i + 1 ) + coef;
      te_details->append( s );
   }

   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );

   //  Plot against the time of the scan:
   scan = 0;

   for ( int i = 0; i < scan_count; i++ )
   {
      if ( ! includes.contains( i ) ) continue;
      scan_time[ scan ] =  data.scanData[ i ].seconds;
      scan++;
   }

   QwtPlotCurve* integrals = us_curve( data_plot, tr( "Integrals" ) );
   integrals->setData( scan_time, absorbance_integral, scan );
   integrals->setPen( QPen( Qt::yellow ) );

   QwtPlotCurve* polyfit = us_curve( data_plot, tr( "Polynomial Fit" ) );
   polyfit->setData( scan_time, fit, scan );
   integrals->setPen( QPen( Qt::magenta ) );

   data_plot->replot();

   delete [] coeffs;
   delete [] absorbance_integral;
   delete [] fit;
   delete [] scan_time;
}


// We want to be able to call this function from other places.
void US_RiNoise::calc_residuals( const US_DataIO2::RawData& data, 
                                 const QList< int >&        includes,
                                 int                        order, 
                                 QList< double >&           residuals )
{
   int scan_count = data.scanData.size();

   double* coeffs              = new double[ order ];
   double* absorbance_integral = new double[ scan_count ];
   double* fit                 = new double[ scan_count ];
   double* scan_time           = new double[ scan_count ];;

   // Calculate the integral of each scan which is needed for the least-squares
   // polynomial fit to correct for radially invariant baseline noise. We also
   // keep track of the total integral at each point.

   int scan = 0;

   for ( int i = 0; i < scan_count; i++ )
   {
      if ( ! includes.contains( i ) ) continue;
      absorbance_integral[ scan ] = 0;

      // For now, all radii are spaces equally at 0.001 cm
      const double delta_r = 0.001;

      const US_DataIO2::Scan* s = &data.scanData[ i ];
      int value_count           = s->readings.size();
      
      // Integrate using trapezoid rule
      for ( int j = 1; j < value_count; j++ )
      {
         double avg = 
            ( s->readings[ j ].value + s->readings[ j - 1 ].value ) / 2.0;
         
         absorbance_integral[ scan ] += avg * delta_r;
      }
   }

   scan = 0;

   for ( int i = 0; i < scan_count; i++ )
   {
      if ( ! includes.contains( i ) ) continue;
      scan_time[ scan++ ] =  data.scanData[ i ].seconds;
   }

   US_Matrix::lsfit( coeffs, scan_time, absorbance_integral, scan, order );

   residuals.clear();
   scan = 0;

   for ( int i = 0; i < scan_count; i++ )
   {
      if ( includes.contains( i ) )
      {
         fit[ scan ] = 0;
         
         for ( int j = 0; j < order; j++ )
            fit[ scan ] +=  coeffs[ j ] * pow( data.scanData[ i ].seconds, j );

         residuals << absorbance_integral[ scan ] - fit[ scan ];
         scan++;
      }
      else
         residuals << 0.0;  // Fill in holes for deleted scans

   }

   delete [] coeffs;
   delete [] absorbance_integral;
   delete [] fit;
   delete [] scan_time;
}
