/******************************************************************************
 *                                                                            *
 * Copyright (c) 2012 Texas Instruments Incorporated - http://www.ti.com/     *
 *                                                                            *
 * All rights reserved. Property of Texas Instruments Incorporated.           *
 * Restricted rights to use, duplicate or disclose this code are              *
 * granted through contract.                                                  *
 *                                                                            *
 * The program may not be used without the written permission                 *
 * of Texas Instruments Incorporated or against the terms and conditions      *
 * stipulated in the agreement under which this program has been              *
 * supplied.                                                                  *
 *                                                                            *
 *****************************************************************************/

/**
 *  \file common_types.h
 *
 *  \brief OSAL header file containing data types.
 *
 */

#ifndef _common_types_
#define _common_types_



#ifndef _USING_RTEMS_INCLUDES_
  typedef unsigned char                         boolean;
#endif

  typedef signed char                           int8;
  typedef short int                             int16;
  typedef long int                              int32;
  typedef unsigned char                         uint8;
  typedef unsigned short int                    uint16;
  typedef unsigned long int                     uint32;
 #ifndef TRUE              /* Boolean true */
   #define TRUE (1)
#endif

#ifndef FALSE              /* Boolean false */
   #define FALSE (0)
#endif


#endif  /* _common_types_ */
