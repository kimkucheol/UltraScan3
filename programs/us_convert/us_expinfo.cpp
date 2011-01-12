//! \file us_expinfo.cpp

#include <QtGui>

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_expinfo.h"
#include "us_convertio.h"
#include "us_project_gui.h"

US_ExpInfo::US_ExpInfo( 
      ExperimentInfo& dataIn ) :
   US_WidgetsDialog( 0, 0 ), expInfo( dataIn )
{
   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle( tr( "Experiment Information" ) );
   setAttribute( Qt::WA_DeleteOnClose );

   // Very light gray, for read-only line edits
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   // Set up left panel with experiment information
   QGridLayout* experiment = new QGridLayout;
   int row = 0;

   // Current experiment information
   QLabel* lb_experiment_banner = us_banner( tr( "Experiment: " ) );
   experiment->addWidget( lb_experiment_banner, row++, 0, 1, 2 );

   // Show current runID
   QLabel* lb_runID = us_label( tr( "Run ID " ) );
   experiment->addWidget( lb_runID, row++, 0, 1, 2 );
   le_runID = us_lineedit();
   le_runID->setPalette ( gray );
   le_runID->setReadOnly( true );

   experiment->addWidget( le_runID, row++, 0, 1, 2 );
 
   // Experiment label
   QLabel* lb_label = us_label( tr( "Label:" ) );
   experiment->addWidget( lb_label, row++, 0, 1, 2 );
   le_label = us_lineedit();
   experiment->addWidget( le_label, row++, 0, 1, 2 );

   // Project
   QLabel* lb_project = us_label( tr( "Project:" ) );
   experiment->addWidget( lb_project, row, 0 );
   QPushButton* pb_project = us_pushbutton( tr( "Select Project" ) );
   connect( pb_project, SIGNAL( clicked() ), SLOT( selectProject() ) );
   pb_project->setEnabled( true );
   experiment->addWidget( pb_project, row++, 1 );

   le_project = us_lineedit();
   le_project->setPalette ( gray );
   le_project->setReadOnly( true );
   experiment->addWidget( le_project, row++, 0, 1, 2 );

   // Experiment type
   QLabel* lb_expType = us_label( tr( "Experiment Type:" ) );
   experiment->addWidget( lb_expType, row, 0 );
   cb_expType = us_expTypeComboBox();
   experiment->addWidget( cb_expType, row++, 1 );

   // Optical system
   QLabel* lb_opticalSystem = us_label( tr( "Optical System:" ) );
   experiment->addWidget( lb_opticalSystem, row, 0 );
   QLineEdit* le_opticalSystem = us_lineedit();
   le_opticalSystem->setPalette ( gray );
   le_opticalSystem->setReadOnly( true );
   experiment->addWidget( le_opticalSystem, row++, 1 );

   // The optical system won't change
   if ( ( expInfo.opticalSystem == "RA" ) ||
        ( expInfo.opticalSystem == "WA" ) )
      le_opticalSystem->setText( "Absorbance" );

   else if ( ( expInfo.opticalSystem == "RI" ) ||
             ( expInfo.opticalSystem == "WI" ) )
      le_opticalSystem->setText( "Intensity" );

   else if ( expInfo.opticalSystem == "IP" )
      le_opticalSystem->setText( "Interference" );

   else if ( expInfo.opticalSystem == "FI" )
      le_opticalSystem->setText( "Fluorescence" );

   else // Unsupported optical system
      le_opticalSystem->setText( "Unsupported" );

   // Now for predominantly hardware info
   QGridLayout* hardware = new QGridLayout;
   row = 0;

   // Selected hardware information
   QLabel* lb_hardware_banner = us_banner( tr( "Hardware: " ) );
   hardware->addWidget( lb_hardware_banner, row++, 0, 1, 2 );

   // sync local hardware info with db
/*
   QLabel* lb_sync = us_label( tr( "Update local hardware info:" ) );
   hardware->addWidget( lb_sync, row, 0 );
   QPushButton* pb_sync = us_pushbutton( tr( "Sync Hardware" ) );
   connect( pb_sync, SIGNAL( clicked() ), SLOT( syncHardware() ) );
   pb_sync->setEnabled( true );
   hardware->addWidget( pb_sync, row++, 1 );
*/

   // labID
   QLabel* lb_lab = us_label( tr( "Lab:" ) );
   hardware->addWidget( lb_lab, row, 0 );
   cb_lab = new US_SelectBox( this );
   connect( cb_lab, SIGNAL( activated ( int ) ),      // Only if the user has changed it
                    SLOT  ( change_lab( int ) ) );
   hardware->addWidget( cb_lab, row++, 1 );

   // instrumentID
   QLabel* lb_instrument = us_label( tr( "Instrument:" ) );
   hardware->addWidget( lb_instrument, row, 0 );
   cb_instrument = new US_SelectBox( this );
   connect( cb_instrument, SIGNAL( activated        ( int ) ),
                           SLOT  ( change_instrument( int ) ) );
   hardware->addWidget( cb_instrument, row++, 1 );

   // operatorID
   QLabel* lb_operator = us_label( tr( "Operator:" ) );
   hardware->addWidget( lb_operator, row, 0 );
   cb_operator = new US_SelectBox( this );
   hardware->addWidget( cb_operator, row++, 1 );

   // Rotor used in experiment
   QLabel* lb_rotor = us_label( tr( "Rotor:" ) );
   hardware->addWidget( lb_rotor, row, 0 );
   cb_rotor = new US_SelectBox( this );
   hardware->addWidget( cb_rotor, row++, 1 );
   cb_rotor->setEditable( false );

   // Rotor speeds
   QLabel* lb_rotorSpeeds = us_label( tr( "Unique Rotor Speeds:" ) );
   hardware->addWidget( lb_rotorSpeeds, row++, 0, 1, 2 );
   lw_rotorSpeeds = us_listwidget();
   lw_rotorSpeeds ->setMaximumHeight( 100 );
   lw_rotorSpeeds ->setPalette( gray );
   hardware->addWidget( lw_rotorSpeeds, row, 0, 2, 2 );
   row += 3;

   // The rotor speed information won't change
   foreach ( double rpm, expInfo.rpms )
      lw_rotorSpeeds -> addItem( QString::number( rpm ) );

   // Run Temperature
   QLabel* lb_runTemp = us_label( tr( "Average Run Temperature:" ) );
   hardware->addWidget( lb_runTemp, row, 0 );
   le_runTemp = us_lineedit();
   hardware->addWidget( le_runTemp, row++, 1 );
   le_runTemp->setPalette ( gray );
   le_runTemp->setReadOnly( true );

   // Run temperature won't change
   le_runTemp           ->setText( expInfo.runTemp );

   // Some pushbuttons
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
   buttons->addWidget( pb_cancel );

   pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );
   buttons->addWidget( pb_accept );

   // Now let's assemble the page
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   row = 0;

   // Database choices
   QStringList DB = US_Settings::defaultDB();
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   main->addWidget( lb_DB, row++, 0, 1, 2 );

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( selectInvestigator() ) );
   main->addWidget( pb_investigator, row, 0 );

   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   le_investigator = us_lineedit( "", 1 );
   le_investigator->setReadOnly( true );
   main->addWidget( le_investigator, row++, 1 );

   main->addLayout( experiment, row, 0 );
   main->addLayout( hardware,   row, 1 );
   row++; // += 10;

   // Experiment comments
   QLabel* lb_comment = us_label( tr( "Comments:" ) );
   main->addWidget( lb_comment, row++, 0, 1, 2 );

   te_comment = us_textedit();
   main->addWidget( te_comment, row, 0, 4, 2 );
   te_comment->setMaximumHeight( 120 );
   te_comment->setReadOnly( false );
   row += 4;

   main->addLayout( buttons, row++, 0, 1, 2 );

   // Let's load everything we can
   if ( ! load() )
   {
      cancel();
      return;
   }

   reset();
}

