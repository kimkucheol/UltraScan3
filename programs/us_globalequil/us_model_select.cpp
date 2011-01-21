//! \file us_model_select.cpp

#include "us_model_select.h"
#include "us_model_adpars.h"
#include "us_long_messagebox.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_sleep.h"

// Main constructor with model index, models list, additional parameters list
US_ModelSelect::US_ModelSelect(
      int&             a_modelx,
      QStringList&     a_models,
      QList< double >& a_udpars )
 : US_WidgetsDialog( 0, 0 ),
   smodelx    ( a_modelx ),
   models     ( a_models ),
   aud_pars   ( a_udpars )
{
   setAttribute  ( Qt::WA_DeleteOnClose );
   setWindowTitle( tr( "Model Selection - UltraScan Analysis" ) );
   setPalette    ( US_GuiSettings::frameColor() );
   setMinimumSize( 400, 300 );

   // Main layout
   QVBoxLayout* main = new QVBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Build the list of possible model selections
   models.clear();
   models << "1-Component, Ideal"
          << "2-Component, Ideal, Noninteracting"
          << "3-Component, Ideal, Noninteracting"
          << "Fixed Molecular Weight Distribution"
          << "Monomer-Dimer Equilibrium"
          << "Monomer-Trimer Equilibrium"
          << "Monomer-Tetramer Equilibrium"
          << "Monomer-Pentamer Equilibrium"
          << "Monomer-Hexamer Equilibrium"
          << "Monomer-Heptamer Equilibrium"
          << "User-Defined Monomer-Nmer Equilibrium"
          << "Monomer-Dimer-Trimer Equilibrium"
          << "Monomer-Dimer-Tetramer Equilibrium"
          << "User-Defined Monomer - N-mer - M-mer Equilibrium"
          << "2-Component Hetero-Association: A + B <=> AB"
          << "User-Defined self/Hetero-Association: A + B <=> AB, nA <=> An"
          << "User-Defined Monomer-Nmer, some monomer is incompetent"
          << "User-Defined Monomer-Nmer, some Nmer is incompetent"
          << "User-Defined irreversible Monomer-Nmer"
          << "User-Defined Monomer-Nmer plus contaminant";

   // Main banner
   QLabel* lb_mbanner  = us_banner( tr( "Please Select a Model:" ) );
   main->addWidget( lb_mbanner );

   // Models list widget
   lw_models           = us_listwidget();

   for ( int ii = 0; ii < models.size(); ii++ )
      lw_models->addItem( models.at( ii ) );

   main->addWidget( lw_models );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_select = us_pushbutton( tr( "Select Model" ) );
   connect( pb_select, SIGNAL( clicked() ), SLOT( selected() ) );
   buttons->addWidget( pb_select );

   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   connect( pb_help,   SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancelled() ) );
   buttons->addWidget( pb_cancel );

   main->addLayout( buttons );

   resize( 480, 440 );
}

// Cancel button:  no models returned
void US_ModelSelect::cancelled()
{
   smodelx = -1;
   aud_pars.clear();

   reject();
   close();
}

// Select Model button:  set up to return data information
void US_ModelSelect::selected()
{
   aud_pars.clear();
   smodelx = lw_models->selectedItems().size() > 0 ?
      lw_models->currentRow() : -1;
   
   if ( smodelx == (-1) )
   {
      QMessageBox::information( this,
            tr( "No Data Selected" ),
            tr( "You have not selected any data.\nSelect Model or Cancel" ) );
      return;
   }

   switch( smodelx )
   {
      case 0:
      case 1:
      case 2:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 11:
      case 12:
      case 14:
         model_pars_0();    // get no additional parameters
         break;
      case 3:
         model_pars_4();    // get 4 additional parameters
         break;
      case 10:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
         model_pars_1();    // get 1 additional parameter
         break;
      case 13:
      default:
         model_pars_2();    // get 2 additional parameters
         break;
   }
         
   accept();        // Signal that selection was accepted
   close();
}

// Model select with no additional parameters
void US_ModelSelect::model_pars_0()
{
   function_dialog();
}

