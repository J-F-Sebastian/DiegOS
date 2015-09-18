/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2015 Diego Gallizioli
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DEBUG_TRACES_H_
#define _DEBUG_TRACES_H_

/*
 * DBG_MODULE must be defined on the compiler's command line to enable
 * debug capabilites.
 * Every module must define DBG_MODULE_ON to enable local traces.
 * Not declaring DBG_MODULE will also undefine DBG_MODULE_ON to clear
 * all possible traces.
 */
#ifdef DBG_MODULE
#ifdef DBG_MODULE_ON

#include <stdio.h>

/*
 * Use at the very beginning of the function
 */
#define TRACE_FN_ENTRY() fprintf(stderr,"[%s,%s:%d] TRACE: ENTERING\n", __FILE__, __FUNCTION__, __LINE__);

/*
 * Use before calling return
 */
#define TRACE_FN_EXIT() fprintf(stderr,"[%s,%s:%d] TRACE: EXITING\n",__FILE__,__FUNCTION__, __LINE__);

/*
 * Use to print a single message
 */
#define TRACE_PRINT(msg) fprintf(stderr,"[%s,%s:%d] TRACE: PRINT\n %s\n",__FILE__, __FUNCTION__, __LINE__, msg);

/*
 * Use to print a hex value
*/
#define TRACE_PRINT_HEX(var) fprintf(stderr,"[%s,%s:%d] TRACE: PRINT HEX\n %s value is 0x%X\n",\
        __FILE__, __FUNCTION__, __LINE__, #var,var);

/*
 * Use to print a decimal value
*/
#define TRACE_PRINT_DEC(var) fprintf(stderr,"[%s,%s:%d] TRACE: PRINT DEC\n %s value is %d\n",__FILE__, \
                                __FUNCTION__, __LINE__, #var,var);

/*
 * Use to print a floating point value
*/
#define TRACE_PRINT_FLOAT(var) fprintf(stderr,"[%s,%s:%d] TRACE: PRINT FLOAT\n %s value is %f\n",__FILE__, \
                                __FUNCTION__, __LINE__, #var,var);
/*
 * Use a combination of TRACE_BEGIN_MSG, TRACE_PRINT_MSG, TRACE_END_MSG to
 * print multiple lines - or - print a multiline formatted message.
 */

#define TRACE_BEGIN_MSG(msg) \
fprintf(stderr," ========= ========= ========= =========  =========  =========  =========\n"); \
TRACE_PRINT(msg)

#define TRACE_PRINT_MSG(msg) fprintf(stderr,"%s\n",msg)

#define TRACE_END_MSG(msg) \
TRACE_PRINT_MSG(msg) \
fprintf(stderr," ========= ========= ========= =========  =========  =========  =========\n");

#else

#define TRACE_FN_ENTRY()

#define TRACE_FN_EXIT()

#define TRACE_PRINT(msg)

#define TRACE_PRINT_HEX(msg)

#define TRACE_PRINT_DEC(msg)

#define TRACE_PRINT_FLOAT(var)

#define TRACE_BEGIN_MSG(msg)

#define TRACE_PRINT_MSG(msg)

#define TRACE_END_MSG(msg)

#endif
#else

#ifdef DBG_MODULE_ON
#undef DBG_MODULE_ON
#endif

#define TRACE_FN_ENTRY()

#define TRACE_FN_EXIT()

#define TRACE_PRINT(msg)

#define TRACE_PRINT_HEX(msg)

#define TRACE_PRINT_DEC(msg)

#define TRACE_PRINT_FLOAT(var)

#define TRACE_BEGIN_MSG(msg)

#define TRACE_PRINT_MSG(msg)

#define TRACE_END_MSG(msg)

#endif

#endif
