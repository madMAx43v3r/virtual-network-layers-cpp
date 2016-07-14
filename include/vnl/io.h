/*
 * io.h
 *
 *  Created on: May 22, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_IO_H_
#define INCLUDE_VNL_IO_H_

#include <vnl/io/StreamBuffer.h>
#include <vnl/io/ByteBuffer.h>
#include <vnl/io/TypeInput.h>
#include <vnl/io/TypeOutput.h>
#include <vnl/io/Serializable.h>


namespace vnl { namespace io {

struct block_t {
	int32_t addr;
	int32_t size;
};


}}

#endif /* INCLUDE_VNL_IO_H_ */
