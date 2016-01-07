/*
 * test_link.cpp
 *
 *  Created on: Dec 20, 2015
 *      Author: mad
 */

#include "producer_consumer.h"
#include "phy/Processor.h"
#include "phy/FiberEngine.h"
#include "phy/ThreadEngine.h"

int main() {
	
	const int N = 1;
	const int M = 10;
	
	Link* linkA = new Processor(new FiberEngine(10));
	Consumer* consumer = new Consumer(linkA);
	linkA->start();
	
	Link* linkB = new Processor(new FiberEngine(1));
	Producer** prods = new Producer*[N];
	for(int i = 0; i < N; ++i) {
		prods[i] = new Producer(linkB, consumer, M);
	}
	linkB->start();
	
	std::this_thread::sleep_for(std::chrono::seconds(1*60));
	
	linkA->stop();
	linkB->stop();
	
	for(int i = 0; i < N; ++i) {
		delete prods[i];
	}
	delete [] prods;
	delete consumer;
	delete linkA;
	delete linkB;
	
}







