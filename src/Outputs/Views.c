#include <string.h>
#include "Message.h"
#include "Views.h"


Views_t* Views_Create(int n)
{
  Views_t* views = (Views_t*) malloc(sizeof(Views_t)) ;
  
  if(!views) {
    arret("Views_Create") ;
  }
  
  Views_GetNbOfViews(views) = n ;
  
  Views_GetView(views) = View_Create(n) ;
  
  return(views);
}


void Views_Delete(Views_t** views)
{
  View_t* view = Views_GetView(*views) ;
  
  View_Delete(&view) ;
  free(*views) ;
}
