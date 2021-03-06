#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Help.h"
#include "BilVersion.h"
#include "BilInfo.h"
#include "Bil.h"
#include "Models.h"
#include "Modules.h"
#include "Message.h"
#include "Context.h"
#include "Mry.h"


static void          (Context_Initialize)(Context_t*) ;


/* Global functions */
Context_t* (Context_Create)(int argc,char** argv)
{
  Context_t* ctx = (Context_t*) Mry_New(Context_t) ;
  
  
  {
    CommandLine_t* cmd = CommandLine_Create(argc,argv) ;
    
    Context_GetCommandLine(ctx) = cmd ;
  }
  
  /* Initialization */
  if(argc > 0 && argv) {
    Context_Initialize(ctx) ;
  }
  /*
  Context_GetHelpOnline(ctx) = NULL ;
  Context_GetPrintInfo(ctx) = NULL ;
  Context_GetInversePermutation(ctx) = NULL ;
  Context_GetPrintModel(ctx) = NULL ;
  Context_GetPrintModule(ctx) = NULL ;
  Context_GetPostProcessing(ctx) = NULL ;
  Context_GetSolver(ctx) = NULL ;
  Context_GetReadOnly(ctx) = NULL ;
  Context_GetDebug(ctx) = NULL ;
  Context_GetPrintLevel(ctx) = NULL ;
  Context_GetUseModule(ctx) = NULL ;
  Context_GetGraph(ctx) = NULL ;
  Context_GetInputFileName(ctx) = NULL ;
  Context_GetMiscellaneous(ctx) = NULL ;
  Context_GetElementOrdering(ctx) = NULL ;
  Context_GetNodalOrdering(ctx) = NULL ;
  */
  

  
  {
    Options_t* opt = Options_Create(ctx) ;
    
    Context_GetOptions(ctx) = opt ;
  }
  
  return(ctx) ;
}


void Context_Delete(void* self)
{
  Context_t** pctx = (Context_t**) self ;
  
  CommandLine_Delete(&(Context_GetCommandLine(*pctx))) ;
  Options_Delete(&(Context_GetOptions(*pctx))) ;
  free(*pctx) ;
  *pctx = NULL ;
}



void (Context_Initialize)(Context_t* ctx)
{
  CommandLine_t* cmd = Context_GetCommandLine(ctx) ;
  int    argc = CommandLine_GetNbOfArg(cmd) ;
  char** argv = CommandLine_GetArg(cmd) ;
  int i ;
  

  if(argc == 1) {
    Context_GetPrintUsage(ctx) = (char**) argv ;
    return ;
  }
  
  
  /* Get line options */
  for(i = 1 ; i < argc ; i++) {
    
    if(argv[i][0] != '-') { /* File name */
      Context_GetInputFileName(ctx) = (char**) argv + i ;
      
    } else if(strncmp(argv[i],"-info",5) == 0) {
      Context_GetPrintInfo(ctx) = (char**) argv + i ;
  
    } else if(strncmp(argv[i],"-help",5) == 0) {
      Context_GetHelpOnline(ctx) = (char**) argv + i ;
      
    } else if(!strncmp(argv[i],"-solver",strlen(argv[i]))) {
      Context_GetSolver(ctx) = (char**) argv + i ;
      if(i + 1 < argc) {
        i++ ;
      } else {
        Message_FatalError("Missing solver") ;
      }
      
      /* Skip two more entries if the following entry is "-ff" 
       * i.e. an input for a Fill Factor for multi-frontal methods */
      {
        if(!strncmp(argv[i + 1],"-ff",strlen(argv[i + 1]))) {
          if(i + 2 < argc) {
            i += 2 ;
          } else {
            Message_FatalError("Missing solver") ;
          }
        }
      }
    
    } else if(strncmp(argv[i],"-debug",strlen(argv[i])) == 0) {
      Context_GetDebug(ctx) = (char**) argv + i ;
      if(i + 1 < argc) {
        i++ ;
      } else {
        Message_FatalError("Missing name of data to be printed") ;
      }

    } else if(strncmp(argv[i],"-level",strlen(argv[i])) == 0) {
      Context_GetPrintLevel(ctx) = (char**) argv + i ;
      if(i + 1 < argc) {
        i++ ;
      } else {
        Message_FatalError("Missing level") ;
      }

    } else if(strncmp(argv[i],"-with",strlen(argv[i])) == 0) {
      Context_GetUseModule(ctx) = (char**) argv + i ;
      if(i + 1 < argc) {
        i++ ;
      } else {
        Message_FatalError("Missing module") ;
      }

    } else if(strncmp(argv[i],"-models",strlen(argv[i])) == 0) {
      Context_GetPrintModel(ctx) = (char**) argv + i ;

    } else if(strncmp(argv[i],"-modules",strlen(argv[i])) == 0) {
      Context_GetPrintModule(ctx) = (char**) argv + i ;

    } else if(strncmp(argv[i],"-readonly",strlen(argv[i])) == 0) {
      Context_GetReadOnly(ctx) = (char**) argv + i ;
      if(i + 1 >= argc) {
        Message_FatalError("Missing file name") ;
      }

    } else if(strncmp(argv[i],"-graph",strlen(argv[i])) == 0) {
      Context_GetGraph(ctx) = (char**) argv + i ;
      if(i + 1 < argc) {
        i++ ;
      } else {
        Message_FatalError("Missing graph method") ;
      }

    } else if(strncmp(argv[i],"-iperm",strlen(argv[i])) == 0) {
      Context_GetInversePermutation(ctx) = (char**) argv + i ;

    } else if(strncmp(argv[i],"-eordering",strlen(argv[i])) == 0) {
      Context_GetElementOrdering(ctx) = (char**) argv + i ;
      if(i + 1 < argc) {
        i++ ;
      } else {
        Message_FatalError("Missing element ordering method") ;
      }

    } else if(strncmp(argv[i],"-nordering",strlen(argv[i])) == 0) {
      Context_GetNodalOrdering(ctx) = (char**) argv + i ;
      if(i + 1 < argc) {
        i++ ;
      } else {
        Message_FatalError("Missing nodal ordering method") ;
      }

    } else if(strncmp(argv[i],"-postprocessing",strlen(argv[i])) == 0) {
      Context_GetPostProcessing(ctx) = (char**) argv + i ;
      if(i + 1 < argc) {
        i++ ;
      } else {
        Message_FatalError("Missing post-processing method") ;
      }

    } else if(strncmp(argv[i],"-miscellaneous",strlen(argv[i])) == 0) {
      Context_GetMiscellaneous(ctx) = (char**) argv + i ;

    } else if(strncmp(argv[i],"-test",strlen(argv[i])) == 0) {
      Context_GetTest(ctx) = (char**) argv + i ;
      
    } else {
      Message_FatalError("Unknown option") ;
    }
    
  }
  
}
