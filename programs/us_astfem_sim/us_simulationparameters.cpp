#include "us_simulationparameters.h"
#include "us_constants.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#if QT_VERSION < 0x050000
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#endif

US_SimulationParametersGui::US_SimulationParametersGui(
      US_SimulationParameters& params )
   : US_WidgetsDialog( 0, 0 ), simparams( params )
{
   setWindowTitle( "Set Simulation Parameters" );
   setPalette    ( US_GuiSettings::frameColor() );
   setAttribute  ( Qt::WA_DeleteOnClose );

   backup_parms();

   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins ( 2, 2, 2, 2 );
   main->setSpacing( 2 );

   int row = 0;

   QLabel* lb_info = us_banner( tr( "Simulation Run Parameter Setup" ) );
   main->addWidget( lb_info, row++, 0, 1, 8 );

   // Left column

   // Speed Profile count
   QLabel* lb_speeds = us_label( tr( "Number of Speed Profiles:" ) );
   main->addWidget( lb_speeds, row, 0, 1, 3 );

   cnt_speeds = us_counter( 2, 1, 100, 1 );
   cnt_speeds->setSingleStep  ( 1 );
   
   main->addWidget( cnt_speeds, row++, 3, 1, 1 );
   connect( cnt_speeds, SIGNAL( valueChanged ( double ) ), 
                        SLOT  ( update_speeds( double ) ) );
   // Speeds combo box
   cmb_speeds = us_comboBox();

   US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ 0 ];

   for ( int i = 0; i < simparams.speed_step.size(); i++ ) 
   {
      US_SimulationParameters::SpeedProfile* spi = &simparams.speed_step[ i ];

      cmb_speeds->addItem( "Speed Profile " +
            QString::number( i + 1                ) + ": " +
            QString::number( spi->duration_hours   ) + " hr " +
            QString::number( spi->duration_minutes ) + " min, " +
            QString::number( spi->rotorspeed       ) + " rpm" );
   }

   main->addWidget( cmb_speeds, row++, 0, 1, 4 );
   connect( cmb_speeds, SIGNAL( activated           ( int ) ), 
                        SLOT  ( select_speed_profile( int ) ) );

   // Experiment hours
   QLabel* lb_hours = us_label( tr( "Length of Experiment (Hours):" ) );
   main->addWidget( lb_hours, row, 0, 1, 3 );

   cnt_duration_hours = us_counter( 3, 0, 5000, sp->duration_hours );
   cnt_duration_hours->setSingleStep    ( 1 );
   cnt_duration_hours->setIncSteps( QwtCounter::Button1,   1 );
   cnt_duration_hours->setIncSteps( QwtCounter::Button2,  10 );
   cnt_duration_hours->setIncSteps( QwtCounter::Button3, 100 );
   
   main->addWidget( cnt_duration_hours, row++, 3, 1, 1 );
   connect( cnt_duration_hours, SIGNAL( valueChanged         ( double ) ), 
                                SLOT  ( update_duration_hours( double ) ) );

   // Experiment minutes
   QLabel* lb_mins = us_label( tr( "Length of Experiment (Minutes):" ) );
   main->addWidget( lb_mins, row, 0, 1, 3 );

   cnt_duration_mins = us_counter( 3, 0, 59, sp->duration_minutes );
   cnt_duration_mins->setSingleStep    ( 1 );
   cnt_duration_mins->setIncSteps( QwtCounter::Button1,   1 );
   cnt_duration_mins->setIncSteps( QwtCounter::Button2,  10 );
   cnt_duration_mins->setIncSteps( QwtCounter::Button3,  10 );
   
   main->addWidget( cnt_duration_mins, row++, 3, 1, 1 );
   connect( cnt_duration_mins, SIGNAL( valueChanged        ( double ) ), 
                               SLOT  ( update_duration_mins( double ) ) );

   // Delay hours
   QLabel* lb_delay_hours = us_label( tr( "Time Delay for Scans (Hours):" ) );
   main->addWidget( lb_delay_hours, row, 0, 1, 3 );

   cnt_delay_hours = us_counter( 3, 0, 5000, sp->delay_hours );
   cnt_delay_hours->setSingleStep    ( 1 );
   cnt_delay_hours->setIncSteps( QwtCounter::Button1,   1 );
   cnt_delay_hours->setIncSteps( QwtCounter::Button2,  10 );
   cnt_delay_hours->setIncSteps( QwtCounter::Button3, 100 );
   
   main->addWidget( cnt_delay_hours, row++, 3, 1, 1 );
   connect( cnt_delay_hours, SIGNAL( valueChanged      ( double ) ), 
                             SLOT  ( update_delay_hours( double ) ) );

   // Delay minutes
   QLabel* lb_delay_mins = us_label( tr( "Time Delay for Scans (Minutes):" ) );
   main->addWidget( lb_delay_mins, row, 0, 1, 3 );

   cnt_delay_mins = us_counter( 3, 0, 59, sp->delay_minutes );
   cnt_delay_mins->setSingleStep    ( 0.1 );
   cnt_delay_mins->setIncSteps( QwtCounter::Button1,   1 );
   cnt_delay_mins->setIncSteps( QwtCounter::Button2,  10 );
   cnt_delay_mins->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_delay_mins, row++, 3, 1, 1 );
   connect( cnt_delay_mins, SIGNAL( valueChanged     ( double ) ), 
                            SLOT  ( update_delay_mins( double ) ) );

   // Rotor Speed
   QLabel* lb_rotor = us_label( tr( "Rotor Speed (rpm):" ) );
   main->addWidget( lb_rotor, row, 0, 1, 3 );

   cnt_rotorspeed = us_counter( 3, 1000, 100000, sp->rotorspeed );
   cnt_rotorspeed->setSingleStep    ( 100 );
   cnt_rotorspeed->setIncSteps( QwtCounter::Button1,   1 );
   cnt_rotorspeed->setIncSteps( QwtCounter::Button2,  10 );
   cnt_rotorspeed->setIncSteps( QwtCounter::Button3, 100 );

   QFontMetrics fm( cnt_rotorspeed->font() );
   cnt_rotorspeed->setMinimumWidth( fm.maxWidth() * 12 );
   
   main->addWidget( cnt_rotorspeed, row++, 3, 1, 1 );
   connect( cnt_rotorspeed, SIGNAL( valueChanged     ( double ) ), 
                            SLOT  ( update_rotorspeed( double ) ) );

   // Simulate rotor accel checkbox
   QLabel* lb_rotorAccel = us_label( tr( "Simulate Rotor Acceleration:" ) );
   main->addWidget( lb_rotorAccel, row, 0, 1, 3 );

   QGridLayout* acceleration_flag = us_checkbox( tr( "(Check to enable)" ), 
         cb_acceleration_flag, sp->acceleration_flag );

   main->addLayout( acceleration_flag, row++, 3, 1, 1 );
   
   connect( cb_acceleration_flag, SIGNAL( clicked          () ), 
                                  SLOT  ( acceleration_flag() ) );

   // Acceleration Profile
   QLabel* lb_accelProfile = us_label( tr( "Acceleration Profile (rpm/sec):" ));
   main->addWidget( lb_accelProfile, row, 0, 1, 3 );

   cnt_acceleration = us_counter( 3, 5, 400 );
   cnt_acceleration->setSingleStep    ( 5 );
   cnt_acceleration->setIncSteps( QwtCounter::Button1,   1 );
   cnt_acceleration->setIncSteps( QwtCounter::Button2,  10 );
   cnt_acceleration->setIncSteps( QwtCounter::Button3, 100 );

   cnt_acceleration->setValue( sp->acceleration );
   
   main->addWidget( cnt_acceleration, row++, 3, 1, 1 );
   connect( cnt_acceleration, SIGNAL( valueChanged       ( double ) ), 
                              SLOT  ( update_acceleration( double ) ) );

   // Scans to be saved
   QLabel* lb_scans = us_label( tr( "Scans to be saved:" ) );
   main->addWidget( lb_scans, row, 0, 1, 3 );

   cnt_scans = us_counter( 3, 2, 5000, sp->scans );
   cnt_scans->setSingleStep    ( 1 );
   cnt_scans->setIncSteps( QwtCounter::Button1,   1 );
   cnt_scans->setIncSteps( QwtCounter::Button2,  10 );
   cnt_scans->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_scans, row++, 3, 1, 1 );
   connect( cnt_scans, SIGNAL( valueChanged( double ) ), 
                       SLOT  ( update_scans( double ) ) );

   // Speed profile
   QLabel* lb_speedProfile = us_label( tr( "Select a Speed Profile:" ) );
   main->addWidget( lb_speedProfile, row, 0, 1, 3 );

   current_speed_step = 0;
   cnt_selected_speed = us_counter( 3, 1, simparams.speed_step.size(), 0 );
   cnt_selected_speed->setSingleStep    ( 1 );
   cnt_selected_speed->setIncSteps( QwtCounter::Button1, 1 );
   cnt_selected_speed->setIncSteps( QwtCounter::Button2, 1 );
   cnt_selected_speed->setIncSteps( QwtCounter::Button3, 1 );
   cnt_selected_speed->setValue( sp->rotorspeed );
   main->addWidget( cnt_selected_speed, row++, 3, 1, 1 );

   connect( cnt_selected_speed, SIGNAL( valueChanged        ( double ) ), 
                                SLOT  ( update_speed_profile( double ) ) );

   // Mesh combo box
   cmb_mesh = us_comboBox();
   cmb_mesh->setMaxVisibleItems( 5 );
   cmb_mesh->addItem( "Adaptive Space Time FE Mesh (ASTFEM)" );
   cmb_mesh->addItem( "Claverie Fixed Mesh" );
   cmb_mesh->addItem( "Moving Hat Mesh" );
   cmb_mesh->addItem( "Specified file (mesh.dat)" );
   cmb_mesh->addItem( "AST Finite Volume Method (ASTFVM)" );
   cmb_mesh->setCurrentIndex( (int)simparams.meshType );
   
   main->addWidget( cmb_mesh, row++, 0, 1, 4 );

   connect( cmb_mesh, SIGNAL( activated  ( int ) ), 
                      SLOT  ( update_mesh( int ) ) );

   // Right Column
   row = 1;
   // Centerpiece
  
   QGridLayout* rb1 = us_radiobutton( tr( "Standard Centerpiece" ), 
         rb_standard, ! simparams.band_forming ); 

   main->addLayout( rb1, row,   4, 1, 2 );

   QGridLayout* rb2 = us_radiobutton( tr( "Band-forming Centerpiece" ), 
         rb_band, simparams.band_forming );
   
   main->addLayout( rb2, row++, 6, 1, 2 );
   
   connect( rb_standard, SIGNAL( toggled           ( bool ) ), 
                         SLOT  ( select_centerpiece( bool ) ) );

   // Band loading
   QLabel* lb_lamella = us_label( tr( "Band loading volume (" )
         + QString( QChar( 181 ) ) + "l):" );
   main->addWidget( lb_lamella, row, 4, 1, 3 );

   cnt_lamella = us_counter( 3, 1, 20, 15 );
   cnt_lamella->setSingleStep    ( 0.1 );
   cnt_lamella->setIncSteps( QwtCounter::Button1,   1 );
   cnt_lamella->setIncSteps( QwtCounter::Button2,  10 );
   cnt_lamella->setEnabled( false );

   main->addWidget( cnt_lamella, row++, 7, 1, 1 );
   connect( cnt_lamella, SIGNAL( valueChanged  ( double ) ), 
                         SLOT  ( update_lamella( double ) ) );

   // Meniscus position 
   QLabel* lb_meniscus = us_label( tr( "Meniscus Position (cm):" ) );
   main->addWidget( lb_meniscus, row, 4, 1, 3 );

   cnt_meniscus = us_counter( 3, 1.0, 10.0, simparams.meniscus );
   cnt_meniscus->setSingleStep    ( 0.001 );
   cnt_meniscus->setValue   ( simparams.meniscus );
   cnt_meniscus->setIncSteps( QwtCounter::Button1,   1 );
   cnt_meniscus->setIncSteps( QwtCounter::Button2,  10 );
   cnt_meniscus->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_meniscus, row++, 7, 1, 1 );
   connect( cnt_meniscus, SIGNAL( valueChanged   ( double ) ), 
                          SLOT  ( update_meniscus( double ) ) );
   
   // Cell bottom
   QLabel* lb_bottom = us_label( tr( "Bottom of Cell Position (cm):" ) );
   main->addWidget( lb_bottom, row, 4, 1, 3 );

   cnt_bottom = us_counter( 3, 3.8, 30.0, simparams.bottom_position );
   cnt_bottom->setSingleStep    ( 0.001 );
   cnt_bottom->setValue   ( simparams.bottom_position );
   cnt_bottom->setIncSteps( QwtCounter::Button1,   1 );
   cnt_bottom->setIncSteps( QwtCounter::Button2,  10 );
   cnt_bottom->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_bottom, row++, 7, 1, 1 );
   connect( cnt_bottom, SIGNAL( valueChanged ( double ) ), 
                        SLOT  ( update_bottom( double ) ) );
   
   // Radial Discretization
   QLabel* lb_simpoints = us_label( tr( "Radial Discretization (points):" ) );
   main->addWidget( lb_simpoints, row, 4, 1, 3 );

   cnt_simpoints = us_counter( 3, 50, 10000, 200 );
   cnt_simpoints->setSingleStep    ( 10 );
   cnt_simpoints->setValue( simparams.simpoints );

   main->addWidget( cnt_simpoints, row++, 7, 1, 1 );
   connect( cnt_simpoints, SIGNAL( valueChanged    ( double ) ), 
                           SLOT  ( update_simpoints( double ) ) );
   
   // Radial Resolution
   QLabel* lb_radial_res = us_label( tr( "Radial Resolution (cm):" ) );
   main->addWidget( lb_radial_res, row, 4, 1, 3 );

   cnt_radial_res = us_counter( 3, 1e-5, 0.1, simparams.radial_resolution );
   cnt_radial_res->setSingleStep    ( 1e-5 );
   cnt_radial_res->setIncSteps( QwtCounter::Button1,   1 );
   cnt_radial_res->setIncSteps( QwtCounter::Button2,  10 );
   cnt_radial_res->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_radial_res, row++, 7, 1, 1 );
   connect( cnt_radial_res, SIGNAL( valueChanged     ( double ) ), 
                            SLOT  ( update_radial_res( double ) ) );
   
   // Random noise, proportional to total concentration
   QLabel* lb_rnoise = us_label( tr( "Random Noise (% total Conc.):" ) );
   main->addWidget( lb_rnoise, row, 4, 1, 3 );

   cnt_rnoise = us_counter( 3, 0, 10, simparams.rnoise );
   cnt_rnoise->setSingleStep    ( 0.01 );
   cnt_rnoise->setIncSteps( QwtCounter::Button1,   1 );
   cnt_rnoise->setIncSteps( QwtCounter::Button2,  10 );
   cnt_rnoise->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_rnoise, row++, 7, 1, 1 );
   connect( cnt_rnoise, SIGNAL( valueChanged ( double ) ), 
                        SLOT  ( update_rnoise( double ) ) );

   // Random noise, proportional to local concentration
   QLabel* lb_lrnoise = us_label( tr( "Random Noise (% local Conc.):" ) );
   main->addWidget( lb_lrnoise, row, 4, 1, 3 );

   cnt_lrnoise = us_counter( 3, 0, 10, simparams.lrnoise );
   cnt_lrnoise->setSingleStep    ( 0.01 );
   cnt_lrnoise->setIncSteps( QwtCounter::Button1,   1 );
   cnt_lrnoise->setIncSteps( QwtCounter::Button2,  10 );
   cnt_lrnoise->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_lrnoise, row++, 7, 1, 1 );
   connect( cnt_lrnoise, SIGNAL( valueChanged ( double ) ), 
                        SLOT  ( update_lrnoise( double ) ) );

   // Time invariant noise
   QLabel* lb_tinoise = us_label( tr( "Time Invariant Noise (% Conc.):" ) );
   main->addWidget( lb_tinoise, row, 4, 1, 3 );

   cnt_tinoise = us_counter( 3, 0, 10, simparams.tinoise );
   cnt_tinoise->setSingleStep    ( 0.01 );
   cnt_tinoise->setIncSteps( QwtCounter::Button1,   1 );
   cnt_tinoise->setIncSteps( QwtCounter::Button2,  10 );
   cnt_tinoise->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_tinoise, row++, 7, 1, 1 );
   connect( cnt_tinoise, SIGNAL( valueChanged  ( double ) ), 
                         SLOT  ( update_tinoise( double ) ) );
   
   // Radially invariant noise
   QLabel* lb_rinoise = us_label( tr( "Radially Invar. Noise (% Conc.):" ) );
   main->addWidget( lb_rinoise, row, 4, 1, 3 );

   cnt_rinoise = us_counter( 3, 0, 10, simparams.rinoise );
   cnt_rinoise->setSingleStep    ( 0.01 );
   cnt_rinoise->setIncSteps( QwtCounter::Button1,   1 );
   cnt_rinoise->setIncSteps( QwtCounter::Button2,  10 );
   cnt_rinoise->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_rinoise, row++, 7, 1, 1 );
   connect( cnt_rinoise, SIGNAL( valueChanged  ( double ) ), 
                         SLOT  ( update_rinoise( double ) ) );
  
   // Temperature
   QLabel* lb_temperature  = us_label( tr( "Temperature (%1):" )
         .arg( DEGC ) );
   main->addWidget( lb_temperature,  row, 4, 1, 3 );

   cnt_temperature    = us_counter( 3, 10.0, 40.0, NORMAL_TEMP );
   cnt_temperature->setSingleStep    ( 0.1 );
   cnt_temperature->setIncSteps( QwtCounter::Button1,   1 );
   cnt_temperature->setIncSteps( QwtCounter::Button2,  10 );
   cnt_temperature->setIncSteps( QwtCounter::Button3, 100 );
   cnt_temperature->setValue   ( simparams.temperature );
   main->addWidget( cnt_temperature, row++, 7, 1, 1 );
   connect( cnt_temperature, SIGNAL( valueChanged( double ) ), 
                             SLOT  ( update_temp(  double ) ) );

   // Moving Grid Combo Box
   cmb_moving = us_comboBox();
   cmb_moving->setMaxVisibleItems( 5 );
   cmb_moving->addItem( "Constant Time Grid (Claverie/Acceleration)" );
   cmb_moving->addItem( "Moving Time Grid (ASTFEM/Moving Hat)" );
   cmb_moving->setCurrentIndex( (int)simparams.gridType );
   connect( cmb_moving, SIGNAL( activated    ( int ) ), 
                        SLOT  ( update_moving( int ) ) );
   
   main->addWidget( cmb_moving, row++, 4, 1, 4 );

   // Button bar
   QBoxLayout* buttons = new QHBoxLayout();

   QPushButton* pb_load = us_pushbutton( tr( "Load Profile" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   buttons ->addWidget( pb_load );

   QPushButton* pb_save = us_pushbutton( tr( "Save Profile" ) );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
   buttons ->addWidget( pb_save );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons ->addWidget( pb_help );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( revert() ) );
   buttons ->addWidget( pb_cancel );

   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accepted() ) );
   buttons ->addWidget( pb_accept );

   main->addLayout( buttons, row++, 0, 1, 8 );
}

