/*
 * test_producer_consumer.h
 *
 *  Created on: Jan 1, 2016
 *      Author: mad
 */

#ifndef TEST_PRODUCER_CONSUMER_H_
#define TEST_PRODUCER_CONSUMER_H_

#include <iostream>
#include <map>

#include "System.h"
#include "phy/Link.h"
#include "phy/FiberEngine.h"
using namespace vnl::phy;

const bool verify = false;
const bool async = false;
const bool usestreams = false;

int stick_this_thread_to_core(int core_id) {
   int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
   if (core_id < 0 || core_id >= num_cores) {
	   printf("invalid core_id!\n");
	   return EINVAL;
   }
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);
   pthread_t current_thread = pthread_self();    
   return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

class Consumer : public vnl::phy::Object {
public:
	uint64_t last = 0;
	uint64_t counter = 0;
	std::map<uint64_t, uint64_t> map;
	Consumer(Object* parent) : Object::Object(parent) {
		last = vnl::System::currentTimeMillis();
		launch(std::bind(&Consumer::setup, this));
	}
	
	typedef MessageType<std::pair<uint64_t, uint64_t>, 0x337f8543> count_seq_t;
	
protected:
	void setup() {
		stick_this_thread_to_core(1);
	}
	void count_seq(Message* msg, std::pair<uint64_t, uint64_t> data) {
		if(verify) {
			volatile uint64_t lseq = map[data.first];
			assert(lseq+1 == data.second);
			map[data.first] = data.second;
		}
		counter++;
		if(counter % (1000*1000) == 0) {
			uint64_t now = vnl::System::currentTimeMillis();
			std::cout << (now-last) << " Consumer " << getMAC() << std::endl;
			last = now;
		}
	}
	virtual void handle(Message* msg) {
		switch(msg->msg_id) {
			case count_seq_t::id: {
				count_seq(msg, msg->cast<count_seq_t>()->data);
				break;
			}
		}
		msg->ack();
	}
};

class Producer : public vnl::phy::Object {
public:
	Producer(Object* parent, Consumer* dst, int M) : Object::Object(parent), dst(dst) {
		for(int i = 0; i < M; ++i) {
			launch(std::bind(&Producer::produce, this));
		}
	}
protected:
	Consumer* dst;
	virtual void handle(Message* msg) {
		// nothing
	}
	void produce() {
		stick_this_thread_to_core(0);
		std::cout << vnl::System::currentTimeMillis() << " Started Producer " << getMAC() << std::endl;
		uint64_t pid = rand();
		uint64_t seq = 0;
		Stream stream(this);
		if(async) {
			SendBuffer<Consumer::count_seq_t, 10> out(this);
			while(true) {
				if(usestreams) {
					stream.send(out.get(Consumer::count_seq_t(dst, std::make_pair(pid, ++seq))), true);
				} else {
					send(out.get(Consumer::count_seq_t(dst, std::make_pair(pid, ++seq))), true);
				}
			}
		} else {
			while(true) {
				if(usestreams) {
					stream.send(Consumer::count_seq_t(dst, std::make_pair(pid, ++seq)));
				} else {
					send(Consumer::count_seq_t(dst, std::make_pair(pid, ++seq)));
				}
			}
		}
	}
};



#endif /* TEST_PRODUCER_CONSUMER_H_ */
