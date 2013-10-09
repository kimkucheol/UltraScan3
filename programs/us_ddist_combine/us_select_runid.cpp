//! \file us_select_runid.cpp

#include "us_select_runid.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_util.h"
#include "us_editor.h"
#include "us_constants.h"
#include "us_report.h"
#include "us_sleep.h"

// Main constructor with flags for select-runID dialog

US_SelectRunid::US_SelectRunid( bool dbase, QStringList& runIDs,
   QStringList& mdescrs ) : US_WidgetsDialog( 0, 0 ),
   runIDs( runIDs ), mDescrs( mdescrs )
{
   sel_db        = dbase;
   dbg_level     = US_Settings::us_debug();
   nimodel       = mDescrs.count();

   setWindowTitle( tr( "Select Run ID(s) for Discrete Distributions (%1)" )
         .arg( sel_db ? "DB" : "Local" ) );
   setPalette    ( US_GuiSettings::frameColor() );
   setMinimumSize( 480, 300 );
DbgLv(1) << "SE:sel_db" << sel_db;

   // Main layout
   QVBoxLayout* main = new QVBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Top layout: buttons and fields above list widget
   QGridLayout* top  = new QGridLayout;

   dkdb_cntrls         = new US_Disk_DB_Controls(
      sel_db ? US_Disk_DB_Controls::DB : US_Disk_DB_Controls::Disk );
   pb_invest           = us_pushbutton( tr( "Select Investigator" ) );
   QString invnum      = QString::number( US_Settings::us_inv_ID() ) + ": ";
   QString invusr      = US_Settings::us_inv_name();
   le_invest           = us_lineedit( invnum + invusr, 0, true );
   pb_invest->setEnabled( sel_db );

   // Search line
   QLabel* lb_filtdata = us_label( tr( "Search" ) );

   le_dfilter      = us_lineedit();

   connect( dkdb_cntrls, SIGNAL( changed( bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );
   connect( pb_invest,   SIGNAL( clicked()    ),
                         SLOT  ( get_person() ) );
   connect( le_dfilter,  SIGNAL( textChanged( const QString& ) ),
                         SLOT  ( search     ( const QString& ) ) );

   int row           = 0;
   top->addLayout( dkdb_cntrls, row++, 0, 1, 3 );
   top->addWidget( pb_invest,   row,   0, 1, 1 );
   top->addWidget( le_invest,   row++, 1, 1, 2 );
   top->addWidget( lb_filtdata, row,   0, 1, 1 );
   top->addWidget( le_dfilter,  row++, 1, 1, 2 );

   main->addLayout( top );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );

   // List widget to show data choices
   lw_data = new QListWidget( this );
   lw_data->setFrameStyle   ( QFrame::NoFrame );
   lw_data->setPalette      ( US_GuiSettings::editColor() );
   lw_data->setFont         ( font );
   lw_data->setSelectionMode( QAbstractItemView::ExtendedSelection );
   connect( lw_data,  SIGNAL( itemSelectionChanged() ),
            this,     SLOT  ( selectionChanged()     ) );

   main->addWidget( lw_data );

   // Button Row
   QHBoxLayout* buttons   = new QHBoxLayout;

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );

   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancelled() ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accepted() ) );

   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons );

   // Status Row
   QFontMetrics fm( font );
   int fhigh = fm.lineSpacing();
   int fwide = fm.width( QChar( '6' ) );
   int lhigh = fhigh * 4 + 12;
   int lwide = fwide * 32;

   te_status               = us_textedit();
   te_status->setMaximumHeight( lhigh );
   te_status->resize( lwide, lhigh );
   us_setReadOnly( te_status, true );
   te_status->setTextColor( Qt::blue );

   main->addWidget( te_status );

   // List from disk or db source
   list_data();
}

// Hide list items that do not contain search string
void US_SelectRunid::search( const QString& search_string )
{
   lw_data->setCurrentItem( NULL );

   for ( int ii = 0; ii < lw_data->count(); ii++ )
   {
      QListWidgetItem* lwi = lw_data->item( ii );
      bool hide = ! lwi->text().contains( search_string, Qt::CaseInsensitive ); 
      lwi->setHidden( hide );
   }
}

