/* General features of the model:
 * Extension of Yuan2 to Aluminium
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "CommonModel.h"

/* The Finite Volume Method */
#include "FVM.h"

/* The Finite Element Method */
#include "FEM.h"

/* Cement chemistry */
#include "HardenedCementChemistry.h"
#include "CementSolutionDiffusion.h"


#define TEMPERATURE   (293)

#define TITLE   "External sulfate attack of concrete (2017)" 
#define AUTHORS "Gu-Dangla"

#include "PredefinedMethods.h"

/* Nb of equations: 6/5 with/without electroneutrality */
//#define NEQ     (5)
#define NEQ     (7)
#define NEQ1    (NEQ-1)

/* Nb of (im/ex)plicit and constant terms */
#define NVE     (CementSolutionDiffusion_NbOfConcentrations + 1)
#define NVI     (63)
#define NV0     (20)

/* Equation Indexes */
#define E_S     (0)
#define E_q     (1)
#define E_Ca    (2)
#define E_K     (3)
#define E_Al    (4)
/* Comment the next line to suppress electroneutrality equation */
#define E_el    (5)
#define E_Mech  (6)

/* Primary Unknown Indexes */
#define U_C_H2SO4     (0)
#define U_PSI         (1)
#define U_ZN_Ca_S     (2)
#define U_C_K         (3)
#define U_ZN_Al_S     (4)
/* Comment the next line to suppress unknown C_OH */
#define U_C_OH        (5)
#define U_Disp        (6)


/* Compiling options */
#define NOLOG_U     1
#define LOG_U       2
#define Ln10        (2.302585093)
#define U_H2SO4     LOG_U
#define U_K         LOG_U
#define U_OH        LOG_U
#define U_Cl        NOLOG_U
#define EXPLICIT  1
#define IMPLICIT  2
#define U_PHI     IMPLICIT
#define ELECTRONEUTRALITY   IMPLICIT


/* Value of the nodal unknown (u and el must be used below) */
#define UNKNOWN(n,i)     Element_GetValueOfNodalUnknown(el,u,n,i)
#define UNKNOWN_n(n,i)   Element_GetValueOfNodalUnknown(el,u_n,n,i)


/* We define some names for nodal unknowns */
#if (U_H2SO4 == LOG_U)
  #define LogC_H2SO4(n)   (UNKNOWN(n,U_C_H2SO4))
  #define LogC_H2SO4n(n)  (UNKNOWN_n(n,U_C_H2SO4))
  #define C_H2SO4(n)      (pow(10,UNKNOWN(n,U_C_H2SO4)))
  #define C_H2SO4n(n)     (pow(10,UNKNOWN_n(n,U_C_H2SO4)))
#else
  #define C_H2SO4(n)      (UNKNOWN(n,U_C_H2SO4))
  #define C_H2SO4n(n)     (UNKNOWN_n(n,U_C_H2SO4))
  #define LogC_H2SO4(n)   (log10(UNKNOWN(n,U_C_H2SO4)))
  #define LogC_H2SO4n(n)  (log10(UNKNOWN_n(n,U_C_H2SO4)))
#endif

#define ZN_Ca_S(n)   (UNKNOWN(n,U_ZN_Ca_S))
#define ZN_Ca_Sn(n)  (UNKNOWN_n(n,U_ZN_Ca_S))

#define ZN_Si_S(n)   (UNKNOWN(n,U_ZN_Si_S))
#define ZN_Si_Sn(n)  (UNKNOWN_n(n,U_ZN_Si_S))

#define PSI(n)       (UNKNOWN(n,U_PSI))
#define PSIn(n)      (UNKNOWN_n(n,U_PSI))

#if (U_K == LOG_U)
  #define LogC_K(n)  (UNKNOWN(n,U_C_K))
  #define LogC_Kn(n) (UNKNOWN_n(n,U_C_K))
  #define C_K(n)     (pow(10,UNKNOWN(n,U_C_K)))
  #define C_Kn(n)    (pow(10,UNKNOWN_n(n,U_C_K)))
#else
  #define C_K(n)     (UNKNOWN(n,U_C_K))
  #define C_Kn(n)    (UNKNOWN_n(n,U_C_K))
  #define LogC_K(n)  (log10(UNKNOWN(n,U_C_K)))
  #define LogC_Kn(n) (log10(UNKNOWN_n(n,U_C_K)))
#endif

#if (U_Cl == LOG_U)
  #define LogC_Cl(n)   (UNKNOWN(n,U_C_Cl))
  #define LogC_Cln(n)  (UNKNOWN_n(n,U_C_Cl))
  #define C_Cl(n)      (pow(10,UNKNOWN(n,U_C_Cl)))
  #define C_Cln(n)     (pow(10,UNKNOWN_n(n,U_C_Cl)))
#else
  #define C_Cl(n)      (UNKNOWN(n,U_C_Cl))
  #define C_Cln(n)     (UNKNOWN_n(n,U_C_Cl))
  #define LogC_Cl(n)   (log10(UNKNOWN(n,U_C_Cl)))
  #define LogC_Cln(n)  (log10(UNKNOWN_n(n,U_C_Cl)))
#endif

#define ZN_Al_S(n)   (UNKNOWN(n,U_ZN_Al_S))
#define ZN_Al_Sn(n)  (UNKNOWN_n(n,U_ZN_Al_S))

#ifdef U_C_OH
  #if (U_OH == LOG_U)
    #define LogC_OH(n)   (UNKNOWN(n,U_C_OH))
    #define LogC_OHn(n)  (UNKNOWN_n(n,U_C_OH))
    #define C_OH(n)      (pow(10,UNKNOWN(n,U_C_OH)))
    #define C_OHn(n)     (pow(10,UNKNOWN_n(n,U_C_OH)))
  #else
    #define C_OH(n)      (UNKNOWN(n,U_C_OH))
    #define C_OHn(n)     (UNKNOWN_n(n,U_C_OH))
    #define LogC_OH(n)   (log10(UNKNOWN(n,U_C_OH)))
    #define LogC_OHn(n)  (log10(UNKNOWN_n(n,U_C_OH)))
  #endif
#endif

#define DISP(n)   (UNKNOWN(n,U_Disp))
#define DISPn(n)  (UNKNOWN_n(n,U_Disp))



/* Nb of nodes (el must be used below) */
#define NN     Element_GetNbOfNodes(el)


/* We define some names for implicit terms */
#define MassAndFlux(f,i,j)  ((f)[((i)*NN + (j))])

#define N_S(n)     (f[(n)])
#define N_q(n)     (f[(2+n)])
#define N_Ca(n)    (f[(4+n)])
#define N_Si(n)    (f[(6+n)])
#define N_K(n)     (f[(8+n)])
#define N_Cl(n)    (f[(10+n)])
#define N_Al(n)    (f[(12+n)])
#define W_S        (f[14])
#define W_q        (f[15])
#define W_Ca       (f[16])
#define W_Si       (f[17])
#define W_K        (f[18])
#define W_Cl       (f[19])
#define W_Al       (f[20])
#define N_CH(n)    (f[(21+n)])
#define N_CSH2(n)  (f[(23+n)])
#define N_AH3(n)   (f[(25+n)])
#define N_AFm(n)   (f[(27+n)])
#define N_AFt(n)   (f[(29+n)])
#define N_C3AH6(n) (f[(31+n)])
#define PHI(n)     (f[(33+n)])
#define PHI_C(n)   (f[(35+n)])

#ifndef U_C_OH
#define C_OH(n)     (f[(39+n)])
#define LogC_OH(n)  (log10(C_OH(n)))
#endif

#define PoreRadius(n)    (f[(41+(n))])

#define SIG              (f+43)

#define PRESSURE         (f+61)


#define N_Sn(n)    (f_n[(n)])
#define N_qn(n)    (f_n[(2+n)])
#define N_Can(n)   (f_n[(4+n)])
#define N_Sin(n)   (f_n[(6+n)])
#define N_Kn(n)    (f_n[(8+n)])
#define N_Cln(n)   (f_n[(10+n)])
#define N_Aln(n)   (f_n[(12+n)])
#define N_CHn(n)   (f_n[(21+n)])
#define N_CSH2n(n) (f_n[(23+n)])
#define N_AH3n(n)  (f_n[(25+n)])
#define N_AFmn(n)  (f_n[(27+n)])
#define N_AFtn(n)  (f_n[(29+n)])
#define N_C3AH6n(n) (f_n[(31+n)])
#define PHIn(n)    (f_n[(33+n)])
#define PHI_Cn(n)  (f_n[(35+n)])

#ifndef U_C_OH
#define C_OHn(n)    (f_n[(39+n)])
#define LogC_OHn(n) (log10(C_OHn(n)))
#endif

#define PoreRadiusn(n)    (f_n[(41+(n))])

#define SIGn              (f_n+43)

#define PRESSUREn         (f_n+61)



/* We define some names for explicit terms */
#define TORTUOSITY    (va[0])
#define CONCENTRATION (va + 1)



/* We define some names for constant terms */
#define V_Cem0(n)   (v0[(0+n)])
#define SIG0        (v0+2)



/* Shorthands of some units */
#include "InternationalSystemOfUnits.h"

#define m     (InternationalSystemOfUnits_OneMeter)
#define m3    (m*m*m)
#define dm    (0.1*m)
#define cm    (0.01*m)
#define dm3   (dm*dm*dm)
#define cm3   (cm*cm*cm)
#define Pa    (InternationalSystemOfUnits_OnePascal)
#define MPa   (1.e6*Pa)
#define GPa   (1.e9*Pa)
#define J     (Pa*m3)




/* Material properties */
//#define SATURATION_CURVE           (Element_GetCurve(el) + 4)
#define SATURATION_CURVE           (satcurve)
#define LiquidSaturationDegree(r)  (Curve_ComputeValue(SATURATION_CURVE,r))
#define dLiquidSaturationDegree(r) (Curve_ComputeDerivative(SATURATION_CURVE,r))
#define PoreEntryRadiusMax         (Curve_GetXRange(SATURATION_CURVE)[1])


/*
  Solids
  CH   = Calcium Hydroxide (Portlandite)
  CSH2 = Calcium Sulfate Dihydrate (Gypsum)
  CSH  = Calcium Silicates Hydrate
  SH   = Amorphous Silica Gel
*/

/* C-S-H Properties */
#define MOLARVOLUMEOFCSH_CURVE           (Element_GetCurve(el) + 2)
//#define MolarVolumeOfCSH(q)    (Curve_ComputeValue(MOLARVOLUMEOFCSH_CURVE,q))
#define V_CSH                  (78.*cm3)
#define V_SH                   (43.*cm3)
#define MolarVolumeOfCSH(x)    ((x)/1.7*V_CSH + (1 - (x)/1.7)*V_SH)
#define CSHSolidContent(zn_si_s)       SiliconContentInCSH(zn_si_s)


/* CH Properties */
/* Molar volume of CH solid (dm3/mole) */
#define V_CH       (33.*cm3)      /* (33.e-3) */
#define CHSolidContent(zn_ca_s)        CalciumContentInCH(zn_ca_s)


/* CSH2 Properties */
/* Molar volume of CSH2 crystal (dm3/mole) */
#define V_CSH2     (75.*cm3)      /* (75.e-3) */
#define CSH2SolidContent_kin(n,s,dt)     MAX((n + dt*r_csh2*(s - 1)),0.)
#define CSH2SolidContent(n,s,dt)         CSH2SolidContent_kin(n,s,dt)

