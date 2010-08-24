//! \file us_lamm_astfvm.h
#ifndef US_LAMMASTFVM_H
#define US_LAMMASTFVM_H

#include <QtCore>
#include "us_extern.h"
#include "us_model.h"
#include "us_simparms.h"
#include "us_dataIO2.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"

//! \brief Module to calculate simulation data by AST Finite Volume Method
class US_EXTERN US_LammAstfvm : public QObject
{
   Q_OBJECT

   public:

      //! \brief Finite volume mesh for ASTFVM solution
      class Mesh 
      {
         public: 
   
            //! \brief Changeable non-ideal mesh
            //! \param xl    Left X (radius) value
            //! \param xr    Right X (radius) value
            //! \param Nelem Number of elements
            //! \param Opt   Mesh option (0 for uniform)
            Mesh( double, double, int, int );

            //! \brief Destroy mesh
            ~Mesh();

            //! \brief Initialize mesh
            //! \param s  Sedimentation coefficient
            //! \param D  Diffusion coefficient
            //! \param w2 Omega squared
            void InitMesh( double, double, double );

            //! \brief Refine mesh
            //! \param u0     Current concentration array
            //! \param u1     Next concentration array
            //! \param ErrTol Error tolerance
            void RefineMesh( double*, double*, double );

            int Nv;	         //!< Number of grids
            int Ne;	         //!< Number of elements
            double *x;			//!< radius coordinates of grids

         private:
            int MaxRefLev;
            int MonScale;
            double MonCutoff;
            double SmoothingWt;
            int    SmoothingCyl;

            int *Eid;			// elem id
            int *RefLev;	        // refinement level of an elem
            double *MeshDen;		// desired mesh density
            int *Mark;			// ref/unref marker

            void ComputeMeshDen_D3(double *u0, double *u1);
            void Smoothing( int n, double *y, double Wt, int Cycle);
            void Unrefine(double alpha);
            void Refine(double beta);
      };

      //! \brief Salt data for co-sedimenting
      class SaltData
      {
         public:

            //! \brief Create salt data
            //! \param fname  File name for salt data
            //! \param Moler  Concentration moler factor
            SaltData( char*, double );

            //! \brief Destroy salt data
            ~SaltData();

            //! \brief Interpolate concentrations of salt
            //! \param N     Number of elements in arrays
            //! \param x     X (radius) array
            //! \param t     Current time value
            //! \param Csalt Concentration of salt for current time
            void InterpolateCSalt( int, double*, double, double* );

            double SaltMoler;		//!< moler number of cosedimenting salt

         private:

            FILE *f_salt;
            int Nx;		// number of points in radial direction
            int Nt;		// number of points in time direction
            double *xs;	// grids in radial direction
            double t0, t1;	// time intervals in use.
            double *Cs0, *Cs1; // salt concentration for the time interval
      };

      //! \brief Create Lamm equations AST Finite Volume Method solver
      //! \param rmodel     Reference to model on which solution is based
      //! \param rsimparms  Reference to simulation parameters
      //! \param parent     Parent object (may be 0)
      US_LammAstfvm( US_Model&, US_SimulationParameters&, QObject* = 0 );

      //! \brief Destroy FVM solver
      ~US_LammAstfvm();

      //! \brief Main method to calculate FVM solution
      //! \param sim_data Reference to simulated AUC data to produce
      void calculate( US_DataIO2::RawData& );

      //! \brief Calculate solution for a model component
      //! \param compx Index to model component to use in solution pass
      void solve_component( int ); 

      //! \brief Get the non-ideal case number from model parameters
      int nonIdealCaseNo( void );

      //! \brief Set up non-ideal case type 1 (concentration-dependent)
      //! \param sigma_k Sigma constant to modify sedimentation coefficient
      //! \param delta_k Delta constant to modify diffusion coefficient
      void SetNonIdealCase_1( double, double );

      //! \brief Set up non-ideal case type 2 (co-sedimenting)
      //! \param fname   Salt data file name
      //! \param Moler   Salt moler factor
      void SetNonIdealCase_2( char*, double );

      //! \brief Set the mesh speed factor: 1.0 (moving) or 0.0 (non-moving)
      //! \param speed   Mesh speed factor of 1.0 or 0.0
      void SetMeshSpeedFactor( double );

      //! \brief Set the mesh refinement option:  1 to refine, 0 to not refine
      //! \param Opt     Mesh refinement option flag: 1/0 for yes/no.
      void SetMeshRefineOpt( int );

   signals:
      //! \brief Signal calculation start and give maximum steps
      //! \param nsteps Number of expected total calculation progress steps
      void calc_start( int nsteps );

      //! \brief Signal calculation progress, giving running step count
      //! \param istep Current progress step count
      void calc_progress( int istep );

      //! \brief Signal that calculations are complete
      void calc_done( void );

      //! \brief Signal component progress, giving running component number
      //! \param icomp Current component begun (1,...)
      void comp_progress( int icomp );


   private:

      US_Model&                 model;
      US_SimulationParameters&  simparams;

      US_AstfemMath::MfemData   af_data;

      Mesh *msh;			    // redial grid

      int NonIdealCaseNo;		    // non-ideal case number
                                 // = 0 : ideal, constant s, D
                                 // = 1 : concentration dependent
                                 // = 2 : co-sedimenting
                                 // = 3 : compressibility

      double sigma;
      double delta;    		    // constants for concentration dependent
                                 // non-ideal case
                                 // s = s_0/(1+sigma*C), D=D_0/(1+delta*C)

      SaltData *saltdata;		    // data handle for cosedimenting

      double MeshSpeedFactor;          // = 1: mesh following sedimentation
                                     // = 0: fixed mesh in each time step

      int MeshRefineOpt;               // = 1: perform mesh local refinement
                                    // = 0: no mesh refinement

      int    comp_x;             // current component index

      double param_m, param_b;	// m, b of cell
      double param_s, param_D; 	// base s, D values
      double param_w2;		// rpm, w2=(rpm*pi/30)^2

      //! \brief Lamm equation step for sedimentation difference - predict
      void LammStepSedDiff_P( double, double, int, double*, double*, double* );

      //! \brief Lamm equation step for sedimentation difference - calculate
      void LammStepSedDiff_C( double, double, int, double*, double*, 
                              int, double*, double*, double* );

      //! \brief Project piecewise quadratic solution onto mesh
      void ProjectQ( int, double*, double*, int, double*, double* );

      //! \brief Integrate piecewise quadratic function defined on mesh
      double IntQs( double*, double*, int, double, int, double );

      void quadInterpolate( double*, double*, int,
                            QVector< double >&, QVector< double >& );

      void LocateStar(int N0, double *x0, int Ns, double *xs,
                      int *ke, double *xi);


      void AdjustSD(double t, int Nv, double *x, double *u, 
                    double *s_adj, double *D_adj);

      void fun_phi(double x, double *y);

      void fun_dphi(double x, double *y);

      void fun_Iphi(double x, double *y);

      double IntQ(double *x, double *u, double xia, double xib);


      void LsSolver53(int m, double **A, double *b, double *x);

      void load_mfem_data(  US_DataIO2::RawData&, US_AstfemMath::MfemData& );
      void store_mfem_data( US_DataIO2::RawData&, US_AstfemMath::MfemData& );
};
#endif