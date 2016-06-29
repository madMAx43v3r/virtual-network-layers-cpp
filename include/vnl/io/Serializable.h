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

class Serializable {
public:
	typedef vnl::io::TypeOutput<vnl::io::PageBuffer> TypeOutput;
	typedef vnl::io::TypeInput<vnl::io::PageBuffer> TypeInput;
	
	virtual ~Serializable() {}
	
	virtual void serialize(TypeOutput& stream) = 0;
	
	virtual void deserialize(TypeInput& stream, uint32_t num_entries) = 0;
	
};


}}

#endif /* INCLUDE_VNL_IO_SERIALIZABLE_H_ */