/* AH3 Properties (Al2O6H6) */
/* Molar volume of AH3 solid (dm3/mole) */
#define V_AH3      (64.44*cm3)
#define AH3SolidContent(zn_al_s)    (0.5*AluminiumContentInAH3(zn_al_s))


/* AFm Properties ((Ca4).(Al2).(SO4).12(OH).6(H2O)) */
/* Molar volume of AFm solid (dm3/mole) */
#define V_AFm      (311.26*cm3)      /* Thermochimie (ANDRA) */
//#define AFmSolidContent(n,s,dt)     (n*pow(s,dt/t_afm))
#define AFmSolidContent(n,s,dt)     MAX((n + dt*r_afm*(s - 1)),0.)


/* AFt Properties ((Ca6).(Al2).3(SO4).12(OH).26(H2O)) */
/* Molar volume of AFt solid (dm3/mole) */
#define V_AFt      (710.32*cm3)      /* Thermochimie (ANDRA) */
//#define AFtSolidContent(n,s,dt)     (n*pow(s,dt/t_aft))
#define AFtSolidContent(n,s,dt)     MAX((n + dt*r_aft*(s - 1)),0.)
/* Surface tension (N/m) */
#define Gamma_AFt   (0.1*Pa*m)
/* Equilibrium saturation index of AFt */
#define EquilibriumAFtSaturationIndex(r)  (exp(2*Gamma_AFt*V_AFt/(RT*r)))
#define dEquilibriumAFtSaturationIndex(r) (-2*Gamma_AFt*V_AFt/(RT*r*r)*EquilibriumAFtSaturationIndex(r))
#define InverseOfEquilibriumAFtSaturationIndex(b)  (2*Gamma_AFt*V_AFt/(RT*log(b)))
/* Crystallization pressure of ettringite */
#define CrystallizationPressure(s_AFt) ((s_AFt > 1) ? RT/V_AFt*log(s_AFt) : 0)


/* C3AH6 Properties ((Ca3).(Al2).12(OH)) */
/* Molar volume of C3AH6 solid (dm3/mole) */
#define V_C3AH6      (149.52*cm3)
#define C3AH6SolidContent(n,s,dt)     MAX((n + dt*r_c3ah6*(s - 1)),0.)


/* Element contents in solid phases  */
//#define CalciumContentInCHAndCSH2(zn_ca_s) (n_ca_ref*MAX(zn_ca_s,0.))
#define CalciumContentInCH(zn_ca_s)        (n_ca_ref*MAX(zn_ca_s,0.))
#define SiliconContentInCSH(zn_si_s)       (n_si_ref*MAX(zn_si_s,0.))
#define AluminiumContentInAH3(zn_al_s)     (n_al_ref*MAX(zn_al_s,0.))


/* Gypsum-based porous material properties */
/* Porosity of gypsum-based porous material (-) */
#define PHI_Gyp    (0.85)
/* Molar volume of gypsum-based porous material */
#define V_Gyp      (V_CSH2/(1 - PHI_Gyp))



/* To retrieve the material properties */
#define GetProperty(a)   (Element_GetProperty(el)[pm(a)])



/* Fonctions */
static int     pm(const char *s) ;
static void    GetProperties(Element_t*) ;


static double* ComputeVariables(Element_t*,double**,double**,double*,double,double,int) ;
static void    ComputeSecondaryVariables(Element_t*,double,double,double*) ;
static double* ComputeVariableDerivatives(Element_t*,double,double,double*,double,int) ;


static void    ComputeTransferCoefficients(Element_t*,double**,double*) ;
static double* ComputeVariableFluxes(Element_t*,int,int) ;
//static double* ComputeVariableFluxDerivatives(Element_t*,int,int,double) ;

static int     TangentCoefficients(Element_t*,double,double,double*) ;
static int     TangentCoefficients2(Element_t*,double,double,double*) ;
static int     TangentCoefficients3(Element_t*,double,double,double*) ;


static double  Radius(double,double,double,Element_t*) ;


static double TortuosityOhJang(double) ;
static double TortuosityBazantNajjar(double) ;

static void   ComputePhysicoChemicalProperties(void) ;

static double* IsotropicElasticTensor(double,double,double*) ;


static double* ComputeVariables2(Element_t*,double**,double**,double*,double,double,int) ;
static void  ComputeSecondaryVariables2(Element_t*,double,double,double*) ;

//#define LiquidTortuosity  TortuosityOhJang
#define LiquidTortuosity  TortuosityBazantNajjar




/* Parameters */
static double phi0 ;
static double r_afm,r_aft,r_c3ah6,r_csh2 ;
static double n_ca_ref,n_si_ref,n_al_ref ;
static double n_afm_0,n_aft_0,n_c3ah6_0,n_csh2_0 ;
static double a_AFt ;
static double RT ;
static Curve_t* satcurve ;
static double* cijkl ;
static double  biot ;


#include "PhysicalConstant.h"
#include "Temperature.h"

void ComputePhysicoChemicalProperties(void)
{
  RT = PhysicalConstant_PerfectGasConstant * Temperature_RoomValue ;
}


static CementSolutionDiffusion_t* csd = NULL ;
static HardenedCementChemistry_t* hcc = NULL ;




#define NbOfVariables    (NEQ+51)
static double Variables[Element_MaxNbOfNodes][NbOfVariables] ;
static double dVariables[NbOfVariables] ;


#define I_ZN_Ca_S      (NEQ+0)
#define I_ZN_Si_S      (NEQ+1)
#define I_ZN_Al_S      (NEQ+2)

#define I_N_Q          (NEQ+4)
#define I_N_S          (NEQ+5)
#define I_N_Ca         (NEQ+6)
#define I_N_Si         (NEQ+7)
#define I_N_K          (NEQ+8)
#define I_N_Cl         (NEQ+9)
#define I_N_Al         (NEQ+10)

#define I_N_CH         (NEQ+11)
#define I_N_CSH2       (NEQ+12)
#define I_N_AH3        (NEQ+13)
#define I_N_AFm        (NEQ+14)
#define I_N_AFt        (NEQ+15)
#define I_N_C3AH6      (NEQ+16)
#define I_N_CSH        (NEQ+17)


#define I_V_Cem        (NEQ+21)

#define I_PHI          (NEQ+22)
#define I_PHI_C        (NEQ+23)

#define I_V_CSH        (NEQ+24)

#define I_P_CSH2       (NEQ+26)



#define I_N_CHn        (NEQ+31)
#define I_N_CSH2n      (NEQ+32)
#define I_N_AH3n       (NEQ+33)
#define I_N_AFmn       (NEQ+34)
#define I_N_AFtn       (NEQ+35)
#define I_N_C3AH6n     (NEQ+36)
#define I_N_CSHn       (NEQ+37)

#define I_V_Cem0       (NEQ+41)


#define I_PHIn         (NEQ+42)
#define I_PHI_Cn       (NEQ+43)


#define I_C_OHn        (NEQ+45)

#define I_Radius       (NEQ+46)
#define I_Radiusn      (NEQ+47)

#define I_S_C          (NEQ+48)

#define I_P_C          (NEQ+49)

#define I_ScPc         (NEQ+50)



#define NbOfVariables2    (41)
static double Variables2[Element_MaxNbOfNodes][NbOfVariables2] ;
static double dVariables2[NbOfVariables2] ;

#define I_DISP         (0)
#define I_STRESS       (3)
#define I_STRAIN       (12)
#define I_PRESSURE     (21)
#define I_STRESS_n     (22)
#define I_STRAIN_n     (31)
#define I_PRESSURE_n   (40)




  
  

#define NbOfVariableFluxes    (7)
static double VariableFluxes[Element_MaxNbOfNodes][NbOfVariableFluxes] ;
//static double dVariableFluxes[NbOfVariableFluxes] ;

#define I_W_S           (0)
#define I_W_Ca          (1)
#define I_W_Si          (2)
#define I_W_K           (3)
#define I_W_Cl          (4)
#define I_W_q           (5)
#define I_W_Al          (6)


int pm(const char *s)
{
  if(strcmp(s,"porosity") == 0)        return (0) ;
  else if(strcmp(s,"N_CH") == 0)       return (1) ;
  else if(strcmp(s,"N_Si") == 0)       return (2) ;
  else if(strcmp(s,"N_CSH") == 0)      return (2) ; /* synonym */
  else if(strcmp(s,"T_CH") == 0)       return (3) ;
  else if(strcmp(s,"T_CSH2") == 0)     return (4) ;
  else if(strcmp(s,"N_CSH2") == 0)     return (5) ;
  else if(strcmp(s,"N_AH3") == 0)      return (6) ;
  else if(strcmp(s,"N_AFm") == 0)      return (7) ;
  else if(strcmp(s,"N_AFt") == 0)      return (8) ;
  else if(strcmp(s,"N_C3AH6") == 0)    return (9) ;
  else if(strcmp(s,"T_AFm") == 0)      return (10) ;
  else if(strcmp(s,"T_AFt") == 0)      return (11) ;
  else if(strcmp(s,"R_AFm") == 0)      return (12) ;
  else if(strcmp(s,"R_AFt") == 0)      return (13) ;
  else if(strcmp(s,"R_C3AH6") == 0)    return (14) ;
  else if(strcmp(s,"R_CSH2") == 0)     return (15) ;
  else if(strcmp(s,"a_AFt") == 0)      return (16) ;
  else if(!strcmp(s,"Poisson"))        return (17) ;
  else if(!strcmp(s,"Young"))          return (18) ;
  else if(!strcmp(s,"Cijkl")) {
    return(19) ;
  } else if(!strcmp(s,"BiotCoef")) {
    return (100) ;
  } else return(-1) ;
}


void GetProperties(Element_t* el)
{
  phi0      = GetProperty("porosity") ;
  n_ca_ref  = GetProperty("N_CH") ;
  n_si_ref  = GetProperty("N_CSH") ;
  n_al_ref  = GetProperty("N_AH3") ;
  n_csh2_0  = GetProperty("N_CSH2") ;
  n_afm_0   = GetProperty("N_AFm") ;
  n_aft_0   = GetProperty("N_AFt") ;
  n_c3ah6_0 = GetProperty("N_C3AH6") ;
  r_afm     = GetProperty("R_AFm") ;
  r_aft     = GetProperty("R_AFt") ;
  r_c3ah6   = GetProperty("R_C3AH6") ;
  r_csh2    = GetProperty("R_CSH2") ;
  a_AFt     = GetProperty("a_AFt") ;
  
  satcurve  = Element_FindCurve(el,"S_r") ;
  
  cijkl     = &GetProperty("Cijkl") ;
  biot      = GetProperty("BiotCoef") ;
}


