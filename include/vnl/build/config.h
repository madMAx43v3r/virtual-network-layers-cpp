/*
 * config.h
 *
 *  Created on: May 8, 2016
 *      Author: mad
 */

#ifndef INCLUDE_BUILD_CONFIG_H_
#define INCLUDE_BUILD_CONFIG_H_


/*
 * The page size in bytes
 */
#define VNL_PAGE_SIZE 4096

/*
 * How far memory is aligned
 */
//#define VNL_MEMORY_ALIGN 16

/*
 * The block size of strings in bytes
 */
#define VNL_STRING_BLOCK_SIZE 128

/*
 * Defining this will use new() instead of paged memory.
 * Useful for valgrind debugging.
 * The memory allocated will not be deleted however.
 */
//#define VNL_MEMORY_DEBUG true

/*
 * If we have boost coroutine >= version 1.56
 * Needed for FiberEngine.
 */
#define VNL_HAVE_BOOST_COROUTINE true



#endif /* INCLUDE_BUILD_CONFIG_H_ */
