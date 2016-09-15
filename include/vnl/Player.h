/*
 * Player.h
 *
 *  Created on: Sep 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_PLAYER_H_
#define INCLUDE_VNL_PLAYER_H_

#include <vnl/PlayerSupport.hxx>
#include <vnl/RecordValue.hxx>
#include <vnl/io/File.h>


namespace vnl {

class Player : public vnl::PlayerBase {
public:
	Player(const vnl::String& domain_, const vnl::String& topic_)
		:	PlayerBase(domain_, topic_),
		 	p_file(0), in(&file), timer(0), playing(false), time_offset(0)
	{
	}
	
protected:
	void main() {
		subscribe(vnl::local_domain_name, "vnl/topic");
		timer = set_timeout(0, std::bind(&Player::process, this), VNL_TIMER_MANUAL);
		reset();
		if(autostart) {
			play();
		}
		run();
		::fclose(p_file);
	}
	
	void handle(const vnl::Topic& topic) {
		topic_map[vnl::Address(topic.domain, topic.name)] = topic;
	}
	
	void reset() {
		if(p_file) {
			::fclose(p_file);
			p_file = 0;
		}
		char buf[1024];
		filename.to_string(buf, sizeof(buf));
		p_file = ::fopen(buf, "r");
		if(!p_file) {
			log(ERROR).out << "Unable to open file for reading: " << filename << vnl::endl;
			return;
		}
		file.fd = ::fileno(p_file);
		vnl::read(in, next);
		log(INFO).out << "Ready to replay: " << filename << vnl::endl;
	}
	
	void play() {
		playing = true;
		time_offset = vnl::currentTimeMicros() - next.time;
		timer->interval = 0;
		timer->reset();
		log(INFO).out << "Playing: time_offset=" << time_offset/1e6 << vnl::endl;
	}
	
	void process() {
		if(!playing) {
			return;
		}
		vnl::Value* value = next.value.release();
		if(value) {
			publish(value, vnl::Address((uint64_t)next.domain, (uint64_t)next.topic));
		}
		vnl::read(in, next);
		if(!in.error()) {
			int64_t now = vnl::currentTimeMicros() - time_offset;
			timer->interval = next.time - now;
			timer->reset();
		} else {
			playing = false;
			if(in.error() == VNL_IO_SUCCESS || in.error() == VNL_IO_CLOSED) {
				log(INFO).out << "End of file reached" << vnl::endl;
			} else {
				log(ERROR).out << "Error reading file: " << in.error() << vnl::endl;
			}
			if(autoloop) {
				reset();
				play();
			}
		}
	}
	
private:
	FILE* p_file;
	vnl::io::File file;
	vnl::io::TypeInput in;
	
	vnl::Timer* timer;
	bool playing;
	int64_t time_offset;
	vnl::RecordValue next;
	
	vnl::Map<vnl::Address, Topic> topic_map;
	
};



}

#endif /* INCLUDE_VNL_PLAYER_H_ */
