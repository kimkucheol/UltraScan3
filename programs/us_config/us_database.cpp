//! \file us_database.cpp
#include "us_database.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_help.h"
#include "us_passwd.h"
#include "us_crypto.h"
#include "us_db.h"

US_Database::US_Database( QWidget* w, Qt::WindowFlags flags ) 
  : US_Widgets( w, flags )
{
  // Frame layout
  setPalette( US_GuiSettings::frameColor() );

  setWindowTitle( "Database Configuration" );

  // Set up the database list window
  QLabel* banner = us_banner( tr( "Database List" ) );

  QBoxLayout* topbox = new QVBoxLayout( this );
  topbox->addWidget( banner );

  lw_entries = new QListWidget();
  lw_entries->setSortingEnabled( true );
  lw_entries->setPalette( US_GuiSettings::editColor() );

  /*  This isn't doing anything
  QFont*        f  = new QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
  QFontMetrics* fm = new QFontMetrics ( *f );
  int h = fm->height() * 3;  // Set box to 3 lines

  qDebug() << "Minimum height is " << h;
  db_list->resize( db_list->size().width(), 10 );
  */

  // Populate db_list
  lw_entries->setCurrentRow( 0 );
  connect( lw_entries, SIGNAL( itemPressed( QListWidgetItem* ) ), 
           this,       SLOT  ( select_db  ( QListWidgetItem* ) ) );

  update_lw();  // Fill in the list widget
  topbox->addWidget( lw_entries );

  // Detail info
  QLabel* info = us_banner( tr( "Database Detailed  Information" ) );
  topbox->addWidget( info );

  int row = 0;
  QGridLayout* details = new QGridLayout();
  
  // Row 1
  QLabel* desc = us_label( "Database Description:" );
  details->addWidget( desc, row, 0 );

  le_description = us_lineedit( "", 0 );
  details->addWidget( le_description, row++, 1 );

  // Row 2
  QLabel* user = us_label( "User Name:" );
  details->addWidget( user, row, 0 );

  le_username = us_lineedit( "", 0 );
  details->addWidget( le_username, row++, 1 );

  // Row 3
  QLabel* password = us_label( "Password:" );
  details->addWidget( password, row, 0 );

  le_password = us_lineedit( "", 0 );
  le_password->setEchoMode( QLineEdit::Password );
  details->addWidget( le_password, row++, 1 );

  // Row 4
  QLabel* DBname = us_label( "Database Name:" );
  details->addWidget( DBname, row, 0 );

  le_dbname = us_lineedit( "", 0 );
  details->addWidget( le_dbname, row++, 1 );

  // Row 5
  QLabel* host = us_label( "Host Address:" );
  details->addWidget( host, row, 0 );

  le_host = us_lineedit( "", 0 );
  details->addWidget( le_host, row++, 1 );

  topbox->addLayout( details );

  //Pushbuttons

  row = 0;
  QGridLayout* buttons = new QGridLayout();

  pb_add = us_pushbutton( tr( "Add New Entry" ) );
  connect( pb_add, SIGNAL( clicked() ), this, SLOT( check_add() ) );
  buttons->addWidget( pb_add, row, 0 );

  pb_delete = us_pushbutton( tr( "Delete Current Entry" ) );
  pb_delete->setEnabled( false );
  connect( pb_delete, SIGNAL( clicked() ), this, SLOT( deleteDB() ) );
  buttons->addWidget( pb_delete, row++, 1 );

  pb_save = us_pushbutton( tr( "Save as Default" ) );
  pb_save->setEnabled( false );
  connect( pb_save, SIGNAL( clicked() ), this, SLOT( save_default() ) );
  buttons->addWidget( pb_save, row, 0 );

  pb_reset = us_pushbutton( tr( "Reset" ) );
  connect( pb_reset, SIGNAL( clicked() ), this, SLOT( reset() ) );
  buttons->addWidget( pb_reset, row++, 1 );

  pb_testConnect = us_pushbutton( tr( "Test DB Connectivity" ) );
  pb_testConnect->setEnabled( false );
  connect( pb_testConnect, SIGNAL( clicked() ), this, SLOT( test_connect() ) );
  buttons->addWidget( pb_testConnect, row++, 0, 1, 2 );

  QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
  connect( pb_help, SIGNAL( clicked() ), this, SLOT( help() ) );
  buttons->addWidget( pb_help, row, 0 );

  QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
  connect( pb_cancel, SIGNAL( clicked() ), this, SLOT( close() ) );
  buttons->addWidget( pb_cancel, row++, 1 );

  topbox->addLayout( buttons );
}

void US_Database::select_db( QListWidgetItem* entry )
{
  // When this is run, we will always have a current dblist
  
  // Need to delete trailing (default) if that is present
  QString item = entry->text().remove( " (default)" );

  for ( int i = 0; i < dblist.size(); i++ )
  {
    if ( item == dblist.at( i ).at( 0 ) )
    {
      le_description->setText( item );
      le_username   ->setText( dblist.at( i ).at( 1 ) );
      le_dbname     ->setText( dblist.at( i ).at( 2 ) );
      le_host       ->setText( dblist.at( i ).at( 3 ) );

      QString cipher = dblist.at( i ).at( 4 );
      QString iv     = dblist.at( i ).at( 5 );
      QString pw     = US_Crypto::decrypt( cipher, iv );
      
      le_password->setText( pw );   
      
      pb_save       ->setEnabled( true );
      pb_delete     ->setEnabled( true );
      pb_testConnect->setEnabled( true );

      break;
    }
  }
}

