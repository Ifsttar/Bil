#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Message.h"
#include "Exception.h"
#include "Tools/Math.h"
#include "Curves.h"
#include "BilPath.h"
#include "Temperature.h"
#include "HardenedCementChemistry.h"
#include "CementSolutionChemistry.h"


#define DEBUG  0


//static double temperature = 293 ;

static HardenedCementChemistry_t* instancehcc = NULL ;


static void (HardenedCementChemistry_AllocateMemory)(HardenedCementChemistry_t*) ;

static void (HardenedCementChemistry_UpdateChemicalConstants)(HardenedCementChemistry_t*) ;




HardenedCementChemistry_t* HardenedCementChemistry_Create(void)
{
  HardenedCementChemistry_t* hcc = (HardenedCementChemistry_t*) malloc(sizeof(HardenedCementChemistry_t)) ;
  
  if(!hcc) arret("HardenedCementChemistry_Create") ;
  
  
  HardenedCementChemistry_AllocateMemory(hcc) ;
  
  
  {
    HardenedCementChemistry_GetTemperature(hcc) = Temperature_RoomValue ;
  }
  
  
  /* Initialize the constants */
  HardenedCementChemistry_UpdateChemicalConstants(hcc) ;
  
  
  
  
  /* Build the C-S-H curves (used by default) */
  {
    Curves_t* curves = HardenedCementChemistry_GetCSHCurves(hcc) ;
    
    /* The Ca/Si ratio (curve 0) */
        
    HardenedCementChemistry_SetDefaultCurveOfCalciumSiliconRatioInCSH(hcc) ;
    
    {
      char line[] = "Curves_log = " BIL_PATH "/src/Models/DataBases/HardenedCementChemistry.CalciumSiliconRatioInCSH s_ch = Range{x0 = 1.e-40 , x1 = 1 , n = 1001} x = Expressions(1){x1 = 0.88 ; n1 = 0.88 ; s1 = 1.87e-6 ; x2 = 0.98 ; n2 = 0.98 ; s2 = 6.9e-2 ; x = x1*(s_ch/s1)**n1/(1 + (s_ch/s1)**n1) + x2*(s_ch/s2)**n2/(1 + (s_ch/s2)**n2) ;}" ;
      
      Curves_ReadCurves(curves,line) ;
    }
    
    /* The water/Si ratio (curve 1) */

    HardenedCementChemistry_SetDefaultCurveOfWaterSiliconRatioInCSH(hcc) ;
    
    {
      char line[] = "Curves = " BIL_PATH "/src/Models/DataBases/HardenedCementChemistry.WaterSiliconRatioInCSH x = Range{x0 = 0 , x1 = 1.7 , n = 2} z = Expressions(1){z = 2.655733 ;}" ;
      
      Curves_ReadCurves(curves,line) ;
    }
    
    /* The saturation index of silica (curve 2) */

    HardenedCementChemistry_SetDefaultCurveOfSaturationIndexOfSH(hcc) ;
    
    {
      char line[] = "Curves_log = " BIL_PATH "/src/Models/DataBases/HardenedCementChemistry.SaturationIndexOfSH s_ch = Range{x0 = 1.e-15 , x1 = 1 , n = 751} s_sh = Expressions(1){x1 = 0.88 ; n1 = 0.88 ; s1 = 1.87e-6 ; x2 = 0.98 ; n2 = 0.98 ; s2 = 6.9e-2 ; s_sh = ((1 + (s_ch/s1)**n1)**(-x1/n1))*((1 + (s_ch/s2)**n2)**(-x2/n2)) ;}" ;
      
      Curves_ReadCurves(curves,line) ;
    }
  }
  
  return(hcc) ;
}



HardenedCementChemistry_t* HardenedCementChemistry_GetInstance(void)
{
  if(!instancehcc) {
    instancehcc = HardenedCementChemistry_Create() ;
  }
  
  return(instancehcc) ;
}




