#ifndef AC_T_BISCUS1_H
#define AC_T_BISCUS1_H

#include "types.h"
#include "m_actor.h"
#include "ac_tools.h"

#ifdef __cplusplus
extern "C" {
#endif

extern ACTOR_PROFILE T_Biscus1_Profile;

typedef void (*BISCUS1_PROC)(ACTOR*);

typedef struct t_biscus1_s{
    TOOLS_ACTOR tools_class;
    u8 pad2[0x8];
    BISCUS1_PROC proc; 
    int current_id;
}BISCUS1_ACTOR;

#ifdef __cplusplus
}
#endif

#endif