int SetModelProp(Model_t* model)
{
  Model_GetNbOfEquations(model) = NEQ ;
  Model_GetNbOfVariables(model) = NbOfVariables ;
  Model_GetNbOfVariableFluxes(model) = NbOfVariableFluxes ;
  
  Model_CopyNameOfEquation(model,E_S, "sulfur") ;
  Model_CopyNameOfEquation(model,E_Ca,"calcium") ;
  Model_CopyNameOfEquation(model,E_q, "charge") ;
  Model_CopyNameOfEquation(model,E_K, "potassium") ;
  Model_CopyNameOfEquation(model,E_Al,"aluminium") ;
#ifdef E_el
  Model_CopyNameOfEquation(model,E_el,"electroneutrality") ;
#endif
  Model_CopyNameOfEquation(model,E_Mech,"mechanics") ;


#if (U_H2SO4 == LOG_U)
  Model_CopyNameOfUnknown(model,U_C_H2SO4,"logc_h2so4") ;
#else
  Model_CopyNameOfUnknown(model,U_C_H2SO4,"c_h2so4") ;
#endif
  Model_CopyNameOfUnknown(model,U_ZN_Ca_S,"z_ca") ;
  Model_CopyNameOfUnknown(model,U_PSI,    "psi") ;
#if (U_K == LOG_U)
  Model_CopyNameOfUnknown(model,U_C_K,    "logc_k") ;
#else
  Model_CopyNameOfUnknown(model,U_C_K,    "c_k") ;
#endif
  Model_CopyNameOfUnknown(model,U_ZN_Al_S,"z_al") ;
#ifdef U_C_OH
  #if (U_OH == LOG_U)
    Model_CopyNameOfUnknown(model,U_C_OH, "logc_oh") ;
  #else
    Model_CopyNameOfUnknown(model,U_C_OH, "c_oh") ;
  #endif
#endif
  Model_CopyNameOfUnknown(model,U_Disp,"disp") ;
  
  return(0) ;
}


int ReadMatProp(Material_t* mat,DataFile_t* datafile)
/* Lecture des donnees materiaux dans le fichier ficd */
{
  int  NbOfProp = 101 ;

  {
    /* Self-initialization */
    Material_GetProperty(mat)[pm("N_CH")]   = 1 ;
    Material_GetProperty(mat)[pm("N_Si")]   = 1 ;
    Material_GetProperty(mat)[pm("N_AH3")]  = 1 ;
    Material_GetProperty(mat)[pm("N_CSH2")] = 0 ;
    Material_GetProperty(mat)[pm("N_AFm")]  = 0 ;
    Material_GetProperty(mat)[pm("N_AFt")]  = 0 ;
    Material_GetProperty(mat)[pm("N_C3AH6")]  = 0 ;
    Material_GetProperty(mat)[pm("R_AFm")]  = 4.6e-4 ; /* 4.6e-4 (mol/L/s) Salgues 2013 */
    Material_GetProperty(mat)[pm("R_AFt")]  = 4.6e-4 ;
    Material_GetProperty(mat)[pm("R_C3AH6")] = 1.e-10 ;
    Material_GetProperty(mat)[pm("R_CSH2")]  = 1.e-10 ;

    Material_ScanProperties(mat,datafile,pm) ;
  }

  {
    ComputePhysicoChemicalProperties() ;
  }

  {
    HardenedCementChemistry_SetTemperature(TEMPERATURE) ;
    
    if(!csd) csd = CementSolutionDiffusion_Create() ;
    if(!hcc) hcc = HardenedCementChemistry_Create() ;
  
    {
      Curves_t* curves = Material_GetCurves(mat) ;
      int i ;

      if((i = Curves_FindCurveIndex(curves,"S_r")) < 0) {
        arret("ReadMatProp: no cumulative pore volume fraction") ;
      }

      if((i = Curves_FindCurveIndex(curves,"X_CSH")) >= 0) {
        Curve_t* curve = Curves_GetCurve(curves) + i ;
      
        HardenedCementChemistry_GetCurveOfCalciumSiliconRatioInCSH(hcc) = curve ;
      }

      if((i = Curves_FindCurveIndex(curves,"Z_CSH")) >= 0) {
        Curve_t* curve = Curves_GetCurve(curves) + i ;
      
        HardenedCementChemistry_GetCurveOfWaterSiliconRatioInCSH(hcc) = curve ;
      }

      if((i = Curves_FindCurveIndex(curves,"S_SH")) >= 0) {
        Curve_t* curve = Curves_GetCurve(curves) + i ;
      
        HardenedCementChemistry_GetCurveOfSaturationIndexOfSH(hcc) = curve ;
      }
    }
  }
  
  /* The 4th rank elastic tensor */
  {
    double* c = Material_GetProperty(mat) + pm("Cijkl") ;
    double young = Material_GetProperty(mat)[pm("Young")] ;
    double poisson = Material_GetProperty(mat)[pm("Poisson")] ;
    
    IsotropicElasticTensor(young,poisson,c) ;
  }

  return(NbOfProp) ;
}



int PrintModelChar(Model_t* model,FILE *ficd)
/* Saisie des donnees materiaux */
{
  
  printf(TITLE) ;
  printf("\n") ;
  
  if(!ficd) return(NEQ) ;
  
  printf("\n") ;
  printf("The 5/6 equations are:\n") ;
  printf("\t- Mass balance of S      (sulfur)\n") ;
  printf("\t- Charge balance         (charge)\n") ;
  printf("\t- Mass balance of Ca     (calcium)\n") ;
  printf("\t- Mass balance of K      (potassium)\n") ;
  printf("\t- Mass balance of Al     (aluminium)\n") ;
#ifdef E_el
  printf("\t- Electroneutrality      (electroneutrality)\n") ;
#endif
  
  printf("\n") ;
  printf("The 5/6 primary unknowns are:\n") ;
  printf("\t- Sulfuric acid concentration     (c_h2so4 or logc_h2so4)\n") ;
  printf("\t- Electric potential              (psi)\n") ;
  printf("\t- Zeta unknown for calcium        (z_ca)\n") ;
  printf("\t- Potassium concentration         (c_k)\n") ;
  printf("\t- Zeta unknown for aluminium      (z_al)\n") ;
#ifdef U_C_OH
  printf("\t- Hydroxide ion concentration     (c_oh or logc_oh)\n") ;
#endif

  printf("\n") ;
  printf("PAY ATTENTION to units : \n") ;
  printf("\t length : dm !\n") ;
  printf("\t time   : s !\n") ;

  printf("\n") ;
  printf("Some other informations\n") ;
  printf("Example of input data\n") ;
  printf("\n") ;

  fprintf(ficd,"porosity = 0.38   # Porosity\n") ;
  fprintf(ficd,"N_CH  = 6.1       # CH mole content (moles/L)\n") ;
  fprintf(ficd,"N_K   = 0.4       # K mole content  (moles/L)\n") ;
  fprintf(ficd,"N_AH3  = 0.4      # Al mole content (moles/L)\n") ;
  fprintf(ficd,"N_AFm  = 0.1      # AFm mole content (moles/L)\n") ;
  fprintf(ficd,"N_AFt  = 0.4      # AFt mole content (moles/L)\n") ;
  fprintf(ficd,"Curves = file     # Pore volume fraction curve:  r  S_r\n") ;
  fprintf(ficd,"Curves = solid    # File name: S_CH  X_CSH  Z_CSH  S_SH\n") ;

  return(NEQ) ;
}


int DefineElementProp(Element_t* el,IntFcts_t* intfcts)
{
  Element_GetNbOfImplicitTerms(el) = NVI ;
  Element_GetNbOfExplicitTerms(el) = NVE ;
  Element_GetNbOfConstantTerms(el) = NV0 ;
  
  /* Compute some new interpolation functions */
  /*
  {
    int dim = Element_GetDimension(el) ;
    int nn  = Element_GetNbOfNodes(el) ;
    int i   = IntFcts_FindIntFct(intfcts,nn,dim,"MidSurface") ;
    
    Element_GetIntFct(el) = IntFcts_GetIntFct(intfcts) + i ;
    
    {
      IntFct_t* intfct = Element_GetIntFct(el) ;
      int NbOfIntPoints = IntFct_GetNbOfPoints(intfct) ;
      
      if(NbOfIntPoints != 1) {
        arret("DefineElementProp: more than 1 integration point") ;
      }
    }
  }
  */
  
  return(0) ;
}


int  ComputeLoads(Element_t* el,double t,double dt,Load_t* cg,double* r)
/* Residu du aux chargements (r) */
{
  int nn = Element_GetNbOfNodes(el) ;
  int ndof = nn*NEQ ;
  FVM_t* fvm = FVM_GetInstance(el) ;
  int    i ;

  {
    double* r1 = FVM_ComputeSurfaceLoadResidu(fvm,cg,t,dt) ;
    
    for(i = 0 ; i < ndof ; i++) r[i] = -r1[i] ;
  }
  
  return(0) ;
}