void HardenedCementChemistry_AllocateMemory(HardenedCementChemistry_t* hcc)
{
  /* Allocation of space for the primary variables */
  {
    size_t sz = HardenedCementChemistry_NbOfPrimaryVariables*sizeof(double) ;
    double* var = (double*) malloc(sz) ;
    
    if(!var) arret("HardenedCementChemistry_Create(1)") ;
    
    HardenedCementChemistry_GetPrimaryVariable(hcc) = var ;
  }
  
  
  /* Allocation of space for the variables */
  {
    size_t sz = HardenedCementChemistry_NbOfVariables*sizeof(double) ;
    double* var = (double*) malloc(sz) ;
    
    if(!var) arret("HardenedCementChemistry_Create(2)") ;
    
    HardenedCementChemistry_GetVariable(hcc) = var ;
  }
  
  
  /* Allocation of space for saturation indexes */
  {
    size_t sz = HardenedCementChemistry_NbOfSaturationIndexes*sizeof(double) ;
    double* sat = (double*) malloc(sz) ;
    
    if(!sat) arret("HardenedCementChemistry_Create(3)") ;
    
    HardenedCementChemistry_GetSaturationIndex(hcc) = sat ;
  }
  
  
  /* Allocation of space for Log10 saturation indexes */
  {
    size_t sz = HardenedCementChemistry_NbOfSaturationIndexes*sizeof(double) ;
    double* sat = (double*) malloc(sz) ;
    
    if(!sat) arret("HardenedCementChemistry_Create(4)") ;
    
    HardenedCementChemistry_GetLog10SaturationIndex(hcc) = sat ;
  }
  
  
  /* Allocation of space for the constants */
  {
    size_t sz = HardenedCementChemistry_NbOfConstants*sizeof(double) ;
    double* cst = (double*) malloc(sz) ;
    
    if(!cst) arret("HardenedCementChemistry_Create(5)") ;
    
    HardenedCementChemistry_GetConstant(hcc) = cst ;
  }
  
  
  /* Allocation of space for solubility product constants */
  {
    size_t sz = HardenedCementChemistry_NbOfSolubilityProductConstants*sizeof(double) ;
    double* ksp = (double*) malloc(sz) ;
    
    if(!ksp) arret("HardenedCementChemistry_Create(6)") ;
    
    HardenedCementChemistry_GetLog10Ksp(hcc) = ksp ;
  }
  
  
  /* Allocate space for CementSolutionChemistry */
  {
    CementSolutionChemistry_t* csc = CementSolutionChemistry_Create(1) ;
    
    HardenedCementChemistry_GetCementSolutionChemistry(hcc) = csc ;
    //HardenedCementChemistry_GetDefaultCementSolutionChemistry(hcc) = csc ;
  }
  
  
  /* Allocate for the CSH curves */
  {
    int nbofcurves = 3 ;
    Curves_t* curves = Curves_Create(nbofcurves) ;
      
    HardenedCementChemistry_GetCSHCurves(hcc) = curves ;
  }
}



#include "Log10EquilibriumConstantOfHomogeneousReactionInWater.h"
#include "Log10DissociationConstantOfCementHydrationProduct.h"
#include "Log10DissociationConstantOfCalciumCarbonate.h"


#define LogKsp(S)  HardenedCementChemistry_GetLog10SolubilityProductConstantOf(hcc,S)



void HardenedCementChemistry_UpdateChemicalConstants(HardenedCementChemistry_t* hcc)
{
  double T = HardenedCementChemistry_GetTemperature(hcc) ;
  
  /* Some solubility product constants */
  #define LogKd(R) Log10DissociationConstantOfCementHydrationProduct(R,T)
  double logk_ch       = LogKd(CH__Ca_2OH) ;
  double logk_sh       = LogKd(S_2H2O__H4SiO4) ;
  double logk_csh2     = LogKd(CSH2__Ca_SO4_2H2O) ;
  double logk_ah3      = LogKd(AH3__2Al_6OH) ;
  double logk_afm      = LogKd(AFm_12H__4Ca_2Al_SO4_18H2O) ;
  double logk_aft      = LogKd(AFt_12H__6Ca_2Al_3SO4_38H2O) ;
  double logk_c3ah6    = LogKd(C3AH6_12H__3Ca_2Al_12H2O) ;
  double logk_c2ah8    = LogKd(C2AH8__2Ca_2AlO4H4_2OH_3H2O) ;
  double logk_cah10    = LogKd(CAH10__Ca_2AlO4H4_6H2O) ;
  #undef LogKd
  
  #define LogKd(R) Log10DissociationConstantOfCalciumCarbonate(R,T)
  double logk_cc       = LogKd(Calcite__Ca_CO3) ;
  #undef LogKd
  
  
  /* Some other constants */
  double loga_h2o   = 0 ;
  #define LogKr(R) Log10EquilibriumConstantOfHomogeneousReactionInWater(R,T)
  double logk_h2o      = LogKr(H2O__H_OH) ;
  /* Carbon compounds */
  double logk_h2co3    = LogKr(H2CO3__CO2_H2O) ;
  double logk_hco3     = LogKr(HCO3_H2O__H2CO3_OH) ;
  double logk_co3      = LogKr(CO3_H2O__HCO3_OH) ;
  /* Sulfur compounds */
  double logk_h2so4    = LogKr(H2SO4__HSO4_H) ;
  double logk_hso4     = LogKr(HSO4__SO4_H) ;
  #undef LogKr

  double loga_co2eq = logk_cc - logk_ch + logk_co3 + logk_hco3 + logk_h2co3 + loga_h2o ;
  
  double loga_h2so4eq = logk_csh2 - logk_ch + 2*logk_h2o - logk_hso4 - logk_h2so4 ;
  
  
  /* Backup */
  LogKsp(CH)    = logk_ch ;
  LogKsp(SH)    = logk_sh ;
  LogKsp(CC)    = logk_cc ;
  LogKsp(CSH2)  = logk_csh2 ;
  LogKsp(AH3)   = logk_ah3 ;
  LogKsp(AFm)   = logk_afm ;
  LogKsp(AFt)   = logk_aft ;
  LogKsp(C3AH6) = logk_c3ah6 ;
  LogKsp(C2AH8) = logk_c2ah8 ;
  LogKsp(CAH10) = logk_cah10 ;
  
  
  HardenedCementChemistry_GetLog10EquilibriumCO2Activity(hcc) = loga_co2eq ;
  HardenedCementChemistry_GetLog10EquilibriumH2SO4Activity(hcc) = loga_h2so4eq ;
}



