/*
 * test_local.cpp
 *
 *  Created on: Jan 1, 2016
 *      Author: mad
 */

#include "producer_consumer.h"
#include "phy/Processor.h"

int main() {
	
	const int N = 1;
	const int M = 10;
	
	Link* link = new Processor(new FiberEngine(100));
	
	Consumer* consumer = new Consumer(link);
	
	Producer** prods = new Producer*[N];
	for(int i = 0; i < N; ++i) {
		prods[i] = new Producer(link, consumer, M);
	}
	
	link->start();
	
	std::this_thread::sleep_for(std::chrono::seconds(1*60));
	
	for(int i = 0; i < N; ++i) {
		delete prods[i];
	}
	delete [] prods;
	delete consumer;
	delete link;
	
}