int ComputeInitialState(Element_t* el)
/* Initialise les variables du systeme (f,va) */ 
{
  double* f = Element_GetImplicitTerm(el) ;
  double* v0 = Element_GetConstantTerm(el) ;
  int nn = Element_GetNbOfNodes(el) ;
  double** u = Element_ComputePointerToNodalUnknowns(el) ;
  int i ;
  
  /*
    Input data
  */
  GetProperties(el) ;
  
  
  /* Pre-initialization */
  for(i = 0 ; i < nn ; i++) {
    double zn_ca_s    = ZN_Ca_S(i) ;
    double zn_si_s    = 1 ;
    double zn_al_s    = ZN_Al_S(i) ;
  
    /* Solve cement chemistry */
    {
      double logc_h2so4 = LogC_H2SO4(i) ;
      double logc_na    = -99 ;
      double logc_k     = LogC_K(i) ;
      double logc_oh    = -7 ;
      double c_cl       = 0 ;
  
      HardenedCementChemistry_GetInput(hcc,SI_Ca) = MIN(zn_ca_s,0) ;
      HardenedCementChemistry_GetInput(hcc,SI_Si) = MIN(zn_si_s,0) ;
      HardenedCementChemistry_GetInput(hcc,SI_Al) = MIN(zn_al_s,0) ;
      HardenedCementChemistry_GetInput(hcc,LogC_H2SO4) = logc_h2so4 ;
      HardenedCementChemistry_GetInput(hcc,LogC_Na)  = logc_na ;
      HardenedCementChemistry_GetInput(hcc,LogC_K)   = logc_k ;
      HardenedCementChemistry_GetInput(hcc,LogC_OH)  = logc_oh ;
    
      HardenedCementChemistry_GetAqueousConcentrationOf(hcc,Cl) = c_cl ;
  
      HardenedCementChemistry_ComputeSystem(hcc,CaO_SiO2_Na2O_K2O_SO3_Al2O3_H2O_2) ;
      
      HardenedCementChemistry_SolveElectroneutrality(hcc) ;
    }
    
  
    {
      /* Liquid components */
      double x_csh      = HardenedCementChemistry_GetCalciumSiliconRatioInCSH(hcc) ;
      //double s_ch   = HardenedCementChemistry_GetSaturationIndexOf(hcc,CH) ;
      //double s_csh2 = HardenedCementChemistry_GetSaturationIndexOf(hcc,CSH2) ;
      double c_oh   = HardenedCementChemistry_GetAqueousConcentrationOf(hcc,OH) ;
    
      /* Solid contents */
      /* ... as components: CH, CSH2, CSH, AH3, AFm, AFt, C3AH6 */
      double n_ch       = CHSolidContent(zn_ca_s) ;
      double n_csh2     = n_csh2_0 ;
      double n_ah3      = AH3SolidContent(zn_al_s) ;
      double n_afm      = n_afm_0 ;
      double n_aft      = n_aft_0 ;
      double n_c3ah6    = n_c3ah6_0 ;
      double n_csh      = CSHSolidContent(zn_si_s) ;
      /* ... as volume */
      double v_csh      = MolarVolumeOfCSH(x_csh) ;
      double v_cem      = V_CH*n_ch + v_csh*n_csh + V_AH3*n_ah3 + V_AFm*n_afm + V_AFt*n_aft + V_C3AH6*n_c3ah6  ;
      /*
      double v_gyp      = V_Gyp*n_csh2 ;
      double v_csh2     = V_CSH2*n_csh2 ;
      */

      /* Porosity */
      double phi_c = phi0 ;
      double phi   = phi_c - V_CSH2*n_csh2 ;
    

      /* Backup what is needed to compute components */
      N_CH(i)    = n_ch ;
      N_CSH2(i)  = n_csh2 ;
      N_AFm(i)   = n_afm ;
      N_AFt(i)   = n_aft ;
      N_C3AH6(i) = n_c3ah6 ;
      PHI(i)     = phi ;
      PHI_C(i)   = phi_c ;

      #ifdef U_C_OH
        #if (U_OH == LOG_U)
          LogC_OH(i) = log10(c_oh) ;
        #else
          C_OH(i)    = c_oh ;
        #endif
      #else
        C_OH(i)    = c_oh ;
      #endif

      V_Cem0(i)  = v_cem ;
    }
    
    {
      PoreRadius(i)    = PoreEntryRadiusMax ;
    }

  }
  
  
  for(i = 0 ; i < nn ; i++) {
    /* Variables */
    //double* x = ComputeVariables(pcm,0,i) ;
    double* x = ComputeVariables(el,u,u,f,0,0,i) ;
    double* mui = CementSolutionDiffusion_GetPotentialAtPoint(csd,i) ;
    
    HardenedCementChemistry_CopyChemicalPotential(hcc,mui) ;

    
    /* Backup */
    N_S(i)  = x[I_N_S] ;
    N_Ca(i) = x[I_N_Ca] ;
    N_Si(i) = x[I_N_Si] ;
    N_K(i)  = x[I_N_K] ;
    N_Cl(i) = x[I_N_Cl] ;
    N_Al(i) = x[I_N_Al] ;
    /* charge density */
    N_q(i)  = x[I_N_Q] ;

    
    N_CH(i)    = x[I_N_CH] ;
    N_CSH2(i)  = x[I_N_CSH2] ;
    N_AFm(i)   = x[I_N_AFm] ;
    N_AFt(i)   = x[I_N_AFt] ;
    N_C3AH6(i) = x[I_N_C3AH6] ;
    PHI(i)     = x[I_PHI] ;
    PHI_C(i)   = x[I_PHI_C] ;

    #ifndef U_C_OH
      C_OH(i)    = x[I_C_OH] ;
    #endif
    
    PoreRadius(i)    = x[I_Radius] ;
  }
  
  
  if(Element_IsSubmanifold(el)) return(0) ;
  
  
  /* Mechanics */
  {
    IntFct_t*  intfct = Element_GetIntFct(el) ;
    int NbOfIntPoints = IntFct_GetNbOfPoints(intfct) ;
    int p ;
    
    if(NbOfIntPoints > 2) {
      arret("ComputeInitialState: more than 2 integration points") ;
    }
    
    for(p = 0 ; p < NbOfIntPoints ; p++) {
      double* sig = SIG + p*9 ;
      /* Pre-initialization */
        
      PRESSURE[p] = 0 ;
      
      {
        for(i = 0 ; i < 9 ; i++) sig[i] = 0 ;
      }
      
      {
        /* Variables */
        double* y = ComputeVariables2(el,u,u,f,0,0,p) ;
        
        PRESSURE[p] = y[I_PRESSURE] ;
    
        /* storage in vim */
        for(i = 0 ; i < 9 ; i++) sig[i] = y[I_STRESS + i] ;
      }
    }
  }
  
  

  /* Coefficient de transfert */
  ComputeTransferCoefficients(el,u,f) ;

  /* Flux */
  {
    double* w = ComputeVariableFluxes(el,0,1) ;

    W_S     = w[I_W_S  ] ;
    W_Ca    = w[I_W_Ca ] ;
    W_Si    = w[I_W_Si ] ;
    W_q     = w[I_W_q  ] ;
    W_K     = w[I_W_K  ] ;
    W_Cl    = w[I_W_Cl ] ;
    W_Al    = w[I_W_Al ] ;
  }
  return(0) ;
}


int  ComputeExplicitTerms(Element_t* el,double t)
/* Thermes explicites (va)  */
{
  double* f = Element_GetPreviousImplicitTerm(el) ;
  double** u = Element_ComputePointerToPreviousNodalUnknowns(el) ;
  
  if(Element_IsSubmanifold(el)) return(0) ;
  
  /*
    Input data
  */
  GetProperties(el) ;
  
  /*
    Transfer coefficient
  */
  
  ComputeTransferCoefficients(el,u,f) ;
  

  return(0) ;
}


int  ComputeImplicitTerms(Element_t* el,double t,double dt)
/* Les variables donnees par la loi de comportement (f_1) */
{
  double* f = Element_GetCurrentImplicitTerm(el) ;
  double* f_n = Element_GetPreviousImplicitTerm(el) ;
  int nn = Element_GetNbOfNodes(el) ;
  double** u   = Element_ComputePointerToCurrentNodalUnknowns(el) ;
  double** u_n = Element_ComputePointerToPreviousNodalUnknowns(el) ;
  int i ;
  
  /*
    Input data
  */
  GetProperties(el) ;
  
  
  /* Contenus molaires */
  for(i = 0 ; i < nn ; i++) {
    /* Variables */
    double* x = ComputeVariables(el,u,u_n,f_n,t,dt,i) ;
    double* mui = CementSolutionDiffusion_GetPotentialAtPoint(csd,i) ;
    
    HardenedCementChemistry_CopyChemicalPotential(hcc,mui) ;

    /* Backup */

    /* Molar contents */
    N_S(i)  = x[I_N_S] ;
    N_Ca(i) = x[I_N_Ca] ;
    N_Si(i) = x[I_N_Si] ;
    N_K(i)  = x[I_N_K] ;
    N_Cl(i) = x[I_N_Cl] ;
    N_Al(i) = x[I_N_Al] ;
    
    /* Charge density */
    N_q(i)  = x[I_N_Q] ;

    
    N_CH(i)    = x[I_N_CH] ;
    N_CSH2(i)  = x[I_N_CSH2] ;
    N_AFm(i)   = x[I_N_AFm] ;
    N_AFt(i)   = x[I_N_AFt] ;
    N_C3AH6(i) = x[I_N_C3AH6] ;
    PHI(i)     = x[I_PHI] ;
    PHI_C(i)   = x[I_PHI_C] ;
#ifndef U_C_OH
    C_OH(i)    = x[I_C_OH] ;
#endif
    
    PoreRadius(i)    = x[I_Radius] ;


    {
      int test = 0 ;
      /*
      if(x[I_C_H2SO4] < 0.) test = -1 ;
      if(x[I_N_Ca_S]  < 0.) test = -1 ;
      if(x[I_N_Si_S]  < 0.) test = -1 ;
      if(x[I_N_Al_S]  < 0.) test = -1 ;
      */
      if(x[I_PHI]     < 0.) test = -1 ;
      
      if(test < 0) {
        double c_h2so4 = HardenedCementChemistry_GetAqueousConcentrationOf(hcc,H2SO4) ;
        double c_oh = HardenedCementChemistry_GetAqueousConcentrationOf(hcc,OH) ;
        double s_ch   = HardenedCementChemistry_GetSaturationIndexOf(hcc,CH) ;
        double s_csh2 = HardenedCementChemistry_GetSaturationIndexOf(hcc,CSH2) ;
        double s_ah3  = HardenedCementChemistry_GetSaturationIndexOf(hcc,AH3) ;
        double s_afm  = HardenedCementChemistry_GetSaturationIndexOf(hcc,AFm) ;
        double s_aft  = HardenedCementChemistry_GetSaturationIndexOf(hcc,AFt) ;
        double s_c3ah6  = HardenedCementChemistry_GetSaturationIndexOf(hcc,C3AH6) ;


        double xx = Element_GetNodeCoordinate(el,i)[0] ;
        
        printf("x         = %e\n",xx) ;
        printf("phi       = %e\n",x[I_PHI]) ;
        printf("phi_c     = %e\n",x[I_PHI_C]) ;
        printf("c_h2so4   = %e\n",c_h2so4) ;
        printf("n_ch      = %e\n",x[I_N_CH]) ;
        printf("n_csh2    = %e\n",x[I_N_CSH2]) ;
        printf("n_csh     = %e\n",x[I_N_CSH]) ;
        printf("n_ah3     = %e\n",x[I_N_AH3]) ;
        printf("n_afm     = %e\n",x[I_N_AFm]) ;
        printf("n_aft     = %e\n",x[I_N_AFt]) ;
        printf("n_c3ah6   = %e\n",x[I_N_C3AH6]) ;
        printf("s_ch      = %e\n",s_ch) ;
        printf("s_csh2    = %e\n",s_csh2) ;
        printf("s_ah3     = %e\n",s_ah3) ;
        printf("s_afm     = %e\n",s_afm) ;
        printf("s_aft     = %e\n",s_aft) ;
        printf("s_c3ah6   = %e\n",s_c3ah6) ;
        printf("zn_ca_s   = %e\n",x[I_ZN_Ca_S]) ;
        printf("zn_al_s   = %e\n",x[I_ZN_Al_S]) ;
        printf("c_oh      = %e\n",c_oh) ;
        printf("p_csh2    = %e\n",x[I_P_CSH2]) ;
        return(-1) ;
      }
    }
  }
  
  
  if(Element_IsSubmanifold(el)) return(0) ;
  

  
  /* Mechanics */
  {
    IntFct_t*  intfct = Element_GetIntFct(el) ;
    int NbOfIntPoints = IntFct_GetNbOfPoints(intfct) ;
    int p ;
    
    for(p = 0 ; p < NbOfIntPoints ; p++) {
      /* Variables */
      double* y = ComputeVariables2(el,u,u_n,f_n,t,dt,p) ;
    
      /* storage in vim */
      PRESSURE[p] = y[I_PRESSURE] ;
      {
        double* sig = SIG + p*9 ;
      
        for(i = 0 ; i < 9 ; i++) sig[i] = y[I_STRESS + i] ;
      }
    }
  }

  /* Flux */
  {
    double* w = ComputeVariableFluxes(el,0,1) ;

    W_S     = w[I_W_S  ] ;
    W_Ca    = w[I_W_Ca ] ;
    W_Si    = w[I_W_Si ] ;
    W_q     = w[I_W_q  ] ;
    W_K     = w[I_W_K  ] ;
    W_Cl    = w[I_W_Cl ] ;
    W_Al    = w[I_W_Al ] ;
  }

  return(0) ;
}