void HardenedCementChemistry_PrintChemicalConstants(HardenedCementChemistry_t* hcc)
{
  double T = HardenedCementChemistry_GetTemperature(hcc) ;
  
    
  Log10DissociationConstantOfCementHydrationProduct_Print(T) ;
  Log10DissociationConstantOfCalciumCarbonate_Print(T) ;
  
  printf("\n") ;
  
  {
    double loga_co2eq = HardenedCementChemistry_GetLog10EquilibriumCO2Activity(hcc) ;
    double loga_h2so4eq = HardenedCementChemistry_GetLog10EquilibriumH2SO4Activity(hcc) ;
  
    printf("Other constants\n") ;
    printf("---------------\n") ;
    printf("Log(a_co2eq)        = %e",loga_co2eq) ;
    printf("\n") ;
    printf("Log(a_h2so4eq)      = %e",loga_h2so4eq) ;
  }
  
  printf("\n") ;
  
  fflush(stdout) ;
}



/* C-S-H Properties (Calcium-Silicate-Hydrates) */
#define CalciumSiliconRatioCurve \
        HardenedCementChemistry_GetCurveOfCalciumSiliconRatioInCSH(hcc)
        
#define WaterSiliconRatioCurve \
        HardenedCementChemistry_GetCurveOfWaterSiliconRatioInCSH(hcc)
        
#define CalciumSiliconRatioInCSH(s_ch) \
        (Curve_ComputeValue(CalciumSiliconRatioCurve,s_ch))

#define WaterSiliconRatioInCSH(x) \
        (Curve_ComputeValue(WaterSiliconRatioCurve,x))



/* Properties of the phase diagram of CaO-H2O */
/* si_ca should be the saturation index of CH (Portlandite) */
#define Log10SaturationIndexOfCH(si_ca)  (si_ca)


/* Properties of the phase diagram of CaO-CO2-H2O */
/* CO2 activity at the invariant point of the phase diagram */
#define Log10a_CO2eq   (HardenedCementChemistry_GetLog10EquilibriumCO2Activity(hcc))
#define Log10S_CH_CO2eq(loga_co2)       MIN(Log10a_CO2eq - loga_co2,0)
/* Saturation Index of Dissolved CH */
/* si_ca should be the saturation index of the system CH-CC */
#define Log10SaturationIndexOfCH_CO2(si_ca,loga_co2)   ((si_ca) + Log10S_CH_CO2eq(loga_co2))


/* Properties of the phase diagram of CaO-SO3-H2O */
/* H2SO4 activity at the invariant point of the phase diagram */
#define Log10a_H2SO4eq    (HardenedCementChemistry_GetLog10EquilibriumH2SO4Activity(hcc))
#define Log10S_CH_H2SO4eq(loga_h2so4)       MIN(Log10a_H2SO4eq - loga_h2so4,0)
/* Saturation Index of Dissolved CH */
/* si_ca should be the saturation index of the system CH-CSH2 */
#define Log10SaturationIndexOfCH_H2SO4(si_ca,loga_h2so4)  ((si_ca) + Log10S_CH_H2SO4eq(loga_h2so4))



/* Properties of the phase diagram of CaO-SiO2-H2O (C-S-H) */
#define SaturationIndexOfSHCurve \
        HardenedCementChemistry_GetCurveOfSaturationIndexOfSH(hcc)
        
#define Log10SaturationIndexOfCSH(si_si)        (si_si)
#define S_SHeq(s_ch)         (Curve_ComputeValue(SaturationIndexOfSHCurve,s_ch))
#define Log10S_SHeq(s_ch)    (log10(S_SHeq(s_ch)))
/* Saturation index of dissolved S-H (Silica gel) */
/* si_si should be the saturation index of the system C-S-H */
#define Log10SaturationIndexOfSH(si_si,s_ch)    ((si_si) + Log10S_SHeq(s_ch))


/* CC Properties (Calcite) */
/* Saturation Index of Dissolved CC */
//#define Log10SaturationIndexOfCC(logs_ch,loga_co2)      (logs_ch + loga_co2 - Log10a_CO2eq)


/* CSH2 Properties (Gypsum) */
/* Saturation Index of Dissolved CSH2 */
//#define Log10SaturationIndexOfCSH2(logs_ch,loga_h2so4)    (logs_ch + loga_h2so4 - Log10a_H2SO4eq)


/* Properties of the phase diagram of Al2O3-H2O */
/* si_al should be the saturation index of AH3 (Gibbsite) */
#define Log10SaturationIndexOfAH3(si_al)  (si_al)


/* AFm Properties (Monofulfoaluminate) */
/* Saturation Index of Dissolved AFm */
//#define Log10SaturationIndexOfAFm()  (xx)





#define Input(U)            HardenedCementChemistry_GetInput(hcc,U)




