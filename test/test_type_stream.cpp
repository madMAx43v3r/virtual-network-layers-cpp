/*
 * test_type_stream.cpp
 *
 *  Created on: Jul 17, 2016
 *      Author: mad
 */

#include <vnl/io.h>
#include <vnl/Instance.hxx>


int main() {
	
	int N = 100000;
	
	vnl::io::ByteBuffer buf;
	vnl::io::TypeOutput out(&buf);
	
	vnl::Instance inst;
	inst.domain = "test_domain";
	inst.topic = "test_topic";
	inst.type = "test_type";
	std::cout << inst.to_string() << std::endl;
	
	for(int i = 0; i < N; ++i) {
		vnl::write(out, inst);
		assert(!out.error());
	}
	out.flush();
	
	std::cout << "buf: size = " << buf.position() << std::endl;
	buf.flip();
	
	vnl::io::TypeInput in(&buf);
	vnl::Instance inst2;
	for(int i = 0; i < N; ++i) {
		vnl::read(in, inst2);
		assert(!in.error());
		assert(inst2.domain == "test_domain");
		assert(inst2.topic == "test_topic");
		assert(inst2.type == "test_type");
	}
	
	std::cout << inst2.to_string() << std::endl;
	
}

