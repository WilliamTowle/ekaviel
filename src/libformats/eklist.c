#include <stdio.h>	/* NULL definition */
#include "eklist.h"

#include "ekmalloc.h"

/*****
 *	Node class
 *****/

EkErrFlag eknode_new(EkNode **nodePtr)
{
	if (nodePtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eknode_new() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkNode	*node= (EkNode *)
				ALLOCATE_MEMORY(1, EkNode,
					"eklist.c::eknode_new()");

	    if ( (*nodePtr= node) == NULL)
		return EK_ERR_MALLOC_FAILURE;
	    else
		return EK_ERR_NONE;
	}
}

EkErrFlag eknode_construct(EkNode *node, void *datum)
{
	if (node == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else
	{
	    EKNODE_FIELD(*node, datum)= datum;
	    EKNODE_FIELD(*node, prev)= EKNODE_FIELD(*node, next)= NULL;
	    return EK_ERR_NONE;
	}
}

EkErrFlag eknode_destroy(EkNode *node)
{
    	if (node == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eknode_destroy() called %s\n",
		    		"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	    return EK_ERR_NONE;
}

EkErrFlag eknode_delete(EkNode **nodePtr)
{
	if (nodePtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eknode_delete() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    FREE_MEMORY(*nodePtr, "eklist.c::eknode_delete()");
	    *nodePtr= NULL;
	    return EK_ERR_NONE;
	}
}

/*****
 *	List class
 *****/

EkErrFlag eklist_construct(EkList *list)
{
	if (list == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eklist_construct() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    EKLIST_FIELD(*list, start)=
		EKLIST_FIELD(*list, end)=
			EKLIST_FIELD(*list, current)= NULL;
	    EKLIST_FIELD(*list, circular)= FL_FALSE;
	    return EK_ERR_NONE;
	}
}

void eklist_set_circular(EkList *list, Flag fot)
{
	EKLIST_FIELD(*list, circular)= fot;
}

EkErrFlag eklist_add(EkList *list, void *datum)
{
  EkNode	*newNode= NULL;
  EkErrFlag	error= EK_ERR_NONE;

	if ( ((error= eknode_new(&newNode)) == EK_ERR_NONE)
		&& ((error= eknode_construct(
				newNode, datum)) == EK_ERR_NONE) )
	{
	    if (EKLIST_FIELD(*list,start) == NULL)
	    {
		EKLIST_FIELD(*list,end)=
			EKLIST_FIELD(*list,start)= newNode;
	    }
	    else
	    {
		EKNODE_FIELD(*EKLIST_FIELD(*list,end),next)= newNode;
		EKNODE_FIELD(*newNode, prev)= EKLIST_FIELD(*list, end);
		EKLIST_FIELD(*list, end)= newNode;
	    }
	}

  return error;
}

Flag eklist_empty(EkList *list)
{
  return (EKLIST_FIELD(*list, start) == NULL)? FL_TRUE : FL_FALSE;
}

Flag eklist_next(EkList *list)
{
	EKLIST_FIELD(*list, current)=
		EKLIST_FIELD(*list,current)?
			EKNODE_FIELD(*EKLIST_FIELD(*list,current),next)
			: EKLIST_FIELD(*list, start);
	if ( EKLIST_FIELD(*list, circular)
		&& (EKLIST_FIELD(*list, current) == NULL) )
	{
	    EKLIST_FIELD(*list, current)= EKLIST_FIELD(*list, start);
	}

  return EKLIST_FIELD(*list, current) != NULL;
}

void *eklist_current(EkList *list)
{
	if (EKLIST_FIELD(*list, current) == NULL)
	    return NULL;
	else
	    return EKNODE_FIELD(*EKLIST_FIELD(*list,current),datum);
}

EkErrFlag eklist_unlink(EkList *list)
{
  EkNode	*current= EKLIST_FIELD(*list, current);

	if (current == NULL)
	    return EK_ERR_NOT_SUPPORTED;
	else
	{
	  EkErrFlag	error= EK_ERR_NONE;

	    if (EKNODE_FIELD(*current,prev) == NULL)
		EKLIST_FIELD(*list, start)= EKNODE_FIELD(*current, next);
	    else
		EKNODE_FIELD(*EKNODE_FIELD(*current,prev),next)=
		    	EKNODE_FIELD(*current,next);

	    if (EKNODE_FIELD(*current,next) == NULL)
		EKLIST_FIELD(*list, end)= EKNODE_FIELD(*current, prev);
	    else
		EKNODE_FIELD(*EKNODE_FIELD(*current,next),prev)=
		    	EKNODE_FIELD(*current,prev);

	    EKLIST_FIELD(*list, current)= EKNODE_FIELD(*current,prev);
	    if ( (error= eknode_destroy(current)) == EK_ERR_NONE )
		error= eknode_delete(&current);

	    return error;
	}
}

EkErrFlag eklist_destroy(EkList *listPtr)
{
    	if (listPtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eklist_destroy() called %s\n",
		    		"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else if (EKLIST_FIELD(*listPtr, start) != NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eklist_destroy() called %s\n",
				"with non-empty list");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    return EK_ERR_NONE;
	}
}
