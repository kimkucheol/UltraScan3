//! \file us_run_details.h
#ifndef US_RUN_DETAILS_H
#define US_RUN_DETAILS_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO.h"
#include "us_plot.h"

//! \brief A class to provide a window with the details of a run

class US_EXTERN US_RunDetails : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param data    A reference to a list of raw data structures
      //! \param index   The index for the current cell/channel/wavelength
      //!                combination
      //! \param runID   The short identifier used in file names to
      //!                identify the run
      //! \param dataDir The data directory where the files were located
      //! \param cell_ch_wl A list of formatted cell/channel/wavelength
      //!                   strings that descripe each raw data structure
      US_RunDetails( const QList< US_DataIO::rawData >&, int,
            const QString&, const QString&, const QStringList& );

   private:
      const QList< US_DataIO::rawData >& dataList;
      const QStringList&                 triples;
      
      // key = rpm, value = triple + scan#
      QMultiMap< int, QString >          map; 

      class graphValue
      {
         public:
         double seconds;
         double rpm;
         double temperature;

         graphValue( double s, double r, double t ):
            seconds( s ), rpm( r ), temperature( t ) {};
         
         bool operator < (const graphValue& other ) 
         const { return seconds < other.seconds; };
      };

      QListWidget* lw_triples;
      QListWidget* lw_rpm;

      QLabel*      lb_red;
      QLabel*      lb_green;

      QLineEdit*   le_desc;
      QLineEdit*   le_avgTemp;
      QLineEdit*   le_runLen;
      QLineEdit*   le_timeCorr;
      QLineEdit*   le_rotorSpeed;

      QwtPlot*     data_plot;

      void         setup        ( void );
      void         show_all_data( void );
      void         draw_plot    ( const double*, const double*,
                                  const double*, const double*, int );
      void         check_temp   ( double, double );

   private slots:
      void update           ( int );
      void show_rpm_details ( int );
};
#endif