void HardenedCementChemistry_ComputeSystem_CaO_SiO2_Na2O_K2O_CO2_H2O(HardenedCementChemistry_t* hcc)
{
  /* Inputs */
  /* SI_Ca is the saturation index of CH-CC ie log(S_CH/S_CHeq) */ 
  double si_ca    = Input(SI_Ca) ;
  /* SI_Si is the saturation index of C-S-H ie log(S_SH/S_SHeq) */ 
  double si_si    = Input(SI_Si) ;
  double loga_co2 = Input(LogA_CO2) ;
  double loga_na  = Input(LogA_Na) ;
  double loga_k   = Input(LogA_K) ;
  double loga_oh  = Input(LogA_OH) ;
  
  
  /* The saturation indexes of CH, SH */
  double logs_ch    = Log10SaturationIndexOfCH_CO2(si_ca,loga_co2) ;
  double s_ch       = pow(10,logs_ch) ;
  double logs_sh    = Log10SaturationIndexOfSH(si_si,s_ch) ;
  double s_sh       = pow(10,logs_sh) ;
  
  
  /* The IAP of CH, SH */
  double logk_ch    = LogKsp(CH) ;
  double logq_ch    = logs_ch + logk_ch ;
  double logk_sh    = LogKsp(SH) ;
  double logq_sh    = logs_sh + logk_sh ;
  
  
  /* Solve chemistry in solution */
  {
    CementSolutionChemistry_t* csc = HardenedCementChemistry_GetCementSolutionChemistry(hcc) ;
  
    CementSolutionChemistry_GetInput(csc,LogQ_CH)  = logq_ch ;
    CementSolutionChemistry_GetInput(csc,LogQ_SH)  = logq_sh ;
    CementSolutionChemistry_GetInput(csc,LogA_CO2) = loga_co2 ;
    CementSolutionChemistry_GetInput(csc,LogA_Na)  = loga_na ;
    CementSolutionChemistry_GetInput(csc,LogA_K)   = loga_k ;
    CementSolutionChemistry_GetInput(csc,LogA_OH)  = loga_oh ;
  
    CementSolutionChemistry_ComputeSystem(csc,CaO_SiO2_Na2O_K2O_CO2_H2O) ;

    CementSolutionChemistry_UpdateSolution(csc) ;
  }
  
  
  /* Backup */
  
  /* Saturation indexes of solid phases */
  HardenedCementChemistry_GetSaturationIndexOf(hcc,CH) = s_ch ;
  HardenedCementChemistry_GetSaturationIndexOf(hcc,SH) = s_sh ;
  
  /* Log10 saturation indexes of solid phases */
  HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,CH) = logs_ch ;
  HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,SH) = logs_sh ;
  
  {
    CementSolutionChemistry_t* csc = HardenedCementChemistry_GetCementSolutionChemistry(hcc) ;
    double loga_ca  = CementSolutionChemistry_GetLogActivityOf(csc,Ca) ;
    double loga_co3 = CementSolutionChemistry_GetLogActivityOf(csc,CO3) ;
    double logk_cc  = LogKsp(CC) ;
    double logs_cc  = loga_ca + loga_co3 - logk_cc ;
    double s_cc     = pow(10,logs_cc) ;
    
    HardenedCementChemistry_GetSaturationIndexOf(hcc,CC) = s_cc ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,CC) = logs_cc ;
  }
  
  /* CSH properties */
  {
    double x_csh = CalciumSiliconRatioInCSH(s_ch) ;
    double z_csh = WaterSiliconRatioInCSH(x_csh) ;
    
    HardenedCementChemistry_GetCalciumSiliconRatioInCSH(hcc) = x_csh ;
    HardenedCementChemistry_GetWaterSiliconRatioInCSH(hcc) = z_csh ;
  }
}