int  ComputeMatrix(Element_t* el,double t,double dt,double* k)
/* Matrice (k) */
{
#define K(i,j)    (k[(i)*ndof + (j)])
  int nn = Element_GetNbOfNodes(el) ;
  int ndof = nn*NEQ ;
  
  
  /*
    Initialisation 
  */
  {
    int i ;
    
    for(i = 0 ; i < ndof*ndof ; i++) k[i] = 0. ;
  }

  if(Element_IsSubmanifold(el)) return(0) ;
  
  /*
    Input data
  */
  GetProperties(el) ;
  
  
  /*
  ** Conservation of masses (NEQ1 first equations)
  */
  {
    double c[Element_MaxNbOfDOF*Element_MaxNbOfDOF] ;
    
    TangentCoefficients(el,t,dt,c) ;
    
    {
      FVM_t* fvm = FVM_GetInstance(el) ;
      double* km = FVM_ComputeMassAndIsotropicConductionMatrix(fvm,c,NEQ) ;
      int i ;
    
      for(i = 0 ; i < nn ; i++) {
        int ei ;
          
        for(ei = 0 ; ei < NEQ1 ; ei++) {
          int j ;
          
          for(j = 0 ; j < nn ; j++) {
            int uj ;
              
            for(uj = 0 ; uj < NEQ1 ; uj++) {
              K(i*NEQ + ei,j*NEQ + uj) = km[(i*NEQ + ei)*ndof + j*NEQ + uj] ;
            }
          }
        }
      }
    }
  }
  
  
  /*
  ** Mechanics
  */
  {
    IntFct_t*  intfct = Element_GetIntFct(el) ;
    double c2[IntFct_MaxNbOfIntPoints*(81+NEQ1*9)] ;
    int shift = TangentCoefficients2(el,t,dt,c2) ;
    
    /*
    ** Elastic Matrix
    */
    {
      FEM_t* fem = FEM_GetInstance(el) ;
      double* kp = FEM_ComputeElasticMatrix(fem,intfct,c2,shift) ;
      int i ;
    
      for(i = 0 ; i < nn ; i++) {
        int j ;
      
        for(j = 0 ; j < nn ; j++) {
          K(i*NEQ + E_Mech,j*NEQ + U_Disp) = kp[i*nn + j] ;
        }
      }
    }
    
    /* 
     ** Biot-like Coupling terms 
     */
    {
      FEM_t* fem = FEM_GetInstance(el) ;
      int I_H = 0 ;
      int n ;
      
      for(n = 0 ; n < NEQ1 ; n++) {
        int I_h = I_H + n ;
        double* c1 = c2 + 81 + n*9 ;
        double* kb = FEM_ComputeBiotMatrix(fem,intfct,c1,shift) ;
        int i = 0 ;
    
        for(i = 0 ; i < nn ; i++) {
          int j ;
          
          for(j = 0 ; j < nn ; j++) {
            K(E_Mech + i*NEQ,I_h + j*NEQ) = kb[(i)*nn + j] ;
          }
        }
      }
    }
  }
  


#if (U_H2SO4 == NOLOG_U)
  {
    double** u = Element_ComputePointerToNodalUnknowns(el) ;
    int i ;
    
    for(i = 0 ; i < 2*NEQ ; i++){
      K(i,U_C_H2SO4)     /= Ln10*C_H2SO4(0) ;
      K(i,U_C_H2SO4+NEQ) /= Ln10*C_H2SO4(1) ;
    }
  }
#endif

#if (U_K == NOLOG_U)
  {
    double** u = Element_ComputePointerToNodalUnknowns(el) ;
    int i ;
    
    for(i = 0 ; i < 2*NEQ ; i++){
      K(i,U_C_K)     /= Ln10*C_K(0) ;
      K(i,U_C_K+NEQ) /= Ln10*C_K(1) ;
    }
  }
#endif
  
#ifdef U_C_OH
  #if (U_OH == NOLOG_U)
  {
    double** u = Element_ComputePointerToNodalUnknowns(el) ;
    int i ;
    
    for(i = 0 ; i < 2*NEQ ; i++){
      K(i,U_C_OH)     /= Ln10*C_OH(0) ;
      K(i,U_C_OH+NEQ) /= Ln10*C_OH(1) ;
    }
  }
  #endif
#endif


  return(0) ;

#undef K
}


int  ComputeResidu(Element_t* el,double t,double dt,double* r)
/* Residu (r) */
{
#define R(n,i)    (r[(n)*NEQ+(i)])
  double* f = Element_GetCurrentImplicitTerm(el) ;
  double* f_n = Element_GetPreviousImplicitTerm(el) ;
  int nn = Element_GetNbOfNodes(el) ;
  int ndof = nn*NEQ ;
  FVM_t* fvm = FVM_GetInstance(el) ;
  double* volume = FVM_ComputeCellVolumes(fvm) ;
  double surf ;
  int    i ;
  double zero = 0. ;
  /*
    INITIALISATION DU RESIDU
  */
  for(i = 0 ; i < ndof ; i++) r[i] = zero ;

  if(Element_IsSubmanifold(el)) return(0) ;
  
  /* Boundary Surface Area */
  {
    double* area = FVM_ComputeCellSurfaceAreas(fvm) ;
    surf = area[1] ;
  }
  
  /*
    Mass balance of elements S, Ca, Si, K, Cl
  */
  R(0,E_S)  -= volume[0]*(N_S(0)  - N_Sn(0))  + dt*surf*W_S ;
  R(1,E_S)  -= volume[1]*(N_S(1)  - N_Sn(1))  - dt*surf*W_S ;
  
  R(0,E_Ca) -= volume[0]*(N_Ca(0) - N_Can(0)) + dt*surf*W_Ca ;
  R(1,E_Ca) -= volume[1]*(N_Ca(1) - N_Can(1)) - dt*surf*W_Ca ;
  
  R(0,E_K)  -= volume[0]*(N_K(0)  - N_Kn(0))  + dt*surf*W_K ;
  R(1,E_K)  -= volume[1]*(N_K(1)  - N_Kn(1))  - dt*surf*W_K ;
  
  R(0,E_Al) -= volume[0]*(N_Al(0) - N_Aln(0)) + dt*surf*W_Al ;
  R(1,E_Al) -= volume[1]*(N_Al(1) - N_Aln(1)) - dt*surf*W_Al ;
  
  /*
    Conservation of charge
  */
  R(0,E_q) -= + dt*surf*W_q ;
  R(1,E_q) -= - dt*surf*W_q ;
  
#ifdef E_el
  /*
    Conservation of charge
  */
  /*
  R(0,E_q) -= volume[0]*(N_q(0) - N_qn(0)) ;
  R(1,E_q) -= volume[1]*(N_q(1) - N_qn(1)) ;
  */
  /*
    Electroneutrality
  */
  R(0,E_el) -= volume[0]*N_q(0) ;
  R(1,E_el) -= volume[1]*N_q(1) ;
#endif


  /* Mechanics (stresses) */
  {
    int dim = Element_GetDimensionOfSpace(el) ;
    IntFct_t*  intfct = Element_GetIntFct(el) ;
    FEM_t* fem = FEM_GetInstance(el) ;
    double* rw = FEM_ComputeStrainWorkResidu(fem,intfct,SIG,9) ;
    
    for(i = 0 ; i < nn ; i++) {
      int j ;
      for(j = 0 ; j < dim ; j++) R(i,E_Mech + j) -= rw[i*dim + j] ;
    }
  }
  
  return(0) ;

#undef R
}


int  ComputeOutputs(Element_t* el,double t,double* s,Result_t* r)
/* Les valeurs exploitees (s) */
{
  double* f = Element_GetCurrentImplicitTerm(el) ;
  FVM_t* fvm = FVM_GetInstance(el) ;
  double** u   = Element_ComputePointerToCurrentNodalUnknowns(el) ;
  int    nso = 58 ;
  double one = 1 ;
  int    i ;

  if(Element_IsSubmanifold(el)) return(0) ;
  
  /*
    Input data
  */
  GetProperties(el) ;


  /* Initialization */
  for(i = 0 ; i < nso ; i++) {
    Result_SetValuesToZero(r + i) ;
  }


  /* output quantities */
  {
    int j = FVM_FindLocalCellIndex(fvm,s) ;
    /* molarites */
    double* x = ComputeVariables(el,u,u,f,t,0,j) ;
    /* Concentrations */
#define ptC(CPD)   &(HardenedCementChemistry_GetLogAqueousConcentrationOf(hcc,CPD))
#define ptS(CPD)   &(HardenedCementChemistry_GetSaturationIndexOf(hcc,CPD))
#define ptPSI      &(HardenedCementChemistry_GetElectricPotential(hcc))
#define ptX_CSH    &(HardenedCementChemistry_GetCalciumSiliconRatioInCSH(hcc))
#define ptZ_CSH    &(HardenedCementChemistry_GetWaterSiliconRatioInCSH(hcc))

    i = 0 ;
    
    {
      double ph        = - *(ptC(H )) ;
      
      Result_Store(r + i++,&ph,"ph",1) ;
    }
    
    Result_Store(r + i++,ptC(OH),"c_oh",1) ;
    Result_Store(r + i++,ptC(H ),"c_h",1) ;
    
    Result_Store(r + i++,ptC(Ca  ),"c_ca",1) ;
    Result_Store(r + i++,ptC(CaOH),"c_caoh",1) ;
    
    Result_Store(r + i++,ptC(H2SiO4),"c_h2sio4",1) ;
    Result_Store(r + i++,ptC(H3SiO4),"c_h3sio4",1) ;
    Result_Store(r + i++,ptC(H4SiO4),"c_h4sio4",1) ;
    
    Result_Store(r + i++,ptC(CaH2SiO4),"c_cah2sio4",1) ;
    Result_Store(r + i++,ptC(CaH3SiO4),"c_cah3sio4",1) ;
    
    Result_Store(r + i++,ptC(H2SO4),"c_h2so4",1) ;
    Result_Store(r + i++,ptC(HSO4 ),"c_hso4",1) ;
    Result_Store(r + i++,ptC(SO4  ),"c_so4",1) ;
    
    Result_Store(r + i++,ptC(CaSO4),"c_caso4aq",1) ;
    Result_Store(r + i++,ptC(CaHSO4 ),"c_cahso4",1) ;

    Result_Store(r + i++,ptC(K  ),"c_k",1) ;
    Result_Store(r + i++,ptC(KOH),"c_koh",1) ;
    
    Result_Store(r + i++,(x + I_ZN_Ca_S),"zn_ca_s",1) ;
    Result_Store(r + i++,&one           ,"zn_si_s",1) ;
    
    Result_Store(r + i++,ptS(CH),"s_ch",1) ;
    Result_Store(r + i++,ptS(CSH2),"s_csh2",1) ;
    
    Result_Store(r + i++,(x + I_N_CH  ),"n_ch",1) ;
    Result_Store(r + i++,(x + I_N_CSH2),"n_csh2",1) ;
    Result_Store(r + i++,(x + I_N_CSH ),"n_csh",1) ;
    
    Result_Store(r + i++,(x + I_PHI),"porosite",1) ;
    Result_Store(r + i++,ptPSI,"potentiel_electrique",1) ;
    
    Result_Store(r + i++,(x + I_N_Q),"charge",1) ;
    
    Result_Store(r + i++,(x + I_V_CSH),"V_CSH",1) ;
    Result_Store(r + i++,ptX_CSH,"C/S",1) ;
    
    Result_Store(r + i++,&W_Si,"W_Si",1) ;
    Result_Store(r + i++,&W_Ca,"W_Ca",1) ;
    Result_Store(r + i++,&W_S ,"W_S",1) ;
    
    Result_Store(r + i++,(x + I_P_CSH2),"P_CSH2",1) ;
    //Result_Store(r + i++,(x + I_D_CON ),"Damage",1) ;
    
    Result_Store(r + i++,ptC(Al   ),"c_al",1) ;
    Result_Store(r + i++,ptC(AlO4H4),"c_alo4h4",1) ;
    
    Result_Store(r + i++,(x + I_ZN_Al_S),"zn_al_s",1) ;
    
    Result_Store(r + i++,ptS(AH3  ),"s_ah3",1) ;
    Result_Store(r + i++,ptS(AFm  ),"s_afm",1) ;
    Result_Store(r + i++,ptS(AFt  ),"s_aft",1) ;
    Result_Store(r + i++,ptS(C3AH6),"s_c3ah6",1) ;
    
    Result_Store(r + i++,(x + I_N_AH3  ),"n_ah3",1) ;
    Result_Store(r + i++,(x + I_N_AFm  ),"n_afm",1) ;
    Result_Store(r + i++,(x + I_N_AFt  ),"n_aft",1) ;
    Result_Store(r + i++,(x + I_N_C3AH6),"n_c3ah6",1) ;
    
    Result_Store(r + i++,&W_Al,"W_Al",1) ;
    
    Result_Store(r + i++,&W_q,"W_q",1) ;
    
    Result_Store(r + i++,&N_Ca(j),"N_Ca",1) ;
    Result_Store(r + i++,&N_Si(j),"N_Si",1) ;
    Result_Store(r + i++,&N_S(j) ,"N_S",1) ;
    Result_Store(r + i++,&N_Al(j),"N_Al",1) ;
    Result_Store(r + i++,&N_K(j) ,"N_K",1) ;
    Result_Store(r + i++,&N_Cl(j),"N_Cl",1) ;
    
    Result_Store(r + i++,(x + I_S_C    ),"Saturation degree of crystal",1) ;
    Result_Store(r + i++,(x + I_Radius ),"Pore entry radius",1) ;
    
    {
      double beta = exp(2*Gamma_AFt*V_AFt/(RT*x[I_Radius])) ;
      Result_Store(r + i++,&beta,"Equilibrium saturation index of AFt",1) ;
    }
    
    Result_Store(r + i++,(x + I_P_C    ),"Crystallization pressure",1) ;
    
    {
      double disp = DISP(j) ;
      
      Result_Store(r + i++,&disp,"Displacement",1) ;
    }
    
    {
      IntFct_t*  intfct = Element_GetIntFct(el) ;
      int NbOfIntPoints = IntFct_GetNbOfPoints(intfct) ;
      double sig0[9] = {0,0,0,0,0,0,0,0,0} ;
      int p ;
    
      for(p = 0 ; p < NbOfIntPoints ; p++) {
        double* sig = SIG + 9*p ;
        int k ;
      
        for(k = 0 ; k < 9 ; k++) sig0[k] += sig[k]/NbOfIntPoints ;
      }
      
      Result_Store(r + i++,sig0,"Stresses",9) ;
    }
    
    
    if(i != nso) {
      Message_RuntimeError("ComputeOutputs: wrong number of outputs") ;
    }
  }

  return(nso) ;
}