void US_SimulationParametersGui::accepted( void )
{
   emit complete();
   accept();
}

void US_SimulationParametersGui::backup_parms( void )
{
   /*
   US_SimulationParameters::SpeedProfile sp;
   simparams_backup.speed_step.clear();

   for ( int i = 0; i < simparams.speed_step.size(); i ++ )
   {
      simparams_backup.speed_step .push_back( sp );

      US_SimulationParameters::SpeedProfile* ss   = &simparams       .speed_step[ i ];
      US_SimulationParameters::SpeedProfile* ssbu = &simparams_backup.speed_step[ i ];

      ssbu->duration_hours    = ss->duration_hours;
      ssbu->duration_minutes  = ss->duration_minutes;
      ssbu->delay_hours       = ss->delay_hours;
      ssbu->delay_minutes     = ss->delay_minutes;
      ssbu->rotorspeed        = ss->rotorspeed;
      ssbu->scans             = ss->scans;
      ssbu->acceleration      = ss->acceleration;
      ssbu->acceleration_flag = ss->acceleration_flag;
   }

   simparams_backup.simpoints         = simparams.simpoints;
   simparams_backup.radial_resolution = simparams.radial_resolution;
   simparams_backup.meniscus          = simparams.meniscus;
   simparams_backup.bottom            = simparams.bottom;
   simparams_backup.rnoise            = simparams.rnoise;
   simparams_backup.tinoise           = simparams.tinoise;
   simparams_backup.rinoise           = simparams.rinoise;
   */
}

