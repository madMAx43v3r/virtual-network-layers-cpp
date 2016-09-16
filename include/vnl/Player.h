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
#include <vnl/info/PlayerStatus.hxx>
#include <vnl/io/File.h>


namespace vnl {

class Player : public vnl::PlayerBase {
public:
	Player(const vnl::String& domain_)
		:	PlayerBase(domain_, "Player"),
		 	p_file(0), in(&file), timer(0)
	{
	}
	
	vnl::Map<Address, Address> addr_map;
	
protected:
	void main() {
		set_timeout(interval, std::bind(&Player::update, this), VNL_TIMER_REPEAT);
		timer = set_timeout(0, std::bind(&Player::process, this), VNL_TIMER_MANUAL);
		open(filename);
		if(p_file && autostart) {
			play();
		}
		run();
		::fclose(p_file);
	}
	
	void open(const vnl::String& file) {
		log(INFO).out << "Opening file: " << file << vnl::endl;
		filename = file;
		status = vnl::info::PlayerStatus();
		reset();
		if(p_file) {
			scan();
			reset();
		}
		update();
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
		file = vnl::io::File(::fileno(p_file));
		in.reset();
		vnl::read(in, next);
		status.filename = filename;
		status.playing = false;
		status.begin_time = next.time;
		status.current_time = next.time;
		log(INFO).out << "Ready to play from " << status.begin_time << vnl::endl;
	}
	
	void play() {
		status.playing = true;
		status.time_offset = vnl::currentTimeMicros() - next.time;
		timer->interval = 0;
		timer->reset();
		update();
		log(INFO).out << "Playing from " << next.time << ", time_offset=" << status.time_offset << vnl::endl;
	}
	
	void pause() {
		status.playing = false;
		log(INFO).out << "Paused at " << next.time << vnl::endl;
	}
	
	void seek(int64_t time) {
		log(INFO).out << "Seeking to " << time << " ..." << vnl::endl;
		
		
		log(INFO).out << "Now at " << time << vnl::endl;
	}
	
	void process() {
		if(!status.playing) {
			return;
		}
		status.current_time = next.time;
		vnl::Value* value = next.value.release();
		if(value) {
			bool pass = true;
			if(blacklist.find(Address(next.domain, ""))) {
				pass = false;
			}
			if(blacklist.find(Address(next.domain, next.topic))) {
				pass = false;
			}
			if(pass) {
				Address dst(next.domain, next.topic);
				Address* remap = addr_map.find(dst);
				if(remap) {
					dst = *remap;
				}
				publish(value, dst);
			} else {
				vnl::destroy(value);
			}
		}
		vnl::read(in, next);
		if(!in.error()) {
			int64_t now = vnl::currentTimeMicros() - status.time_offset;
			timer->interval = next.time - now;
			timer->reset();
		} else {
			status.playing = false;
			if(in.error() == VNL_IO_SUCCESS || in.error() == VNL_IO_EOF) {
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
	
	void update() {
		blacklist.clear();
		for(String& domain : domain_blacklist) {
			blacklist[Address(domain, "")] = true;
		}
		for(Topic& topic : topic_blacklist) {
			blacklist[Address(topic.domain, topic.name)] = true;
		}
		publish(status.clone(), my_domain, "PlayerStatus");
	}
	
private:
	void scan() {
		log(INFO).out << "Scanning file ..." << vnl::endl;
		int counter = 0;
		while(!in.error()) {
			status.end_time = next.time;
			vnl::read(in, next);
			if(counter++ % 1000 == 0) {
				poll(0);
			}
		}
		log(INFO).out << "Finished: end_time=" << status.end_time << ", duration=" << (status.end_time-status.begin_time) << vnl::endl;
	}
	
private:
	FILE* p_file;
	vnl::io::File file;
	vnl::io::TypeInput in;
	
	vnl::Timer* timer;
	vnl::info::PlayerStatus status;
	vnl::RecordValue next;
	
	vnl::Map<Address, bool> blacklist;
	
};



}

#endif /* INCLUDE_VNL_PLAYER_H_ */
