/*
 * Pipe.cpp
 *
 *  Created on: Dec 6, 2016
 *      Author: mwittal
 */

#include <vnl/Pipe.h>


namespace vnl {

int Pipe::num_open = 0;
Pool<Pipe>* Pipe::pool = 0;
std::mutex Pipe::sync;


} // vnl