void US_SimulationParametersGui::revert( void )
{
   US_SimulationParameters::SpeedProfile sp;
   simparams.speed_step.clear();

   for ( int i = 0; i < simparams_backup.speed_step.size(); i ++ )
   {
      simparams.speed_step .push_back( sp );

      US_SimulationParameters::SpeedProfile* ss   = &simparams       .speed_step[ i ];
      US_SimulationParameters::SpeedProfile* ssbu = &simparams_backup.speed_step[ i ];

      ss->duration_hours    = ssbu->duration_hours;
      ss->duration_minutes  = ssbu->duration_minutes;
      ss->delay_hours       = ssbu->delay_hours;
      ss->delay_minutes     = ssbu->delay_minutes;
      ss->rotorspeed        = ssbu->rotorspeed;
      ss->scans             = ssbu->scans;
      ss->acceleration      = ssbu->acceleration;
      ss->acceleration_flag = ssbu->acceleration_flag;
   }

   simparams.simpoints         = simparams_backup.simpoints;
   simparams.radial_resolution = simparams_backup.radial_resolution;
   simparams.meniscus          = simparams_backup.meniscus;
   simparams.bottom            = simparams_backup.bottom;
   simparams.rnoise            = simparams_backup.rnoise;
   simparams.lrnoise           = simparams_backup.lrnoise;
   simparams.tinoise           = simparams_backup.tinoise;
   simparams.rinoise           = simparams_backup.rinoise;
   simparams.bottom_position   = simparams_backup.bottom_position;

   reject();
}