void ComputeTransferCoefficients(Element_t* el,double** u,double* f)
/* Transfer coefficients  */
{
  double* va = Element_GetExplicitTerm(el) ;
  int i ;

  /* initialisation */
  for(i = 0 ; i < NVE ; i++) va[i] = 0. ;

  for(i = 0 ; i < 2 ; i++) {
    double* x = ComputeVariables(el,u,u,f,0,0,i) ;
    
    /* Liquid tortuosity */
    {
      double phi    = x[I_PHI] ;
      double iff    = LiquidTortuosity(phi) ;
        
      TORTUOSITY += iff ;
    }
    
    /* Concentrations */
    {
      double* c = HardenedCementChemistry_GetAqueousConcentration(hcc) ;
      int n = HardenedCementChemistry_NbOfConcentrations ;
      int j ;
    
      for(j = 0 ; j < n ; j++) {
        CONCENTRATION[j] += c[j] ;
      }
    }
  }


  /* Averaging */
  {
    for(i = 0 ; i < NVE ; i++) {
      va[i] *= 0.5 ;
    }
  }


  /* Dividing by the intercell distance */
  {
    FVM_t* fvm   = FVM_GetInstance(el) ;
    double* dist = FVM_ComputeIntercellDistances(fvm) ;
    double dij   = dist[1] ;
  
    TORTUOSITY /= dij ;
  }
}



int TangentCoefficients(Element_t* el,double t,double dt,double* c)
/**  Tangent matrix coefficients (c) */
{
  double* f_n = Element_GetPreviousImplicitTerm(el) ;
  int nn = Element_GetNbOfNodes(el) ;
  int ndof = nn*NEQ ;
  ObVal_t* obval = Element_GetObjectiveValue(el) ;
  double** u   = Element_ComputePointerToCurrentNodalUnknowns(el) ;
  double** u_n = Element_ComputePointerToPreviousNodalUnknowns(el) ;
  int    dec = NEQ*NEQ ;
  double dxi[NEQ] ;
  int    i ;
  
  /* Initialization */
  for(i = 0 ; i < ndof*ndof ; i++) c[i] = 0. ;

  if(Element_IsSubmanifold(el)) return(0) ;
  
  
  for(i = 0 ; i < NEQ ; i++) {
    dxi[i] =  1.e-2*ObVal_GetValue(obval + i) ;
  }

  
  for(i = 0 ; i < nn ; i++) {
    /* Variables */
    double* x   = ComputeVariables(el,u,u_n,f_n,t,dt,i) ;
    double* mui = CementSolutionDiffusion_GetPotentialAtPoint(csd,i) ;
    int ii = (i == 0) ? 1 : 0 ;  /* In arbitrary ii there is muii = mui + dmu */
    double* muii = CementSolutionDiffusion_GetPotentialAtPoint(csd,ii) ;
    int k ;
    
    HardenedCementChemistry_CopyChemicalPotential(hcc,mui) ;


    #if (U_H2SO4 == NOLOG_U)
    dxi[U_C_H2SO4] =  1.e-2*ObVal_GetValue(obval + U_C_H2SO4)/(Ln10*x[U_C_H2SO4]) ;
    #endif
    
    #if (U_K == NOLOG_U)
    dxi[U_C_K] =  1.e-2*ObVal_GetValue(obval + U_C_K)/(Ln10*x[U_C_K]) ;
    #endif
    
    #ifdef U_C_OH
      #if (U_OH == NOLOG_U)
      dxi[U_C_OH] =  1.e-2*ObVal_GetValue(obval + U_C_OH)/(Ln10*x[U_C_OH]) ;
      #endif
    #endif
    
     /* Only the NEQ1 first equationse assuming that the mechanics
      * is located at the position NEQ */
    for(k = 0 ; k < NEQ1 ; k++) {
    //for(k = 0 ; k < NEQ ; k++) {
      double dxk    = dxi[k] ;
      double* dx    = ComputeVariableDerivatives(el,t,dt,x,dxk,k) ;
    
      HardenedCementChemistry_CopyChemicalPotential(hcc,muii) ;
    
      /* Content terms at node i */
      {
        double* cii = c + (i*nn + i)*NEQ*NEQ ;
        
        cii[E_S*NEQ    + k] = dx[I_N_S ] ;
        cii[E_Ca*NEQ   + k] = dx[I_N_Ca] ;
        cii[E_K*NEQ    + k] = dx[I_N_K ] ;
        cii[E_Al*NEQ   + k] = dx[I_N_Al] ;
#ifdef E_el
        //cii[E_q*NEQ    + k] = dx[I_N_Q] ;
        cii[E_el*NEQ   + k] = dx[I_N_Q] ;
#endif
      }
      
      /* Transfer terms from node i to node j: d(w_ij)/d(u_i) */
      {
        double* dw = ComputeVariableFluxes(el,ii,i) ;
        double  dtu = dt/dxk ;
        int j ;
        
        for(j = 0 ; j < nn ; j++) {
          if(j != i) {
            double* cij = c + (i*nn + j)*NEQ*NEQ ;
        
            cij[E_S*NEQ    + k] = dtu * dw[I_W_S ] ;
            cij[E_Ca*NEQ   + k] = dtu * dw[I_W_Ca] ;
            cij[E_K*NEQ    + k] = dtu * dw[I_W_K ] ;
            cij[E_Al*NEQ   + k] = dtu * dw[I_W_Al] ;
            cij[E_q*NEQ    + k] = dtu * dw[I_W_q ] ;
          }
        }
      }
    }
  }

  return(dec) ;
}






int TangentCoefficients3(Element_t* el,double t,double dt,double* c)
/**  Tangent matrix coefficients (c) */
{
  double* f_n = Element_GetPreviousImplicitTerm(el) ;
  int nn = Element_GetNbOfNodes(el) ;
  int ndof = nn*NEQ ;
  ObVal_t* obval = Element_GetObjectiveValue(el) ;
  double** u   = Element_ComputePointerToCurrentNodalUnknowns(el) ;
  double** u_n = Element_ComputePointerToPreviousNodalUnknowns(el) ;
  int    dec = NEQ ;
  double dxi[NEQ] ;
  int    i ;
  
  /* Initialization */
  for(i = 0 ; i < ndof ; i++) c[i] = 0. ;

  if(Element_IsSubmanifold(el)) return(0) ;
  
  
  for(i = 0 ; i < NEQ ; i++) {
    dxi[i] =  1.e-2*ObVal_GetValue(obval + i) ;
  }

  
  for(i = 0 ; i < nn ; i++) {
    /* Variables */
    double* x   = ComputeVariables(el,u,u_n,f_n,t,dt,i) ;
    int k ;

    #if (U_H2SO4 == NOLOG_U)
    dxi[U_C_H2SO4] =  1.e-2*ObVal_GetValue(obval + U_C_H2SO4)/(Ln10*x[U_C_H2SO4]) ;
    #endif
    
    #if (U_K == NOLOG_U)
    dxi[U_C_K] =  1.e-2*ObVal_GetValue(obval + U_C_K)/(Ln10*x[U_C_K]) ;
    #endif
    
    #ifdef U_C_OH
      #if (U_OH == NOLOG_U)
      dxi[U_C_OH] =  1.e-2*ObVal_GetValue(obval + U_C_OH)/(Ln10*x[U_C_OH]) ;
      #endif
    #endif
    
    for(k = 0 ; k < NEQ1 ; k++) {
      double dxk    = dxi[k] ;
      double* dx    = ComputeVariableDerivatives(el,t,dt,x,dxk,k) ;
    
      /* Content terms at node i */
      {
        double* cii = c + i*NEQ ;

        cii[k] = dx[I_ScPc] ;
      }
    }
  }

  return(dec) ;
}





int TangentCoefficients2(Element_t* el,double t,double dt,double* c)
/*
**  Tangent matrix (c), return the shift (dec).
*/
{
#define T4(a,i,j,k,l)  ((a)[(((i)*3+(j))*3+(k))*3+(l)])
#define T2(a,i,j)      ((a)[(i)*3+(j)])
#define C1(i,j,k,l)    T4(c1,i,j,k,l)
#define B1(i,j)        T2(c1,i,j)
  IntFct_t*  intfct = Element_GetIntFct(el) ;
  int np = IntFct_GetNbOfPoints(intfct) ;
  double d[Element_MaxNbOfDOF] ;
  
  int    dec = 81 + 9*NEQ1 ;
  int    p ;
  
  
  TangentCoefficients3(el,t,dt,d) ;
  
  
  for(p = 0 ; p < np ; p++) {
    double* c0 = c + p*dec ;

    /* initialization */
    {
      int i ;
      
      for(i = 0 ; i < dec ; i++) c0[i] = 0 ;
    }
    

    /* Mechanics */
    {
      /* Tangent stiffness matrix */
      {
        double* c1 = c0 ;
        int i ;
        
        for(i = 0 ; i < 81 ; i++) {
          c1[i] = cijkl[i] ;
        }
      }
      
      
      /* Coupling matrices */
      {
        int j ;
        
        for(j = 0 ; j < NEQ1 ; j++) {
          double* c1 = c0 + 81 + 9*j ;
          double* d1 = d + j ;
          double dp = IntFct_InterpolateAtPoint(intfct,d1,NEQ,p) ;
          int i ;
        
          for(i = 0 ; i < 3 ; i++) {
            B1(i,i) = - biot * dp ;
          }
        }
      }
    }

  }
  
  return(dec) ;
#undef C1
#undef B1
#undef T2
#undef T4
}



