/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_L_H
#define AH_PROVIDER_L_H

#include "provider.h"
#include "hbci_l.h"


AH_HBCI *AH_Provider_GetHbci(const AB_PROVIDER *pro);


#endif
