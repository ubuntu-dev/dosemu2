/*
 * (C) Copyright 1992, ..., 2003 the "DOSEMU-Development-Team".
 *
 * for details see file COPYING in the DOSEMU distribution
 */


#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "emu.h"
#include "memory.h"
#include "doshelpers.h"
#include "../coopthreads/coopthreads.h"

#include "msetenv.h"

#define peek	com_peek
#define MCB	com_MCB

/*
   envptr - returns pointer to parent command.com's copy of the environment
*/
static char *envptr(int *size, int takeown)
{
    int parent_p;
    struct MCB *mcb;

    parent_p=peek(_psp,0x16);    /* find pointer to parent in psp */
    if (takeown) parent_p = _psp;
    if (peek(parent_p,0x2c)==0) {
       mcb = (struct MCB *) (((long) (peek(parent_p-1,0x3) + parent_p)) << 4);
    }
    else {
       mcb = (struct MCB *) (((long) (peek(parent_p,0x2c) - 1)) << 4);
    }
    *size = mcb->size * 16;
    return ((char *) ((long) (FP_SEG32(mcb) + 1) << 4));
}


/*
   msetenv - place an environment variable in command.com's copy of
             the envrionment.
*/

int com_msetenv(char *variable, char *value, int takeown)
{
    char *env1, *env2;
    char *cp;
    char *var;
    int size;
    int l;

    env1 = env2 = envptr(&size, takeown);
    l = strlen(variable);
    var = alloca(l+1);
    memcpy(var, variable, l+1);
    strupr(var);

    /*
       Delete any existing variable with the name (var).
    */
    while (*env2) {
        if ((strncmp(var,env2,l) == 0) && (env2[l] == '=')) {
            cp = env2 + strlen(env2) + 1;
            memcpy(env2,cp,size-(cp-env1));
        }
        else {
            env2 += strlen(env2) + 1;
        }
    }

    if (!value || !value[0]) return 0;

    /*
       If the variable fits, shovel it in at the end of the envrionment.
    */
    if (strlen(value) && (size-(env2-env1)) >= (l + strlen(value) + 3)) {
        strcpy(env2,var);
        strcat(env2,"=");
        strcat(env2,value);
        env2[strlen(env2)+1] = 0;
        return(0);
    }

    /*
       Return error indication if variable did not fit.
    */
    return(-1);
}


/*
   msetenv - place an environment variable in command.com's copy of
             the envrionment.
*/

int msetenv(char *var, char *value)
{
    return com_msetenv(var, value, 0);
}