// Model select with 1 additional parameter
void US_ModelSelect::model_pars_1()
{
   function_dialog();

   US_ModelAdPars* adiag = new US_ModelAdPars( 1, aud_pars );
   adiag->exec();
}

// Model select with 2 additional parameters
void US_ModelSelect::model_pars_2()
{
   function_dialog();

   US_ModelAdPars* adiag = new US_ModelAdPars( 2, aud_pars );
   adiag->exec();
}

// Model select with 4 additional parameters
void US_ModelSelect::model_pars_4()
{
   function_dialog();

   US_ModelAdPars* adiag = new US_ModelAdPars( 4, aud_pars );
   adiag->exec();
}

// Show model functions message dialog
void US_ModelSelect::function_dialog()
{
   QString wtitle = models.at( smodelx );  // window title from list entry
   QStringList comps;
   comps.clear();
   int notef = 1;

   // Get equation string and list of equation elements
   QString fequ = function_equation( comps );

   // Use elements to generate terms-definition string
   QString fcmp = function_components( comps, notef );

   // Initial message includes equation and terms definitions
   QString msg = tr( "Fitting to Function:\n\n" ) + fequ + fcmp; 

   // Add any notes for terms
   msg = msg + tr( "\n* indicates that this parameter can be floated.\n" );

   if ( notef > 1 )
   {
      msg = msg +
         tr( "\n** Equilibrium Constants are calculated in log of"
             " molar units;\n conversion to concentration is performed in"
             " the Model Control Windows.\n" );

      if ( notef > 2 )
         msg = msg +
            tr( "\n*** vbar_AB = ( vbar_A * M_A + vbar_B * M_B )"
                " / ( M_A + M_B )\n" );
   }

   // Show a long message dialog with model type explanation
   US_LongMessageBox* textbox = new US_LongMessageBox( wtitle, msg );
   textbox->exec();

   return;
}

