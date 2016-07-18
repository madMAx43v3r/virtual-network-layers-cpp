/*
 * test_type_stream.cpp
 *
 *  Created on: Jul 17, 2016
 *      Author: mad
 */

#include <vnl/io.h>
#include <vnl/Layer.h>
#include <test/TestType.hxx>


int main() {
	
	int N = 100000;
	
	vnl::Layer layer("test_type_stream");
	
	vnl::io::ByteBuffer buf;
	vnl::io::TypeOutput out(&buf);
	
	test::TestType input;
	input.text = "test_test_test_test_test_test_test_test_test";
	std::cout << input.to_string() << std::endl;
	
	for(int i = 0; i < N; ++i) {
		vnl::write(out, input);
		assert(!out.error());
	}
	out.flush();
	
	std::cout << "buf: size = " << buf.position() << std::endl;
	buf.flip();
	
	vnl::io::TypeInput in(&buf);
	test::TestType output;
	for(int i = 0; i < N; ++i) {
		vnl::read(in, output);
		assert(!in.error());
		assert(output.text == "test_test_test_test_test_test_test_test_test");
	}
	
	std::cout << output.to_string() << std::endl;
	
}