void US_SimulationParametersGui::update_speeds( double value )
{
   int                 old_size = simparams.speed_step.size();
   US_SimulationParameters::SpeedProfile sp;
   
   for ( int i = old_size; i < (int) value; i++ )
   {
      simparams.speed_step .push_back( sp );

      // Only initialize the new elements, leave the previously assigned
      // elements alone.  New elements simply get copies of the last old
      // element if old_size > new_size then we won't go through this loop and
      // simply truncate the list

      US_SimulationParameters::SpeedProfile* ss     = &simparams.speed_step[ i ];
      US_SimulationParameters::SpeedProfile* ss_old = &simparams.speed_step[ old_size - 1 ];
      
      ss->duration_hours    = ss_old->duration_hours;
      ss->duration_minutes  = ss_old->duration_minutes;
      ss->delay_hours       = ss_old->delay_hours;
      ss->delay_minutes     = ss_old->delay_minutes;
      ss->rotorspeed        = ss_old->rotorspeed;
      ss->scans             = ss_old->scans;
      ss->acceleration      = ss_old->acceleration;
      ss->acceleration_flag = ss_old->acceleration_flag;
   }

   cnt_selected_speed->setMaximum( simparams.speed_step.size() );
   update_combobox();
}

void US_SimulationParametersGui::update_combobox( void )
{
   cmb_speeds->disconnect();
   cmb_speeds->clear();
   
   for ( int i = 0; i < simparams.speed_step.size(); i++ )
   {
      US_SimulationParameters::SpeedProfile* spi = &simparams.speed_step[ i ];

      cmb_speeds->addItem( "Speed Profile " +
            QString::number( i + 1                 ) + ": " +
            QString::number( spi->duration_hours   ) + " hr " +
            QString::number( spi->duration_minutes ) + " min, " +
            QString::number( spi->rotorspeed       ) + " rpm" );
   }

   connect( cmb_speeds, SIGNAL( activated           ( int ) ),
                        SLOT  ( select_speed_profile( int ) ) );
   
   cmb_speeds->setCurrentIndex( current_speed_step );
}

