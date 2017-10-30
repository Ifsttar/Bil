#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "Context.h"
#include "module.h"


#define AUTHORS  "Dangla"
#define TITLE    "Fully Coupled Approach"

#include "PredefinedMethods.h"

static Module_ComputeProblem_t   calcul ;
static Module_SolveProblem_t     Algorithm ;
static void   ComputeInitialState(Mesh_t*,double) ;
static int    ComputeExplicitTerms(Mesh_t*,double) ;
static int    ComputeMatrix(Mesh_t*,double,double,Matrix_t*) ;
static void   ComputeResidu(Mesh_t*,double,double,double*,Loads_t*) ;
static int    ComputeImplicitTerms(Mesh_t*,double,double) ;



/*
  Extern functions
*/

int SetModuleProp(Module_t* module)
{
  Module_CopyShortTitle(module,TITLE) ;
  Module_CopyNameOfAuthors(module,AUTHORS) ;
  Module_GetComputeProblem(module) = calcul ;
  Module_GetSolveProblem(module) = Algorithm ;
  return(0) ;
}

/*
  Intern functions
*/

int calcul(DataSet_t* jdd)
{
  Mesh_t* mesh = DataSet_GetMesh(jdd) ;
  int n_sol = 2 ; /* Must be 2 at minimum but works with more */
  Solutions_t* sols = Solutions_Create(mesh,n_sol) ;

  /* 1. Execute this line to set only one allocation of space for explicit terms. */
  /* This is not mandatory except in some models where constant terms are saved as 
   * explicit terms and updated only once during initialization. 
   * It is then necessary to merge explicit terms. Otherwise it is not mandatory.
   * Should be eliminated in the future. */
  Solutions_MergeExplicitTerms(sols) ;
  /* This is done 11/05/2015 */
  //Message_Warning("Explicit terms are not merged anymore in this version.") ;
  
  {
    DataFile_t* datafile = DataSet_GetDataFile(jdd) ;
    char*   filename = DataFile_GetFileName(datafile) ;
    Dates_t*  dates    = DataSet_GetDates(jdd) ;
    int     nbofdates  = Dates_GetNbOfDates(dates) ;
    Points_t* points   = DataSet_GetPoints(jdd) ;
    int     n_points   = Points_GetNbOfPoints(points) ;
    OutputFiles_t* outputfiles = OutputFiles_Create(filename,nbofdates,n_points) ;
    Options_t* options = DataSet_GetOptions(jdd) ;
    Solver_t* solver = Solver_Create(mesh,options) ;
    
  /* 2. Calculation */
    {
      Solution_t* sol = Solutions_GetSolution(sols) ;
      double* date    = Dates_GetDate(dates) ;
      
      Solution_GetTime(sol) = date[0] ;
      
      Algorithm(jdd,sols,solver,outputfiles) ;
    }
  
  /* 3. Close the output files */
    OutputFiles_Delete(&outputfiles) ;
  }
  
  return(0) ;
}


/*
  Intern functions
*/


