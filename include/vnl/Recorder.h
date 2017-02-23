/*
 * Recorder.h
 *
 *  Created on: Jul 4, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_RECORDER_H_
#define CPP_INCLUDE_VNI_RECORDER_H_

#include <vnl/RecorderSupport.hxx>
#include <vnl/RecorderStatus.hxx>
#include <vnl/RecordValue.hxx>
#include <vnl/RecordHeader.hxx>
#include <vnl/RecordTypeInfo.hxx>
#include <vnl/Entry.hxx>
#include <vnl/Sample.h>
#include <vnl/io/File.h>


namespace vnl {

class Recorder : public vnl::RecorderBase {
public:
	Recorder(const vnl::String& domain_ = vnl::local_domain_name, const vnl::String& topic_ = "Recorder")
		:	RecorderBase(domain_, topic_), out(&file)
	{
	}
	
protected:
	void main() {
		char buf[1024];
		filepath = filename << vnl::currentDate("%Y%m%d_%H%M%S") << ".dat";
		filepath.to_string(buf, sizeof(buf));
		file = ::fopen(buf, "w");
		if(!file.good()) {
			log(ERROR).out << "Unable to open file for writing: " << filepath << vnl::endl;
			return;
		}
		log(INFO).out << "Recording to file: " << filepath << vnl::endl;
		
		header.header_size = header_size;
		header.have_type_info = true;
		write_header();
		::fseek(file, header_size-4, SEEK_SET);
		out.writeInt(-1);
		
		RecordTypeInfo type_info;
		type_info.type_map = vnl::get_type_info();
		vnl::write(out, type_info);
		out.flush();
		
		for(const String& domain : domains) {
			subscribe(domain, "");
			domain_map[domain] = domain;
			log(DEBUG).out << "Recording domain: " << domain << vnl::endl;
		}
		
		set_timeout(interval, std::bind(&Recorder::update, this), VNL_TIMER_REPEAT);
		run();
		
		write_header();
		::fflush(file);
		::fclose(file);
	}
	
	bool handle(vnl::Packet* packet) {
		if(packet->pkt_id == vnl::Sample::PID) {
			Sample* sample = (Sample*)packet->payload;
			if(sample->data) {
				Hash64 domain = packet->dst_addr.domain();
				if(domain_map.find(domain)) {
					Entry* p_entry = dynamic_cast<Entry*>(sample->data);
					int64_t* p_version = 0;
					if(p_entry) {
						p_version = storage_map.find(p_entry->key);
					}
					if(!p_version || p_entry->version > *p_version) {
						RecordValue rec;
						rec.time = vnl::currentTimeMicros();
						rec.domain = domain;
						rec.topic = packet->dst_addr.topic();
						rec.header = sample->header;
						rec.value = sample->data;
						vnl::write(out, rec);
						out.flush();
						if(!header.num_samples++) {
							header.begin_time = rec.time;
						}
						header.end_time = rec.time;
						rec.header.release();
						rec.value.release();
						if(p_entry) {
							if(p_version) {
								*p_version = p_entry->version;
							} else {
								storage_map[p_entry->key] = p_entry->version;
							}
						}
						if(sample->header && !topic_map.find(packet->dst_addr)) {
							topic_map[packet->dst_addr] = sample->header->dst_topic;
						}
					}
				}
			}
		}
		return Super::handle(packet);
	}
	
private:
	void update() {
		write_header();
		::fflush(file);
		
		RecorderStatus* status = RecorderStatus::create();
		status->time = vnl::currentTimeMicros();
		status->filename = filepath;
		status->is_recording = true;
		status->is_error = out.error();
		publish(status, my_private_domain, "recorder_status");
	}
	
	void write_header() {
		out.flush();
		::fseek(file, 0, SEEK_SET);
		header.topics = topic_map.values();
		vnl::write(out, header);
		out.flush();
		::fseek(file, 0, SEEK_END);
	}
	
private:
	String filepath;
	vnl::io::File file;
	vnl::io::TypeOutput out;
	
	RecordHeader header;
	
	Map<Hash64, String> domain_map;
	Map<Address, Topic> topic_map;
	Map<Hash64, int64_t> storage_map;
	
};


} // vnl

#endif /* CPP_INCLUDE_VNI_RECORDER_H_ */
