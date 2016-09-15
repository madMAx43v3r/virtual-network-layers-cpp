/*
 * Recorder.h
 *
 *  Created on: Jul 4, 2016
 *      Author: mad
 */

#ifndef CPP_INCLUDE_VNI_RECORDER_H_
#define CPP_INCLUDE_VNI_RECORDER_H_

#include <vnl/RecorderSupport.hxx>
#include <vnl/RecordValue.hxx>
#include <vnl/io/File.h>


namespace vnl {

class Recorder : public vnl::RecorderBase {
public:
	Recorder(const vnl::String& domain_, const vnl::String& topic_)
		:	RecorderBase(domain_, topic_), out(&file)
	{
		domains.push_back();
	}
	
protected:
	void main() {
		String filepath = filename << vnl::currentTime() << ".dat";
		char buf[1024];
		filepath.to_string(buf, sizeof(buf));
		FILE* p_file = ::fopen(buf, "w");
		if(!p_file) {
			log(ERROR).out << "Unable to open file for writing: " << filepath << vnl::endl;
			return;
		}
		log(INFO).out << "Recording to file: " << filepath << vnl::endl;
		file.fd = ::fileno(p_file);
		for(String& domain : domains) {
			subscribe(domain, "");
			domain_map[vnl::hash64(domain)] = domain;
			log(DEBUG).out << "Recording domain: " << domain << vnl::endl;
		}
		run();
		::fclose(p_file);
	}
	
	bool handle(vnl::Packet* packet) {
		if(packet->pkt_id == vnl::Sample::PID) {
			vnl::Sample* sample = (vnl::Sample*)packet->payload;
			if(sample->data) {
				uint64_t domain = packet->dst_addr.domain();
				if(domain_map.find(domain)) {
					vnl::RecordValue rec;
					rec.time = vnl::currentTimeMicros();
					rec.domain = domain;
					rec.topic = packet->dst_addr.topic();
					rec.value = sample->data;
					vnl::write(out, rec);
					out.flush();
					rec.value.release();
				}
			}
		}
		return Super::handle(packet);
	}
	
private:
	vnl::io::File file;
	vnl::io::TypeOutput out;
	
	vnl::Map<uint64_t, String> domain_map;
	
};



}

#endif /* CPP_INCLUDE_VNI_RECORDER_H_ */
