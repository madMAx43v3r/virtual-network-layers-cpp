
#ifndef INCLUDE_VNI_GENERATED_vnl_StorageServer_CLIENT_HXX_
#define INCLUDE_VNI_GENERATED_vnl_StorageServer_CLIENT_HXX_

// AUTO GENERATED by virtual-network-interface codegen

#include <vnl/Address.h>
#include <vnl/Array.h>
#include <vnl/Entry.hxx>
#include <vnl/Hash64.h>
#include <vnl/List.h>
#include <vnl/Pointer.h>
#include <vnl/String.h>

#include <vnl/ObjectClient.hxx>


namespace vnl {

class StorageServerClient : public vnl::ObjectClient {
public:
	class Writer : public vnl::ObjectClient::Writer {
	public:
		Writer(vnl::io::TypeOutput& _out, bool _top_level = false) 
			:	vnl::ObjectClient::Writer(_out, false),
				_out(_out), _top_level(_top_level)
		{
			if(_top_level) {
				_out.putEntry(VNL_IO_INTERFACE, VNL_IO_BEGIN);
				_out.putHash(0x2362f4c1);
			}
		}
		~Writer() {
			if(_top_level) {
				_out.putEntry(VNL_IO_INTERFACE, VNL_IO_END);
			}
		}
		void delete_entry(const vnl::Hash64& key) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0x6be27404);
			vnl::write(_out, key);
		}
		void put_entry(const vnl::Pointer<vnl::Entry >& value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0x53e783cb);
			vnl::write(_out, value);
		}
		void get_entries(const vnl::Array<vnl::Hash64 >& keys) {
			_out.putEntry(VNL_IO_CONST_CALL, 1);
			_out.putHash(0xdc37fd46);
			vnl::write(_out, keys);
		}
		void get_entry(const vnl::Hash64& key) {
			_out.putEntry(VNL_IO_CONST_CALL, 1);
			_out.putHash(0x2d43064b);
			vnl::write(_out, key);
		}
		void set_filename(const vnl::String& _value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0xb60d3446);
			vnl::write(_out, _value);
		}
		void set_interval(int32_t _value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0xd129c896);
			vnl::write(_out, _value);
		}
		void set_readonly(bool _value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0x972da0ea);
			vnl::write(_out, _value);
		}
		void set_truncate(bool _value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0x1725750d);
			vnl::write(_out, _value);
		}
		void set_topics(const vnl::List<vnl::Address >& _value) {
			_out.putEntry(VNL_IO_CALL, 1);
			_out.putHash(0x22dd6aa2);
			vnl::write(_out, _value);
		}
		void get_filename() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0xb60d3446);
		}
		void get_interval() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0xd129c896);
		}
		void get_readonly() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0x972da0ea);
		}
		void get_truncate() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0x1725750d);
		}
		void get_topics() {
			_out.putEntry(VNL_IO_CONST_CALL, 0);
			_out.putHash(0x22dd6aa2);
		}
	protected:
		vnl::io::TypeOutput& _out;
		bool _top_level;
	};
	
	StorageServerClient() {}
	
	StorageServerClient(const StorageServerClient& other) {
		vnl::Client::set_address(other.vnl::Client::get_address());
	}
	
	StorageServerClient(const vnl::Address& addr) {
		vnl::Client::set_address(addr);
	}
	
	StorageServerClient& operator=(const vnl::Address& addr) {
		vnl::Client::set_address(addr);
		return *this;
	}
	
	void delete_entry(const vnl::Hash64& key) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.delete_entry(key);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void put_entry(const vnl::Pointer<vnl::Entry >& value) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.put_entry(value);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	vnl::Array<vnl::Pointer<vnl::Entry > > get_entries(const vnl::Array<vnl::Hash64 >& keys) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_entries(keys);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		vnl::Array<vnl::Pointer<vnl::Entry > > _result;
		if(_pkt) {
			vnl::read(_in, _result);
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
		return _result;
	}
	
	vnl::Pointer<vnl::Entry > get_entry(const vnl::Hash64& key) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_entry(key);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		vnl::Pointer<vnl::Entry > _result;
		if(_pkt) {
			vnl::read(_in, _result);
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
		return _result;
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
	
	void set_readonly(bool readonly) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.set_readonly(readonly);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void set_truncate(bool truncate) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.set_truncate(truncate);
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CALL);
		if(_pkt) {
			_pkt->ack();
		} else {
			throw vnl::IOException();
		}
	}
	
	void set_topics(const vnl::List<vnl::Address >& topics) {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.set_topics(topics);
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
	
	bool get_readonly() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_readonly();
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
	
	bool get_truncate() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_truncate();
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
	
	vnl::List<vnl::Address > get_topics() {
		_buf.wrap(_data);
		{
			Writer _wr(_out);
			_wr.get_topics();
		}
		vnl::Packet* _pkt = _call(vnl::Frame::CONST_CALL);
		vnl::List<vnl::Address > _result;
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

#endif // INCLUDE_VNI_GENERATED_vnl_StorageServer_CLIENT_HXX_
