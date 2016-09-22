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
#include <vnl/RecordHeader.hxx>
#include <vnl/info/PlayerStatus.hxx>
#include <vnl/Sample.h>
#include <vnl/io/File.h>


namespace vnl {

class Player : public vnl::PlayerBase {
public:
	Player(const vnl::String& domain_, Basic* target)
		:	PlayerBase(domain_, "Player"),
		 	target(target), in(&file), timer(0), begin_pos(0)
	{
		type_blacklist["vnl.Announce"] = true;
		type_blacklist["vnl.LogMsg"] = true;
		type_blacklist["vnl.Shutdown"] = true;
		type_blacklist["vnl.Exit"] = true;
	}
	
	vnl::Map<Address, Address> addr_map;
	vnl::Map<vnl::Hash32, bool> type_blacklist;
	
protected:
	void main() {
		set_timeout(interval, std::bind(&Player::update, this), VNL_TIMER_REPEAT);
		timer = set_timeout(0, std::bind(&Player::process, this), VNL_TIMER_MANUAL);
		if(autostart) {
			open(filename);
			play();
		}
		run();
		file.close();
	}
	
	void open(const vnl::String& file) {
		log(INFO).out << "Opening file: " << file << vnl::endl;
		filename = file;
		reset();
		update();
	}
	
	void reset() {
		char buf[1024];
		file.close();
		status = vnl::info::PlayerStatus();
		filename.to_string(buf, sizeof(buf));
		file = ::fopen(buf, "r");
		if(!file.good()) {
			status.error = true;
			log(ERROR).out << "Unable to open file for reading: " << filename << vnl::endl;
			return;
		}
		in.reset();
		Pointer<Value> ptr;
		vnl::read(in, ptr);
		RecordHeader* header = dynamic_cast<RecordHeader*>(ptr.value());
		if(header) {
			status.end_time = header->end_time;
			begin_pos = header->header_size;
			log(INFO).out << "Found header: size=" << header->header_size << ", end_time=" << header->end_time << vnl::endl;
			seek_begin();
		} else {
			begin_pos = 0;
			scan();
		}
		status.filename = filename;
		status.playing = false;
		status.error = false;
		status.begin_time = next.time;
		status.current_time = next.time;
		if(!in.error()) {
			log(INFO).out << "Ready to play from " << status.begin_time << vnl::endl;
		} else {
			status.error = true;
			log(ERROR).out << "File empty, invalid or corrupt." << vnl::endl;
		}
	}
	
	void scan() {
		if(status.playing) {
			return;
		}
		log(INFO).out << "Scanning file ..." << vnl::endl;
		seek_begin();
		while(!in.error()) {
			status.end_time = next.time;
			vnl::read(in, next);
		}
		seek_begin();
		log(INFO).out << "Finished: end_time=" << status.end_time << vnl::endl;
	}
	
	void stop() {
		status.playing = false;
		status.current_time = status.begin_time;
		seek_begin();
		log(INFO).out << "Stopped, ready to play." << vnl::endl;
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
		if(status.playing) {
			status.playing = false;
			log(INFO).out << "Paused at " << next.time << vnl::endl;
		}
	}
	
	void seek(int64_t time) {
		if(!file.good()) {
			return;
		}
		log(INFO).out << "Seeking to " << time << " ..." << vnl::endl;
		if(time < next.time) {
			seek_begin();
		}
		while(!in.error()) {
			status.current_time = next.time;
			if(next.time >= time) {
				break;
			}
			vnl::read(in, next);
		}
		if(status.playing) {
			play();
		} else {
			log(INFO).out << "Now at " << next.time << vnl::endl;
		}
	}
	
	void seek_rel(float pos) {
		seek(status.begin_time + double(pos) * (status.end_time-status.begin_time));
	}
	
	void process() {
		if(!file.good() || in.error()) {
			status.error = true;
			status.playing = false;
		}
		if(!status.playing) {
			return;
		}
		status.current_time = next.time;
		vnl::Value* value = next.value.release();
		if(value) {
			bool pass = true;
			if(type_blacklist.find(value->vni_hash())) {
				pass = false;
			} else if(blacklist.find(Address(next.domain, ""))) {
				pass = false;
			} else if(blacklist.find(Address(next.domain, next.topic))) {
				pass = false;
			}
			if(pass) {
				Address dst(next.domain, next.topic);
				Address* remap = addr_map.find(dst);
				if(remap) {
					dst = *remap;
				}
				Sample* msg = buffer.create<Sample>();
				msg->data = value;
				send_async(msg, target);
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
			if(in.error() == VNL_IO_EOF) {
				log(INFO).out << "End of file reached." << vnl::endl;
				seek_begin();
				if(autoloop) {
					play();
				}
			} else {
				status.error = true;
				log(ERROR).out << "Error reading file: " << in.error() << vnl::endl;
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
	
	vnl::info::PlayerStatus get_status() const {
		return status;
	}
	
private:
	void seek_begin() {
		if(!file.good()) {
			return;
		}
		if(::fseek(file, begin_pos, SEEK_SET) < 0) {
			log(ERROR).out << "fseek() failed with " << errno << vnl::endl;
			return;
		}
		in.reset();
		vnl::read(in, next);
		status.current_time = status.begin_time;
	}
	
private:
	Basic* target;
	vnl::io::File file;
	vnl::io::TypeInput in;
	
	vnl::Timer* timer;
	vnl::info::PlayerStatus status;
	vnl::RecordValue next;
	int begin_pos;
	
	vnl::Map<Address, bool> blacklist;
	
};



}

#endif /* INCLUDE_VNL_PLAYER_H_ */
