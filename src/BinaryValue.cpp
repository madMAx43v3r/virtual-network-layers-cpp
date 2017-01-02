/*
 * BinaryValue.cpp
 *
 *  Created on: Sep 17, 2016
 *      Author: mad
 */

#include <vnl/BinaryValue.h>
#include <vnl/Type.hxx>


namespace vnl {

const uint32_t BinaryValue::VNI_HASH;

BinaryValue* BinaryValue::create() {
	return vnl::create<BinaryValue>();
}

BinaryValue* BinaryValue::clone() const {
	return vnl::clone<BinaryValue>(*this);
}

void BinaryValue::destroy() {
	this->BinaryValue::~BinaryValue();
	return vnl::internal::global_pool_->push_back(this, sizeof(BinaryValue));
}

void BinaryValue::serialize(vnl::io::TypeOutput& out) const {
	out.putEntry(VNL_IO_CLASS, size);
	out.putHash(hash);
	if(data.data) {
		vnl::io::ByteBuffer buf(data.data, data.size);
		vnl::io::TypeInput in(&buf);
		in.copy(&out, VNL_IO_CLASS, size, hash);
	}
}

void BinaryValue::deserialize(vnl::io::TypeInput& in, int _size) {
	size = _size;
	if(!data.data) {
		data.data = vnl::Page::alloc();
	}
	vnl::io::ByteBuffer buf(data.data);
	vnl::io::TypeOutput out(&buf);
	in.copy(&out, VNL_IO_CLASS, size, hash);
	out.flush();
	data.size = out.error() ? 0 : buf.position();
}


} // namespace