void US_SimulationParametersGui::update_speed_profile( double profile )
{
   select_speed_profile( (int) profile - 1 );
}

void US_SimulationParametersGui::select_speed_profile( int index )
{
   current_speed_step = index;
   cnt_speeds->setValue( index + 1 );
   
   if ( cb_acceleration_flag->isChecked() )
   {
      cnt_acceleration->setEnabled( true );

      // If there is acceleration we need to set the scan delay
      // minimum to the time it takes to accelerate:
    
      check_delay();
   }
   else
   {
      cnt_acceleration->setEnabled( false );
   }

   cmb_speeds->setCurrentIndex( index );
   
   US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ index ];

   cnt_duration_hours->setValue( sp->duration_hours   );
   cnt_duration_mins ->setValue( sp->duration_minutes );
   cnt_delay_hours   ->setValue( sp->delay_hours      );
   cnt_delay_mins    ->setValue( sp->delay_minutes    );
   cnt_rotorspeed    ->setValue( sp->rotorspeed       );
   cnt_scans         ->setValue( sp->scans            );
   cnt_acceleration  ->setValue( sp->acceleration     );

   cb_acceleration_flag->setChecked( sp->acceleration_flag );
}

void US_SimulationParametersGui::check_delay( void )
{
   QVector< int >    hours;
   QVector< double > minutes;
   QVector< int >    speed;
   
   speed.clear();
   speed .push_back( 0 );

   int steps = simparams.speed_step.size();

   for ( int i = 0; i < steps; i++ )
   {
      hours  .push_back( 0 );
      minutes.push_back( 0.0 );

      US_SimulationParameters::SpeedProfile* ss = &simparams.speed_step[ i ];
      speed .push_back( ss->rotorspeed );
      
      int lower_limit = 1 + 
         ( abs( (speed[ i + 1 ] - speed[ i ] ) ) + 1 ) / ss->acceleration;
      
      hours  [ i ] = lower_limit / 3600;
      int secs     = lower_limit - hours[ i ] * 3600;
      int mins     = qRound( (double)secs / 60.0 );
      minutes[ i ] = (double)mins;
   }

   //cnt_delay_mins ->setMinimum( minutes[ current_speed_step ] );
   cnt_delay_hours->setMinimum( hours  [ current_speed_step ] );
   
   US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];

   if ( sp->delay_hours  == hours[   current_speed_step]  &&
        sp->delay_minutes < minutes[ current_speed_step] )
   {
      sp->delay_minutes = minutes[ current_speed_step ];
      cnt_delay_mins->setValue( minutes[ current_speed_step ] );
   }

   if ( sp->delay_hours < (int)hours[ current_speed_step ] )
   {
      sp->delay_hours = hours[ current_speed_step ];
      cnt_delay_hours->setValue( hours[ current_speed_step ] );
   }
   
   if ( sp->duration_hours == 0 && sp->duration_minutes < sp->delay_minutes + 1 ) 
   {
      sp->duration_minutes = (int) sp->delay_minutes + 1;
      
      cnt_duration_mins->setValue( sp->duration_minutes);
      
      cnt_duration_mins->setMinimum( sp->delay_minutes + 1 );
   }
   else if ( sp->duration_hours > 0)
   {
      cnt_duration_mins->setMinimum( 0 );
   }
   else if ( sp->duration_hours == 0 && 
             sp->duration_minutes > sp->delay_minutes + 1 )
   {
      cnt_duration_mins->disconnect();
      cnt_duration_mins->setMinimum( sp->delay_minutes + 1 );
      connect( cnt_duration_mins, SIGNAL( valueChanged        ( double ) ), 
                                  SLOT  ( update_duration_mins( double ) ) );
   }
}

