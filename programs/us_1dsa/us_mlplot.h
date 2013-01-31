//! \file us_mlplot.h
#ifndef US_MLINES_PLOT_H
#define US_MLINES_PLOT_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO2.h"
#include "us_plot.h"
#include "us_resids_bitmap.h"
#include "us_noise.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class to provide a window with a model lines plot

class US_MLinesPlot : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param parent A pointer to the parent widget of this one
      US_MLinesPlot( double&, double&, double&, double&, double&, int&,
            QWidget* );

   private:
      double&       fmin;
      double&       fmax;
      double&       finc;
      double&       smin;
      double&       smax;

      int&          nlpts;

      QHBoxLayout*  mainLayout;
      QVBoxLayout*  leftLayout;
      QVBoxLayout*  rightLayout;
      QGridLayout*  pltctrlsLayout;
      QVBoxLayout*  buttonsLayout;

      US_Plot*      plotLayout1;

      int           dbg_level;

   protected:
      QwtPlot*      data_plot1;

      US_DataIO2::EditedData*     edata;
      US_DataIO2::Scan*           escan;
      US_DataIO2::RawData*        sdata;
      US_Noise*                   ri_noise;
      US_Noise*                   ti_noise;
      QPointer< US_ResidsBitmap > resbmap;

      QLineEdit*  le_mtype;
      QLineEdit*  le_nlines;
      QLineEdit*  le_npoints;
      QLineEdit*  le_kincr;

   private slots:
      void close_all  ( void );
      void plot_data(   void );
};
#endif
