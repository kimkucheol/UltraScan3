//! \file us_data_process.cpp

#include "us_data_process.h"
#include "us_data_model.h"
#include "us_sync_exper.h"
#include "us_util.h"
#include "us_settings.h"

// class to process operatations on data:  upload/download/remove
US_DataProcess::US_DataProcess( US_DataModel* dmodel, QWidget* parent /*=0*/ )
{
   parentw          = parent;
   da_model         = dmodel;
   QString investig = da_model->invtext();
   QString invID    = investig.section( ":", 0, 0 );
   db               = da_model->dbase();

   syncExper        = new US_SyncExperiment( db, invID, parent );
}

// perform a record upload to the database from local disk
int US_DataProcess::record_upload( int irow )
{
qDebug() << "REC_ULD: row" << irow+1;
   int stat = 0;

   cdesc            = da_model->row_datadesc( irow );

   QStringList query;
   QString filepath = cdesc.filename;
   QString pathdir  = filepath.section( "/",  0, -2 );
   QString filename = filepath.section( "/", -1, -1 );
   int     idData   = cdesc.recordID;

   if      ( cdesc.recType == 1 )
   {  // upload a Raw record
      //US_DataIO2::RawData        rdata;

      QString runID    = filename.section( ".",  0,  0 );
      QString tripl    = filename.section( ".", -4, -2 )
                         .replace( ".", " / " );
qDebug() << "REC_ULD:RAW: runID" << runID << "  tripl" << tripl;

      //US_DataIO2::readRawData( filepath, rdata );

      stat = syncExper->synchronize( cdesc );
qDebug() << "REC_ULD:RAW: parentGUID" << cdesc.parentGUID;
      QString expGUID  = cdesc.parentGUID;

      if ( stat == 0 )
      {
         stat   = db->writeBlobToDB( filepath,
                                     QString( "upload_aucData" ),
                                     idData );
         errMsg = tr( "Raw upload writeBlobToDB() status %1" ).arg( stat );
         stat   = ( stat == 0 ) ? 0 : 3041;

      }
   }

   else if ( cdesc.recType == 2 )
   {  // upload an EditedData record
      QString runID    = filename.section( ".",  0,  0 );
      QString label    = filename.section( ".",  0,  1 );
      QString editGUID = cdesc.dataGUID;
      QString rawGUID  = cdesc.parentGUID;
      QString editID   = QString::number( idData );

      query.clear();
      query << "get_rawDataID_from_GUID" << rawGUID;
      db->query( query );

      if ( ( stat = db->lastErrno() ) != US_DB2::OK )
      {
         errMsg = tr( "Raw DB record for Edit could not be accessed: %1" )
                  .arg( stat );
         stat   = 3052;
      }

      else
      {
         db->next();
         qDebug() << "editUpld: rawGUID" << rawGUID;

         QString rawDataID = db->value( 0 ).toString();

         qDebug() << "editUpld: rawDataID" << rawDataID;
         query.clear();

         if ( idData < 0 )
         {
            query << "new_editedData" << rawDataID << editGUID << runID
               << filename << cdesc.description;
            db->query( query );
            idData   = db->lastInsertID();
            editID   = QString::number( idData );
            qDebug() << "editUpld: NEW idData" << idData << editID;
         }

         query << "update_editedData" << editID << rawDataID << editGUID
            << label << filename << cdesc.description;
         db->query( query );
         qDebug() << "editUpld: label" << label;

         stat = db->writeBlobToDB( filepath,
                                   QString( "upload_editData" ),
                                   idData );
         errMsg = tr( "writeBlob Edited stat %1, idData %2" )
            .arg( stat ).arg( idData );
         stat = ( stat == 0 ) ? 0 : 3042;
      }
   }

   else if ( cdesc.recType == 3 )
   {  // upload a Model record
      US_Model model;
      filepath = get_model_filename( cdesc.dataGUID );

      model.load( filepath );         // load model from local disk
      model.update_coefficients();    // fill in any missing coefficients
      stat   = model.write( db );     // store model to database
      errMsg = tr( "model write to DB stat %1" ).arg( stat );
      stat   = ( stat == 0 ) ? 0 : 3043;
   }

   else if ( cdesc.recType == 4 )
   {  // upload a Noise record
      US_Noise noise;
qDebug() << "NOISE: (1) filepath" << filepath;
      filepath = get_noise_filename( cdesc.dataGUID );
qDebug() << "NOISE: (2) filepath" << filepath;

      noise.load( filepath );
      stat   = noise.write( db );
      errMsg = tr( "noise write to DB stat %1" ).arg( stat );
      stat   = ( stat == 0 ) ? 0 : 3044;
   }

   else
   {  // *ERROR*:  invalid type
      errMsg = tr( "upload attempt with type=%1" ).arg( cdesc.recType );
      stat   = 3045;
   }

   return stat;
}