void US_SimulationParametersGui::update_duration_hours( double hours )
{
   US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->duration_hours   = (int)hours;
   check_delay();
   update_combobox();
}

void US_SimulationParametersGui::update_duration_mins( double minutes )
{
   US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->duration_minutes = minutes;
   check_delay();
   update_combobox();
}

void US_SimulationParametersGui::update_delay_hours( double hours )
{
   US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->delay_hours   = (int) hours;
}

void US_SimulationParametersGui::update_delay_mins( double minutes )
{
   US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->delay_minutes = (int) minutes;
   check_delay();
}

void US_SimulationParametersGui::update_rotorspeed( double speed )
{
   US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->rotorspeed = (long) speed;
   update_combobox();
 
   // If there is acceleration we need to set the scan delay
   // minimum to the time it takes to accelerate:
   if ( cb_acceleration_flag->isChecked() ) check_delay();
}

void US_SimulationParametersGui::acceleration_flag( void )
{
   US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];

   bool state = cb_acceleration_flag->isChecked();
   
   sp->acceleration_flag = state;
   cnt_acceleration->setEnabled( state );
   
   // If there is acceleration we need to set the scan delay
   // minimum to the time it takes to accelerate:
   if ( state ) check_delay();
}

void US_SimulationParametersGui::update_acceleration( double accel )
{
   US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->acceleration = (int)accel;
 
   // If there is acceleration we need to set the scan delay
   // minimum to the time it takes to accelerate:
   if ( cb_acceleration_flag->isChecked() ) check_delay();
}

void US_SimulationParametersGui::update_scans( double scans )
{
   US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->scans = (int)scans;
}

void US_SimulationParametersGui::save( void )
{
   QString fn = QFileDialog::getSaveFileName( this,
         tr( "Save Simulation Parameters in:" ),
         US_Settings::etcDir(), 
         tr( "SimParams files (sp_*.xml);;"
             "All XML files (*.xml);;"
             "All files (*)" ) );
        
   if ( fn.isEmpty() ) return;

   fn   = fn.replace( "\\", "/" );
   int     jj   = fn.lastIndexOf( "/" ) + 1;
   QString fdir = fn.left( jj );
   QString fnam = fn.mid( jj );
   
   // Make sure file name is in "sp_<name>.xml" form

   if ( fn.endsWith( "." ) )
   {  // ending with '.' signals no ".xml" to be added
      fn   = fn.left( fn.length() - 1 );
      fnam = fnam.left( fnam.length() - 1 );
   }

   else if ( !fn.endsWith( ".xml" ) )
   {  // if no .xml extension, add one
      fn   = fn   + ".xml";
      fnam = fnam + ".xml";
   }

   if ( fnam.startsWith( "." ) )
   {  // starting with '.' signals no "sp_" prefix
      fn = fdir + fnam.mid( 1 );
   }

   else if ( !fnam.startsWith( "sp_" ) )
   {  // if no sp_ prefix, add one
      fn = fdir + "sp_" + fnam;
   }

   QFile f( fn );

   if ( f.exists() )
   {
      if(  QMessageBox::No == QMessageBox::warning( this,
               tr( "Warning" ), 
               tr( "Attention:\n"
                    "This file exists already!\n\n"
                    "Do you want to overwrite it?" ), 
               QMessageBox::Yes, QMessageBox::No ) )
      {
         return;
      }
   }

   if ( simparams.save_simparms( fn ) == 0 )
   {
      QMessageBox::information( this, 
            tr( "UltraScan Information" ),
            tr( "Please note:\n\n"
                "The Simulation Profile was successfully saved to:\n\n" ) + 
                fn );
   }
   else
   {
      QMessageBox::information( this, 
            tr( "UltraScan Error" ),
            tr( "Please note:\n\n"
                "The Simulation Profile could not be saved to:\n\n" ) + 
                fn );
   }
}