static int   Algorithm(DataSet_t* jdd,Solutions_t* sols,Solver_t* solver,OutputFiles_t* outputfiles)
{
#define SOL_1     Solutions_GetSolution(sols)
#define SOL_n     Solution_GetPreviousSolution(SOL_1)

#define T_n       Solution_GetTime(SOL_n)
#define DT_n      Solution_GetTimeStep(SOL_n)
#define STEP_n    Solution_GetStepIndex(SOL_n)

#define T_1       Solution_GetTime(SOL_1)
#define DT_1      Solution_GetTimeStep(SOL_1)
#define STEP_1    Solution_GetStepIndex(SOL_1)

  DataFile_t*    datafile    = DataSet_GetDataFile(jdd) ;
  Options_t*     options     = DataSet_GetOptions(jdd) ;
  Mesh_t*        mesh        = DataSet_GetMesh(jdd) ;
  BConds_t*      bconds      = DataSet_GetBConds(jdd) ;
  Loads_t*       loads       = DataSet_GetLoads(jdd) ;
  Dates_t*       dates       = DataSet_GetDates(jdd) ;
  TimeStep_t*    timestep    = DataSet_GetTimeStep(jdd) ;
  IterProcess_t* iterprocess = DataSet_GetIterProcess(jdd) ;
  
  Nodes_t*       nodes       = Mesh_GetNodes(mesh) ;
  unsigned int   nbofdates   = Dates_GetNbOfDates(dates) ;
  double*        date        = Dates_GetDate(dates) ;

  unsigned int   idate ;
  double t_0 ;
  
  
  /*
   * 1. Initialization
   */
  Mesh_InitializeSolutionPointers(mesh,sols) ;
  
  //T_1 = date[0] ;
  //DT_1 = 0. ;
  //STEP_1 = 0 ;
  
  {
    int i = Mesh_LoadCurrentSolution(mesh,datafile,&T_1) ;
    
    idate = 0 ;
    
    if(i) {
      while(idate + 1 < nbofdates && T_1 >= date[idate + 1]) idate++ ;
      
      Message_Direct("Continuation ") ;
      
      if(DataFile_ContextIsFullInitialization(datafile)) {
        Message_Direct("(full initialization) ") ;
      } else if(DataFile_ContextIsPartialInitialization(datafile)) {
        Message_Direct("(partial initialization) ") ;
      } else if(DataFile_ContextIsNoInitialization(datafile)) {
        Message_Direct("(no initialization) ") ;
      }
      
      Message_Direct("at t = %e (between steps %d and %d)\n",T_1,idate,idate+1) ;
    }
    
    if(DataFile_ContextIsInitialization(datafile)) {
      IConds_t* iconds = DataSet_GetIConds(jdd) ;
    
      IConds_AssignInitialConditions(iconds,mesh,T_1) ;

      ComputeInitialState(mesh,T_1) ;
    }
  }
  
  
  /*
   * 2. Backup
   */
  t_0 = T_1 ;
  OutputFiles_BackupSolutionAtPoint(outputfiles,jdd,T_1,t_0) ;
  OutputFiles_BackupSolutionAtTime(outputfiles,jdd,T_1,idate) ;
  
  
  /*
   * 3. Loop on dates
   */
  for(; idate < nbofdates - 1 ; idate++) {
    
    /*
     * 3.1 Loop on time steps
     */
    do {
      /*
       * 3.1.1 Looking for a new solution at t + dt
       * We step forward (point to the next solution) 
       */
      //SOL_1 = Solution_GetNextSolution(SOL_1) ;
      Solutions_StepForward(sols) ;
      Mesh_InitializeSolutionPointers(mesh,sols) ;
      
      /*
       * 3.1.1b Backup the previous solution if the previous 
       * saved environment is restored after a nonlocal jump.
       */
      {
        if(Exception_SaveEnvironment) {
          backupandreturn :
          Solutions_StepBackward(sols) ;
          Mesh_InitializeSolutionPointers(mesh,sols) ;
          OutputFiles_BackupSolutionAtTime(outputfiles,jdd,T_1,idate+1) ;
          Mesh_StoreCurrentSolution(mesh,datafile,T_1) ;
          //Mesh_InitializeSolutionPointers(mesh,SOL_n) ;
          //OutputFiles_BackupSolutionAtTime(outputfiles,jdd,T_n,idate+1) ;
          //Mesh_StoreCurrentSolution(mesh,datafile,T_n) ;
          //Solutions_GetSolution(sols) = SOL_n ;
          return(-1) ;
        }
      }
      
      /*
       * 3.1.2 Compute the explicit terms with the previous solution
       */
      {
        int i = ComputeExplicitTerms(mesh,T_n) ;
        
        if(i != 0) {
          Message_Direct("\n") ;
          Message_Direct("Algorithm(1): undefined explicit terms\n") ;
          /* Backup the previous solution */
          if(T_n > t_0) {
            goto backupandreturn ;
          }
          return(-1) ;
        }
      }
        
      /*
       * 3.1.3 Compute and set the time step
       */
      {
        double dt = TimeStep_ComputeTimeStep(timestep,nodes,T_n,DT_n,date[idate],date[idate+1]) ;
        
        DT_1 = dt ;
        STEP_1 = STEP_n + 1 ;
      }
      
      /*
       * 3.1.3b The time at which we compute
       */
      IterProcess_GetRepetitionIndex(iterprocess) = 0 ;
      recommences :
      {
        int irecom = IterProcess_GetRepetitionIndex(iterprocess) ;
        
        if(irecom > 0) Message_Direct("Repetition no %d\n",irecom) ;
      }
      T_1 = T_n + DT_1 ;
      Message_Direct("Step %d  t = %e (dt = %4.2e)",STEP_1,T_1,DT_1) ;
      
      /*
       * 3.1.4 Initialize the unknowns
       */
      Mesh_SetCurrentUnknownsWithBoundaryConditions(mesh,bconds,T_1) ;
      
      /*
       * 3.1.5 Loop on iterations
       */
      IterProcess_GetIterationIndex(iterprocess) = 0 ;
      while(IterProcess_LastIterationIsNotReached(iterprocess)) {
        IterProcess_IncrementIterationIndex(iterprocess) ;
        
        /*
         * 3.1.5.1 The implicit terms (constitutive equations)
         */
        {
          int i = ComputeImplicitTerms(mesh,T_1,DT_1) ;
          
          if(i != 0) {
            if(IterProcess_LastRepetitionIsNotReached(iterprocess)) {
              IterProcess_IncrementRepetitionIndex(iterprocess) ;
              DT_1 *= TimeStep_GetReductionFactor(timestep) ;
              {
                double t_ini = TimeStep_GetInitialTimeStep(timestep) ;
              
                if(DT_1 > t_ini) DT_1 = t_ini ;
              }
              goto recommences ;
            } else {
              int iter = IterProcess_GetIterationIndex(iterprocess) ;
              
              Message_Direct("\n") ;
              Message_Direct("Algorithm(2): undefined implicit terms at iteration %d\n",iter) ;
              goto backupandreturn ;
              //return ;
            }
          }
        }
        
        /*
         * 3.1.5.2 The residu
         */
        {
          double*  rhs = Solver_GetRHS(solver) ;
          
          ComputeResidu(mesh,T_1,DT_1,rhs,loads) ;
          
          {
            char*  debug = Options_GetDebug(options) ;
            
            if(!strcmp(debug,"residu")) {
              Solver_Print(solver,debug) ;
            }
          }
        }
        
        /*
         * 3.1.5.3 The matrix
         */
        {
          Matrix_t*  a = Solver_GetMatrix(solver) ;
          int i = ComputeMatrix(mesh,T_1,DT_1,a) ;
          
          if(i != 0) {
            if(IterProcess_LastRepetitionIsNotReached(iterprocess)) {
              IterProcess_IncrementRepetitionIndex(iterprocess) ;
              DT_1 *= TimeStep_GetReductionFactor(timestep) ;
              {
                double t_ini = TimeStep_GetInitialTimeStep(timestep) ;
              
                if(DT_1 > t_ini) DT_1 = t_ini ;
              }
              goto recommences ;
            } else {
              int iter = IterProcess_GetIterationIndex(iterprocess) ;
              
              Message_Direct("\n") ;
              Message_Direct("Algorithm(3): undefined matrix at iteration %d\n",iter) ;
              goto backupandreturn ;
              //return ;
            }
          }
          
          {
            char*  debug = Options_GetDebug(options) ;
            
            if(!strncmp(debug,"matrix",4)) {
              Solver_Print(solver,debug) ;
            }
          }
        }
        
        /*
         * 3.1.5.4 Resolution
         */
        {
          int i = Solver_Solve(solver) ;
          
          if(i != 0) {
            if(IterProcess_LastRepetitionIsNotReached(iterprocess)) {
              IterProcess_IncrementRepetitionIndex(iterprocess) ;
              DT_1 *= TimeStep_GetReductionFactor(timestep) ;
              {
                double t_ini = TimeStep_GetInitialTimeStep(timestep) ;
              
                if(DT_1 > t_ini) DT_1 = t_ini ;
              }
              goto recommences ;
            } else {
              int iter = IterProcess_GetIterationIndex(iterprocess) ;
              
              Message_Direct("\n") ;
              Message_Direct("Algorithm(4): unable to solve at iteration %d\n",iter) ;
              goto backupandreturn ;
              //return ;
            }
          }
        }
        
        /*
         * 3.1.5.5 Update the unknowns
         */
        Mesh_UpdateCurrentUnknowns(mesh,solver) ;
        
        /*
         * 3.1.5.6 The error
         */
        IterProcess_SetCurrentError(iterprocess,nodes,solver) ;
        
        /*
         * 3.1.5.7 We get out if convergence is met
         */
        if(IterProcess_ConvergenceIsMet(iterprocess)) break ;
        
        {
          char*  level = Options_GetPrintLevel(options) ;
          
          if(!strcmp(level,"2") && IterProcess_LastIterationIsNotReached(iterprocess)) {
            IterProcess_PrintCurrentError(iterprocess) ;
          }
        }
      }
      
      {
        IterProcess_PrintCurrentError(iterprocess) ;
      }
      
      /*
       * 3.1.6 Back to 3.1.3 with a smaller time step
       */
      if(IterProcess_ConvergenceIsNotMet(iterprocess) && 
         IterProcess_LastRepetitionIsNotReached(iterprocess)) {
        IterProcess_IncrementRepetitionIndex(iterprocess) ;
        DT_1 *= TimeStep_GetReductionFactor(timestep) ;
        goto recommences ;
      }
      
      /*
       * 3.1.7 Backup for specific points
       */
      OutputFiles_BackupSolutionAtPoint(outputfiles,jdd,T_1,t_0) ;
      /*
       * 3.1.8 Go to 3.2 if convergence was not met
       */
      if(IterProcess_ConvergenceIsNotMet(iterprocess)) break ;
    } while(T_1 < date[idate + 1]) ;
    
    /*
     * 3.2 Backup for this time
     */
    OutputFiles_BackupSolutionAtTime(outputfiles,jdd,T_1,idate+1) ;
    
    /*
     * 3.3 Go to 4. if convergence was not met
     */
    if(IterProcess_ConvergenceIsNotMet(iterprocess)) break ;
  }
  
  /*
   * 4. Store for future resume
   */
  if(IterProcess_ConvergenceIsMet(iterprocess)) {
    Mesh_StoreCurrentSolution(mesh,datafile,T_1) ;
    //Solutions_GetSolution(sols) = SOL_1 ;
  } else {
    Solutions_StepBackward(sols) ;
    Mesh_InitializeSolutionPointers(mesh,sols) ;
    Mesh_StoreCurrentSolution(mesh,datafile,T_1) ;
    //Mesh_InitializeSolutionPointers(mesh,SOL_n) ;
    //Mesh_StoreCurrentSolution(mesh,datafile,T_n) ;
    //Solutions_GetSolution(sols) = SOL_n ;
    return(-1) ;
  }
  
  return(0) ;

#undef T_n
#undef DT_n
#undef STEP_n
#undef T_1
#undef DT_1
#undef STEP_1
#undef SOL_n
#undef SOL_1
}


