/*
 * config.h
 *
 *  Created on: May 8, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_BUILD_CONFIG_H_
#define INCLUDE_VNL_BUILD_CONFIG_H_


/*
 * The page size in bytes
 */
#ifndef VNL_PAGE_SIZE
#define VNL_PAGE_SIZE 4096
#endif

/*
 * The block size in bytes
 */
#ifndef VNL_BLOCK_SIZE
#define VNL_BLOCK_SIZE 256
#endif

/*
 * Defining this will assert on correct memory usage.
 */
//#define VNL_MEMORY_DEBUG

/*
 * Defining this will assert on IO errors.
 */
//#define VNL_IO_DEBUG

/*
 * The maximum number of hops a packet can travel.
 */
#ifndef VNL_MAX_ROUTE_LENGTH
#define VNL_MAX_ROUTE_LENGTH 16
#endif

/*
 * The maximum size of an element.
 */
#ifndef VNL_IO_MAX_SIZE
#define VNL_IO_MAX_SIZE 2147483648
#endif

/*
 * If we have boost coroutine >= version 1.56
 * Needed for FiberEngine.
 */
#ifdef VNL_HAVE_BOOST_COROUTINE
#define VNL_HAVE_FIBER_ENGINE
#endif



#endif /* INCLUDE_VNL_BUILD_CONFIG_H_ */