void US_ExpInfo::reset( void )
{
   reload();

   le_investigator ->clear();
   le_label        ->clear();
   le_runID        ->setText( expInfo.runID );
   le_project      ->setText( expInfo.projectDesc );
   te_comment      ->clear();

   cb_lab          ->load();
   cb_instrument   ->load();
   cb_operator     ->load();
   cb_rotor        ->load();

   pb_accept       ->setEnabled( false );

   // Update controls to represent selected experiment
   cb_lab          ->setLogicalIndex( expInfo.labID        );
   cb_instrument   ->setLogicalIndex( expInfo.instrumentID );
   cb_operator     ->setLogicalIndex( expInfo.operatorID   );
   cb_rotor        ->setLogicalIndex( expInfo.rotorID      );

   le_label        ->setText( expInfo.label                );
   te_comment      ->setText( expInfo.comments             );
         
   // Experiment types combo
   cb_expType->setCurrentIndex( 3 );  // default is "other"
   for ( int i = 0; i < experimentTypes.size(); i++ )
   {
      if ( experimentTypes[ i ].toUpper() == expInfo.expType.toUpper() )
      {
         cb_expType->setCurrentIndex( i );
         break;
      }
   }
   
   // Display investigator
   expInfo.invID = US_Settings::us_inv_ID();

   if ( expInfo.invID > 0 )
   {
      le_investigator->setText( QString::number( expInfo.invID ) + ": " 
         + US_Settings::us_inv_name() );

      if ( expInfo.expID > 0 )
         pb_accept       ->setEnabled( true );
 
      else if ( US_ConvertIO::checkRunID( expInfo.runID ) == 0 )
      {
         // Then an investigator has been chosen, and 
         //  the current runID doesn't exist in the db
         pb_accept       ->setEnabled( true );
      }

      // However, project needs to be selected
      if ( expInfo.projectID == 0 )
         pb_accept       ->setEnabled( false );
   }

   else
      le_investigator->setText( "Not Selected" );

}

