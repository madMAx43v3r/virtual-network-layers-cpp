/*
 * Error.h
 *
 *  Created on: Jun 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_IO_ERROR_H_
#define INCLUDE_VNL_IO_ERROR_H_


enum {
	VNL_SUCCESS = 0,
	VNL_ERROR = -1,
	VNL_IO_OVERFLOW = -2,
	VNL_IO_UNDERFLOW = -3,
	VNL_IO_INVALID_ID = -4,
	VNL_IO_INVALID_SIZE = -5,
	VNL_IO_STACK_OVERFLOW = -6,
	VNL_IO_EOF = -7
};


#endif /* INCLUDE_VNL_IO_ERROR_H_ */