void HardenedCementChemistry_ComputeSystem_CaO_SiO2_Na2O_K2O_SO3_H2O(HardenedCementChemistry_t* hcc)
{
  /* Inputs */
  /* SI_Ca is the saturation index of CH-CSH2 ie log(S_CH/S_CHeq) */ 
  double si_ca      = Input(SI_Ca) ;
  /* SI_Si is the saturation index of C-S-H ie log(S_SH/S_SHeq) */ 
  double si_si      = Input(SI_Si) ;
  double loga_h2so4 = Input(LogA_H2SO4) ;
  double loga_na    = Input(LogA_Na) ;
  double loga_k     = Input(LogA_K) ;
  double loga_oh    = Input(LogA_OH) ;
  
  
  /* The saturation indexes of CH, SH */
  double logs_ch    = Log10SaturationIndexOfCH_H2SO4(si_ca,loga_h2so4) ;
  double s_ch       = pow(10,logs_ch) ;
  double logs_sh    = Log10SaturationIndexOfSH(si_si,s_ch) ;
  double s_sh       = pow(10,logs_sh) ;
  
  
  /* The IAP of CH, SH */
  double logk_ch    = LogKsp(CH) ;
  double logq_ch    = logs_ch + logk_ch ;
  double logk_sh    = LogKsp(SH) ;
  double logq_sh    = logs_sh + logk_sh ;
  
  /* Solve chemistry in solution */
  {
    CementSolutionChemistry_t* csc = HardenedCementChemistry_GetCementSolutionChemistry(hcc) ;
  
    CementSolutionChemistry_GetInput(csc,LogQ_CH)    = logq_ch ;
    CementSolutionChemistry_GetInput(csc,LogQ_SH)    = logq_sh ;
    CementSolutionChemistry_GetInput(csc,LogA_H2SO4) = loga_h2so4 ;
    CementSolutionChemistry_GetInput(csc,LogA_Na)    = loga_na ;
    CementSolutionChemistry_GetInput(csc,LogA_K)     = loga_k ;
    CementSolutionChemistry_GetInput(csc,LogA_OH)    = loga_oh ;
  
    CementSolutionChemistry_ComputeSystem(csc,CaO_SiO2_Na2O_K2O_SO3_H2O) ;

    CementSolutionChemistry_UpdateSolution(csc) ;
  }
  
  
  /* Backup */
  
  /* Saturation indexes of solid phases */
  HardenedCementChemistry_GetSaturationIndexOf(hcc,CH)   = s_ch ;
  HardenedCementChemistry_GetSaturationIndexOf(hcc,SH)   = s_sh ;
  
  /* Log10 saturation indexes of solid phases */
  HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,CH) = logs_ch ;
  HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,SH) = logs_sh ;
  
  {
    CementSolutionChemistry_t* csc = HardenedCementChemistry_GetCementSolutionChemistry(hcc) ;
    double loga_ca   = CementSolutionChemistry_GetLogActivityOf(csc,Ca) ;
    double loga_so4  = CementSolutionChemistry_GetLogActivityOf(csc,SO4) ;
    double loga_h2o  = 0 ;
    double logk_csh2 = LogKsp(CSH2) ;
    double logs_csh2 = loga_ca + loga_so4 + 2*loga_h2o - logk_csh2 ;
    double s_csh2    = pow(10,logs_csh2) ;
    
    HardenedCementChemistry_GetSaturationIndexOf(hcc,CSH2) = s_csh2 ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,CSH2) = logs_csh2 ;
  }
  
  /* CSH properties */
  {
    double x_csh = CalciumSiliconRatioInCSH(s_ch) ;
    double z_csh = WaterSiliconRatioInCSH(x_csh) ;
    
    HardenedCementChemistry_GetCalciumSiliconRatioInCSH(hcc) = x_csh ;
    HardenedCementChemistry_GetWaterSiliconRatioInCSH(hcc) = z_csh ;
  }
}



void HardenedCementChemistry_ComputeSystem_CaO_SiO2_Na2O_K2O_SO3_H2O_2(HardenedCementChemistry_t* hcc)
{
  /* Inputs */
  /* SI_Ca is the saturation index of calcium hydroxide ie log(S_CH) */ 
  double si_ca      = Input(SI_Ca) ;
  /* SI_Si is the saturation index of C-S-H ie log(S_SH/S_SHeq) */ 
  double si_si      = Input(SI_Si) ;
  double loga_h2so4 = Input(LogA_H2SO4) ;
  double loga_na    = Input(LogA_Na) ;
  double loga_k     = Input(LogA_K) ;
  double loga_oh    = Input(LogA_OH) ;
  
  
  /* The saturation indexes of CH, SH */
  double logs_ch    = Log10SaturationIndexOfCH(si_ca) ;
  double s_ch       = pow(10,logs_ch) ;
  double logs_sh    = Log10SaturationIndexOfSH(si_si,s_ch) ;
  double s_sh       = pow(10,logs_sh) ;
  
  
  /* The IAP of CH, SH */
  double logk_ch    = LogKsp(CH) ;
  double logq_ch    = logs_ch + logk_ch ;
  double logk_sh    = LogKsp(SH) ;
  double logq_sh    = logs_sh + logk_sh ;
  
  /* Solve chemistry in solution */
  {
    CementSolutionChemistry_t* csc = HardenedCementChemistry_GetCementSolutionChemistry(hcc) ;
  
    CementSolutionChemistry_GetInput(csc,LogQ_CH)    = logq_ch ;
    CementSolutionChemistry_GetInput(csc,LogQ_SH)    = logq_sh ;
    CementSolutionChemistry_GetInput(csc,LogA_H2SO4) = loga_h2so4 ;
    CementSolutionChemistry_GetInput(csc,LogA_Na)    = loga_na ;
    CementSolutionChemistry_GetInput(csc,LogA_K)     = loga_k ;
    CementSolutionChemistry_GetInput(csc,LogA_OH)    = loga_oh ;
  
    CementSolutionChemistry_ComputeSystem(csc,CaO_SiO2_Na2O_K2O_SO3_H2O) ;

    CementSolutionChemistry_UpdateSolution(csc) ;
  }
  
  
  /* Backup */
  
  /* Saturation indexes of solid phases */
  HardenedCementChemistry_GetSaturationIndexOf(hcc,CH)   = s_ch ;
  HardenedCementChemistry_GetSaturationIndexOf(hcc,SH)   = s_sh ;
  
  /* Log10 saturation indexes of solid phases */
  HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,CH) = logs_ch ;
  HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,SH) = logs_sh ;
  
  {
    CementSolutionChemistry_t* csc = HardenedCementChemistry_GetCementSolutionChemistry(hcc) ;
    double loga_ca   = CementSolutionChemistry_GetLogActivityOf(csc,Ca) ;
    double loga_so4  = CementSolutionChemistry_GetLogActivityOf(csc,SO4) ;
    double loga_h2o  = 0 ;
    double logk_csh2 = LogKsp(CSH2) ;
    double logs_csh2 = loga_ca + loga_so4 + 2*loga_h2o - logk_csh2 ;
    double s_csh2    = pow(10,logs_csh2) ;
    
    HardenedCementChemistry_GetSaturationIndexOf(hcc,CSH2) = s_csh2 ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,CSH2) = logs_csh2 ;
  }
  
  /* CSH properties */
  {
    double x_csh = CalciumSiliconRatioInCSH(s_ch) ;
    double z_csh = WaterSiliconRatioInCSH(x_csh) ;
    
    HardenedCementChemistry_GetCalciumSiliconRatioInCSH(hcc) = x_csh ;
    HardenedCementChemistry_GetWaterSiliconRatioInCSH(hcc) = z_csh ;
  }
}



