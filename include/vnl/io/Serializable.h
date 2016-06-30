/*
 * Serializable.h
 *
 *  Created on: May 22, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_IO_SERIALIZABLE_H_
#define INCLUDE_VNL_IO_SERIALIZABLE_H_

#include "vnl/io/TypeInput.h"
#include "vnl/io/TypeOutput.h"
#include "vnl/io/PageBuffer.h"


namespace vnl { namespace io {

typedef vnl::io::TypeOutput<vnl::io::PageBuffer> TypeOutputStream;
typedef vnl::io::TypeInput<vnl::io::PageBuffer> TypeInputStream;

class Serializable {
public:
	virtual ~Serializable() {}
	
	virtual void serialize(TypeOutputStream& stream) const = 0;
	
	virtual void deserialize(TypeInputStream& stream, uint32_t num_entries) = 0;
	
};



}}

#endif /* INCLUDE_VNL_IO_SERIALIZABLE_H_ */