void US_SimulationParametersGui::load( void )
{
   QString fn = QFileDialog::getOpenFileName( this,
         tr( "Load Simulation Parameters from:" ),
         US_Settings::etcDir(), 
         tr( "SimParams files (sp_*.xml);;"
             "All XML files (*.xml);;"
             "All files (*)" ) );

   if ( fn.isEmpty() ) return;
  
   if ( simparams.load_simparms( fn ) == 0 )
   {
      current_speed_step = 0;
      int steps = simparams.speed_step.size();
      disconnect_all();

      cnt_speeds->setValue( steps );
      cmb_speeds->clear();

      for ( int i = 0; i < steps; i++ )
      {
         US_SimulationParameters::SpeedProfile* spi = &simparams.speed_step[ i ];

         cmb_speeds->addItem( "Speed Profile " +
            QString::number( i + 1                ) + ": " +
            QString::number( spi->duration_hours   ) + " hr " +
            QString::number( spi->duration_minutes ) + " min, " +
            QString::number( spi->rotorspeed       ) + " rpm" );
      }

      connect( cmb_speeds, SIGNAL( activated           ( int ) ),
                           SLOT  ( select_speed_profile( int ) ) );

      // Initialize all counters with the first speed profile:

      US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ 0 ];

      cnt_duration_hours  ->setValue( 5.0                  );
      cnt_duration_hours  ->setValue( 5                    );
      cnt_duration_hours  ->setValue( sp->duration_hours   );
      cnt_duration_mins   ->setValue( sp->duration_minutes );
      cnt_delay_hours     ->setValue( sp->delay_hours      );
      cnt_delay_mins      ->setValue( sp->delay_minutes    );
      cnt_rotorspeed      ->setValue( sp->rotorspeed       );
      cnt_acceleration    ->setValue( sp->acceleration     );
      cnt_scans           ->setValue( sp->scans            );
      
      cb_acceleration_flag->setChecked( sp->acceleration_flag );
      cnt_acceleration    ->setEnabled( sp->acceleration_flag );
      
      cnt_lamella         ->setValue( simparams.band_volume * 1000.0  );
      cnt_simpoints       ->setValue( simparams.simpoints         );
      cnt_radial_res      ->setValue( simparams.radial_resolution );
      cnt_meniscus        ->setValue( simparams.meniscus          );
      cnt_bottom          ->setValue( simparams.bottom            );
      cnt_rnoise          ->setValue( simparams.rnoise            );
      cnt_lrnoise         ->setValue( simparams.lrnoise           );
      cnt_tinoise         ->setValue( simparams.tinoise           );
      cnt_rinoise         ->setValue( simparams.rinoise           );
      cnt_temperature     ->setValue( simparams.temperature       );

      cmb_mesh            ->setCurrentIndex( (int)simparams.meshType );
      cmb_moving          ->setCurrentIndex( (int)simparams.gridType );

      rb_band    ->setChecked( simparams.band_forming );
      rb_standard->setChecked( ! simparams.band_forming );
      cnt_lamella->setEnabled( simparams.band_forming );

      reconnect_all();

      QMessageBox::information( this, 
            tr( "UltraScan Information" ),
            tr( "Please note:\n\n"
                "The Simulation Profile was successfully loaded from:\n\n" ) + 
                fn );
   }

   else
   {
      QMessageBox::information( this, 
            tr( "UltraScan Error" ),
            tr( "Please note:\n\n"
                "Could not read the Simulation Profile:\n\n" ) + 
                fn );
   }
}
   
void US_SimulationParametersGui::update_mesh( int mesh )
{
   simparams.meshType = (US_SimulationParameters::MeshType)mesh;

   // By default, the simpoints can be set by the user
   cnt_simpoints->setEnabled( true );

   if ( mesh == 3 )
   {
      QMessageBox::information( this, 
            tr( "UltraScan Information" ),
            tr( "Please note:\n\n"
                "The radial mesh file should have\n"
                "the following format:\n\n"
                "radius_value1\n"
                "radius_value2\n"
                "radius_value3\n"
                "etc...\n\n"
                "Radius values smaller than the meniscus or\n"
                "larger than the bottom of the cell will be\n"
                "excluded from the concentration vector." ) );

      QFile meshfile( US_Settings::appBaseDir() + "/mesh.dat");
      
      if ( meshfile.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         QTextStream ts( &meshfile );
         simparams.mesh_radius.clear();

         bool first = true;
         
         while ( ! ts.atEnd() )
         {
            double value;
            ts >> value;

            // Ignore values outside the meniscus/bottom range 
            if ( value >= simparams.meniscus && value <= simparams.bottom )
            {
               if ( first )
               {
                  if ( value > simparams.meniscus )
                  {
                     simparams.mesh_radius .push_back( simparams.meniscus );
                  }

                  first = false;
               }

               simparams.mesh_radius .push_back( value );
            }
         }

         meshfile.close();

         int mesh_size = simparams.mesh_radius.size();

         if ( simparams.mesh_radius[ mesh_size - 1 ] < simparams.bottom )
         {
            simparams.mesh_radius .push_back( simparams.bottom );
         }

         
         simparams.simpoints = simparams.mesh_radius.size();
         cnt_simpoints->setValue( (double) simparams.simpoints );
         
         // Can't change the simulation points after defining a mesh 
         cnt_simpoints->setEnabled( false ); 
      }
      else
      {
         //simparams.mesh = 0; // Set to default mesh
         cmb_mesh->setCurrentIndex( 0 );
         
         // By default, the simpoints can be set by the user 
         cnt_simpoints->setEnabled( true ); 
         
         QMessageBox::warning( this, 
               tr( "UltraScan Warning" ),
               tr( "Please note:\n\n"
                   "UltraScan could not open the mesh file!\n"
                   "The file:\n\n" ) +  
                   US_Settings::appBaseDir() + tr( "/mesh.dat\n\n"
                   "could not be opened." ) );
      }
   }
}