// function to load what we can initially
// returns true if successful
bool US_ExpInfo::load( void )
{
   if ( expInfo.invID == 0 )
   {
      // Try to get info from settings
      int inv = US_Settings::us_inv_ID();
      if ( inv > -1 )
      {
         expInfo.invID     = inv;
         getInvestigatorInfo();
      }
   }

   // Find out what labs we have
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return( false );
   }

   QStringList q( "get_lab_names" );
   db.query( q );

   QList<listInfo> options;
   while ( db.next() )
   {
      struct listInfo option;
      option.ID      = db.value( 0 ).toString();
      option.text    = db.value( 1 ).toString();
      options << option;
   }

   cb_lab->clear();
   if ( options.size() > 0 )
   {
      cb_lab->addOptions( options );

      // is the lab ID in the list?
      int index = 0;
      for ( int i = 0; i < options.size(); i++ )
      {
         if ( expInfo.labID == options[ i ].ID.toInt() )
         {
            index = i;
            break;
         }
      }
   
      // Replace labID with one from the list
      expInfo.labID = options[ index ].ID.toInt();
   }

   cb_changed = true; // so boxes will go through the reload code 1st time

   return( true );
}

void US_ExpInfo::reload( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   if ( cb_changed )
   {
      setInstrumentList();
      setRotorList();
      setOperatorList();

      cb_changed = false;
   }
}

void US_ExpInfo::syncHardware( void )
{
}

void US_ExpInfo::selectInvestigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true, expInfo.invID );

   connect( inv_dialog, 
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( assignInvestigator   ( int, const QString&, const QString& ) ) );

   inv_dialog->exec();
}

void US_ExpInfo::assignInvestigator( int invID,
      const QString& lname, const QString& fname )
{
   expInfo.invID = invID;
   le_investigator->setText( QString::number( invID ) + ": " +
         lname + ", " + fname );
}