// Compose string showing function equation for a model selection
QString US_ModelSelect::function_equation( QStringList& comps )
{
   QString msg = QString( "C(X) = " );
   comps.clear();
   
   switch( smodelx )
   {  // Create unique function string and components (terms) list, ea. type
      case 0:
         msg = msg + "exp[ ln(A) + M * omega^2 * (1 - vbar * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "X" << "Xr" << "A" << "M" << "E" << "D" << "R" << "T"
               << "B";
         break;
      case 1:
         msg = msg + "exp[ ln(A[1]) + M[1] * omega^2 * (1 - vbar[1] * D)"
            + " * (X^2 - Xr^2) / (2 * R * T) ]\n"
            + "  + exp[ (ln(A[2]) * omega^2 * (1 - vbar[2] * D) * "
            + "(X^2 - Xr ^2) / (2 * R * T) ] + B";
         comps << "X" << "Xr" << "A" << "M" << "E" << "D" << "R" << "T"
               << "B";
         break;
      case 2:
         msg = msg + "exp[ ln(A[1]) + M[1] * omega^2 * (1 - vbar[1] * D)"
            + " * (X^2 - Xr^2) / (2 * R * T) ]\n"
            + "  + exp[ (ln(A[2]) + M[2] * omega^2 * (1 - vbar[2] * D) * "
            + "(X^2 - Xr^2) / (2 * R * T) ]\n"
            + "  + exp[ (ln(A[3]) + M[3] * omega^2 * (1 - vbar[3] * D) * "
            + "(X^2 - Xr^2) / (2 * R * T) ] + B";
         comps << "X" << "Xr" << "A" << "M" << "E" << "D" << "R" << "T"
               << "B";
         break;
      case 3:
         msg = msg + "A[i] * SUM[ (exp[(M[i] * omega^2 * (1 - vbar[1] * D)"
            + " * (X^2 - Xr^2) / (2 * R * T) ] + B\n";
         comps << "X" << "Xr" << "Ai" << "Mi" << "D" << "R" << "T" << "B";
         break;
      case 4:
         msg = msg + "exp[ (ln(A) + M * omega^2 * (1 - vbar * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (2 * (ln(A) + ln(2/(E*L)) + ln(K1,2) + 2 * M * omega^2"
            + " * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "X" << "Xr" << "A" << "E" << "L" << "K1" << "M" << "D"
               << "R" << "T" << "B";
         break;
      case 5:
         msg = msg + "exp[ (ln(A) + M * omega^2 * (1 - vbar * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (3 * (ln(A) + ln(3/(E*L)^2) + ln(K1,3) + 3 * M *"
            + " omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "X" << "Xr" << "A" << "E" << "L" << "K1" << "M" << "D"
               << "R" << "T" << "B";
         break;
      case 6:
         msg = msg + "exp[ (ln(A) + M * omega^2 * (1 - vbar * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (4 * (ln(A) + ln(4/(E*L)^3) + ln(K1,4) + 4 * M *"
            + " omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "X" << "Xr" << "A" << "E" << "L" << "K1" << "M" << "D"
               << "R" << "T" << "B";
         break;
      case 7:
         msg = msg + "exp[ (ln(A) + M * omega^2 * (1 - vbar * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (5 * (ln(A) + ln(5/(E*L)^4) + ln(K1,5) + 5 * M *"
            + " omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "X" << "Xr" << "A" << "E" << "L" << "K1" << "M" << "D"
               << "R" << "T" << "B";
         break;
      case 8:
         msg = msg + "exp[ (ln(A) + M * omega^2 * (1 - vbar * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (6 * (ln(A) + ln(6/(E*L)^5) + ln(K1,6) + 6 * M *"
            + " omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "X" << "Xr" << "A" << "E" << "L" << "K1" << "M" << "D"
               << "R" << "T" << "B";
         break;
      case 9:
         msg = msg + "exp[ (ln(A) + M * omega^2 * (1 - vbar * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (7 * (ln(A) + ln(7/(E*L)^6) + ln(K1,7) + 7 * M *"
            + " omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "X" << "Xr" << "A" << "E" << "L" << "K1" << "M" << "D"
               << "R" << "T" << "B";
         break;
      case 10:
         msg = msg + "exp[ (ln(A) + M * omega^2 * (1 - vbar * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (N * ln(A) + ln(N/(E*L)^(N-1)) + ln(K1,N) + N * M *"
            + " omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "N" << "X" << "Xr" << "A" << "E" << "L" << "K1" << "M" << "D"
               << "R" << "T" << "B";
         break;
      case 11:
         msg = msg + "exp[ (ln(A) + M * omega^2 * (1 - vbar * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (2 * (ln(A) + ln(2/(E*L)) + ln(K1,2) + 2 * M * omega^2"
            + " * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (3 * (ln(A) + ln(3/(E*L)^2) + ln(K1,3) + 3 * M *"
            + " omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "X" << "Xr" << "A" << "E" << "L" << "K1" << "M" << "D"
               << "R" << "T" << "B";
         break;
      case 12:
         msg = msg + "exp[ (ln(A) + M * omega^2 * (1 - vbar * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (2 * (ln(A) + ln(2/(E*L)) + ln(K1,2) + 2 * M * omega^2"
            + " * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (4 * (ln(A) + ln(4/(E*L)^3) + ln(K1,4) + 4 * M *"
            + " omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "X" << "Xr" << "A" << "E" << "L" << "K1" << "M" << "D"
               << "R" << "T" << "B";
         break;
      case 13:
         msg = msg + "exp[ (ln(A) + M * omega^2 * (1 - vbar * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (N1 * (ln(A) + ln(N1/(E*L)^(N1 - 1)) + ln(K1,N1) + N1 *"
            + " M * omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (N2 * (ln(A) + ln(N2/(E*L)^(N1 - 1)) + ln(K1,N2) + N2 *"
            + " M * omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "X" << "Xr" << "A" << "E" << "L" << "N1" << "N2" << "K1"
               << "M" << "D" << "R" << "T" << "B";
         break;
      case 14:
         msg = msg + "exp[ (ln(A) + M_A * omega^2 * (1 - vbar_A * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (ln(B) + M_B * omega^2 * (1 - vbar * D) *"
            + " (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (ln(A) + ln(B) + ln(K_AB) + ln(E_AB/(E_A * E_B * L))"
            + " + (M_A + M_B) * omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) /"
            + " (2 * R * T) ] + B";
         comps << "X"  << "Xr" << "A" << "B" << "E" << "L" << "KC" << "MA"
               << "MB" << "MC" << "D" << "R" << "T" << "B";
         break;
      case 15:
         msg = msg + "exp[ (ln(A) + M_A * omega^2 * (1 - vbar_A * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (ln(B) + M_B * omega^2 * (1 - vbar * D) *"
            + " (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (ln(A) + ln(B) + ln(K_AB) + ln(E_AB/(E_A * E_B * L))"
            + " + (M_A + M_B) * omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) /"
            + " (2 * R * T) ]\n"
            + "  + exp[ (N *ln(A) + ln(N/(E*L)^(N-1)) + ln(K1,N) + N * M_A"
            + " * omega^2 * (1 - vbar_A * D) * (X^2 - Xr^2)) /"
            + " (2 * R * T) ] + B";
         comps << "X"  << "Xr" << "A" << "B" << "E" << "L" << "KC" << "KN"
               << "MA" << "MB" << "MC" << "VA" << "VB" << "VC"
               << "D" << "R" << "T" << "B";
         break;
      case 16:
         msg = msg + "exp[ (ln(A1) + M * omega^2 * (1 - vbar_A * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (N * ln(A1) + ln(N/(E * L)^(N - 1) + ln(K1,N) +"
            + " N * M * omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) /"
            + " (2 * R * T) ]\n"
            + "  + exp[ (ln(A2) + M * omega^2 * (1 - vbar_A * D) *"
            + " (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "N"  << "X"  << "Xr" << "A" << "E" << "L" << "K1"
               << "M"  << "A1" << "A2" << "D" << "R" << "T" << "B";
         break;
      case 17:
         msg = msg + "exp[ (ln(A1) + M * omega^2 * (1 - vbar_A * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (N * ln(A1) + ln(N/(E * L)^(N - 1) + ln(K1,N) +"
            + " N * M * omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) /"
            + " (2 * R * T) ]\n"
            + "  + exp[ (ln(A2) + N * M * omega^2 * (1 - vbar_A * D) *"
            + " (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "N"  << "X"  << "Xr" << "A" << "E" << "L" << "K1"
               << "M"  << "A1" << "A2" << "D" << "R" << "T" << "B";
         break;
      case 18:
         msg = msg + "exp[ (ln(A1) + M * omega^2 * (1 - vbar_A * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (N * ln(A1) + ln(N/(E * L)^(N - 1) + ln(K1,N) +"
            + " N * M * omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) /"
            + " (2 * R * T) ]\n"
            + "  + exp[ (ln(A2) + M * omega^2 * (1 - vbar_A * D) *"
            + " (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (ln(A3) + N * M * omega^2 * (1 - vbar_A * D) *"
            + " (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "N"  << "X"  << "Xr" << "A"  << "E" << "L" << "K1"
               << "M"  << "A1" << "A2" << "A3" << "D" << "R" << "T" << "B";
         break;
      case 19:
         msg = msg + "exp[ (ln(A1) + M1 * omega^2 * (1 - vbar_A * D)"
            + " * (X^2 - Xr^2)) / (2 * R * T) ]\n"
            + "  + exp[ (N * ln(A1) + ln(N/(E * L)^(N - 1) + ln(K1,N) +"
            + " N * M * omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) /"
            + " (2 * R * T) ]\n"
            + "  + exp[ (ln(A2) + M2 * omega^2 * (1 - vbar_A * D) *"
            + " (X^2 - Xr^2)) / (2 * R * T) ] + B";
         comps << "N"  << "X"  << "Xr" << "A"  << "E" << "L" << "K1"
               << "M1" << "M2" << "A1" << "A2" << "D" << "R" << "T" << "B";
         break;
      default:
         break;
   }

   return msg;
}