void HardenedCementChemistry_ComputeSystem_CaO_SiO2_Na2O_K2O_SO3_Al2O3_H2O(HardenedCementChemistry_t* hcc)
{
  /* Inputs */
  /* SI_Ca is the saturation index of CH-CSH2 ie log(S_CH/S_CHeq) */ 
  double si_ca      = Input(SI_Ca) ;
  /* SI_Si is the saturation index of C-S-H ie log(S_SH/S_SHeq) */ 
  double si_si      = Input(SI_Si) ;
  /* SI_Al is the saturation index of AH3 ie log(S_AH3) */ 
  double si_al      = Input(SI_Al) ;
  double loga_h2so4 = Input(LogA_H2SO4) ;
  double loga_na    = Input(LogA_Na) ;
  double loga_k     = Input(LogA_K) ;
  double loga_oh    = Input(LogA_OH) ;
  
  
  /* The saturation indexes of CH, SH, AH3 */
  double logs_ch    = Log10SaturationIndexOfCH_H2SO4(si_ca,loga_h2so4) ;
  double s_ch       = pow(10,logs_ch) ;
  double logs_sh    = Log10SaturationIndexOfSH(si_si,s_ch) ;
  double s_sh       = pow(10,logs_sh) ;
  double logs_ah3   = Log10SaturationIndexOfAH3(si_al) ;
  double s_ah3      = pow(10,logs_ah3) ;
  
  
  /* The IAP of CH, SH, AH3 */
  double logk_ch    = LogKsp(CH) ;
  double logq_ch    = logs_ch + logk_ch ;
  double logk_sh    = LogKsp(SH) ;
  double logq_sh    = logs_sh + logk_sh ;
  double logk_ah3   = LogKsp(AH3) ;
  double logq_ah3   = logs_ah3 + logk_ah3 ;
  
  /* Solve chemistry in solution */
  {
    CementSolutionChemistry_t* csc = HardenedCementChemistry_GetCementSolutionChemistry(hcc) ;
  
    CementSolutionChemistry_GetInput(csc,LogQ_CH)    = logq_ch ;
    CementSolutionChemistry_GetInput(csc,LogQ_SH)    = logq_sh ;
    CementSolutionChemistry_GetInput(csc,LogA_H2SO4) = loga_h2so4 ;
    CementSolutionChemistry_GetInput(csc,LogA_Na)    = loga_na ;
    CementSolutionChemistry_GetInput(csc,LogA_K)     = loga_k ;
    CementSolutionChemistry_GetInput(csc,LogA_OH)    = loga_oh ;
    CementSolutionChemistry_GetInput(csc,LogQ_AH3)   = logq_ah3 ;
  
    CementSolutionChemistry_ComputeSystem(csc,CaO_SiO2_Na2O_K2O_SO3_Al2O3_H2O) ;

    CementSolutionChemistry_UpdateSolution(csc) ;
  }
  
  
  /* Backup */
  
  /* Saturation indexes of solid phases */
  HardenedCementChemistry_GetSaturationIndexOf(hcc,CH)   = s_ch ;
  HardenedCementChemistry_GetSaturationIndexOf(hcc,SH)   = s_sh ;
  HardenedCementChemistry_GetSaturationIndexOf(hcc,AH3)  = s_ah3 ;
  
  /* Log10 saturation indexes of solid phases */
  HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,CH)  = logs_ch ;
  HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,SH)  = logs_sh ;
  HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,AH3) = logs_ah3 ;
  
  {
    CementSolutionChemistry_t* csc = HardenedCementChemistry_GetCementSolutionChemistry(hcc) ;
    double loga_ca     = CementSolutionChemistry_GetLogActivityOf(csc,Ca) ;
    double loga_so4    = CementSolutionChemistry_GetLogActivityOf(csc,SO4) ;
    double loga_al     = CementSolutionChemistry_GetLogActivityOf(csc,Al) ;
    double loga_alo4h4 = CementSolutionChemistry_GetLogActivityOf(csc,AlO4H4) ;
    double loga_h      = CementSolutionChemistry_GetLogActivityOf(csc,H) ;
    double loga_h2o    = CementSolutionChemistry_GetLogActivityOf(csc,H2O) ;
    
    double logq_csh2  = loga_ca + loga_so4 + 2*loga_h2o ;
    double logk_csh2  = LogKsp(CSH2) ;
    double logs_csh2  = logq_csh2 - logk_csh2 ;
    double s_csh2     = pow(10,logs_csh2) ;
    
    double logq_afm   = 4*loga_ca + 2*loga_al + loga_so4 + 18*loga_h2o - 12*loga_h ;
    double logk_afm   = LogKsp(AFm) ;
    double logs_afm   = logq_afm - logk_afm ;
    double s_afm      = pow(10,logs_afm) ;
    
    double logq_aft   = 6*loga_ca + 2*loga_al + 3*loga_so4 + 38*loga_h2o - 12*loga_h ;
    double logk_aft   = LogKsp(AFt) ;
    double logs_aft   = logq_aft - logk_aft ;
    double s_aft      = pow(10,logs_aft) ;
    
    double logq_c3ah6 = 3*loga_ca + 2*loga_al + 12*loga_h2o - 12*loga_h ;
    double logk_c3ah6 = LogKsp(C3AH6) ;
    double logs_c3ah6 = logq_c3ah6 - logk_c3ah6 ;
    double s_c3ah6    = pow(10,logs_c3ah6) ;
    
    double logq_c2ah8 = 2*loga_ca + 2*loga_alo4h4 + 2*loga_oh + 3*loga_h2o ;
    double logk_c2ah8 = LogKsp(C2AH8) ;
    double logs_c2ah8 = logq_c2ah8 - logk_c2ah8 ;
    double s_c2ah8    = pow(10,logs_c2ah8) ;
    
    double logq_cah10 =   loga_ca + 2*loga_alo4h4 + 6*loga_h2o ;
    double logk_cah10 = LogKsp(CAH10) ;
    double logs_cah10 = logq_cah10 - logk_cah10 ;
    double s_cah10    = pow(10,logs_cah10) ;
    
    HardenedCementChemistry_GetSaturationIndexOf(hcc,CSH2)  = s_csh2 ;
    HardenedCementChemistry_GetSaturationIndexOf(hcc,AFm)   = s_afm ;
    HardenedCementChemistry_GetSaturationIndexOf(hcc,AFt)   = s_aft ;
    HardenedCementChemistry_GetSaturationIndexOf(hcc,C3AH6) = s_c3ah6 ;
    HardenedCementChemistry_GetSaturationIndexOf(hcc,C2AH8) = s_c2ah8 ;
    HardenedCementChemistry_GetSaturationIndexOf(hcc,CAH10) = s_cah10 ;
    
    
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,CSH2)  = logs_csh2 ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,AFm)   = logs_afm ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,AFt)   = logs_aft ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,C3AH6) = logs_c3ah6 ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,C2AH8) = logs_c2ah8 ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,CAH10) = logs_cah10 ;
  }
  
  /* CSH properties */
  {
    double x_csh = CalciumSiliconRatioInCSH(s_ch) ;
    double z_csh = WaterSiliconRatioInCSH(x_csh) ;
    
    HardenedCementChemistry_GetCalciumSiliconRatioInCSH(hcc) = x_csh ;
    HardenedCementChemistry_GetWaterSiliconRatioInCSH(hcc) = z_csh ;
  }
}