void US_ExpInfo::getInvestigatorInfo( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   expInfo.invID = US_Settings::us_inv_ID();     // just to be sure
   expInfo.name  = US_Settings::us_inv_name();
   QStringList q( "get_person_info" );
   q << QString::number( expInfo.invID );
   db.query( q );

   if ( db.next() )
   {
      expInfo.invGUID   = db.value( 9 ).toString();
   }
   
}

void US_ExpInfo::selectProject( void )
{
   US_Project project;
   project.projectID   = expInfo.projectID;
   project.projectDesc = expInfo.projectDesc;

   US_ProjectGui* projInfo = new US_ProjectGui( true, US_Disk_DB_Controls::DB, project );
   connect( projInfo, 
      SIGNAL( updateProjectGuiSelection( US_Project& ) ),
      SLOT  ( assignProject            ( US_Project& ) ) );
   connect( projInfo, 
      SIGNAL( cancelProjectGuiSelection( ) ),
      SLOT  ( cancelProject            ( ) ) );
   projInfo->exec();
}

void US_ExpInfo::assignProject( US_Project& project )
{
   expInfo.projectID   = project.projectID;
   expInfo.projectDesc = project.projectDesc;
   expInfo.projectGUID = project.projectGUID;

   reset();
}

void US_ExpInfo::cancelProject( void )
{
  reset();
}

QComboBox* US_ExpInfo::us_expTypeComboBox( void )
{
   QComboBox* cb = us_comboBox();

   // Experiment types
   experimentTypes.clear();
   experimentTypes << "Velocity"
                   << "Equilibrium"
                   << "Diffusion"
                   << "other";

   cb->addItems( experimentTypes );

   return cb;
}

void US_ExpInfo::setInstrumentList( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "get_instrument_names" );
   q << QString::number( expInfo.labID );     // In this lab
   db.query( q );

   QList<listInfo> options;
   while ( db.next() )
   {
      struct listInfo option;
      option.ID      = db.value( 0 ).toString();
      option.text    = db.value( 1 ).toString();
      options << option;
   }

   cb_instrument->clear();
   if ( options.size() > 0 )
   {
      cb_instrument->addOptions( options );

      // is the instrument ID in the list?
      int index = 0;
      for ( int i = 0; i < options.size(); i++ )
      {
         if ( expInfo.instrumentID == options[ i ].ID.toInt() )
         {
            index = i;
            break;
         }
      }

      // Replace instrument ID with one from the list
      expInfo.instrumentID = options[ index ].ID.toInt();
         
   }

}

void US_ExpInfo::setOperatorList( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "get_operator_names" );
   q << QString::number( expInfo.instrumentID );  // who can use this instrument
   db.query( q );

   QList<listInfo> options;
   while ( db.next() )
   {
      struct listInfo option;
      option.ID      = db.value( 0 ).toString();
      option.text    = db.value( 1 ).toString();
      options << option;
   }

   cb_operator->clear();
   if ( options.size() > 0 )
   {
      cb_operator->addOptions( options );

      // is the operator ID in the list?
      int index = 0;
      for ( int i = 0; i < options.size(); i++ )
      {
         if ( expInfo.operatorID == options[ i ].ID.toInt() )
         {
            index = i;
            break;
         }
      }

      // Replace operator ID with one from the list
      expInfo.operatorID = options[ index ].ID.toInt();
   }
}

