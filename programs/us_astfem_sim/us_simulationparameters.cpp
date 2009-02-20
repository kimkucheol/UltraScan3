#include "us_simulationparameters.h"
#include "us_femglobal.h"
#include "us_gui_settings.h"
#include "us_settings.h"

US_SimulationParameters::US_SimulationParameters(
      struct SimulationParameters& params, 
      QWidget* p, 
      Qt::WindowFlags f ) 
   : US_WidgetsDialog( p, f ), simparams( params )
{
   setWindowTitle( "ASTFEM Simulation Parameters" );
   setPalette    ( US_GuiSettings::frameColor() );
   setAttribute  ( Qt::WA_DeleteOnClose );

   backup_parms();

   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins ( 2, 2, 2, 2 );
   main->setSpacing( 2 );

   int row = 0;

   QLabel* lb_info = us_banner( tr( "Simulation Run Parameter Setup" ) );
   main->addWidget( lb_info, row++, 0, 1, 4 );

   // Left column

   // Speed Profile count
   QLabel* lb_speeds = us_label( tr( "Number of Speed Profiles:" ) );
   main->addWidget( lb_speeds, row, 0 );

   cnt_speeds = us_counter( 2, 1, 100, 1 );
   cnt_speeds->setStep    ( 1 );
   
   main->addWidget( cnt_speeds, row++, 1 );
   connect( cnt_speeds, SIGNAL( valueChanged ( double ) ), 
                        SLOT  ( update_speeds( double ) ) );

   // Speeds combo box
   cmb_speeds = us_comboBox();
   main->addWidget( cmb_speeds, row++, 0, 1, 2 );

   struct SpeedProfile* sp = &simparams.speed_step[ 0 ];

   for ( int i = 0; i < simparams.speed_step.size(); i++ ) 
   {
      struct SpeedProfile* spi = &simparams.speed_step[ i ];

      cmb_speeds->addItem( "Speed Profile " +
            QString::number( i + 1                ) + ": " +
            QString::number( spi->duration_hours   ) + " hr " +
            QString::number( spi->duration_minutes ) + " min, " +
            QString::number( spi->rotorspeed       ) + " rpm" );
   }

   connect( cmb_speeds, SIGNAL( activated           ( int ) ), 
                        SLOT  ( select_speed_profile( int ) ) );

   // Experiment hours
   QLabel* lb_hours = us_label( tr( "Length of Experiment (Hours):" ) );
   main->addWidget( lb_hours, row, 0 );

   cnt_duration_hours = us_counter( 3, 0, 5000, sp->duration_hours );
   cnt_duration_hours->setStep    ( 1 );
   cnt_duration_hours->setIncSteps( QwtCounter::Button1,   1 );
   cnt_duration_hours->setIncSteps( QwtCounter::Button2,  10 );
   cnt_duration_hours->setIncSteps( QwtCounter::Button3, 100 );
   
   main->addWidget( cnt_duration_hours, row++, 1 );
   connect( cnt_duration_hours, SIGNAL( valueChanged         ( double ) ), 
                                SLOT  ( update_duration_hours( double ) ) );

   // Experiment minutes
   QLabel* lb_mins = us_label( tr( "Length of Experiment (Minutes):" ) );
   main->addWidget( lb_mins, row, 0 );

   cnt_duration_mins = us_counter( 2, 0, 59, sp->duration_minutes );
   cnt_duration_mins->setStep    ( 1 );
   cnt_duration_mins->setIncSteps( QwtCounter::Button1,   1 );
   cnt_duration_mins->setIncSteps( QwtCounter::Button2,  10 );
   
   main->addWidget( cnt_duration_mins, row++, 1 );
   connect( cnt_duration_mins, SIGNAL( valueChanged        ( double ) ), 
                               SLOT  ( update_duration_mins( double ) ) );

   // Delay hours
   QLabel* lb_delay_hours = us_label( tr( "Time Delay for Scans (Hours):" ) );
   main->addWidget( lb_delay_hours, row, 0 );

   cnt_delay_hours = us_counter( 3, 0, 5000, sp->delay_hours );
   cnt_delay_hours->setStep    ( 1 );
   cnt_delay_hours->setIncSteps( QwtCounter::Button1,   1 );
   cnt_delay_hours->setIncSteps( QwtCounter::Button2,  10 );
   cnt_delay_hours->setIncSteps( QwtCounter::Button3, 100 );
   
   main->addWidget( cnt_delay_hours, row++, 1 );
   connect( cnt_delay_hours, SIGNAL( valueChanged      ( double ) ), 
                             SLOT  ( update_delay_hours( double ) ) );

   // Delay minutes
   QLabel* lb_delay_mins = us_label( tr( "Time Delay for Scans (Minutes):" ) );
   main->addWidget( lb_delay_mins, row, 0 );

   cnt_delay_mins = us_counter( 2, 0, 59, sp->delay_minutes );
   cnt_delay_mins->setStep    ( 1 );
   cnt_delay_mins->setIncSteps( QwtCounter::Button1,   1 );
   cnt_delay_mins->setIncSteps( QwtCounter::Button2,  10 );

   main->addWidget( cnt_delay_mins, row++, 1 );
   connect( cnt_delay_mins, SIGNAL( valueChanged     ( double ) ), 
                            SLOT  ( update_delay_mins( double ) ) );

   // Rotor Speed
   QLabel* lb_rotor = us_label( tr( "Rotor Speed (rpm):" ) );
   main->addWidget( lb_rotor, row, 0 );

   cnt_rotorspeed = us_counter( 3, 1000, 100000, sp->rotorspeed );
   cnt_rotorspeed->setStep    ( 100 );
   cnt_rotorspeed->setIncSteps( QwtCounter::Button1,   1 );
   cnt_rotorspeed->setIncSteps( QwtCounter::Button2,  10 );
   cnt_rotorspeed->setIncSteps( QwtCounter::Button3, 100 );

   QFontMetrics fm( cnt_rotorspeed->font() );
   cnt_rotorspeed->setMinimumWidth( fm.maxWidth() * 12 );
   
   main->addWidget( cnt_rotorspeed, row++, 1 );
   connect( cnt_rotorspeed, SIGNAL( valueChanged     ( double ) ), 
                            SLOT  ( update_rotorspeed( double ) ) );

   // Simulate rotor accel checkbox
   QLabel* lb_rotorAccel = us_label( tr( "Simulate Rotor Acceleration:" ) );
   main->addWidget( lb_rotorAccel, row, 0 );

   cb_acceleration_flag = us_checkbox( 
         tr( "(Check to enable)" ) ); // sp->acceleration_flag );
   main->addWidget( cb_acceleration_flag, row++, 1 );
   
   connect( cb_acceleration_flag, SIGNAL( clicked          () ), 
                                  SLOT  ( acceleration_flag() ) );

   // Acceleration Profile
   QLabel* lb_accelProfile = us_label( tr( "Acceleration Profile (rpm/sec):" ) );
   main->addWidget( lb_accelProfile, row, 0 );

   cnt_acceleration = us_counter( 2, 5, 400 );
   cnt_acceleration->setStep    ( 5 );
   cnt_acceleration->setIncSteps( QwtCounter::Button1,   1 );
   cnt_acceleration->setIncSteps( QwtCounter::Button2,  10 );

   cnt_acceleration->setValue( sp->acceleration );
   
   main->addWidget( cnt_acceleration, row++, 1 );
   connect( cnt_acceleration, SIGNAL( valueChanged       ( double ) ), 
                              SLOT  ( update_acceleration( double ) ) );

   // Scans to be saved
   QLabel* lb_scans = us_label( tr( "Scans to be saved:" ) );
   main->addWidget( lb_scans, row, 0 );

   cnt_scans = us_counter( 3, 2, 1000, sp->scans );
   cnt_scans->setStep    ( 1 );
   cnt_scans->setIncSteps( QwtCounter::Button1,   1 );
   cnt_scans->setIncSteps( QwtCounter::Button2,  10 );
   cnt_scans->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_scans, row++, 1 );
   connect( cnt_scans, SIGNAL( valueChanged( double ) ), 
                       SLOT  ( update_scans( double ) ) );

   // Speed profile
   QLabel* lb_speedProfile = us_label( tr( "Select a Speed Profile:" ) );
   main->addWidget( lb_speedProfile, row, 0 );

   current_speed_step = 0;
   cnt_selected_speed = us_counter( 1, 1, simparams.speed_step.size(), 0 );
   cnt_selected_speed->setStep    ( 1 );
   cnt_selected_speed->setIncSteps( QwtCounter::Button1, 1 );

   main->addWidget( cnt_selected_speed, row++, 1 );
   connect( cnt_selected_speed, SIGNAL( valueChanged        ( double ) ), 
                                SLOT  ( update_speed_profile( double ) ) );

   // Right Column
   row = 1;

   // Centerpiece
   rb_standard = us_radiobutton( 
         tr( "Standard Centerpiece" ), ! simparams.band_forming ); 
   main->addWidget( rb_standard, row, 2 );

   rb_band = us_radiobutton( 
         tr( "Band-forming Centerpiece" ), simparams.band_forming );
   
   main->addWidget( rb_band, row++, 3 );
   
   connect( rb_standard, SIGNAL( toggled           ( bool ) ), 
                         SLOT  ( select_centerpiece( bool ) ) );

   // Band loading
   QLabel* lb_lamella = us_label( tr( "Band loading volume (ml):" ) );
   main->addWidget( lb_lamella, row, 2 );

   cnt_lamella = us_counter( 3, 0.001, 0.1, simparams.band_volume );
   cnt_lamella->setStep    ( 0.0001 );
   cnt_lamella->setIncSteps( QwtCounter::Button1,   1 );
   cnt_lamella->setIncSteps( QwtCounter::Button2,  10 );
   cnt_lamella->setEnabled( false );

   main->addWidget( cnt_lamella, row++, 3 );
   connect( cnt_lamella, SIGNAL( valueChanged  ( double ) ), 
                         SLOT  ( update_lamella( double ) ) );

   // Meniscus position 
   QLabel* lb_meniscus = us_label( tr( "Meniscus Position (cm):" ) );
   main->addWidget( lb_meniscus, row, 2 );

   cnt_meniscus = us_counter( 3, 5.8, 7.2, simparams.meniscus );
   cnt_meniscus->setStep    ( 0.001 );
   cnt_meniscus->setIncSteps( QwtCounter::Button1,   1 );
   cnt_meniscus->setIncSteps( QwtCounter::Button2,  10 );
   cnt_meniscus->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_meniscus, row++, 3 );
   connect( cnt_meniscus, SIGNAL( valueChanged   ( double ) ), 
                          SLOT  ( update_meniscus( double ) ) );
   
   // Cell bottom
   QLabel* lb_bottom = us_label( tr( "Bottom of Cell Position (cm):" ) );
   main->addWidget( lb_bottom, row, 2 );

   cnt_bottom = us_counter( 3, 5.8, 7.5, simparams.bottom );
   cnt_bottom->setStep    ( 0.001 );
   cnt_bottom->setIncSteps( QwtCounter::Button1,   1 );
   cnt_bottom->setIncSteps( QwtCounter::Button2,  10 );
   cnt_bottom->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_bottom, row++, 3 );
   connect( cnt_bottom, SIGNAL( valueChanged ( double ) ), 
                        SLOT  ( update_bottom( double ) ) );
   
   // Radial Discretization
   QLabel* lb_simpoints = us_label( tr( "Radial Discretization (points):" ) );
   main->addWidget( lb_simpoints, row, 2 );

   cnt_simpoints = us_counter( 3, 50, 5000, simparams.simpoints );
   cnt_simpoints->setStep    ( 10 );
   cnt_simpoints->setIncSteps( QwtCounter::Button1,   1 );
   cnt_simpoints->setIncSteps( QwtCounter::Button2,  10 );
   cnt_simpoints->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_simpoints, row++, 3 );
   connect( cnt_simpoints, SIGNAL( valueChanged    ( double ) ), 
                           SLOT  ( update_simpoints( double ) ) );
   
   // Radial Resolution
   QLabel* lb_radial_res = us_label( tr( "Radial Resolution (cm):" ) );
   main->addWidget( lb_radial_res, row, 2 );

   cnt_radial_res = us_counter( 3, 1e-5, 0.1, simparams.radial_resolution );
   cnt_radial_res->setStep    ( 1e-5 );
   cnt_radial_res->setIncSteps( QwtCounter::Button1,   1 );
   cnt_radial_res->setIncSteps( QwtCounter::Button2,  10 );
   cnt_radial_res->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_radial_res, row++, 3 );
   connect( cnt_radial_res, SIGNAL( valueChanged     ( double ) ), 
                            SLOT  ( update_radial_res( double ) ) );
   
   // Random noise   
   QLabel* lb_rnoise = us_label( tr( "Random Noise (% Conc.):" ) );
   main->addWidget( lb_rnoise, row, 2 );

   cnt_rnoise = us_counter( 3, 0, 10, simparams.rnoise );
   cnt_rnoise->setStep    ( 0.01 );
   cnt_rnoise->setIncSteps( QwtCounter::Button1,   1 );
   cnt_rnoise->setIncSteps( QwtCounter::Button2,  10 );
   cnt_rnoise->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_rnoise, row++, 3 );
   connect( cnt_rnoise, SIGNAL( valueChanged ( double ) ), 
                        SLOT  ( update_rnoise( double ) ) );
   
   // Time invariant noise
   QLabel* lb_tinoise = us_label( tr( "Time Invariant Noise (% Conc.):" ) );
   main->addWidget( lb_tinoise, row, 2 );

   cnt_tinoise = us_counter( 3, 0, 10, simparams.tinoise );
   cnt_tinoise->setStep    ( 0.01 );
   cnt_tinoise->setIncSteps( QwtCounter::Button1,   1 );
   cnt_tinoise->setIncSteps( QwtCounter::Button2,  10 );
   cnt_tinoise->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_tinoise, row++, 3 );
   connect( cnt_tinoise, SIGNAL( valueChanged  ( double ) ), 
                         SLOT  ( update_tinoise( double ) ) );
   
   // Radially invariant noise
   QLabel* lb_rinoise = us_label( tr( "Radially Invariant Noise (% Conc.):" ) );
   main->addWidget( lb_rinoise, row, 2 );

   cnt_rinoise = us_counter( 3, 0, 10, simparams.tinoise );
   cnt_rinoise->setStep    ( 0.01 );
   cnt_rinoise->setIncSteps( QwtCounter::Button1,   1 );
   cnt_rinoise->setIncSteps( QwtCounter::Button2,  10 );
   cnt_rinoise->setIncSteps( QwtCounter::Button3, 100 );

   main->addWidget( cnt_rinoise, row++, 3 );
   connect( cnt_rinoise, SIGNAL( valueChanged  ( double ) ), 
                         SLOT  ( update_rinoise( double ) ) );
  
   // Mesh combo box
   cmb_mesh = us_comboBox();
   cmb_mesh->setMaxVisibleItems( 5 );
   cmb_mesh->addItem( "Adaptive Space Mesh (ASTFEM)" );
   cmb_mesh->addItem( "Claverie Fixed Mesh" );
   cmb_mesh->addItem( "Moving Hat Mesh" );
   cmb_mesh->addItem( "Specified file (mesh.dat)" );
   cmb_mesh->setCurrentIndex( simparams.mesh );
   
   main->addWidget( cmb_mesh, row++, 2, 1, 2 );

   connect( cmb_mesh, SIGNAL( activated  ( int ) ), 
                      SLOT  ( update_mesh( int ) ) );

   // Moving Grid Combo Box
   cmb_moving = us_comboBox();
   cmb_moving->setMaxVisibleItems( 5 );
   cmb_moving->addItem( "Constant Time Grid (Claverie/Acceleration)" );
   cmb_moving->addItem( "Moving Time Grid (ASTFEM/Moving Hat)" );
   cmb_moving->setCurrentIndex( simparams.moving_grid );
   connect( cmb_moving, SIGNAL( activated    ( int ) ), 
                        SLOT  ( update_moving( int ) ) );
   
   main->addWidget( cmb_moving, row++, 2, 1, 2 );

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
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );
   buttons ->addWidget( pb_accept );

   main->addLayout( buttons, row++, 0, 1, 4 );
}

