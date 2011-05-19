#ifndef US_VHW_ENHCD_H
#define US_VHW_ENHCD_H

#include "us_dataIO2.h"
#include "us_analysis_base2.h"
#include "us_distrib_plot.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_run_details2.h"
#include "us_buffer_gui.h"
#include "qwt_plot_marker.h"

#define PA_POINTS 20     // plateau average points to left and right

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#endif

typedef struct groupinfo_s
{
   double        x1;          // x of top mouse pick
   double        y1;          // y of top mouse pick
   double        x2;          // x of bottom mouse pick
   double        y2;          // y of bottom mouse pick
   double        sed;         // average intercept sedcoeff of group
   double        percent;     // percent fraction of all divisions
   int           ndivs;       // number of division lines included in pick
   QList< int >  idivs;       // list of divisions (0 to n-1) included
} GrpInfo;

class US_vHW_Enhanced : public US_AnalysisBase2
{
   Q_OBJECT

   public:
      US_vHW_Enhanced();

   private:

      enum { NONE, START, END } groupstep;

      QLabel*       lb_tolerance;
      QLabel*       lb_division;

      QLineEdit*    le_temper;

      US_Help       showHelp;
 
      QwtCounter*   ct_tolerance;
      QwtCounter*   ct_division;

      QwtPlotCurve*  curve;
      QwtPlotCurve*  dcurve;
      US_PlotPicker* gpick;

      QPushButton*  pb_dstrpl;
      QPushButton*  pb_rsetda;
      QPushButton*  pb_selegr;

      double        boundPct;
      double        positPct;
      double        baseline;
      double        plateau;
      double        correc;
      double        C0;
      double        c0term;
      double        Swavg;
      double        omega;
      double        range;
      double        span;
      double        basecut;
      double        platcut;
      double        bdtoler;
      double        invert;
      double        divfac;
      double        cpij;
      double        sumcpij;
      double        sedc;
      double        sdiff;
      double        bdiffc;

      int           row;
      int           run_id;
      int           divsCount;
      int           scanCount;
      int           valueCount;
      int           dbg_level;

      bool          haveZone;
      bool          groupSel;

      QString       run_name;
      QString       cell;
      QString       wavelength;
      QString       workingDir;
      QString       runID;
      QString       editID;

      QList< QList< double > > cpds;       // Cpij lists, divs in scans
      QList< double >          aseds;      // all division sedcoeff values
      QList< double >          dseds;      // division sedcoeff intercepts
      QList< double >          dslos;      // division slope values
      QList< double >          dsigs;      // division sigma values
      QList< double >          dcors;      // division correlation values
      QList< int >             dpnts;      // division fitted line points
      QList< double >          bdrads;     // back-diffusion radii
      QList< double >          bdcons;     // back-diffusion concentrations
      QList< double >          groupxy;    // group select pick coordinates
      QList< GrpInfo >         groupdat;   // selected group info structures

      QVector< double >        scplats;    // scan plateaus for current triple

      US_DataIO2::EditedData*  d;
      US_DataIO2::Scan*        s;

   private slots:

      void load(        void );
      void data_plot(   void );
      void distr_plot(  void );
      void save_data(   void );
      void view_report( void );
      void sel_groups(  void );
      void div_seds(    void );
      void update_density(   double );
      void update_viscosity( double );
      void update_vbar(      double );
      void update_bdtoler(   double );
      void update_divis(     double );
      int  first_gteq( double, QVector< US_DataIO2::Reading >&, int, int );
      int  first_gteq( double, QVector< US_DataIO2::Reading >&, int );
      double sed_coeff( double, double );
      double avg_plateau(  void );
      double find_root( double );
      double sedcoeff_intercept( void );
      double back_diff_coeff( double );
      void groupClick( const QwtDoublePoint& );
      void add_group_info( void );
      void write_vhw(      void );
      void write_dis(      void );
      void write_model(    void );
      void write_report(   QTextStream& );
      void write_plot(     const QString, const QwtPlot* );
      QString text_time( double, int );
      QString text_time( double );
      QStringList last_edit_files( QStringList );
      void new_triple  ( int );
      void update      ( int );

      void help     ( void )
      { showHelp.show_help( "vhw_enhanced.html" ); };

};
#endif
