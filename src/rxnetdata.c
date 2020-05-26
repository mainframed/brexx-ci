#include <rexx.h>
#include <rxdefs.h>
#include "lstring.h"
#include "netdata.h"

void R_receive(int func) {

}

void R_transmit(int func) {

}

void RxNetDataRegFunctions()
{
    RxRegFunction("RECEIVE",	R_receive,	0);
    RxRegFunction("TRANSMIT",	R_transmit,	0);
} /* RxNetDataInitialize() */

