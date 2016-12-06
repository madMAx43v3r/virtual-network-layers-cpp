/*
 * Pipe.cpp
 *
 *  Created on: Dec 6, 2016
 *      Author: mwittal
 */

#include <vnl/Pipe.h>


namespace vnl {

Pool<Pipe> Pipe::pool;
std::mutex Pipe::sync;


} // vnl