// List data choices (from db or disk)
void US_SelectRunid::list_data()
{
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   rlabels.clear();

   if ( sel_db )              // Scan database data
   {
      scan_dbase_runs();
   }
   else                       // Scan local disk data
   {
      scan_local_runs();
   }

   QApplication::restoreOverrideCursor();
   lw_data->clear();

   if ( rlabels.size() == 0 )
   {  // Report and return now if no items found
      QString clabel = tr( "No data found." );
      lw_data->addItem( new QListWidgetItem( clabel ) );
      return;
   }
DbgLv(1) << "LD:sel_db" << sel_db << "rlsize" << rlabels.size();

   for ( int ii = 0; ii < rlabels.size(); ii++ )
   {  // Propagate list widget with labels
      QString  clabel  = rlabels.at( ii );

      lw_data->addItem( new QListWidgetItem( clabel ) );
   }

   // Report list state in status box
   count_list = lw_data->count();
   count_seld = lw_data->selectedItems().size();
   te_status->setText(
      tr( "%1 scanned run IDs were used to derive the list. Of these,\n"
          "%2 have associated distributions (models), and\n"
          "%3 %4 currently selected for combination plot components." )
      .arg( count_allr ).arg( count_list ).arg( count_seld )
      .arg( count_seld != 1 ? tr( "runs are" ) : tr( "run is" ) ) );
}

// Cancel button:  no runIDs returned
void US_SelectRunid::cancelled()
{
   reject();
   close();
}

// Accept button:  set up to return runs and associated model information
void US_SelectRunid::accepted()
{
DbgLv(1) << "SE:accepted()";
   QList< QListWidgetItem* > selitems = lw_data->selectedItems();

   if ( selitems.size() == 0 )
   {
      QMessageBox::information( this,
            tr( "No Data Selected" ),
            tr( "You have not selected any data.\nSelect or Cancel" ) );
      return;
   }

   QString slines = te_status->toPlainText() + 
      tr( "\nScanning models for selected run(s)..." );
   te_status->setText( slines );
   qApp->processEvents();
DbgLv(1) << "SE: slines" << slines;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   runIDs.clear();

   // Get and return runIDs from selected edit items
   for ( int ii = 0; ii < selitems.size(); ii++ )
   {
      QListWidgetItem* lwi_data = selitems.at( ii );
      QString clabel = lwi_data->text();
DbgLv(1) << "SE:  ii clabel" << ii << clabel;

      runIDs << clabel;
   }
DbgLv(1) << "SE: runID0" << runIDs[0];

   // Scan models to build descriptions for selected runs
   if ( sel_db )
      scan_dbase_models();
   else
      scan_local_models();

   QApplication::restoreOverrideCursor();
   qApp->processEvents();
   int namodel = wDescrs.size();

   if ( namodel == 0 )
   {
      QMessageBox::warning( this,
         tr( "No Implied Models" ),
         tr( "There were no Discrete Distributions associated\n"
             " with the selected runs.\n"
             "Cancel or select a new set of runs." ) );
      return;
   }

   mDescrs << wDescrs;      // Append new model descriptions

   accept();                // Signal that selection was accepted
   close();
}

// Scan database for run sets
void US_SelectRunid::scan_dbase_runs()
{
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );
   count_allr = 0;
   count_list = 0;
   count_seld = 0;

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::critical( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" )
         + db.lastError() );
      return;
   }

   QStringList  query;
   QString invID    = QString::number( US_Settings::us_inv_ID() );
QTime timer;
timer.start();

   // Scan experiments. getting run and experiment IDs
   query.clear();
   query << "get_experiment_desc" << invID;
   db.query( query );

   while ( db.next() )
   {  // Get a list of all runs in the database
      QString expid    = db.value( 0 ).toString();
      QString runid    = db.value( 1 ).toString();
DbgLv(1) << "ScDB:     runid" << runid << "expid" << expid;

      if ( ! runIDs.contains( runid ) )
      {
         count_allr++;
         rlabels << runid;     // Save run ID to list of selectable runs
         runIDs  << runid;     // Save run ID to list of total runs
         expIDs  << expid;     // Save experiment ID to list for total runs
      }
   }
