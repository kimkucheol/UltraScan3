//! \file us_dataIO.h
#ifndef US_DATAIO_H
#define US_DATAIO_H

#include <QtCore>
#include <vector>

using namespace std;

class US_DataIO
{
   public:

      class reading
      {
         public:
         union
         {
            double radius;
            double wavelength; // Will be wavelength for W data
         } d;
         double value;
         double stdDev;  // Doesn't exist for P data
      };

      /*!  This is the structure of a Beckman raw data file.  The file
       *   types are:
       *   - I intensity
       *   - P interference
       *   - R absorbance
       *   - W multi-wavelength
       *   - F fluorescence
       *
       *   For multi wavelength data, the wavelength and radius data are in
       *   opposite positions.
       */

      class beckmanRaw
      {
         public:
         QString description;
         char    type;   // I P R W F
         char    channel;
         int     cell;
         double  temperature;
         double  rpm;
         double  seconds;
         double  omega2t;
         union
         {
            double wavelength;
            double radius;      // Will be radius for W data
         } t;
         int     count;

         QList< reading > readings;
      };

      class scan
      {
         public:
         double temperature;
         double rpm;
         double seconds;
         double omega2t;
         double wavelength;
         QList< reading > values;
         QByteArray interpolated; 
      };

      class rawData
      {
         public:
         char    type[ 2 ];
         char    guid[ 16 ];
         int     cell;
         char    channel;
         QString description;
         
         QList< scan > scanData;
      };


      enum ioError { OK, CANTOPEN, BADCRC, NOT_USDATA, BADTYPE, NOTSYNC, 
                     NODATA };

      static bool readLegacyFile( const QString&, beckmanRaw& );
      static int  writeRawData  ( const QString&, rawData& );
      static int  readRawData   ( const QString&, rawData& );

   private:

      class parameters
      {
         public:
         double min_data1;
         double max_data1;
         double min_data2;
         double max_data2;
      };

      static void writeScan( QDataStream&, const scan&, ulong&, 
                             const parameters& );

      static void write( QDataStream&, const char*, int, ulong& );
      static void read ( QDataStream&,       char*, int, ulong& );
};

#endif