// Compose string showing function equation components for a model selection
QString US_ModelSelect::function_components( QStringList& comps, int& notef )
{
   notef      = 1;
   QString ms = tr( "\n\nwhere:\n" );

   // Add a component explanation line for each listed component
   if ( comps.contains( "X"  ) )  ms = ms + "\t"
      + tr( "X  = Radius\n" );
   if ( comps.contains( "Xr" ) )  ms = ms + "\t"
      + tr( "Xr = Reference Radius\n" );
   if ( comps.contains( "A"  ) )  ms = ms + "\t"
      + tr( "A  = Amplitude of component A *\n" );
   if ( comps.contains( "B"  ) )  ms = ms + "\t"
      + tr( "B  = Amplitude of component B *\n" );
   if ( comps.contains( "M"  ) )  ms = ms + "\t"
      + tr( "M  = Molecular Weight *\n" );
   if ( comps.contains( "E"  ) )  ms = ms + "\t"
      + tr( "E  = Extinction Coefficient\n" );
   if ( comps.contains( "L"  ) )  ms = ms + "\t"
      + tr( "L  = Pathlength\n" );
   if ( comps.contains( "Ai" ) )  ms = ms + "\t"
      + tr( "A[i] = Amplitude of component \"i\" *\n" );
   if ( comps.contains( "Mi" ) )  ms = ms + "\t"
      + tr( "M[i] = Molecular Weight of component \"i\" *\n" );
   if ( comps.contains( "N1" ) )  ms = ms + "\t"
      + tr( "N1 = Stoichiometry of first Association\n" );
   if ( comps.contains( "N2" ) )  ms = ms + "\t"
      + tr( "N2 = Stoichiometry of second Association\n" );
   if ( comps.contains( "K1" ) )  ms = ms + "\t"
      + tr( "K1 = Monomer-Dimer Equilibrium Constant *,**\n" );
   if ( comps.contains( "KB" ) )  ms = ms + "\t"
      + tr( "K_AB = AB Equilibrium Constant *,**\n" );
   if ( comps.contains( "KN" ) )  ms = ms + "\t"
      + tr( "K_AN = A monomer-nmer Equilibrium Constant *,**\n" );
   if ( comps.contains( "MA" ) )  ms = ms + "\t"
      + tr( "M_A  = Molecular Weight of A *\n" );
   if ( comps.contains( "MB" ) )  ms = ms + "\t"
      + tr( "M_B  = Molecular Weight of B *\n" );
   if ( comps.contains( "MC" ) )  ms = ms + "\t"
      + tr( "M_AB = Molecular Weight of AB\n" );
   if ( comps.contains( "M1" ) )  ms = ms + "\t"
      + tr( "M1 = Monomer Molecular Weight *\n" );
   if ( comps.contains( "M2" ) )  ms = ms + "\t"
      + tr( "M2 = Contaminant Molecular Weight *\n" );
   if ( comps.contains( "VA" ) )  ms = ms + "\t"
      + tr( "vbar_A  = vbar of A *\n" );
   if ( comps.contains( "VB" ) )  ms = ms + "\t"
      + tr( "vbar_B  = vbar of B *\n" );
   if ( comps.contains( "VC" ) )  ms = ms + "\t"
      + tr( "vbar_AB = vbar of AB\n" );
   if ( comps.contains( "D"  ) )  ms = ms + "\t"
      + tr( "D  = Density Coefficient\n" );
   if ( comps.contains( "R"  ) )  ms = ms + "\t"
      + tr( "R  = Gas Constant\n" );
   if ( comps.contains( "T"  ) )  ms = ms + "\t"
      + tr( "T  = Temperature\n" );
   if ( comps.contains( "A1" ) )  ms = ms + "\t"
      + tr( "A1 = Reference Concentration of monomer *\n" );
   if ( comps.contains( "A2" ) )  ms = ms + "\t"
      + tr( "A2 = Reference Concentration of incompetent monomer *\n" );
   if ( comps.contains( "A3" ) )  ms = ms + "\t"
      + tr( "A3 = Reference Concentration of incompetent N-mer *\n" );
   if ( comps.contains( "B"  ) )  ms = ms + "\t"
      + tr( "B  = Baseline *\n" );

   // Set the number of notes to add (basicly, count of trailing asterisks)
   notef = 1;

   if ( ms.contains( "**\n" ) )
      notef = ms.contains( "***\n" ) ? 3 : 2;

   return ms;
}