void US_Database::check_add()
{
  // Check that all fields have at least something
  if ( le_description->text().isEmpty() )
  {
    QMessageBox::information( this, 
        tr( "Attention" ), 
        tr( "Please enter a Description before saving..." ) );
    return;
  }

  if ( le_username->text().isEmpty() )
  {
    QMessageBox::information( this, 
        tr( "Attention" ), 
        tr( "Please enter a User Name before saving..." ) );
    return;
  }

  if ( le_password->text().isEmpty() )
  {
    QMessageBox::information( this,
        tr( "Attention" ), 
        tr( "Please enter a Password before saving..." ) );
    return;
  }

  if ( le_dbname->text().isEmpty() )
  {
    QMessageBox::information( this,
        tr( "Attention" ), 
        tr( "Please enter a Database Name before saving..."));
    return;
  }

  if ( le_host->text().isEmpty() )
  {
    QMessageBox::information( this,
        tr( "Attention" ), 
        tr( "Please enter a Host Address (possibly  'localhost') "
            "before saving..." ) );
    return;
  }
  
  // Encrypt the DB password with the master password
  QString     db_pw = le_password->text();
  QStringList pw    = US_Crypto::encrypt( db_pw );

  // Save the DB entry
  dblist = US_Settings::databases();
  bool updated = false;

  for ( int i = 0; i < dblist.size(); i++ )
  {
     QStringList db = dblist.at( i );
     if ( db.at( 0 ) == le_description->text() )
     {
        db.replace( 1, le_username->text() );
        db.replace( 2, le_dbname->text()   );
        db.replace( 3, le_host->text()     ) ;
        db.replace( 4, pw.at( 0 )          );  // Ecnrypted password
        db.replace( 5, pw.at( 1 )          );  // Initialization vector

        dblist.replace( i, db );
        updated     = true;
        break;
     }
  }

  if ( ! updated )
  {
    QStringList entry;
    entry << le_description->text() << le_username->text() << le_dbname->text()
          << le_host->text()        << pw.at( 0 )          << pw.at( 1 );

    dblist << entry;
  }

  // Update the list widget
  US_Settings::set_databases( dblist );
  update_lw( le_description->text() );

  pb_save       ->setEnabled( true );
  pb_delete     ->setEnabled( true );
  pb_testConnect->setEnabled( true );
} 

void US_Database::update_lw( const QString& current )
{
  lw_entries->clear();

  dblist            = US_Settings::databases();
  QString defaultDB = US_Settings::defaultDB().at( 0 );

  for ( int i = 0; i < dblist.size(); i++ )
  {
    QString desc    = dblist.at( i ).at( 0 );
    QString display = desc;
    
    if ( desc == defaultDB ) display.append( " (default)" );
    
    QListWidgetItem* widget = new QListWidgetItem( display );
    lw_entries->addItem( widget );  
     
    if ( desc == current ) 
      lw_entries->setCurrentItem( widget, QItemSelectionModel::Select );
  }
}

void US_Database::reset( void )
{
  QString defaultDB = US_Settings::defaultDB().at( 0 );
  update_lw( defaultDB );

  le_description->setText("");
  le_username   ->setText("");
  le_password   ->setText("");
  le_dbname     ->setText("");
  le_host       ->setText("");

  pb_save       ->setEnabled( false );
  pb_delete     ->setEnabled( false );
  pb_testConnect->setEnabled( false );
}

void US_Database::help( void )
{
  US_Help* showhelp = new US_Help(this);
  showhelp->show_help( "manual/database_config.html" );
}
  
void US_Database::save_default( void )
{
  for ( int i = 0; i < dblist.size(); i++ )
  {
    QString desc = dblist.at( i ).at( 0 );
     
    // Look for the current description
    if ( desc == le_description->text() ) 
    {
      US_Settings::set_defaultDB( dblist.at( i ) );
      reset();
      
      QMessageBox::information( this, 
          tr( "Default Database" ),
          tr( "The default database has been updated." ) );
      return;
    }
  }
     
  QMessageBox::warning( this, 
      tr( "Default Database Problem" ),
      tr( "The description does not match any in the database list.\n"
          "The default database has not been updated." ) );
}

void US_Database::deleteDB( void )
{
  QString item = le_description->text();

  // Go through list and delete the one matching description
  for ( int i = 0; i < dblist.size(); i++ )
  {
    QString desc = dblist.at( i ).at( 0 );
     
    // Look for the current description
    if ( desc == item ) 
    {
      dblist.removeAt( i );
      US_Settings::set_databases( dblist );
      
      // Check if the default DB matches
      QStringList defaultDB = US_Settings::defaultDB();
      if ( defaultDB.at( 0 ) == item )
        US_Settings::set_defaultDB( QStringList() );
      
      reset();

      QMessageBox::information( this, 
          tr( "Database Removed" ),
          tr( "The database has been removed." ) );
      return;
    }
  }
     
  QMessageBox::warning( this, 
      tr( "Database Problem" ),
      tr( "The description does not match any in the database list.\n"
          "The database has not been removed." ) );
}

void US_Database::test_connect( void )
{
  QString error;
  bool ok = US_DB::test_db_connection( le_host->text(), le_dbname->text(), 
              le_username->text(), le_password->text(), error );

  if ( ok ) 
    QMessageBox::information( this,
      tr( "Database Connectiom" ),
      tr( "The connection was successful." ) );
  else
    QMessageBox::warning( this,
      tr( "Database Connectiom" ),
      tr( "The connection failed.\n" ) + error );
}
