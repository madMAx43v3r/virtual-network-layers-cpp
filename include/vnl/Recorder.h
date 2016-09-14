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
		file.fd = ::fileno(p_file);
		for(String& domain : domains) {
			subscribe(domain, "");
			log(DEBUG).out << "Recording domain: " << domain << vnl::endl;
		}
		run();
		::fclose(p_file);
	}
	
	void handle(const vnl::Value& sample, const vnl::Packet& packet) {
		vnl::RecordValue rec;
		rec.time = vnl::currentTimeMicros();
		rec.domain = packet.dst_addr.domain();
		rec.topic = packet.dst_addr.topic();
		rec.value = sample.clone();
		rec.serialize(out);
		out.flush();
	}
	
private:
	vnl::io::File file;
	vnl::io::TypeOutput out;
	
};



}

#endif /* CPP_INCLUDE_VNI_RECORDER_H_ */
