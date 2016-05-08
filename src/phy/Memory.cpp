/*
 * Memory.cpp
 *
 *  Created on: May 6, 2016
 *      Author: mad
 */

#include "phy/Memory.h"


namespace vnl { namespace phy {

std::atomic<Page*> Page::begin;

size_t Page::num_alloc = 0;


}}