void US_ExpInfo::setRotorList( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   // Get a list of rotors in this lab
   QStringList q( "get_rotor_names" );
   q << QString::number( expInfo.labID );     // In this lab
   db.query( q );

   QList<listInfo> options;
   while ( db.next() )
   {
      struct listInfo option;
      option.ID      = db.value( 0 ).toString();
      option.text    = db.value( 1 ).toString();
      options << option;
   }

   cb_rotor->clear();
   if ( options.size() > 0 )
   {
      cb_rotor->addOptions( options );

      // is the rotor ID in the list?
      int index = 0;
      for ( int i = 0; i < options.size(); i++ )
      {
         if ( expInfo.rotorID == options[ i ].ID.toInt() )
         {
            index = i;
            break;
         }
      }

      // Replace rotor ID with one from the list
      expInfo.rotorID = options[ index ].ID.toInt();

      // For now get the first rotor calibration profile for this rotor
      q.clear();
      q  << QString( "get_rotor_calibration_profiles" )
         << QString::number( expInfo.rotorID );
      db.query( q );

      expInfo.calibrationID = 0;
      expInfo.rotorCoeff1   = 0.0;
      expInfo.rotorCoeff2   = 0.0;
      if ( db.next() )
      {
         expInfo.calibrationID = db.value( 0 ).toInt();

         q.clear();
         q  << QString( "get_rotor_calibration_info" )
            << QString::number( expInfo.calibrationID );
         db.query( q );
         if ( db.next() )
         {
            expInfo.rotorCoeff1 = db.value( 3 ).toDouble();
            expInfo.rotorCoeff2 = db.value( 4 ).toDouble();
         }
      }
   }
}

// Function to change the current lab
void US_ExpInfo::change_lab( int )
{
   // First time through here the combo box might not be displayed yet
   expInfo.labID = ( cb_lab->getLogicalID() == -1 )
                   ? expInfo.labID
                   : cb_lab->getLogicalID();
 
   // Save other elements on the page too
   expInfo.label         = le_label   ->text(); 
   expInfo.comments      = te_comment ->toPlainText();
   expInfo.expType       = cb_expType ->currentText();

   cb_changed = true;
   reset();
}

// Function to change the current instrument
void US_ExpInfo::change_instrument( int )
{
   // First time through here the combo box might not be displayed yet
   expInfo.instrumentID = ( cb_instrument->getLogicalID() == -1 )
                          ? expInfo.instrumentID
                          : cb_instrument->getLogicalID();

   // Save other elements on the page too
   expInfo.label         = le_label   ->text(); 
   expInfo.comments      = te_comment ->toPlainText();
   expInfo.expType       = cb_expType ->currentText();

   cb_changed = true;
   reset();
}

void US_ExpInfo::accept( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   // Overwrite data directly from the form

   // First get the invID
   expInfo.invID   = US_Settings::us_inv_ID();
   expInfo.name    = US_Settings::us_inv_name();
   getInvestigatorInfo();

   // Other experiment information
   expInfo.runID         = le_runID         ->text();
   expInfo.labID         = cb_lab           ->getLogicalID();
   expInfo.instrumentID  = cb_instrument    ->getLogicalID();
   expInfo.operatorID    = cb_operator      ->getLogicalID();
   expInfo.rotorID       = cb_rotor         ->getLogicalID();
   expInfo.expType       = cb_expType       ->currentText();
   expInfo.runTemp       = le_runTemp       ->text(); 
   expInfo.label         = le_label         ->text(); 
   expInfo.comments      = te_comment       ->toPlainText();

   // Experiment information
   QString status = US_ConvertIO::readExperimentInfoDB( expInfo );
   expInfo.syncOK = ( status.isEmpty() );

   emit updateExpInfoSelection( expInfo );
   close();
}

void US_ExpInfo::cancel( void )
{
   expInfo.clear();

   emit cancelExpInfoSelection();
   close();
}

void US_ExpInfo::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to databasee \n" ) + error );
}

US_ExpInfo::ExperimentInfo::ExperimentInfo()
{
   ExperimentInfo::clear();
}

// Zero out all data structures
void US_ExpInfo::ExperimentInfo::clear( void )
{
   invID              = US_Settings::us_inv_ID();
   invGUID            = QString( "" );
   name               = US_Settings::us_inv_name();
   expID              = 0;
   expGUID            = QString( "" );
   projectID          = 0;
   projectGUID        = QString( "" );
   projectDesc        = QString( "" );
   runID              = QString( "" );
   labID              = 0;
   labGUID            = QString( "" );
   instrumentID       = 0;
   instrumentSerial   = QString( "" );
   operatorID         = 0;
   operatorGUID       = QString( "" );
   rotorID            = 0;
   calibrationID      = 0;
   rotorCoeff1        = 0.0;
   rotorCoeff2        = 0.0;
   rotorGUID          = QString( "" );
   rotorSerial        = QString( "" );
   expType            = QString( "" );
   opticalSystem      = QByteArray( "  " );
   rpms.clear();
   runTemp            = QString( "" );
   label              = QString( "" );
   comments           = QString( "" );
   centrifugeProtocol = QString( "" );
   date               = QString( "" );
   syncOK             = false;

}

