
#ifndef INCLUDE_VNI_GENERATED_vnl_Player_CLIENT_HXX_
#define INCLUDE_VNI_GENERATED_vnl_Player_CLIENT_HXX_

// AUTO GENERATED by virtual-network-interface codegen

#include <vnl/Array.h>
#include <vnl/Hash32.h>
#include <vnl/Map.h>
#include <vnl/String.h>
#include <vnl/Topic.hxx>
#include <vnl/info/PlayerStatus.hxx>
#include <vnl/info/Type.hxx>

#include <vnl/ObjectClient.hxx>


namespace vnl {

class PlayerClient : public vnl::ObjectClient {
public:
	class Writer : public vnl::ObjectClient::Writer {
	public:
		Writer(vnl::io::TypeOutput& _out, bool _top_level = false) 
			:	vnl::ObjectClient::Writer(_out, false),
				_out(_out), _top_level(_top_level)
		{
			if(_top_level) {
				_out.putEntry(VNL_IO_INTERFACE, VNL_IO_BEGIN);
				_out.putHash(0xe536451b);
			}
		}
		~Writer() {
			if(_top_level) {
				_out.putEntry(VNL_IO_INTERFACE, VNL_IO_END);
			}
		}
		void get_type_info() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0xf73490b7);
		}
		void pause() {
			_out.putEntry(VNL_IO_CALL, 0);
			_out.putHash(0xe745115d);
		}
		void seek_rel(float pos) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0xe1fbf4c8);
			vnl::write(_out, pos);
		}
		void stop() {
			_out.putEntry(VNL_IO_CALL, 0);
			_out.putHash(0xf2cbfddb);
		}
		void reset() {
			_out.putEntry(VNL_IO_CALL, 0);
			_out.putHash(0xbd19b5cb);
		}
		void get_topics() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0x71ce1b5a);
		}
		void scan() {
			_out.putEntry(VNL_IO_CALL, 0);
			_out.putHash(0x73fed50f);
		}
		void play() {
			_out.putEntry(VNL_IO_CALL, 0);
			_out.putHash(0x25c4211b);
		}
		void get_status() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0x4370e1e0);
		}
		void seek(int64_t time) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0x9b2fb63);
			vnl::write(_out, time);
		}
		void open(const vnl::String& file) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0x5c4affeb);
			vnl::write(_out, file);
		}
		void set_filename(const vnl::String& _value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0xb60d3446);
			vnl::write(_out, _value);
		}
		void set_domain_blacklist(const vnl::Array<vnl::String >& _value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0xa9decee6);
			vnl::write(_out, _value);
		}
		void set_topic_blacklist(const vnl::Array<vnl::Topic >& _value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0xca73652e);
			vnl::write(_out, _value);
		}
		void set_autostart(bool _value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0x5902c26f);
			vnl::write(_out, _value);
		}
		void set_autoloop(bool _value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0x3cc6f24e);
			vnl::write(_out, _value);
		}
		void set_autoshutdown(bool _value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0x51558795);
			vnl::write(_out, _value);
		}
		void set_interval(int32_t _value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0xd129c896);
			vnl::write(_out, _value);
		}
		void get_filename() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0xb60d3446);
		}
		void get_domain_blacklist() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0xa9decee6);
		}
		void get_topic_blacklist() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0xca73652e);
		}
		void get_autostart() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0x5902c26f);
		}
		void get_autoloop() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0x3cc6f24e);
		}
		void get_autoshutdown() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0x51558795);
		}
		void get_interval() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0xd129c896);
		}
	protected:
		vnl::io::TypeOutput& _out;
		bool _top_level;
	};
	
	PlayerClient() {}
	
	PlayerClient(const PlayerClient& other) {
		vnl::Client::set_address(other.vnl::Client::get_address());
	}
	
	PlayerClient(const vnl::Address& addr) {
		vnl::Client::set_address(addr);
	}
	
	PlayerClient& operator=(const vnl::Address& addr) {
		vnl::Client::set_address(addr);
		return *this;
	}
	
	vnl::Map<vnl::Hash32, vnl::info::Type > get_type_info() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_type_info();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		vnl::Map<vnl::Hash32, vnl::info::Type > _result;
		if(_pkt) {
			vnl::read(_in, _result);
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
		return _result;
	}
	
	void pause() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.pause();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void seek_rel(float pos) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.seek_rel(pos);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void stop() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.stop();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void reset() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.reset();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	vnl::Array<vnl::Topic > get_topics() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_topics();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		vnl::Array<vnl::Topic > _result;
		if(_pkt) {
			vnl::read(_in, _result);
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
		return _result;
	}
	
	void scan() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.scan();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void play() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.play();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	vnl::info::PlayerStatus get_status() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_status();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		vnl::info::PlayerStatus _result;
		if(_pkt) {
			vnl::read(_in, _result);
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
		return _result;
	}
	
	void seek(int64_t time) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.seek(time);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void open(const vnl::String& file) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.open(file);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void set_filename(const vnl::String& filename) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.set_filename(filename);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void set_domain_blacklist(const vnl::Array<vnl::String >& domain_blacklist) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.set_domain_blacklist(domain_blacklist);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void set_topic_blacklist(const vnl::Array<vnl::Topic >& topic_blacklist) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.set_topic_blacklist(topic_blacklist);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void set_autostart(bool autostart) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.set_autostart(autostart);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void set_autoloop(bool autoloop) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.set_autoloop(autoloop);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void set_autoshutdown(bool autoshutdown) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.set_autoshutdown(autoshutdown);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void set_interval(int32_t interval) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.set_interval(interval);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	vnl::String get_filename() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_filename();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		vnl::String _result;
		if(_pkt) {
			vnl::read(_in, _result);
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
		return _result;
	}
	
	vnl::Array<vnl::String > get_domain_blacklist() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_domain_blacklist();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		vnl::Array<vnl::String > _result;
		if(_pkt) {
			vnl::read(_in, _result);
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
		return _result;
	}
	
	vnl::Array<vnl::Topic > get_topic_blacklist() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_topic_blacklist();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		vnl::Array<vnl::Topic > _result;
		if(_pkt) {
			vnl::read(_in, _result);
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
		return _result;
	}
	
	bool get_autostart() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_autostart();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		bool _result;
		if(_pkt) {
			vnl::read(_in, _result);
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
		return _result;
	}
	
	bool get_autoloop() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_autoloop();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		bool _result;
		if(_pkt) {
			vnl::read(_in, _result);
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
		return _result;
	}
	
	bool get_autoshutdown() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_autoshutdown();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		bool _result;
		if(_pkt) {
			vnl::read(_in, _result);
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
		return _result;
	}
	
	int32_t get_interval() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_interval();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		int32_t _result;
		if(_pkt) {
			vnl::read(_in, _result);
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
		return _result;
	}
	
};


} // namespace

#endif // INCLUDE_VNI_GENERATED_vnl_Player_CLIENT_HXX_