DbgLv(1) << "ScDB:count_allr" << count_allr;
DbgLv(1) << "ScDB:scan time(1)" << timer.elapsed();

   // Scan edited data, getting runs and experiments for edits
   query.clear();
   query << "all_editedDataIDs" << invID;
   db.query( query );

   while ( db.next() )
   {
      QString edtid    = db.value( 0 ).toString();
      QString rawid    = db.value( 3 ).toString();
      QString expid    = db.value( 4 ).toString();
      int     kk       = expIDs.indexOf( expid );
      QString runid    = ( kk >=0 ) ? runIDs[ kk ] : "";
      edtIDs  << edtid;
      erunIDs << runid;
      eexpIDs << expid;
   }
DbgLv(1) << "ScDB:scan time(2)" << timer.elapsed();

   // Scan saved edits, adding model counts
   for ( int ee = 0; ee < edtIDs.count(); ee++ )
   {
      QString edtid    = edtIDs[ ee ];
      query.clear();
      query << "count_models_by_editID" << invID << edtid;
      int     nemods   = db.functionQuery( query );
      emodKnts << nemods;
   }
DbgLv(1) << "ScDB:scan time(3)" << timer.elapsed();

   // Build run edit and run model counts lists
   for ( int rr = 0; rr < runIDs.count(); rr++ )
   {
      QString runid    = runIDs[ rr ];
      int     nredts   = 0;
      int     nrmods   = 0;
      for ( int ee = 0; ee < edtIDs.count(); ee++ )
      {
         QString edtid    = edtIDs [ ee ];
         QString erunid   = erunIDs[ ee ];

         if ( erunid == runid )
         {
            nredts++;
            nrmods       += emodKnts[ ee ];
         }
      }
      redtKnts << nredts;
      rmodKnts << nrmods;
   }
DbgLv(1) << "ScDB:scan time(4)" << timer.elapsed();

   // Reduce the run list to only those with associated models
   rlabels.clear();
   for ( int rr = 0; rr < runIDs.count(); rr++ )
   {
      if ( rmodKnts[ rr ] > 0 )
         rlabels << runIDs[ rr ];
   }
DbgLv(1) << "ScDB:scan time(5)" << timer.elapsed();
DbgLv(1) << "ScDB:counts: runIDs" << runIDs.count() << "rlabels" << rlabels.count();
DbgLv(1) << "ScDB:scan time(9)" << timer.elapsed();
}


// Scan local disk for edit sets
void US_SelectRunid::scan_local_runs( void )
{
QTime timer;
timer.start();
   QString     mdir    = US_Settings::dataDir() + "/models";
   QStringList mfilt( "M*.xml" );
   QStringList f_names = QDir( mdir )
                         .entryList( mfilt, QDir::Files, QDir::Name );

   for ( int ii = 0; ii < f_names.size(); ii++ )
   {  // Examine each model file
      QString fname( mdir + "/" + f_names[ ii ] );
      QFile m_file( fname );

      if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
         continue;

      QXmlStreamReader xml( &m_file );

      while( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "model" )
            {  // Get the description and GUID of model and test it
               QXmlStreamAttributes attr = xml.attributes();
               QString mdesc = attr.value( "description" ).toString();
               QString ddesc = attr.value( "dataDescrip" ).toString();
               QString mGUID = attr.value( "modelGUID"   ).toString();
               int     kk    = mdesc.lastIndexOf( ".model" );
                       mdesc = ( kk < 1 ) ? mdesc : mdesc.left( kk );
               QString runid = mdesc.section( ".", 0, -3 );
               // Skip the model if it has no valid runID part
               if ( runid.isEmpty() || runid.length() < 2 )  continue;

               // Save run ID and model string of RunID+GUID+Description
               QString odesc  = runid + "\t" + mGUID + "\t" + mdesc
                                      + "\t" + ddesc;
               mRunIDs << runid;
               wDescrs << odesc;
if((dbg_level>0) && (!mdesc.contains("-MC_")||mdesc.contains("_mc0001")))
 DbgLv(1) << "ScLo: odesc" << odesc;
            }

            if ( xml.name() == "analyte" )
               break;
         }
      }
   }
