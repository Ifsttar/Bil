#ifndef NODESSOL_H
#define NODESSOL_H

/* class-like structures "NodesSol_t" and attributes */

/* vacuous declarations and typedef names */
struct NodesSol_s     ; typedef struct NodesSol_s     NodesSol_t ;



#include "Mesh.h"

extern NodesSol_t*    (NodesSol_Create)(Mesh_t*) ;
extern void           (NodesSol_AllocateMemory)(NodesSol_t*) ;
extern void           (NodesSol_Copy)(NodesSol_t*,NodesSol_t*) ;
 
 
 
#define NodesSol_GetNbOfDOF(NSS)               ((NSS)->NbOfDOF)
#define NodesSol_GetNbOfNodes(NSS)             ((NSS)->NbOfNodes)
#define NodesSol_GetNodeSol(NSS)               ((NSS)->nodesol)





/* Acces to the dof */
#define NodesSol_GetDOF(NSS) \
        NodeSol_GetUnknown(NodesSol_GetNodeSol(NSS))




#include "NodeSol.h"


/* complete the structure types by using the typedef */
struct NodesSol_s {           /* Nodal Solutions */
  unsigned int NbOfNodes ;
  unsigned int NbOfDOF ;      /* Nb of DOF */
  NodeSol_t* nodesol ;
} ;

#endif
