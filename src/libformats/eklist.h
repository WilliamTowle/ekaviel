#ifndef __EKLIST_H__
#define __EKLIST_H__

#include "ektype.h"

typedef struct ekNode {
	void		*datum;
	struct ekNode	*next, *prev;
	} EkNode;

EkErrFlag eknode_new(EkNode **nodePtr);
EkErrFlag eknode_construct(EkNode *node, void *datum);
#define EKNODE_FIELD(name, field)	\
	(name).field
EkErrFlag eknode_destroy(EkNode *node);
EkErrFlag eknode_delete(EkNode **nodePtr);



typedef struct ekList {
	EkNode	*start, *end, *current;
	Flag	circular;
	} EkList;

EkErrFlag eklist_construct(EkList *list);
void eklist_set_circular(EkList *list, Flag fot);
EkErrFlag eklist_add(EkList *list, void *datum);
Flag eklist_empty(EkList *list);
Flag eklist_next(EkList *list);
void *eklist_current(EkList *list);
EkErrFlag eklist_unlink(EkList *list);
#define EKLIST_FIELD(name, field)	\
	(name).field
EkErrFlag eklist_destroy(EkList *list);

#endif	/* __EKLIST_H__ */
