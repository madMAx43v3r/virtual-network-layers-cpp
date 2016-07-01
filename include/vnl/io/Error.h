/*
 * Error.h
 *
 *  Created on: Jun 4, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_IO_ERROR_H_
#define INCLUDE_VNL_IO_ERROR_H_


namespace vnl { namespace io {

enum {
	VNL_IO_SUCCESS = 0,
	VNL_IO_OVERFLOW = 1,
	VNL_IO_UNDERFLOW = 2,
	VNL_IO_ERROR = 3
};

}}

#endif /* INCLUDE_VNL_IO_ERROR_H_ */
