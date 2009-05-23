/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*                      http://www.grisbi.org                                    */
/*      Version : 0.6.0                                                          */
/*                                                                               */
/* *******************************************************************************/

/* *******************************************************************************/
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/

#if HAVE_CUNIT

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <CUnit/Automated.h>
#include <CUnit/Basic.h>

#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_account_cunit.h"
#include "gsb_real_cunit.h"
#include "utils_dates_cunit.h"
/*END_INCLUDE*/

#endif/*HAVE_CUNIT*/

/*START_STATIC*/
/*END_STATIC*/

#if HAVE_CUNIT

/*START_EXTERN*/
/*END_EXTERN*/

int gsb_cunit_run_tests()
{
    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    if ( ( NULL == gsb_data_account_cunit_create_suite ( ) )
      || ( NULL == gsb_real_cunit_create_suite ( ) )
      || ( NULL == utils_dates_cunit_create_suite ( ) )
        )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests */
#ifdef _WIN32
    CU_automated_run_tests();
#else//_WIN32
	CU_basic_run_tests();
#endif//_WIN32
    CU_cleanup_registry();
    return CU_get_error();
}

#else/*HAVE_CUNIT*/

int gsb_cunit_run_tests()
{
    return 0;
}

#endif/*HAVE_CUNIT*/

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