// perform a record download from the database to local disk
int US_DataProcess::record_download( int irow )
{
   int stat = 0;
   QStringList query;

   cdesc            = da_model->row_datadesc( irow );

   int idData       = cdesc.recordID;
   QString dataID   = QString::number( idData );
   QString filepath = cdesc.filename;
   QString dataGUID = cdesc.dataGUID;
   QString filename = filepath.section( "/", -1, -1 );
   QString runID    = filename.section( ".",  0,  0 );

   filepath         = ( filepath == filename ) ?
                      US_Settings::resultDir() + "/" + runID + "/" + filename :
                      filepath;

   if      ( cdesc.recType == 1 )
   {  // download a Raw record
      
      stat   = db->readBlobFromDB( filepath, "download_aucData", idData );
      if ( stat != 0 )
         errMsg = tr( "Raw download status %1" ).arg( stat )
                  + "\n " + filepath;
   }

   else if ( cdesc.recType == 2 )
   {  // download an EditedData record
      stat   = db->readBlobFromDB( filepath, "download_editData", idData );
      if ( stat != 0 )
         errMsg = tr( "Edited download status %1" ).arg( stat )
                  + "\n " + filepath;
   }

   else if ( cdesc.recType == 3 )
   {  // download a Model record
      US_Model model;

      filepath = get_model_filename( dataGUID );
      stat     = model.load( dataID, db );

      if ( stat == US_DB2::OK )
      {
         model.update_coefficients();

         stat = model.write( filepath );

         if ( stat != US_DB2::OK )
         {
            errMsg = tr( "Model write for download, status %1" ).arg( stat );
            stat   = 3023;  // download write error
         }

         else
            cdesc.filename = filepath;
      }

      else
      {
         errMsg = tr( "Model load for download, status %1" ).arg( stat );
         stat   = 3033;     // download read error
      }
   }

   else if ( cdesc.recType == 4 )
   {  // download a Noise record
      US_Noise noise;

      filepath = get_noise_filename( dataGUID );
      stat     = noise.load( dataID, db );

      if ( stat == US_DB2::OK )
      {
         stat     = noise.write( filepath );

         if ( stat != US_DB2::OK )
         {
            errMsg = tr( "Noise write for download, status %1" ).arg( stat );
            stat   = 3024;  // download write error
         }

         else
            cdesc.filename = filepath;
      }

      else
      {
         errMsg = tr( "Noise load for download, status %1" ).arg( stat );
         stat   = 3034;     // download read error
      }
   }

   else
   {  // *ERROR*:  invalid type
      errMsg = tr( "download attempt with type=%1" ).arg( cdesc.recType );
      stat   = 3015;
   }

   return stat;
}
// perform a record remove from the database
int US_DataProcess::record_remove_db( int irow )
{
   int stat = 0;
   QStringList query;

   cdesc            = da_model->row_datadesc( irow );
   QString dataID   = QString::number( cdesc.recordID );

   if      ( cdesc.recType == 1 )
   {  // remove a Raw record
      query.clear();
      query << "delete_rawData" << dataID;

      if ( ( stat = db->statusQuery( query ) ) != 0 )
      {
         errMsg = tr( "delete_rawData status=%1" ).arg( stat );
         stat   = 2011;
      }
   }

   else if ( cdesc.recType == 2 )
   {  // remove an EditedData record
      query.clear();
      query << "delete_editedData" << dataID;

      if ( ( stat = db->statusQuery( query ) ) != 0 )
      {
         errMsg = tr( "delete_editedData status=%1" ).arg( stat );
         stat   = 2012;
      }
   }

   else if ( cdesc.recType == 3 )
   {  // remove a Model record
      query.clear();
      query << "delete_model" << dataID;

      if ( ( stat = db->statusQuery( query ) ) != 0 )
      {
         errMsg = tr( "delete_model status=%1" ).arg( stat );
         stat   = 2013;
      }
   }

   else if ( cdesc.recType == 4 )
   {  // remove a Noise record
      query.clear();
      query << "delete_noise" << dataID;

      if ( ( stat = db->statusQuery( query ) ) != 0 )
      {
         errMsg = tr( "delete_noise status=%1" ).arg( stat );
         stat   = 2014;
      }
   }

   else
   {  // *ERROR*:  invalid type
      errMsg = tr( "DB remove attempt for type=%1" ).arg( cdesc.recType );
      stat   = 2015;
   }
   return stat;
}

