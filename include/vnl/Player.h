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
#include <vnl/RecordTypeInfo.hxx>
#include <vnl/info/PlayerStatus.hxx>
#include <vnl/Sample.h>
#include <vnl/io/File.h>


namespace vnl {

class Player : public vnl::PlayerBase {
public:
	Player(const vnl::String& domain_ = vnl::local_domain_name, const vnl::String& topic_ = "Player", vnl::Basic* target = vnl::Router::instance)
		:	PlayerBase(domain_, topic_),
		 	target(target), in(&file), timer(0), begin_pos(0)
	{
		type_blacklist["vnl.Announce"] = true;
		type_blacklist["vnl.LogMsg"] = true;
		type_blacklist["vnl.Shutdown"] = true;
		type_blacklist["vnl.Exit"] = true;
	}
	
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
		close();
	}
	
	void open(const vnl::String& file) {
		log(INFO).out << "Opening file: " << file << vnl::endl;
		filename = file;
		reset();
	}
	
	void reset() {
		char buf[1024];
		close();
		header = vnl::RecordHeader();
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
		RecordHeader* p_header = dynamic_cast<RecordHeader*>(ptr.get());
		if(p_header) {
			header = *p_header;
			status.end_time = header.end_time;
			begin_pos = header.header_size;
			log(INFO).out << "Found header: size=" << header.header_size << ", end_time=" << header.end_time << vnl::endl;
			for(Topic& topic : header.topics) {
				log(INFO).out << "Topic " << topic.domain << ":" << topic.name << vnl::endl;
			}
			seek_begin();
			if(header.have_type_info) {
				log(INFO).out << "Have RecordTypeInfo: " << type_info.type_map.size() << " types" << vnl::endl;
			}
		} else {
			begin_pos = 0;
			do_scan();
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
		update();
	}
	
	void scan() {
		set_timeout(0, std::bind(&Player::do_scan, this), VNL_TIMER_ONCE);
	}
	
	void do_scan() {
		if(status.playing) {
			return;
		}
		log(INFO).out << "Scanning file ..." << vnl::endl;
		seek_begin();
		int counter = 0;
		while(!in.error()) {
			status.end_time = next.time;
			vnl::read(in, next);
			if(counter++ % 1000 == 0) {
				update();
			}
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
		set_timeout(0, std::bind(&Player::do_seek, this, time), VNL_TIMER_ONCE);
	}
	
	void seek_rel(float pos) {
		seek(status.begin_time + double(pos) * (status.end_time-status.begin_time));
	}
	
	void do_seek(int64_t time) {
		if(!file.good()) {
			return;
		}
		log(INFO).out << "Seeking to " << time << " ..." << vnl::endl;
		if(time < next.time) {
			seek_begin();
		}
		int counter = 0;
		while(!in.error()) {
			status.current_time = next.time;
			if(next.time >= time) {
				break;
			}
			vnl::read(in, next);
			if(counter++ % 1000 == 0) {
				update();
			}
		}
		if(status.playing) {
			play();
		} else {
			log(INFO).out << "Now at " << next.time << vnl::endl;
		}
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
		send_next();
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
				} else if(autoshutdown) {
					log(INFO).out << "Triggering shutdown ..." << vnl::endl;
					publish(vnl::Shutdown::create(), vnl::local_domain_name, "vnl.shutdown");
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
		publish(status.clone(), my_private_domain, "player_status");
	}
	
	vnl::info::PlayerStatus get_status() const {
		return status;
	}
	
	vnl::Array<vnl::Topic> get_topics() const {
		return header.topics;
	}
	
	Map<Hash32, vnl::info::Type> get_type_info() const {
		return type_info.type_map;
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
		if(header.have_type_info) {
			vnl::read(in, type_info);
		}
		vnl::read(in, next);
		status.current_time = status.begin_time;
	}
	
	void send_next() {
		if(next.value && type_blacklist.find(next.value->get_vni_hash())) {
			return;
		} else if(blacklist.find(Address(next.domain, ""))) {
			return;
		} else if(blacklist.find(Address(next.domain, next.topic))) {
			return;
		}
		
		Sample* msg = vnl_sample_buffer.create();
		msg->dst_addr = Address(next.domain, next.topic);
		msg->header = next.header.release();
		msg->data = next.value.release();
		send_async(msg, target);
	}
	
	void close() {
		if(file) {
			::fclose(file);
		}
	}
	
private:
	vnl::Basic* target;
	vnl::io::File file;
	vnl::io::TypeInput in;
	
	vnl::Timer* timer;
	vnl::info::PlayerStatus status;
	RecordTypeInfo type_info;
	RecordHeader header;
	vnl::RecordValue next;
	int begin_pos;
	
	vnl::Map<Address, bool> blacklist;
	
};


} // vnl

#endif /* INCLUDE_VNL_PLAYER_H_ */
