/*
 * Random.cpp
 *
 *  Created on: May 8, 2016
 *      Author: mad
 */

#include "phy/Random.h"


namespace vnl { namespace phy {

vnl::util::spinlock Random64::sync;

Random64* Random64::instance = 0;

std::atomic<int> Random64::counter;


}}


