/*
 * test_packet.cpp
 *
 *  Created on: Jul 17, 2016
 *      Author: mad
 */

#include <vnl/io.h>
#include <vnl/Layer.h>
#include <vnl/Sample.h>
#include <test/TestType.hxx>


int main() {
	
	vnl::Layer layer("test");
	
	vnl::io::ByteBuffer buf;
	vnl::io::TypeOutput out(&buf);
	
	test::TestType input;
	input.text = "test_test_test_test_test_test_test_test_test";
	
	vnl::Sample* sample = new vnl::Sample();
	sample->seq_num = 1337;
	sample->src_addr = vnl::Address("domain", "topic");
	sample->dst_addr = vnl::Address("domain", "topic");
	sample->data = &input;
	
	sample->serialize(out);
	out.flush();
	
	std::cout << "buf: size = " << buf.position() << std::endl;
	buf.flip();
	
	vnl::io::TypeInput in(&buf);
	vnl::Sample* sample_out = new vnl::Sample();
	int size = 0;
	int id = in.getEntry(size);
	assert(id == VNL_IO_INTERFACE);
	uint32_t hash = 0;
	in.getHash(hash);
	assert(hash == vnl::Sample::PID);
	sample_out->deserialize(in, size);
	std::cout << sample_out->data->to_string() << std::endl;
	
	assert(sample_out->seq_num == sample->seq_num);
	assert(sample_out->num_hops == sample->num_hops);
	assert(sample_out->src_addr == sample->src_addr);
	assert(sample_out->dst_addr == sample->dst_addr);
	
}


