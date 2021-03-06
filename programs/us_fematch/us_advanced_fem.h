//! \file us_advanced_fem.h
#ifndef US_ADVANCED_H
#define US_ADVANCED_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_help.h"

#include <qwt_plot.h>

//! \brief A class to provide a window for advanced analysis controls

class US_AdvancedFem : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_AdvancedFem constructor
      //! \param amodel   Pointer to model
      //! \param adv_vals Reference to advanced values map
      //! \param p        Pointer to the parent of this widget
      US_AdvancedFem( US_Model*, QMap< QString, QString>&,
                      QWidget* p = 0 );

   private:
      US_Model*                 model;
      QMap< QString, QString >& parmap;

      QGridLayout*  mainLayout;

      QWidget*      parentw;

      QwtCounter*   ct_simpoints;
      QwtCounter*   ct_bldvolume;
      QwtCounter*   ct_parameter;

      QComboBox*    cb_mesh;
      QComboBox*    cb_grid;

   protected:
      US_Help       showHelp;

   private slots:

      void done(           void );
      void help     ( void )
      { showHelp.show_help( "fe_match_adv.html" ); };
};
#endif

