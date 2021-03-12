#ifndef __ATOM_DEFS_H__
#define __ATOM_DEFS_H__

typedef enum {
    #define ATOM_DEF(name) name,
    #define ATOM_LAST(name) name
    #include "AtomDefs_macros.h"
    #undef ATOM_LAST
    #undef ATOM_DEF
} AtomDefEnum;

typedef struct WmAtomList {
    const char *atom_name;
    const Atom atom;
} WmAtomList;

extern WmAtomList _wm_atoms[];

#endif /* __ATOM_DEFS_H__ */