DbgLv(1) << "ScLo:scan time(1)" << timer.elapsed();

   QString     rdir    = US_Settings::resultDir();
   QStringList aucdirs = QDir( rdir ).entryList( 
         QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
DbgLv(1) << "ScLo:rdir" << rdir << "aucdir count" << aucdirs.count();
   
   QStringList aucfilt( "*.auc" );
   QStringList datfilt;
   count_allr = 0;
   count_list = 0;
   count_seld = 0;
   
   for ( int ii = 0; ii < aucdirs.count(); ii++ )
   {  // Examine all the AUC files that exist locally
      QString     subdir   = rdir + "/" + aucdirs.at( ii );
      QStringList aucfiles = QDir( subdir ).entryList( 
            aucfilt, QDir::Files, QDir::Name );
DbgLv(1) << "ScLo:  subdir" << subdir << "aucfiles count" << aucfiles.count();

      if ( aucfiles.count() < 1 )
         continue;

      QString aucfbase  = aucfiles.at( 0 );
      QString runID     = aucfbase.section( ".",  0, -6 );
      count_allr++;             // Bump the count of all runIDs examined

      if ( mRunIDs.contains( runID ) )
      {  // If this runID is represented for models, it is selectable
         count_list++;          // Bump the count of runIDs to list
         rlabels << runID;      // Save selectable runID
      }
DbgLv(1) << "ScLo:    count_allr" << count_allr << "count_list" << count_list
 << "   runID" << runID;
   }
DbgLv(1) << "ScLo:rlabels count" << count_list << rlabels.count();
DbgLv(1) << "ScLo:scan time(9)" << timer.elapsed();
}


// Investigator button clicked:  get investigator from dialog
void US_SelectRunid::get_person()
{
   int invID     = US_Settings::us_inv_ID();
   US_Investigator* dialog = new US_Investigator( true, invID );

   connect( dialog, SIGNAL( investigator_accepted( int ) ),
                    SLOT(   update_person(         int ) ) );

   dialog->exec();
}

// Slot to handle accept in investigator dialog
void US_SelectRunid::update_person( int ID )
{
   QString number = ( ID > 0 ) ? QString::number( ID ) + ": " : "";
   le_invest->setText( number + US_Settings::us_inv_name() );

   list_data();
}

// Slot to update disk/db selection
void US_SelectRunid::update_disk_db( bool isDB )
{
   emit changed( isDB );

   sel_db     = isDB;
   list_data();

   pb_invest->setEnabled( isDB );
   setWindowTitle( tr( "Select Run ID(s) for Discrete Distributions (%1)" )
         .arg( sel_db ? "DB" : "Local" ) );
}

// Slot to record a change in list item selection
void US_SelectRunid::selectionChanged()
{
   count_seld = lw_data->selectedItems().size();
   count_list = rlabels.count();

   te_status->setText(
      tr( "%1 scanned run IDs were used to derive the list. Of these,\n"
          "%2 have associated distributions (models), and\n"
          "%3 %4 currently selected for combination plot components." )
      .arg( count_allr ).arg( count_list ).arg( count_seld )
      .arg( count_seld != 1 ? tr( "runs are" ) : tr( "run is" ) ) );
}

// Scan database for models associated with run sets
void US_SelectRunid::scan_dbase_models()
{
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" )
         + db.lastError() );
      return;
   }

   QStringList        mmIDs;        // Model modelIDs
   QStringList        mmGUIDs;      // Model modelGUIDs
   QStringList        meIDs;        // Model editIDs;
   QVector< QString > mmDescs;      // Model descriptions
   QMap< QString, QString > ddmap;  // editID,dataDescr map

   QStringList query;
   QString     invID  = QString::number( US_Settings::us_inv_ID() );
   int          ntmodel = 0;
   int          nmodel  = 0;