US_ExpInfo::ExperimentInfo& US_ExpInfo::ExperimentInfo::operator=( const ExperimentInfo& rhs )
{
   if ( this != &rhs )            // Guard against self assignment
   {
      invID         = rhs.invID;
      invGUID       = rhs.invGUID;
      name          = rhs.name;
      expID         = rhs.expID;
      expGUID       = rhs.expGUID;
      projectID     = rhs.projectID;
      projectGUID   = rhs.projectGUID;
      projectDesc   = rhs.projectDesc;
      runID         = rhs.runID;
      labID         = rhs.labID;
      labGUID       = rhs.labGUID;
      instrumentID  = rhs.instrumentID;
      instrumentSerial  = rhs.instrumentSerial;
      operatorID    = rhs.operatorID;
      rotorID       = rhs.rotorID;
      calibrationID = rhs.calibrationID;
      rotorCoeff1   = rhs.rotorCoeff1;
      rotorCoeff2   = rhs.rotorCoeff2;
      rotorGUID     = rhs.rotorGUID;
      rotorSerial   = rhs.rotorSerial;
      expType       = rhs.expType;
      opticalSystem = rhs.opticalSystem;
      runTemp       = rhs.runTemp;
      label         = rhs.label;
      comments      = rhs.comments;
      centrifugeProtocol = rhs.centrifugeProtocol;
      date          = rhs.date;
      syncOK        = rhs.syncOK;

      rpms.clear();
      for ( int i = 0; i < rhs.rpms.size(); i++ )
         rpms << rhs.rpms[ i ];

   }

   return *this;
}

void US_ExpInfo::ExperimentInfo::show( void )
{
   QString syncOK_text = ( syncOK ) ? "true" : "false";

   qDebug() << "invID        = " << invID << '\n'
            << "invGUID      = " << invGUID << '\n'
            << "name         = " << name << '\n'
            << "expID        = " << expID << '\n'
            << "expGUID      = " << expGUID << '\n'
            << "projectID    = " << projectID << '\n'
            << "projectGUID  = " << projectGUID << '\n'
            << "projectDesc  = " << projectDesc << '\n'
            << "runID        = " << runID << '\n'
            << "labID        = " << labID << '\n'
            << "labGUID      = " << labGUID << '\n'
            << "instrumentID = " << instrumentID << '\n'
            << "instrumentSerial = " << instrumentSerial << '\n'
            << "operatorID   = " << operatorID << '\n'
            << "operatorGUID = " << operatorGUID << '\n'
            << "rotorID      = " << rotorID << '\n'
            << "calibrationID = " << calibrationID << '\n'
            << "rotorCoeff1  = " << rotorCoeff1 << '\n'
            << "rotorCoeff2  = " << rotorCoeff2 << '\n'
            << "rotorGUID    = " << rotorGUID << '\n'
            << "rotorSerial  = " << rotorSerial << '\n'
            << "expType      = " << expType << '\n'
            << "opticalSystem = " << opticalSystem << '\n'
            << "runTemp      = " << runTemp << '\n'
            << "label        = " << label << '\n'
            << "comments     = " << comments << '\n'
            << "centrifugeProtocol = " << centrifugeProtocol << '\n'
            << "date         = " << date << '\n'
            << "syncOK       = " << syncOK_text << '\n';

   for ( int i = 0; i < rpms.size(); i++ )
   {
      qDebug() << "i = " << i ;
      qDebug() << "rpm = " << rpms[ i ];
   }

}