void ComputeInitialState(Mesh_t* mesh,double t)
{
  unsigned int n_el = Mesh_GetNbOfElements(mesh) ;
  Element_t* el = Mesh_GetElement(mesh) ;
  unsigned int    ie ;

  for(ie = 0 ; ie < n_el ; ie++) {
    Material_t* mat = Element_GetMaterial(el + ie) ;
    
    if(mat) {
      Element_FreeBuffer(el + ie) ;
      Element_ComputeInitialState(el + ie,t) ;
    }
  }
}


int ComputeExplicitTerms(Mesh_t* mesh,double t)
{
  unsigned int n_el = Mesh_GetNbOfElements(mesh) ;
  Element_t* el = Mesh_GetElement(mesh) ;
  unsigned int    ie ;

  for(ie = 0 ; ie < n_el ; ie++) {
    Material_t* mat = Element_GetMaterial(el + ie) ;
    
    if(mat) {
      int    i ;
      
      Element_FreeBuffer(el + ie) ;
      i = Element_ComputeExplicitTerms(el + ie,t) ;
      if(i != 0) return(i) ;
    }
  }
  
  return(0) ;
}


int ComputeMatrix(Mesh_t* mesh,double t,double dt,Matrix_t* a)
{
#define NE (Element_MaxNbOfNodes*Model_MaxNbOfEquations)
  int    cole[NE],lige[NE] ;
  double ke[NE*NE] ;
  unsigned int n_el = Mesh_GetNbOfElements(mesh) ;
  Element_t* el = Mesh_GetElement(mesh) ;
  unsigned int    ie ;
  double zero = 0. ;

  {
    unsigned int    nnz = Matrix_GetNbOfNonZeroValues(a) ;
    unsigned int    j ;
    
    for(j = 0 ; j < nnz ; j++) Matrix_GetNonZeroValue(a)[j] = zero ;
  }
  
  for(ie = 0 ; ie < n_el ; ie++) {
    int  nn = Element_GetNbOfNodes(el + ie) ;
    Material_t* mat = Element_GetMaterial(el + ie) ;
    
    if(mat) {
      int    neq = Material_GetNbOfEquations(mat) ;
      int    i ;
      
      Element_FreeBuffer(el + ie) ;
      i = Element_ComputeMatrix(el + ie,t,dt,ke) ;
      if(i != 0) return(i) ;
      
      /* assembling */
      for(i = 0 ; i < nn ; i++) {
        Node_t* node_i = Element_GetNode(el + ie,i) ;
        int    j ;
        for(j = 0 ; j < neq ; j++) {
          int ij = i*neq + j ;
          int jj_col = Element_GetUnknownPosition(el + ie)[ij] ;
          int jj_row = Element_GetEquationPosition(el + ie)[ij] ;
          cole[ij] = (jj_col >= 0) ? Node_GetMatrixColumnIndex(node_i)[jj_col] : -1 ;
          lige[ij] = (jj_row >= 0) ? Node_GetMatrixRowIndex(node_i)[jj_row] : -1 ;
        }
      }
      Matrix_AssembleElementMatrix(a,ke,cole,lige,nn*neq) ;
    }
  }
  
  return(0) ;
#undef NE
}