void US_SimulationParametersGui::disconnect_all( )
{
   cnt_speeds          ->disconnect();
   cmb_speeds          ->disconnect();
   cnt_duration_hours  ->disconnect();
   cnt_duration_mins   ->disconnect();
   cnt_delay_hours     ->disconnect();
   cnt_delay_mins      ->disconnect();
   cnt_rotorspeed      ->disconnect();
   cb_acceleration_flag->disconnect();
   cnt_acceleration    ->disconnect();
   cnt_scans           ->disconnect();
   cnt_selected_speed  ->disconnect();
   cnt_lamella         ->disconnect();
   cnt_meniscus        ->disconnect();
   cnt_bottom          ->disconnect();
   cnt_simpoints       ->disconnect();
   cnt_radial_res      ->disconnect();
   cnt_rnoise          ->disconnect();
   cnt_lrnoise         ->disconnect();
   cnt_tinoise         ->disconnect();
   cnt_rinoise         ->disconnect();
   cmb_mesh            ->disconnect();
   cmb_moving          ->disconnect();
}

void US_SimulationParametersGui::reconnect_all( )
{
   connect( cnt_speeds,           SIGNAL( valueChanged (         double ) ), 
                                  SLOT  ( update_speeds(         double ) ) );
   connect( cmb_speeds,           SIGNAL( activated           (  int ) ), 
                                  SLOT  ( select_speed_profile(  int ) ) );
   connect( cnt_duration_hours,   SIGNAL( valueChanged         ( double ) ), 
                                  SLOT  ( update_duration_hours( double ) ) );
   connect( cnt_duration_mins,    SIGNAL( valueChanged        (  double ) ), 
                                  SLOT  ( update_duration_mins(  double ) ) );
   connect( cnt_delay_hours,      SIGNAL( valueChanged      (    double ) ), 
                                  SLOT  ( update_delay_hours(    double ) ) );
   connect( cnt_delay_mins,       SIGNAL( valueChanged     (     double ) ), 
                                  SLOT  ( update_delay_mins(     double ) ) );
   connect( cnt_rotorspeed,       SIGNAL( valueChanged     (     double ) ), 
                                  SLOT  ( update_rotorspeed(     double ) ) );
   connect( cb_acceleration_flag, SIGNAL( clicked          () ), 
                                  SLOT  ( acceleration_flag() ) );
   connect( cnt_acceleration,     SIGNAL( valueChanged       (   double ) ), 
                                  SLOT  ( update_acceleration(   double ) ) );
   connect( cnt_scans,            SIGNAL( valueChanged(          double ) ), 
                                  SLOT  ( update_scans(          double ) ) );
   connect( cnt_selected_speed,   SIGNAL( valueChanged        (  double ) ), 
                                  SLOT  ( update_speed_profile(  double ) ) );
   connect( cnt_lamella,          SIGNAL( valueChanged  (        double ) ), 
                                  SLOT  ( update_lamella(        double ) ) );
   connect( cnt_meniscus,         SIGNAL( valueChanged   (       double ) ), 
                                  SLOT  ( update_meniscus(       double ) ) );
   connect( cnt_bottom,           SIGNAL( valueChanged (         double ) ), 
                                  SLOT  ( update_bottom(         double ) ) );
   connect( cnt_simpoints,        SIGNAL( valueChanged    (      double ) ), 
                                  SLOT  ( update_simpoints(      double ) ) );
   connect( cnt_radial_res,       SIGNAL( valueChanged     (     double ) ), 
                                  SLOT  ( update_radial_res(     double ) ) );
   connect( cnt_rnoise,           SIGNAL( valueChanged (         double ) ), 
                                  SLOT  ( update_rnoise(         double ) ) );
   connect( cnt_lrnoise,          SIGNAL( valueChanged (         double ) ), 
                                  SLOT  ( update_lrnoise(        double ) ) );
   connect( cnt_tinoise,          SIGNAL( valueChanged  (        double ) ), 
                                  SLOT  ( update_tinoise(        double ) ) );
   connect( cnt_rinoise,          SIGNAL( valueChanged  (        double ) ), 
                                  SLOT  ( update_rinoise(        double ) ) );
   connect( cmb_mesh,             SIGNAL( activated  (           int ) ), 
                                  SLOT  ( update_mesh(           int ) ) );
   connect( cmb_moving,           SIGNAL( activated    (         int ) ), 
                                  SLOT  ( update_moving(         int ) ) );
}

