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
 * The block size in bytes
 */
#define VNL_BLOCK_SIZE 256

/*
 * Defining this will alloc/free paged memory every time.
 * Useful for valgrind debugging.
 */
#define VNL_MEMORY_DEBUG

/*
 * Defining this will assert on IO errors.
 */
//#define VNL_IO_DEBUG

/*
 * The maximum number of hops a packet can travel.
 */
#define VNL_MAX_ROUTE_LENGTH 16

/*
 * The maximum size of an element.
 */
#define VNL_IO_MAX_SIZE 2147483648

/*
 * The maximum degree of struct and class nesting.
 */
#define VNL_IO_MAX_RECURSION 256

/*
 * If we have boost coroutine >= version 1.56
 * Needed for FiberEngine.
 */
//#define VNL_HAVE_BOOST_COROUTINE true

#ifdef VNL_HAVE_BOOST_COROUTINE
#define VNL_HAVE_FIBER_ENGINE true
#endif



#endif /* INCLUDE_BUILD_CONFIG_H_ */