void HardenedCementChemistry_ComputeSystem_CaO_SiO2_Na2O_K2O_SO3_Al2O3_H2O_2(HardenedCementChemistry_t* hcc)
{
  /* Inputs */
  /* SI_Ca is the saturation index of CH ie log(S_CH) */ 
  double si_ca      = Input(SI_Ca) ;
  /* SI_Si is the saturation index of C-S-H ie log(S_SH/S_SHeq) */ 
  double si_si      = Input(SI_Si) ;
  /* SI_Al is the saturation index of AH3 ie log(S_AH3) */ 
  double si_al      = Input(SI_Al) ;
  double loga_h2so4 = Input(LogA_H2SO4) ;
  double loga_na    = Input(LogA_Na) ;
  double loga_k     = Input(LogA_K) ;
  double loga_oh    = Input(LogA_OH) ;
  
  
  /* The saturation indexes of CH, SH, AH3 */
  double logs_ch    = Log10SaturationIndexOfCH(si_ca) ;
  double s_ch       = pow(10,logs_ch) ;
  double logs_sh    = Log10SaturationIndexOfSH(si_si,s_ch) ;
  double s_sh       = pow(10,logs_sh) ;
  double logs_ah3   = Log10SaturationIndexOfAH3(si_al) ;
  double s_ah3      = pow(10,logs_ah3) ;
  
  
  /* The IAP of CH, SH, AH3 */
  double logk_ch    = LogKsp(CH) ;
  double logq_ch    = logs_ch + logk_ch ;
  double logk_sh    = LogKsp(SH) ;
  double logq_sh    = logs_sh + logk_sh ;
  double logk_ah3   = LogKsp(AH3) ;
  double logq_ah3   = logs_ah3 + logk_ah3 ;
  
  /* Solve chemistry in solution */
  {
    CementSolutionChemistry_t* csc = HardenedCementChemistry_GetCementSolutionChemistry(hcc) ;
  
    CementSolutionChemistry_GetInput(csc,LogQ_CH)    = logq_ch ;
    CementSolutionChemistry_GetInput(csc,LogQ_SH)    = logq_sh ;
    CementSolutionChemistry_GetInput(csc,LogA_H2SO4) = loga_h2so4 ;
    CementSolutionChemistry_GetInput(csc,LogA_Na)    = loga_na ;
    CementSolutionChemistry_GetInput(csc,LogA_K)     = loga_k ;
    CementSolutionChemistry_GetInput(csc,LogA_OH)    = loga_oh ;
    CementSolutionChemistry_GetInput(csc,LogQ_AH3)   = logq_ah3 ;
  
    CementSolutionChemistry_ComputeSystem(csc,CaO_SiO2_Na2O_K2O_SO3_Al2O3_H2O) ;

    CementSolutionChemistry_UpdateSolution(csc) ;
  }
  
  
  /* Backup */
  
  /* Saturation indexes of solid phases */
  HardenedCementChemistry_GetSaturationIndexOf(hcc,CH)   = s_ch ;
  HardenedCementChemistry_GetSaturationIndexOf(hcc,SH)   = s_sh ;
  HardenedCementChemistry_GetSaturationIndexOf(hcc,AH3)  = s_ah3 ;
  
  /* Log10 saturation indexes of solid phases */
  HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,CH)  = logs_ch ;
  HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,SH)  = logs_sh ;
  HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,AH3) = logs_ah3 ;
  
  {
    CementSolutionChemistry_t* csc = HardenedCementChemistry_GetCementSolutionChemistry(hcc) ;
    double loga_ca     = CementSolutionChemistry_GetLogActivityOf(csc,Ca) ;
    double loga_so4    = CementSolutionChemistry_GetLogActivityOf(csc,SO4) ;
    double loga_al     = CementSolutionChemistry_GetLogActivityOf(csc,Al) ;
    double loga_alo4h4 = CementSolutionChemistry_GetLogActivityOf(csc,AlO4H4) ;
    double loga_h      = CementSolutionChemistry_GetLogActivityOf(csc,H) ;
    double loga_h2o    = CementSolutionChemistry_GetLogActivityOf(csc,H2O) ;
    
    double logq_csh2  = loga_ca + loga_so4 + 2*loga_h2o ;
    double logk_csh2  = LogKsp(CSH2) ;
    double logs_csh2  = logq_csh2 - logk_csh2 ;
    double s_csh2     = pow(10,logs_csh2) ;
    
    double logq_afm   = 4*loga_ca + 2*loga_al + loga_so4 + 18*loga_h2o - 12*loga_h ;
    double logk_afm   = LogKsp(AFm) ;
    double logs_afm   = logq_afm - logk_afm ;
    double s_afm      = pow(10,logs_afm) ;
    
    double logq_aft   = 6*loga_ca + 2*loga_al + 3*loga_so4 + 38*loga_h2o - 12*loga_h ;
    double logk_aft   = LogKsp(AFt) ;
    double logs_aft   = logq_aft - logk_aft ;
    double s_aft      = pow(10,logs_aft) ;
    
    double logq_c3ah6 = 3*loga_ca + 2*loga_al + 12*loga_h2o - 12*loga_h ;
    double logk_c3ah6 = LogKsp(C3AH6) ;
    double logs_c3ah6 = logq_c3ah6 - logk_c3ah6 ;
    double s_c3ah6    = pow(10,logs_c3ah6) ;
    
    double logq_c2ah8 = 2*loga_ca + 2*loga_alo4h4 + 2*loga_oh + 3*loga_h2o ;
    double logk_c2ah8 = LogKsp(C2AH8) ;
    double logs_c2ah8 = logq_c2ah8 - logk_c2ah8 ;
    double s_c2ah8    = pow(10,logs_c2ah8) ;
    
    double logq_cah10 =   loga_ca + 2*loga_alo4h4 + 6*loga_h2o ;
    double logk_cah10 = LogKsp(CAH10) ;
    double logs_cah10 = logq_cah10 - logk_cah10 ;
    double s_cah10    = pow(10,logs_cah10) ;
    
    HardenedCementChemistry_GetSaturationIndexOf(hcc,CSH2)  = s_csh2 ;
    HardenedCementChemistry_GetSaturationIndexOf(hcc,AFm)   = s_afm ;
    HardenedCementChemistry_GetSaturationIndexOf(hcc,AFt)   = s_aft ;
    HardenedCementChemistry_GetSaturationIndexOf(hcc,C3AH6) = s_c3ah6 ;
    HardenedCementChemistry_GetSaturationIndexOf(hcc,C2AH8) = s_c2ah8 ;
    HardenedCementChemistry_GetSaturationIndexOf(hcc,CAH10) = s_cah10 ;
    
    
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,CSH2)  = logs_csh2 ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,AFm)   = logs_afm ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,AFt)   = logs_aft ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,C3AH6) = logs_c3ah6 ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,C2AH8) = logs_c2ah8 ;
    HardenedCementChemistry_GetLog10SaturationIndexOf(hcc,CAH10) = logs_cah10 ;
  }
  
  /* CSH properties */
  {
    double x_csh = CalciumSiliconRatioInCSH(s_ch) ;
    double z_csh = WaterSiliconRatioInCSH(x_csh) ;
    
    HardenedCementChemistry_GetCalciumSiliconRatioInCSH(hcc) = x_csh ;
    HardenedCementChemistry_GetWaterSiliconRatioInCSH(hcc) = z_csh ;
  }
}



#define TEST   0

#if TEST

/*
 * gcc 
 */

int main(int argc, char** argv)
{
  double T = 293 ;
  HardenedCementChemistry_t* hcc = HardenedCementChemistry_Create(T) ;
}

#endif