QTime timer;
timer.start();
   // Accumulate model information for edits present
   for ( int ee = 0; ee < edtIDs.count(); ee++ )
   {
      QString edtid    = edtIDs [ ee ];
      QString runid    = erunIDs[ ee ];
      if ( ! runIDs.contains( runid ) )
         continue;

      query.clear();
      query << "get_model_desc_by_editID" << invID << edtid;
      db.query( query );

      while( db.next() )
      {
         QString mdlid    = db.value( 0 ).toString();
         QString mdlGid   = db.value( 1 ).toString();
         QString mdesc    = db.value( 2 ).toString();
         int     kk       = mdesc.lastIndexOf( ".model" );
         mdesc            = ( kk < 1 ) ? mdesc : mdesc.left( kk );
         mmIDs   << mdlid;
         mmGUIDs << mdlGid;
         meIDs   << edtid;
         mmDescs << mdesc;
         nmodel++;
      }
   }
DbgLv(1) << "ScMd:scan time(1)" << timer.elapsed();

   query.clear();
   query << "count_models" << invID;
   ntmodel  = db.functionQuery( query );
DbgLv(1) << "ScMd: ntmodel" << ntmodel << "nmodel" << nmodel;
DbgLv(1) << "ScMd:scan time(2)" << timer.elapsed();
int m=nmodel-1;
if ( m>1 ) {
DbgLv(1) << "ScMd: 0: id,gid,eid,desc" << mmIDs[0] << mmGUIDs[0] << meIDs[0] << mmDescs[0];
DbgLv(1) << "ScMd: m: id,gid,eid,desc" << mmIDs[m] << mmGUIDs[m] << meIDs[m] << mmDescs[m]; }

   // Scan all saved models from the end back, saving any
   //   cell description by edit ID
   for ( int mm = nmodel - 1; mm >=0; mm-- )
   {
      QString medtid   = meIDs[ mm ];

      if ( ddmap.contains( medtid ) )  continue;   // Skip if already mapped

      // Not yet mapped, so find any cell description in the model XML
      QString mdlid    = mmIDs[ mm ];
      query.clear();
      query << "get_model_info" << mdlid;
      db.query( query );
      db.next();
      QString mxml     = db.value( 2 ).toString();
      int     kk       = mxml.indexOf( "dataDescrip=" );

      if ( kk > 0 )
      {  // We have found the data description, so map it
         QString ddesc    = mxml.mid( kk + 13 );
         kk               = ddesc.indexOf( "\"" );
         ddesc            = ddesc.left( kk );
         ddmap[ medtid ]  = ddesc;
      }
   }
DbgLv(1) << "ScMd:scan time(3)" << timer.elapsed();

   // Do one more pass through all the models, completing
   //  the model descriptions
   for ( int mm = 0; mm < nmodel; mm++ )
   {
      QString mID    = mmIDs  [ mm ];
      QString mGUID  = mmGUIDs[ mm ];
      QString mdesc  = mmDescs[ mm ];
      QString meID   = meIDs  [ mm ];
      QString ddesc  = ddmap.contains( meID ) ? ddmap[ meID ] : "";
      QString runid  = mdesc.section( ".", 0, -3 );
      QString odesc  = runid + "\t" + mGUID + "\t" + mdesc + "\t" + ddesc;
      wDescrs << odesc;
   }

DbgLv(1) << "ScMd:scan time(9)" << timer.elapsed();
}

// Scan local disk for models associated with run sets
void US_SelectRunid::scan_local_models()
{
   QStringList aDescrs = wDescrs;
   wDescrs.clear();

   // Model scan was already done for local. All we need to do
   //  now is limit the descriptions to runIDs selected.
   for ( int mm = 0; mm < aDescrs.count(); mm++ )
   {
      QString mdesc   = aDescrs[ mm ];
      QString runid   = mdesc.section( "\t", 0, 0 );

      if ( runIDs.contains( runid ) )
         wDescrs << mdesc;
   }
DbgLv(1) << "ScMl:counts: aDescrs" << aDescrs.count() << "wDescrs" << wDescrs.count();
}