double* ComputeVariables(Element_t* el,double** u,double** u_n,double* f_n,double t,double dt,int n)
{
  double* x = Variables[n] ;
  
  /* Primary Variables */
  x[U_C_H2SO4 ] = LogC_H2SO4(n) ;
  x[U_ZN_Ca_S ] = ZN_Ca_S(n) ;
  x[U_C_K     ] = LogC_K(n) ;
  x[U_PSI     ] = PSI(n) ;
  x[U_ZN_Al_S ] = ZN_Al_S(n) ;
#ifdef U_C_OH
  x[U_C_OH    ] = LogC_OH(n) ;
#endif
  
  /* Needed variables to compute secondary components */
  x[I_N_CHn  ]  = N_CHn(n) ;
  x[I_N_CSH2n]  = N_CSH2n(n) ;
  x[I_N_AFmn ]  = N_AFmn(n) ;
  x[I_N_AFtn ]  = N_AFtn(n) ;
  x[I_N_C3AH6n] = N_C3AH6n(n) ;
  x[I_PHIn   ]  = PHIn(n) ;
  x[I_PHI_Cn ]  = PHI_Cn(n) ;
  x[I_C_OHn  ]  = C_OHn(n) ;
  x[I_Radiusn]  = PoreRadiusn(n) ;
  

  {
    double* v0   = Element_GetConstantTerm(el) ;
    
    x[I_V_Cem0 ]  = V_Cem0(n) ;
  }
  
  ComputeSecondaryVariables(el,t,dt,x) ;
  return(x) ;
}



void  ComputeSecondaryVariables(Element_t* el,double t,double dt,double* x)
{
  /* Primary variables */
  double zn_si_s    = 1 ;
  double zn_ca_s    = x[U_ZN_Ca_S] ;
  double zn_al_s    = x[U_ZN_Al_S] ;
  double c_cl       = 0 ;
  double psi        = x[U_PSI] ;
  
  /* Solve cement chemistry */
  {
    double logc_h2so4 = x[U_C_H2SO4] ;
    double logc_na    = -99 ;
    double logc_k     = x[U_C_K] ;
#ifdef U_C_OH
    double logc_oh    = x[U_C_OH] ;
#else
    double logc_oh    = log10(x[I_C_OHn]) ;
#endif
  
    HardenedCementChemistry_GetInput(hcc,SI_Ca) = MIN(zn_ca_s,0) ;
    HardenedCementChemistry_GetInput(hcc,SI_Si) = MIN(zn_si_s,0) ;
    HardenedCementChemistry_GetInput(hcc,SI_Al) = MIN(zn_al_s,0) ;
    HardenedCementChemistry_GetInput(hcc,LogC_H2SO4) = logc_h2so4 ;
    HardenedCementChemistry_GetInput(hcc,LogC_Na)  = logc_na ;
    HardenedCementChemistry_GetInput(hcc,LogC_K)   = logc_k ;
    HardenedCementChemistry_GetInput(hcc,LogC_OH)  = logc_oh ;
    HardenedCementChemistry_GetElectricPotential(hcc) = psi ;
    
    HardenedCementChemistry_GetAqueousConcentrationOf(hcc,Cl) = c_cl ;
  
    HardenedCementChemistry_ComputeSystem(hcc,CaO_SiO2_Na2O_K2O_SO3_Al2O3_H2O_2) ;

#ifndef E_el
  #if (ELECTRONEUTRALITY == IMPLICIT)
    HardenedCementChemistry_SolveElectroneutrality(hcc) ;
  #endif
#endif
  }
  

  
  /* Backup */
  double c_q_l  = HardenedCementChemistry_GetLiquidChargeDensity(hcc) ;
  
  //double I = HardenedCementChemistry_GetIonicStrength(hcc) ;
  
  //double rho_l  = HardenedCementChemistry_GetLiquidMassDensity(hcc) ;
  
  double c_ca_l = HardenedCementChemistry_GetElementAqueousConcentrationOf(hcc,Ca) ;
  double c_si_l = HardenedCementChemistry_GetElementAqueousConcentrationOf(hcc,Si) ;
  double c_k_l  = HardenedCementChemistry_GetElementAqueousConcentrationOf(hcc,K) ;
  double c_s_l  = HardenedCementChemistry_GetElementAqueousConcentrationOf(hcc,S) ;
  double c_al_l = HardenedCementChemistry_GetElementAqueousConcentrationOf(hcc,Al) ;
  
  //double s_ch   = HardenedCementChemistry_GetSaturationIndexOf(hcc,CH) ;
  //double s_sh   = HardenedCementChemistry_GetSaturationIndexOf(hcc,SH) ;
  double s_csh2 = HardenedCementChemistry_GetSaturationIndexOf(hcc,CSH2) ;
  //double s_ah3  = HardenedCementChemistry_GetSaturationIndexOf(hcc,AH3) ;
  double s_afm  = HardenedCementChemistry_GetSaturationIndexOf(hcc,AFm) ;
  double s_aft  = HardenedCementChemistry_GetSaturationIndexOf(hcc,AFt) ;
  double s_c3ah6 = HardenedCementChemistry_GetSaturationIndexOf(hcc,C3AH6) ;
       
    
    
  /* Compute the crystal saturation as a function of s_aft */
  double r_n = x[I_Radiusn] ;
  double r   = Radius(r_n,s_aft,dt,el) ;
  double s_l = LiquidSaturationDegree(r) ;
  double s_c = 1 - s_l ;
  
  
  /* Solid contents */
  /* ... as components: CH, CSH2, CSH, AH3, AFm, AFt, C3AH6 */
  //double n_chn      = x[I_N_CHn] ;
  double n_ch       = CHSolidContent(zn_ca_s) ;
  double n_csh2n    = x[I_N_CSH2n] ;
  double n_csh2     = CSH2SolidContent(n_csh2n,s_csh2,dt) ;
  double n_ah3      = AH3SolidContent(zn_al_s) ;
  double n_afmn     = x[I_N_AFmn] ;
  double n_afm      = AFmSolidContent(n_afmn,s_afm,dt) ;
  //double n_aftn     = x[I_N_AFtn] ;
  //double n_aft      = AFtSolidContent(n_aftn,s_aft,dt) ;
  double n_aft      = phi0*s_c/V_AFt ;
  double n_c3ah6n   = x[I_N_C3AH6n] ;
  double n_c3ah6    = C3AH6SolidContent(n_c3ah6n,s_c3ah6,dt) ;
  double n_csh      = CSHSolidContent(zn_si_s) ;
  /* ... as elements: S, Ca, Si, Al */
  //double x_csh      = CalciumSiliconRatioInCSH(s_ch) ;
  double x_csh      = HardenedCementChemistry_GetCalciumSiliconRatioInCSH(hcc) ;
  double n_si_s     = n_csh ;
  double n_ca_s     = n_ch + n_csh2 + x_csh*n_csh + 4*n_afm + 6*n_aft + 3*n_c3ah6 ;
  double n_s_s      = n_csh2 + n_afm + 3*n_aft ;
  double n_al_s     = 2*(n_ah3 + n_afm + n_aft + n_c3ah6) ;
  /* ... as volume */
  double v_csh      = MolarVolumeOfCSH(x_csh) ;
  double v_cem      = V_CH*n_ch + v_csh*n_csh + V_AH3*n_ah3 + V_AFm*n_afm + V_AFt*n_aft + V_C3AH6*n_c3ah6 ;
  double v_gyp      = V_Gyp*n_csh2 ;
  double v_csh2     = V_CSH2*n_csh2 ;


  /* Porosities in unconfined conditions (no pressure) */
  double v_cem0     = x[I_V_Cem0] ;
  /* ... of concrete */
  double phi_con    = phi0 + v_cem0 - v_cem ;
  /* ... of gypsum */
  double phi_gyp    = PHI_Gyp ;


  /* total porosity */
  double phi_c      = phi_con ;
  double phi_t      = phi_con - v_csh2 ;
  

#if (U_PHI == IMPLICIT)
  double phi_l        = phi_t ;
#else
  double phi_l        = x[I_PHIn] ;
#endif
    
    
  /* Liquid contents */
  /* ... as elements: S, Ca, Si, K, Cl, Al */
  double c_cl_l = c_cl ;
  double n_s_l  = phi_l*c_s_l ;
  double n_ca_l = phi_l*c_ca_l ;
  double n_si_l = phi_l*c_si_l ;
  double n_al_l = phi_l*c_al_l ;
  double n_k_l  = phi_l*c_k_l ;
  double n_cl_l = phi_l*c_cl_l ;
  double n_q_l  = phi_l*c_q_l ;


#ifndef E_el
  #if (ELECTRONEUTRALITY == EXPLICIT)
  c_q_l = HardenedCementChemistry_SolveExplicitElectroneutrality(hcc) ;
  n_q_l = phi_l*c_q_l ;
  #endif
#endif


  

  /* Backup */


  /* Solid components */
  x[I_N_CH      ] = n_ch ;
  x[I_N_CSH2    ] = n_csh2 ;
  x[I_N_AH3     ] = n_ah3 ;
  x[I_N_AFm     ] = n_afm ;
  x[I_N_AFt     ] = n_aft ;
  x[I_N_C3AH6   ] = n_c3ah6 ;
  x[I_N_CSH     ] = n_csh ;
  
  x[I_ZN_Ca_S   ] = zn_ca_s ;  
  x[I_ZN_Al_S   ] = zn_al_s ;  
  
  x[I_V_CSH     ] = v_csh ;
  
  x[I_V_Cem     ] = v_cem ;
  
  
  /* Porosities */
  x[I_PHI       ] = phi_t ;
  x[I_PHI_C     ] = phi_c ;
  
  /* Saturation degree of crystal */
  x[I_S_C       ] = s_c ;
  
  /* Radius */
  x[I_Radius    ] = r ;
  
  
  /* Element contents */
  x[I_N_S       ] = n_s_l  + n_s_s ;
  x[I_N_Ca      ] = n_ca_l + n_ca_s ;
  x[I_N_Si      ] = n_si_l + n_si_s ;
  x[I_N_K       ] = n_k_l  ;
  x[I_N_Cl      ] = n_cl_l  ;
  x[I_N_Al      ] = n_al_l + n_al_s ;

  /* Charge density */
  x[I_N_Q       ] = n_q_l ;
    

  /* Pressure */
  {
    double p_c = CrystallizationPressure(s_aft) ;
    
    x[I_P_C       ] = p_c ;
    x[I_ScPc      ] = s_c*p_c ;
  }
    
}



double* ComputeVariableDerivatives(Element_t* el,double t,double dt,double* x,double dxi,int i)
{
  double* dx = dVariables ;
  int j ;
  
  /* Primary Variables */
  for(j = 0 ; j < NbOfVariables ; j++) {
    dx[j] = x[j] ;
  }
  
  /* We increment the variable as (x + dx) */
  dx[i] += dxi ;
  
  ComputeSecondaryVariables(el,t,dt,dx) ;
  
  /* The numerical derivative as (f(x + dx) - f(x))/dx */
  for(j = 0 ; j < NbOfVariables ; j++) {
    dx[j] -= x[j] ;
    dx[j] /= dxi ;
  }

  return(dx) ;
}




