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
#include "util/pool.h"
using namespace vnl::phy;

const bool verify = false;
const bool async = true;


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
		count_seq_t() : Generic() {}
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
			std::cout << (now-last) << " Consumer " << this_mac << std::endl;
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
			workers.push_back(launch(std::bind(&Producer::produce, this)));
		}
	}
protected:
	Consumer* dst;
	std::vector<taskid_t> workers;
	void produce() {
		const int N = 103;
		uint64_t pid = rand();
		uint64_t seq = 0;
		std::cout << vnl::System::currentTimeMillis() << " Started Producer " << pid << std::endl;
		vnl::util::pool<Consumer::count_seq_t> msgs;
		auto callback = [&msgs](Message* msg) {
			msgs.free((Consumer::count_seq_t*)msg);
		};
		while(dst) {
			if(async) {
				for(int i = 0; i < N; ++i) {
					Consumer::count_seq_t* msg = msgs.alloc();
					msg->data = std::make_pair(pid, ++seq);
					msg->callback = callback;
					send(msg, dst, true);
				}
				flush();
			} else {
				send(Consumer::count_seq_t(std::make_pair(pid, ++seq)), dst);
			}
		}
		std::cout << "Producer task exit " << pid << std::endl;
	}
	virtual bool handle(Message* msg) {
		if(msg->mid == delete_t::id) {
			dst = 0;
		}
		return false;
	}
	
};


int main() {
	
	Consumer* consumer = 0;
	Producer* producer = 0;
	
	Engine* engineA = new FiberEngine();
	engineA->start();
	engineA->exec([&consumer]() {
		std::cout << "engineA starting..." << std::endl;
		vnl::Util::stick_to_core(0);
		consumer = new Consumer();
	});
	
	Engine* engineB = new FiberEngine();
	engineB->start();
	engineB->exec([&producer, consumer]() {
		std::cout << "engineB starting..." << std::endl;
		vnl::Util::stick_to_core(1);
		producer = new Producer(consumer, 1);
	});
	
	//std::this_thread::sleep_for(std::chrono::seconds(1*60));
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	
	engineA->exec([consumer, producer]() {
		std::cout << "stopping..." << std::endl;
		Stream s;
		s.send(Object::delete_t(), producer);
		s.send(Object::delete_t(), consumer);
		delete consumer;
		delete producer;
	});
	
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	
	engineA->stop();
	engineB->stop();
	
	delete engineA;
	delete engineB;
	
}






