/*
 * Random.cpp
 *
 *  Created on: May 8, 2016
 *      Author: mad
 */

#include "vnl/Random.h"


namespace vnl {

vnl::util::spinlock Random64::sync;

Random64* Random64::instance = 0;

std::atomic<int> Random64::counter;



}