double* ComputeVariableFluxes(Element_t* el,int ni,int nj)
{
  double* va = Element_GetExplicitTerm(el) ;


  /* Gradients */
  {
    double* g = CementSolutionDiffusion_GetGradient(csd) ;
    double* mui = CementSolutionDiffusion_GetPotentialAtPoint(csd,ni) ;
    double* muj = CementSolutionDiffusion_GetPotentialAtPoint(csd,nj) ;
    
    {
      int n = CementSolutionDiffusion_NbOfConcentrations ;
      int i ;
      
      for(i = 0 ; i < n ; i++) {
        double grad = muj[i] - mui[i] ;
      
        g[i] = TORTUOSITY * CONCENTRATION[i] * grad ;
      }
    }
  }

  
  /* Fluxes */
  {
    double* w = VariableFluxes[ni] ;
      
    CementSolutionDiffusion_ComputeFluxes(csd) ;
      
    w[I_W_Ca]  = CementSolutionDiffusion_GetElementFluxOf(csd,Ca) ;
    w[I_W_Si]  = CementSolutionDiffusion_GetElementFluxOf(csd,Si) ;
    w[I_W_S ]  = CementSolutionDiffusion_GetElementFluxOf(csd,S) ;
    w[I_W_K ]  = CementSolutionDiffusion_GetElementFluxOf(csd,K) ;
    w[I_W_Cl]  = CementSolutionDiffusion_GetElementFluxOf(csd,Cl) ;
    w[I_W_Al]  = CementSolutionDiffusion_GetElementFluxOf(csd,Al) ;
    w[I_W_q ]  = CementSolutionDiffusion_GetIonCurrent(csd) ;
    
    return(w) ;
  }
}



double* ComputeVariables2(Element_t* el,double** u,double** u_n,double* vim_n,double t,double dt,int p)
{
  IntFct_t* intfct = Element_GetIntFct(el) ;
  int dim = Element_GetDimensionOfSpace(el) ;
  FEM_t*    fem    = FEM_GetInstance(el) ;
  double*   y      = Variables2[p] ;
  
  {
    /* Primary Variables */
    {
      int i ;
    
      for(i = 0 ; i < dim ; i++) {
        y[U_Disp + i] = FEM_ComputeUnknown(fem,u,intfct,p,U_Disp + i) ;
      }
    
      for(i = dim ; i < 3 ; i++) {
        y[U_Disp + i] = 0 ;
      }
    }
    
    /* Strains */
    {
      double* eps =  FEM_ComputeLinearStrainTensor(fem,u,intfct,p,U_Disp) ;
      int i ;
  
      for(i = 0 ; i < 9 ; i++) {
        y[I_STRAIN + i] = eps[i] ;
      }
      
      FEM_FreeBufferFrom(fem,eps) ;
    }

  
    {
      double* x = Variables[0] ;
      double scpc = IntFct_InterpolateAtPoint(intfct,x+I_ScPc,NbOfVariables,p) ;
      
      y[I_PRESSURE] = scpc ;
    }
  }
  
  /* Needed variables to compute secondary components */
  {
    double* eps_n =  FEM_ComputeLinearStrainTensor(fem,u_n,intfct,p,U_Disp) ;
    double* f_n = vim_n ;
    double* sign = SIGn + 9*p ;
    
    {
      int i ;
  
      for(i = 0 ; i < 9 ; i++) {
        y[I_STRAIN_n + i] = eps_n[i] ;
        y[I_STRESS_n + i] = sign[i] ;
      }
      
      y[I_PRESSURE_n] = PRESSUREn[p] ;
    }
    

  }
    
  ComputeSecondaryVariables2(el,t,dt,y) ;
  
  return(y) ;
}



void  ComputeSecondaryVariables2(Element_t* el,double t,double dt,double* y)
{
  /* Strains */
  double* eps   =  y + I_STRAIN ;
  double* eps_n =  y + I_STRAIN_n ;
    
    
  /* Backup pressure */
    

  /* Backup stresses */
  {
    double* sig   = y + I_STRESS ;
    double* sig_n = y + I_STRESS_n ;
    double p   = y[I_PRESSURE] ;
    double p_n = y[I_PRESSURE_n] ;
    double dp  = p - p_n ;
    double  deps[9] ;
    int    i ;
    
    /* Incremental deformations */
    for(i = 0 ; i < 9 ; i++) deps[i] =  eps[i] - eps_n[i] ;
    
    /* Elastic stresses */
    for(i = 0 ; i < 9 ; i++) sig[i] = sig_n[i] ;
    
    #define C(i,j)  (cijkl[(i)*9+(j)])
    for(i = 0 ; i < 9 ; i++) {
      int  j ;
      
      for(j = 0 ; j < 9 ; j++) {
        sig[i] += C(i,j)*deps[j] ;
      }
    }
    #undef C
    
    sig[0] -= biot*dp ;
    sig[4] -= biot*dp ;
    sig[8] -= biot*dp ;
  }
}



double* ComputeVariableDerivatives2(Element_t* el,double t,double dt,double* y,double dyi,int i)
{
  double* dy = dVariables2 ;
  int j ;
  
  /* Primary Variables */
  for(j = 0 ; j < NbOfVariables2 ; j++) {
    dy[j] = y[j] ;
  }
  
  /* We increment the variable as (x + dx) */
  dy[i] += dyi ;
  
  ComputeSecondaryVariables2(el,t,dt,dy) ;
  
  /* The numerical derivative as (f(x + dx) - f(x))/dx */
  for(j = 0 ; j < NbOfVariables2 ; j++) {
    dy[j] -= y[j] ;
    dy[j] /= dyi ;
  }

  return(dy) ;
}



double TortuosityOhJang(double phi)
/** Liquid totuosity according to Oh and Jang, CCR2003 */
{
  double phi_cap = 0.5*phi  ;
  double phi_c   = 0.17 ;  /* Percolation capilar porosity */
    
  double n = 2.7 ; 		      /* OPC n = 2.7,        Fly ash n = 4.5 */
  double ds_norm = 1.e-4 ;	/* OPC ds_norm = 1e-4, Fly ash ds_norm = 5e-5 */
  double m_phi = 0.5*(pow(ds_norm,1/n) + phi_cap/(1-phi_c)*(1 - pow(ds_norm,1/n)) - phi_c/(1-phi_c)) ;
  double iff =  pow(m_phi + sqrt(m_phi*m_phi +  pow(ds_norm,1/n)*phi_c/(1-phi_c)),n) ;
    
  return(iff) ;
}



double TortuosityBazantNajjar(double phi)
/** Liquid totuosity according to Bazant and Najjar */
{
  double iff = (phi < 0.8) ? 2.9e-4*exp(9.95*phi) : phi ;
    
  return(iff) ;
}



double Radius(double r_n,double s_aft,double dt,Element_t* el)
{
  double r_max  = PoreEntryRadiusMax ;
  double r = r_n ;
  
  //if(s_aft < 1) return(r_max) ;
  
  {
    double s_ln   = LiquidSaturationDegree(r_n) ;
    double beta_n = EquilibriumAFtSaturationIndex(r_n) ;
    double beta_min = EquilibriumAFtSaturationIndex(r_max) ;
    double beta_inf = (s_aft > beta_min) ? s_aft : beta_min ;
    double r_inf = (s_aft > beta_min) ? InverseOfEquilibriumAFtSaturationIndex(s_aft) : r_max ;
    double s_linf  = LiquidSaturationDegree(r_inf) ;
    int iterations = 40 ;
    double tol = 1.e-6 ;
    int i ;
    
    if(r_n == r_inf) return(r_n) ;
    
    for(i = 0 ; i < iterations ; i++) {
      double s_l   = LiquidSaturationDegree(r) ;
      double ds_l  = dLiquidSaturationDegree(r) ;
      //double ds_l  = (s_linf - s_ln)/(r_inf - r_n) ;
      double beta  = EquilibriumAFtSaturationIndex(r) ;
      double dbeta = dEquilibriumAFtSaturationIndex(r) ;
      //double dbeta = (beta_inf - beta_n)/(r_inf - r_n) ;
      
      /* Kinetic law */
      /* Modified 03/06/2017 */
      double eq    = s_l - s_ln + dt*a_AFt*(1 - beta/s_aft) ;
      double deq   = ds_l - dt*a_AFt*dbeta/s_aft ;
      //double eq    = s_l - s_ln + dt*a_AFt*(s_aft/beta - 1) ;
      //double deq   = ds_l - dt*a_AFt*s_aft*dbeta/beta/beta ;
      
      double dr    = (fabs(deq) > 0) ? - eq/deq : 0. ;
      
      /* The solution r should be in the range between r_n and r_inf.
       * So let us assume that, at a given iteration, an estimation r
       * has been found between r_n and r_inf. Then we look for a new 
       * estimation r + dr in the range between r_0 and r_1 by using
       * an under-relaxation technique so that dr should be given by 
       *         r + dr = a*r_1 + (1 - a)*r_0
       * with a being in the range between 0 and 1.
       * The under-relaxation technique is such that r_0 should be in 
       * the range between r_n and r and r_1 should be in the range 
       * between r_inf and r i.e
       *         r_0 = b0*r_n   + (1 - b0)*r
       *         r_1 = b1*r_inf + (1 - b1)*r
       * with b0 and b1 being in between 0 and 1. So we get
       *         dr = a*b1*(r_inf - r) + (1 - a)*b0*(r_n - r)
       * If b0=b1=b then
       *         dr = (a*(r_inf - r) + (1 - a)*(r_n - r))*b
       * The bigger b the larger the range where r is looked for, without
       * exceding the initial range defined by r_n and r_inf.
       * The value b=0.5 corresponds to half the initial range.
       */
      {
        /* The predicted a is computed from the predicted dr */
        //double b = 0.5 ;
        double b = 0.5 ;
        double a = (dr/b - r_n + r)/(r_inf - r_n) ;
       
        /* If the predicted a is < 0 then the used value is set to 0 */
        if(a < 0) a = 0 ;
      
        /* if the predicted a is > 1 then the used value is set to 1 */
        if(a > 1) a = 1 ;
        
        {
          dr = b*(a*(r_inf - r) + (1 - a)*(r_n - r)) ;
        }
        
        /*
        printf("\n") ;
        printf("a      = %e\n",a) ;
        printf("s_ln   = %e, ",s_ln) ;
        printf("s_linf = %e, ",s_linf) ;
        printf("s_l    = %e\n",s_l) ;
        printf("ds_l   = %e, ",ds_l) ;
        printf("dbeta  = %e\n",dbeta) ;
        printf("r_n    = %e, ",r_n) ;
        printf("r_inf  = %e, ",r_inf) ;
        printf("r      = %e\n",r) ;
        */
      }
      
      r += dr ;
      if(fabs(dr/r_n) < tol) {
        return(r) ;
      }
    }
  }
  
  Message_Warning("Radius: not converged") ;
  Exception_Interrupt ;

  return(r) ;
}



double* IsotropicElasticTensor(double Young,double Poisson,double* c)
/** Compute the 4th rank isotropic elastic tensor in c.
 *  Return c  */
{
#define C(i,j,k,l)  (c[(((i)*3+(j))*3+(k))*3+(l)])
  double twomu   = Young/(1 + Poisson) ;
  double mu      = twomu/2 ;
  double lame    = twomu*Poisson/(1 - 2*Poisson) ;
   
  {
    int    i ;

    for(i = 0 ; i < 81 ; i++) c[i] = 0. ;
    
    for(i = 0 ; i < 3 ; i++) {
      int j ;
      
      for(j = 0 ; j < 3 ; j++) {
        C(i,i,j,j) += lame ;
        C(i,j,i,j) += mu ;
        C(i,j,j,i) += mu ;
      }
    }
  }
  
  return(c) ;
#undef C
}
