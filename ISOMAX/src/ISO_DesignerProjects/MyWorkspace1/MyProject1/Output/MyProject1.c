// ISO-Designer ISO 11783   Version 5.4.0.4246 Jetter AG
// Do not change!

#include "MyProject1.iop.h"
#include "MyProject1.c.h"

#define WORD(w)  (unsigned char)w, (unsigned char)(w >> 8)
#define LONG(l)  (unsigned char)l, \
					(unsigned char)((unsigned long)l >> 8), \
					(unsigned char)((unsigned long)l >> 16), \
					(unsigned char)((unsigned long)l >> 24)
#define ID(id)           WORD(id)
#define REF(id)          WORD(id)
#define XYREF(id, x, y)  WORD(id), WORD(x), WORD(y)
#define MACRO(ev, id)    ev, id
#define COLOR(c)         c

const unsigned char ISO_OP_MEMORY_CLASS isoOP_MyProject1[] = {
	ID(WorkingSet_0), TYPEID_WORKSET, COLOR(249), 1, ID(DataMask_1000), 1, 0, 2,
		XYREF(Rectangle_14000, 84, 48),
		'e', 'n',
		'd', 'e',
	ID(DataMask_1000), TYPEID_DATAMASK, COLOR(249), ID(ID_NULL), 0, 0, 
	ID(Rectangle_14000), TYPEID_OUTRECT, ID(LineAttributes_24000), 
		WORD(36), WORD(24), 0, ID(FillAttributes_25000), 0, 
	ID(LineAttributes_24000), TYPEID_LINEATTR, COLOR_BLACK, 1, WORD(65535), 0, 
	ID(FillAttributes_25000), TYPEID_FILLATTR, 2, COLOR_WHITE, ID(ID_NULL), 0, 
}; // isoOP_MyProject1