// perform a record remove from local disk
int US_DataProcess::record_remove_local( int irow )
{
   int stat = 0;
   cdesc            = da_model->row_datadesc( irow );

   QString filepath = cdesc.filename;
   QString filename = filepath.section( "/", -1, -1 );
   QFile   file( filepath );

   if ( file.exists() )
   {  // the file to remove does exist

      if ( file.remove() )
      {  // the remove was successful

         if ( ( cdesc.recState & US_DataModel::REC_DB ) == 0 )
         {  // it was local-only, so now it's a dummy
            cdesc.recState  = US_DataModel::NOSTAT;
         }

         else
         {  // it was on both, so now it's db-only
            cdesc.recState &= ~US_DataModel::REC_LO;
         }
      }

      else
      {  // an error occurred in removing
         errMsg = tr( "File remove error,\n" ) + filepath;
         stat   = 1000;
      }
   }

   else
   {  // file did not exist
      errMsg = tr( "*ERROR* attempt to remove non-existent file\n" ) + filepath;
      stat   = 2000;
   }

   return stat;
}

QString US_DataProcess::get_model_filename( QString guid )
{
   QString fname = "";
   QString path;

   if ( ! US_Model::model_path( path ) )
      return fname;

   QDir f( path );
   QStringList filter( "M???????.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );
   f_names.sort();

   int         nnames  = f_names.size();
   int         newnum  = nnames + 1;
   bool        found   = false;

   for ( int ii = 0; ii < nnames; ii++ )
   {
      QString fn = f_names[ ii ];
      int     kf = fn.mid( 1, 7 ).toInt() - 1;  // expected index in file name
      fn         = path + "/" + fn;             // full path file name

      if ( kf != ii  &&  newnum > nnames )
         newnum     = kf;                       // 1st opened number slot

      QFile m_file( fn );

      if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

      QXmlStreamReader xml( &m_file );

      while ( !xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "model" )
         {
            QXmlStreamAttributes a = xml.attributes();

            if ( a.value( "modelGUID" ).toString() == guid )
            {
               fname    = fn;                   // name of file with match
               found    = true;                 // match to guid found
               break;
            }
         }

      }

      m_file.close();

      if ( found )
         break;
   }

 
   // if no guid match found, create new file name with a numeric part from
   //   the first gap in the file list sequence or from count plus one
   if ( ! found )
      fname     = path + "/M" + QString().sprintf( "%07i", newnum ) + ".xml";

   return fname;
}

QString US_DataProcess::get_noise_filename( QString guid )
{
   QString fname = "";
   QString path  = US_Settings::dataDir() + "/noises";
   QDir    dir;

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
         return fname;
   }

   QDir f( path );
   QStringList filter( "N???????.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );
   f_names.sort();

   int         nnames  = f_names.size();
   int         newnum  = nnames + 1;
   bool        found   = false;

   for ( int ii = 0; ii < nnames; ii++ )
   {
      QString fn = f_names[ ii ];
      int     kf = fn.mid( 1, 7 ).toInt() - 1;  // expected index in file name
      fn         = path + "/" + fn;             // full path file name

      if ( kf != ii  &&  newnum > nnames )
         newnum     = kf;                       // 1st opened number slot

      QFile m_file( fn );

      if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

      QXmlStreamReader xml( &m_file );

      while ( !xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "noise" )
         {
            QXmlStreamAttributes a = xml.attributes();

            if ( a.value( "noiseGUID" ).toString() == guid )
            {
               fname    = fn;                   // name of file with match
               found    = true;                 // match to guid found
               break;
            }
         }

      }

      m_file.close();

      if ( found )
         break;
   }

 
   // if no guid match found, create new file name with a numeric part from
   //   the first gap in the file list sequence or from count plus one
   if ( ! found )
      fname     = path + "/M" + QString().sprintf( "%07i", newnum ) + ".xml";

   return fname;
}

