//! \file us_properties.h
#ifndef US_PROPERTIES_H
#define US_PROPERTIES_H

#include <QtGui>

#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_femglobal_new.h"
//#include "us_help.h"
#include "us_predict1.h"
#include "us_analyte_gui.h"
#include "us_buffer.h"
#include "us_model_editor_new2.h"

//! \brief A class to manage analytes in a model

class US_Properties : public US_WidgetsDialog
{
	Q_OBJECT

	public:
      //! Constructor.
      //! \param buf - The buffer to use in the analyte calculations
      //! \param mod - The model to use
      //! \param optical_sys - The current optical system type
      //! \param invID - The investigator ID in the database (-1 if not defined)
      //! \param access - A flag to determine if analyte data access should be
      //!          disk (false) or DB (true)
      US_Properties( const US_Buffer&, 
                     const US_FemGlobal_New::ModelSystem&,
                     int  = -1,
                     bool = false );
   signals:
      //! A signal that the class is complete
      //! \param hydro - this will probably change soon
      void valueChanged( US_Predict1::Hydrosim hydro );

   private:
      // Passed parameters
      US_Buffer                     buffer;
      US_FemGlobal_New::ModelSystem model;
      int                           investigator;
      bool                          db_access;

      bool                          inUpdate;
      int                           oldRow;
      US_Analyte       analyte;

      enum { MW, S, D, F, F_F0 } check_type;

      US_Predict1::Hydrosim hydro_data;
      US_Predict1::Hydrosim working_data;

      QIcon        check;
                   
      QPalette     gray;
      QPalette     normal;

      QListWidget* lw_components;
      QPushButton* pb_load_c0;
      QComboBox*   cmb_shape;

      QLineEdit*   le_density;
      QLineEdit*   le_viscosity;
      QLineEdit*   le_vbar;
      QLineEdit*   le_mw;
      QLineEdit*   le_s;
      QLineEdit*   le_D;
      QLineEdit*   le_f;
      QLineEdit*   le_f_f0;
      QLineEdit*   le_temperature;
      QLineEdit*   le_extinction;
      QLineEdit*   le_wavelength;
      QLineEdit*   le_analyteConc;
      QLineEdit*   le_molar;
      QLineEdit*   le_sigma;
      QLineEdit*   le_delta;
      QLineEdit*   le_guid;
                  
      QCheckBox*   cb_mw;
      QCheckBox*   cb_s;
      QCheckBox*   cb_D;
      QCheckBox*   cb_f;
      QCheckBox*   cb_f_f0;
      QCheckBox*   cb_co_sed;


      void setInvalid    ( void );
      int  countChecks   ( void );
      void update_lw     ( void );
      void enable        ( QLineEdit*, bool, const QPalette& );

	private slots:
      void newAnalyte    ( void );
      void update        ( int );
      void calculate     ( void );
      void checkbox      ( int = 0 );
      void acceptProp    ( void );
      void simulate      ( void );
      void new_hydro     ( US_Analyte );
      void lambda_up     ( void );
      void lambda_down   ( void );
      void select_shape  ( int );
      void load_c0       ( void );
      void co_sed        ( int );
      void set_molar     ( void );

      void del_component ( void );
      void edit_component( QListWidgetItem* );
};
#endif
