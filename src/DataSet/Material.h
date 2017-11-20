#ifndef MATERIAL_H
#define MATERIAL_H


/* vacuous declarations and typedef names */

/* class-like structures */
struct Material_s     ; typedef struct Material_s     Material_t ;


#include "DataFile.h"
#include <stdlib.h>
#include "Model.h"


extern Material_t* (Material_Create)(int) ;
extern int         (Material_ReadProperties)(Material_t*,DataFile_t*) ;
//extern void        (Material_ScanProperties)(Material_t*,DataFile_t*, int (*)(const char*)) ;
extern void        (Material_ScanProperties)(Material_t*,DataFile_t*,Model_ComputePropertyIndex_t*) ;
//extern void        (Material_ScanProperties1)(Material_t*,FILE*,int (*)(const char*),int) ;
extern void        (Material_ScanProperties1)(Material_t*,FILE*,Model_ComputePropertyIndex_t*,int) ;
//extern void        (Material_ScanProperties2)(Material_t*,FILE*,int (*)(const char*),int,int) ;
extern void        (Material_ScanProperties2)(Material_t*,FILE*,Model_ComputePropertyIndex_t*,int,int) ;


#define Material_MaxLengthOfKeyWord            (30)
#define Material_MaxLengthOfTextLine           (500)

#define Material_MaxNbOfCurves                 (20)     /* Max nb of curves per mat */
#define Material_MaxNbOfProperties             (100)    /* Max nb of scalar inputs */


#define Material_GetNbOfProperties(MAT)   ((MAT)->n)
#define Material_GetProperty(MAT)         ((MAT)->pr)
#define Material_GetCurves(MAT)           ((MAT)->curves)
#define Material_GetFields(MAT)           ((MAT)->fields)
#define Material_GetFunctions(MAT)        ((MAT)->functions)
#define Material_GetModel(MAT)            ((MAT)->model)
#define Material_GetMethod(MAT)           ((MAT)->method)
#define Material_GetCodeNameOfModel(MAT)  ((MAT)->codenameofmodel)


/* Material properties */
#define Material_GetNbOfCurves(MAT) \
        Curves_GetNbOfCurves(Material_GetCurves(MAT))

#define Material_GetCurve(MAT) \
        Curves_GetCurve(Material_GetCurves(MAT))

#define Material_GetNbOfFields(MAT) \
        Fields_GetNbOfFields(Material_GetFields(MAT))

#define Material_GetField(MAT) \
        Fields_GetField(Material_GetFields(MAT))

#define Material_GetNbOfFunctions(MAT) \
        Functions_GetNbOfFunctions(Material_GetFunctions(MAT))

#define Material_GetFunction(MAT) \
        Functions_GetFunction(Material_GetFunctions(MAT))

#define Material_GetDimension(MAT) \
        Geometry_GetDimension(Model_GetGeometry(Material_GetModel(MAT)))

#define Material_FindCurve(MAT,S) \
        Curves_FindCurve(Material_GetCurves(MAT),S)

/*
** #define Material_ReadProperties(MAT,datafile) \
*          Model_GetReadMaterialProperties(Material_GetModel(MAT))(MAT,datafile)
*/

/* Equations/unknowns */
#define Material_GetNbOfEquations(MAT) \
        Model_GetNbOfEquations(Material_GetModel(MAT))

#define Material_GetNameOfEquation(MAT) \
        Model_GetNameOfEquation(Material_GetModel(MAT))

#define Material_GetNameOfUnknown(MAT) \
        Model_GetNameOfUnknown(Material_GetModel(MAT))
        
#define Material_CopyNameOfEquation(MAT,index,name) \
        (strcpy(Material_GetNameOfEquation(MAT)[index],name))

#define Material_CopyNameOfUnknown(MAT,index,name) \
        (strcpy(Material_GetNameOfUnknown(MAT)[index],name))

#define Material_GetObjectiveValue(MAT) \
        Model_GetObjectiveValue(Material_GetModel(MAT))




#include "Fields.h"
#include "Functions.h"
#include "Curves.h"

struct Material_s {           /* material */
  char*   codenameofmodel ;   /**< Code name of the model */
  char*   method ;            /**< Characterize a method */
  int     n ;                 /**< Nb of properties */
  double* pr ;                /**< The properties */
  Curves_t* curves ;          /**< Curves */
  Fields_t* fields ;          /**< Fields */
  Functions_t* functions ;    /**< Time functions */
  Model_t* model ;            /**< Model */
  
  /* for compatibility with former version (should be eliminated) */
  unsigned short int neq ;    /**< nombre d'equations du modele */
  char**   eqn ;              /**< nom des equations */
  char**   inc ;              /**< nom des inconnues */
  int      nc ;               /**< nb of curves */
  Curve_t* cb ;               /**< curves */
  
#ifdef NOTDEFINED             /* NON UTILISE POUR LE MOMENT */
  int      nfd ;              /**< nombre de donnees formelles */
  int      fdl ;              /**< longueur en caractere des donnees formelles */
  char**   fd ;               /**< les donnees formelles en mode char */
#endif
} ;


/* Old notations which should be eliminated */
#define mate_t                 Material_t
#define dmat                   Material_ScanProperties1
#define lit_mate               Material_ScanProperties2

#endif