void ComputeResidu(Mesh_t* mesh,double t,double dt,double* r,Loads_t* loads)
{
#define NE (Element_MaxNbOfNodes*Model_MaxNbOfEquations)
  double re[NE] ;
  unsigned int n_el = Mesh_GetNbOfElements(mesh) ;
  Element_t* el = Mesh_GetElement(mesh) ;
  unsigned int n_cg = Loads_GetNbOfLoads(loads) ;
  Load_t* cg = Loads_GetLoad(loads) ;
  unsigned int    ie,i_cg ;
  double zero = 0. ;
  
  {
    unsigned int    n_col = Mesh_GetNbOfMatrixColumns(mesh) ;
    unsigned int    j ;
    
    for(j = 0 ; j < n_col ; j++) r[j] = zero ;
  }
  
  /* Residu */
  for(ie = 0 ; ie < n_el ; ie++) {
    int  nn = Element_GetNbOfNodes(el + ie) ;
    Material_t* mat = Element_GetMaterial(el + ie) ;
    
    if(mat) {
      int    neq = Material_GetNbOfEquations(mat) ;
      int i ;
      
      Element_FreeBuffer(el + ie) ;
      Element_ComputeResidu(el + ie,t,dt,re) ;
      
      /* assembling */
      for(i = 0 ; i < nn ; i++) {
        Node_t* node_i = Element_GetNode(el + ie,i) ;
        int    j ;
        for(j = 0 ; j < neq ; j++) {
          int ij = i*neq + j ;
          int jj = Element_GetUnknownPosition(el + ie)[ij] ;
          if(jj >= 0) {
            int k = Node_GetMatrixColumnIndex(node_i)[jj] ;
            if(k >= 0) r[k] += re[ij] ;
          }
        }
      }
    }
  }
  
  /* Loads */
  for(i_cg = 0 ; i_cg < n_cg ; i_cg++) {
    int reg_cg = Load_GetRegionIndex(cg + i_cg) ;
    for(ie = 0 ; ie < n_el ; ie++) if(Element_GetRegionIndex(el + ie) == reg_cg) {
      int  nn = Element_GetNbOfNodes(el + ie) ;
      Material_t* mat = Element_GetMaterial(el + ie) ;
    
      if(mat) {
        int    neq = Material_GetNbOfEquations(mat) ;
        int i ;
        
        Element_FreeBuffer(el + ie) ;
        Element_ComputeLoads(el + ie,t,dt,cg + i_cg,re) ;
        
        /* assembling */
        for(i = 0 ; i < nn ; i++) {
          Node_t* node_i = Element_GetNode(el + ie,i) ;
          int    j ;
          for(j = 0 ; j < neq ; j++) {
            int ij = i*neq + j ;
            int jj = Element_GetUnknownPosition(el + ie)[ij] ;
            if(jj >= 0) {
              int k = Node_GetMatrixColumnIndex(node_i)[jj] ;
              if(k >= 0) r[k] += re[ij] ;
            }
          }
        }
      }
    }
  }
#undef NE
}


int ComputeImplicitTerms(Mesh_t* mesh,double t,double dt)
{
  unsigned int n_el = Mesh_GetNbOfElements(mesh) ;
  Element_t* el = Mesh_GetElement(mesh) ;
  unsigned int    ie ;

  for(ie = 0 ; ie < n_el ; ie++) {
    Material_t* mat = Element_GetMaterial(el + ie) ;
    
    if(mat) {
      int    i ;
      
      Element_FreeBuffer(el + ie) ;
      i = Element_ComputeImplicitTerms(el + ie,t,dt) ;
      if(i != 0) return(i) ;
    }
  }
  
  return(0) ;
}