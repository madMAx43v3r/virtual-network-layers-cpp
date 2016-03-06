/*
 * test_phy.cpp
 *
 *  Created on: Feb 27, 2016
 *      Author: mad
 */

#include <assert.h>
#include <iostream>
#include <map>

#include "phy/FiberEngine.h"
#include "phy/ThreadEngine.h"
#include "phy/Object.h"
using namespace vnl::phy;

const bool verify = false;
const bool usestreams = false;

class Consumer : public vnl::phy::Object {
public:
	uint64_t last = 0;
	uint64_t counter = 0;
	std::map<uint64_t, uint64_t> map;
	Consumer() {
		last = vnl::System::currentTimeMillis();
	}
	
	class count_seq_t : public Generic<std::pair<uint64_t, uint64_t>, 0x337f8543> {
	public:
		count_seq_t(const std::pair<uint64_t, uint64_t>& data, bool async = false) : Generic(data, async) {}
		virtual std::string toString() override {
			std::ostringstream ss;
			ss << Message::toString() << " pid=" << data.first << " seq=" << data.second;
			return ss.str();
		}
	};
	
protected:
	void count_seq(Message* msg, std::pair<uint64_t, uint64_t> data) {
		if(verify) {
			volatile uint64_t lseq = map[data.first];
			assert(lseq+1 == data.second);
			map[data.first] = data.second;
		}
		counter++;
		if(counter % (1000*1000) == 0) {
			uint64_t now = vnl::System::currentTimeMillis();
			std::cout << (now-last) << " Consumer " << mac << std::endl;
			last = now;
		}
	}
	virtual bool handle(Message* msg) {
		switch(msg->mid) {
			case count_seq_t::id: {
				count_seq(msg, ((count_seq_t*)msg)->data);
				msg->ack();
				return true;
			}
		}
		return false;
	}
};

class Producer : public vnl::phy::Object {
public:
	Producer(Consumer* dst, int M) : dst(dst) {
		for(int i = 0; i < M; ++i) {
			launch(std::bind(&Producer::produce, this));
		}
	}
protected:
	Consumer* dst;
	void produce() {
		std::cout << vnl::System::currentTimeMillis() << " Started Producer " << mac << std::endl;
		uint64_t pid = rand();
		uint64_t seq = 0;
		Stream stream;
		while(true) {
			if(usestreams) {
				stream.send(Consumer::count_seq_t(std::make_pair(pid, ++seq)), dst);
			} else {
				send(Consumer::count_seq_t(std::make_pair(pid, ++seq)), dst);
			}
		}
	}
};


class ProcessorA : public FiberEngine {
public:
	ProcessorA() : consumer(0) {
		start();
	}
	~ProcessorA() {
		delete consumer;
	}
	virtual void run() override {
		vnl::Util::stick_to_core(0);
		consumer = new Consumer();
	}
	Consumer* consumer;
};

class ProcessorB : public FiberEngine {
public:
	ProcessorB(Consumer* dst) : dst(dst), producer(0) {
		start();
	}
	~ProcessorB() {
		delete producer;
	}
	virtual void run() override {
		vnl::Util::stick_to_core(1);
		producer = new Producer(dst, 10);
	}
	Producer* producer;
	Consumer* dst;
};


int main() {
	
	ProcessorA* procA = new ProcessorA();
	ProcessorB* procB = new ProcessorB(procA->consumer);
	
	std::this_thread::sleep_for(std::chrono::seconds(1*60));
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	
	procA->stop();
	procB->stop();
	
	delete procB;
	delete procA;
	
}