void US_SimulationParameters::backup_parms( void )
{
   struct SpeedProfile sp;
   simparams_backup.speed_step.clear();

   for ( int i = 0; i < simparams.speed_step.size(); i ++ )
   {
      simparams_backup.speed_step << sp;

      struct SpeedProfile* ss   = &simparams       .speed_step[ i ];
      struct SpeedProfile* ssbu = &simparams_backup.speed_step[ i ];

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
}

void US_SimulationParameters::revert( void )
{
   struct SpeedProfile sp;
   simparams.speed_step.clear();

   for ( int i = 0; i < simparams_backup.speed_step.size(); i ++ )
   {
      simparams.speed_step << sp;

      struct SpeedProfile* ss   = &simparams       .speed_step[ i ];
      struct SpeedProfile* ssbu = &simparams_backup.speed_step[ i ];

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
   simparams.tinoise           = simparams_backup.tinoise;
   simparams.rinoise           = simparams_backup.rinoise;

   reject();
}

void US_SimulationParameters::update_speeds( double value )
{
   int                 old_size = simparams.speed_step.size();
   struct SpeedProfile sp;
   
   for ( int i = old_size; i < (int) value; i++ )
   {
      simparams.speed_step << sp;

      // Only initialize the new elements, leave the previously assigned
      // elements alone.  New elements simply get copies of the last old
      // element if old_size > new_size then we won't go through this loop and
      // simply truncate the list

      struct SpeedProfile* ss     = &simparams.speed_step[ i ];
      struct SpeedProfile* ss_old = &simparams.speed_step[ old_size - 1 ];
      
      ss->duration_hours    = ss_old->duration_hours;
      ss->duration_minutes  = ss_old->duration_minutes;
      ss->delay_hours       = ss_old->delay_hours;
      ss->delay_minutes     = ss_old->delay_minutes;
      ss->rotorspeed        = ss_old->rotorspeed;
      ss->scans             = ss_old->scans;
      ss->acceleration      = ss_old->acceleration;
      ss->acceleration_flag = ss_old->acceleration_flag;
   }

   cnt_selected_speed->setMaxValue( simparams.speed_step.size() );
   update_combobox();
}

void US_SimulationParameters::update_combobox( void )
{
   cmb_speeds->disconnect();
   cmb_speeds->clear();
   
   for ( int i = 0; i < simparams.speed_step.size(); i++ )
   {
      struct SpeedProfile* spi = &simparams.speed_step[ i ];

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

void US_SimulationParameters::update_speed_profile( double profile )
{
   select_speed_profile( (int) profile - 1 );
}

void US_SimulationParameters::select_speed_profile( int index )
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
   
   struct SpeedProfile* sp = &simparams.speed_step[ index ];

   cnt_duration_hours->setValue( sp->duration_hours   );
   cnt_duration_mins ->setValue( sp->duration_minutes );
   cnt_delay_hours   ->setValue( sp->delay_hours      );
   cnt_delay_mins    ->setValue( sp->delay_minutes    );
   cnt_rotorspeed    ->setValue( sp->rotorspeed       );
   cnt_scans         ->setValue( sp->scans            );
   cnt_acceleration  ->setValue( sp->acceleration     );

   cb_acceleration_flag->setChecked( sp->acceleration_flag );
}

void US_SimulationParameters::check_delay( void )
{
   QList< int    > hours;
   QList< double > minutes;
   QList< int    > speed;
   
   speed.clear();
   speed << 0;

   int steps = simparams.speed_step.size();

   for ( int i = 0; i < steps; i++ )
   {
      hours   << 0;
      minutes << 0.0;

      struct SpeedProfile* ss = &simparams.speed_step[ i ];
      speed << ss->rotorspeed;
      
      int lower_limit = 1 + 
         ( abs( (speed[ i + 1 ] - speed[ i ] ) ) + 1 ) / ss->acceleration;
      
      hours  [ i ] = lower_limit / 3600;
      minutes[ i ] =  1.0 / 60.0 
                     + ( lower_limit - hours[ i ] * 3600.0 ) / 60.0 ;
   }

   cnt_delay_mins ->setMaxValue( minutes[ current_speed_step ] );
   cnt_delay_hours->setMinValue( hours  [ current_speed_step ] );
   
   struct SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];

   if ( sp->delay_minutes < minutes[ current_speed_step] )
   {
      sp->delay_minutes = minutes[ current_speed_step ];
      cnt_delay_mins->setValue( minutes[ current_speed_step ] );
   }

   if ( sp->delay_hours < (unsigned int)hours[ current_speed_step ] )
   {
      sp->delay_hours = hours[ current_speed_step ];
      cnt_delay_hours->setValue( hours[ current_speed_step ] );
   }
   
   if ( sp->duration_hours == 0 && sp->duration_minutes < sp->delay_minutes + 1 ) 
   {
      sp->duration_minutes = (unsigned int) sp->delay_minutes + 1;
      
      cnt_duration_mins->setValue( sp->duration_minutes);
      
      cnt_duration_mins->setMinValue( sp->delay_minutes + 1 );
   }
   else if ( sp->duration_hours > 0)
   {
      cnt_duration_mins->setMinValue( 0 );
   }
   else if ( sp->duration_hours == 0 && 
             sp->duration_minutes > sp->delay_minutes + 1 )
   {
      cnt_duration_mins->disconnect();
      cnt_duration_mins->setMinValue( sp->delay_minutes + 1 );
      connect( cnt_duration_mins, SIGNAL( valueChanged        ( double ) ), 
                                  SLOT  ( update_duration_mins( double ) ) );
   }
}

void US_SimulationParameters::update_duration_hours( double hours )
{
   struct SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->duration_hours = (unsigned int)hours;
   check_delay();
   update_combobox();
}

void US_SimulationParameters::update_duration_mins( double minutes )
{
   struct SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->duration_minutes = (unsigned int)minutes;
   check_delay();
   update_combobox();
}

void US_SimulationParameters::update_delay_hours( double hours )
{
   struct SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->delay_hours = (unsigned int) hours;
}

void US_SimulationParameters::update_delay_mins( double minutes )
{
   struct SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->delay_hours = (unsigned int) minutes;
   check_delay();
}

void US_SimulationParameters::update_rotorspeed( double speed )
{
   struct SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->rotorspeed = (unsigned long) speed;
   update_combobox();
 
   // If there is acceleration we need to set the scan delay
   // minimum to the time it takes to accelerate:
   if ( cb_acceleration_flag->isChecked() ) check_delay();
}

void US_SimulationParameters::acceleration_flag( void )
{
   struct SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];

   bool state = cb_acceleration_flag->isChecked();
   
   sp->acceleration_flag = state;
   cnt_acceleration->setEnabled( state );
   
   // If there is acceleration we need to set the scan delay
   // minimum to the time it takes to accelerate:
   if ( state ) check_delay();
}

void US_SimulationParameters::update_acceleration( double accel )
{
   struct SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->acceleration = (unsigned long) accel;
 
   // If there is acceleration we need to set the scan delay
   // minimum to the time it takes to accelerate:
   if ( cb_acceleration_flag->isChecked() ) check_delay();
}

void US_SimulationParameters::update_scans( double scans )
{
   struct SpeedProfile* sp = &simparams.speed_step[ current_speed_step ];
   sp->scans = (unsigned int)scans;
}

void US_SimulationParameters::save( void )
{
   QString fn = QFileDialog::getSaveFileName( this,
         tr( "Save Simulation Paramaters in:" ),
         US_Settings::resultDir(), 
         "*.simulation_parameters" );
        
   if ( fn.isEmpty() ) return;
   
  
   // If an extension was given, strip it.
   int k = fn.lastIndexOf( "." );
   if ( k != -1 ) fn.truncate( k );

   QFile f( fn + ".simulation_parameters" );

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

   if ( US_FemGlobal::write_simulationParameters( simparams, fn ) == 0 )
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

void US_SimulationParameters::load( void )
{
   QString fn = QFileDialog::getOpenFileName( this,
         tr( "" ),
         US_Settings::resultDir(), "*.simulation_parameters" );

   if ( fn.isEmpty() ) return;
  
   if ( US_FemGlobal::read_simulationParameters( simparams, fn ) == 0 )
   {
      int steps = simparams.speed_step.size();

      cnt_speeds->setValue( steps );
      cmb_speeds->clear();

      for ( int i = 0; i < steps; i++ )
      {
         struct SpeedProfile* spi = &simparams.speed_step[ i ];

         cmb_speeds->addItem( "Speed Profile " +
            QString::number( i + 1                ) + ": " +
            QString::number( spi->duration_hours   ) + " hr " +
            QString::number( spi->duration_minutes ) + " min, " +
            QString::number( spi->rotorspeed       ) + " rpm" );
      }

      // Initialize all counters with the first speed profile:

      struct SpeedProfile* sp = &simparams.speed_step[ 0 ];

      cnt_duration_hours->setValue( sp->duration_hours   );
      cnt_duration_mins ->setValue( sp->duration_minutes );
      cnt_delay_hours   ->setValue( sp->delay_hours      );
      cnt_delay_mins    ->setValue( sp->delay_minutes    );
      cnt_rotorspeed    ->setValue( sp->rotorspeed       );
      cnt_acceleration  ->setValue( sp->acceleration     );
      cnt_scans         ->setValue( sp->scans            );
      
      cb_acceleration_flag->setChecked( sp->acceleration_flag );
      cnt_acceleration    ->setEnabled( sp->acceleration_flag );
      
      cnt_simpoints ->setValue( simparams.simpoints         );
      cnt_radial_res->setValue( simparams.radial_resolution );
      cnt_meniscus  ->setValue( simparams.meniscus          );
      cnt_bottom    ->setValue( simparams.bottom            );
      cnt_rnoise    ->setValue( simparams.rnoise            );
      cnt_tinoise   ->setValue( simparams.tinoise           );
      cnt_rinoise   ->setValue( simparams.rinoise           );

      cmb_mesh      ->setCurrentIndex( simparams.mesh        );
      cmb_moving    ->setCurrentIndex( simparams.moving_grid );

      rb_band->setChecked( simparams.band_forming );

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
   
void US_SimulationParameters::update_mesh( int mesh )
{
   simparams.mesh = mesh;

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

      QFile meshfile( US_Settings::usHomeDir() + "/mesh.dat");
      
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
                     simparams.mesh_radius << (double)simparams.meniscus;
                  }

                  first = false;
               }

               simparams.mesh_radius << value;
            }
         }

         meshfile.close();

         int mesh_size = simparams.mesh_radius.size();

         if ( simparams.mesh_radius[ mesh_size - 1 ] < simparams.bottom )
         {
            simparams.mesh_radius << simparams.bottom;
         }

         
         simparams.simpoints = simparams.mesh_radius.size();
         cnt_simpoints->setValue( (double) simparams.simpoints );
         
         // Can't change the simulation points after defining a mesh 
         cnt_simpoints->setEnabled( false ); 
      }
      else
      {
         simparams.mesh = 0; // Set to default ASTFEM mesh
         cmb_mesh->setCurrentIndex( 0 );
         
         // By default, the simpoints can be set by the user 
         cnt_simpoints->setEnabled( true ); 
         
         QMessageBox::warning( this, 
               tr( "UltraScan Warning" ),
               tr( "Please note:\n\n"
                   "UltraScan could not open the mesh file!\n"
                   "The file:\n\n" ) +  
                   US_Settings::usHomeDir() + tr( "/mesh.dat\n\n"
                   "could not be opened." ) );
      }
   }
}
