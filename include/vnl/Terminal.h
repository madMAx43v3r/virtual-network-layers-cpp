/*
 * Terminal.h
 *
 *  Created on: Jun 19, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_TERMINAL_H_
#define INCLUDE_VNL_TERMINAL_H_

#include "vnl/Node.h"
#include "vnl/Layer.h"


namespace vnl {

class Terminal : public Node {
public:
	
	
protected:
	virtual void main(Engine* engine) override {
		Receiver logs(this, layer->global_logs);
		set_timeout(500*1000, std::bind(Terminal::update, this), vnl::Timer::REPEAT);
		run();
	}
	
	virtual bool handle(Packet* pkt) override {
		if(pkt->dst_addr == layer->global_logs) {
			handle((log_msg_t*)pkt->payload);
		}
		return false;
	}
	
	void handle(log_msg_t* msg) {
		
	}
	
	void update() {
		
	}
	
	void set_log_level(uint64_t node, int level) {
		Reference<Basic> ref(this, node);
		Basic* dst = ref.try_get();
		if(dst) {
			Module::set_log_level_t msg(level);
			send(&msg, dst);
		}
	}
	
private:
	
};


}

#endif /* INCLUDE_VNL_TERMINAL_H_ */
