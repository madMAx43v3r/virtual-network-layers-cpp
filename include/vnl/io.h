/*
 * io.h
 *
 *  Created on: May 22, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_IO_H_
#define INCLUDE_VNL_IO_H_

#include "io/Buffer.h"
#include "io/PageBuffer.h"
#include "io/ByteOutput.h"
#include "io/ByteInput.h"
#include "io/TypeInput.h"
#include "io/Serializable.h"


namespace vnl { namespace io {

struct block_t {
	int32_t addr;
	int32_t size;
};


}}

#endif /* INCLUDE_VNL_IO_H_ */
