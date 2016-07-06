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
//#define VNL_MEMORY_DEBUG true

/*
 * The maximum size of an element.
 */
#define VNL_IO_MAX_SIZE 2147483648

/*
 * The maximum number of fields in a struct or class.
 */
#define VNL_IO_MAX_NUM_FIELDS 128

/*
 * The maximum degree of struct and class nesting.
 * Minimum stack size = VNL_IO_MAX_NUM_FIELDS * VNL_IO_MAX_RECURSION * 4
 */
#define VNL_IO_MAX_RECURSION 32

/*
 * If we have boost coroutine >= version 1.56
 * Needed for FiberEngine.
 */
//#define VNL_HAVE_BOOST_COROUTINE true

#ifdef VNL_HAVE_BOOST_COROUTINE
#define VNL_HAVE_FIBER_ENGINE true
#endif



#endif /* INCLUDE_BUILD_CONFIG_H_ */
