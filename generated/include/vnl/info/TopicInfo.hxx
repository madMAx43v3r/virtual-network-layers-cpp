
#ifndef INCLUDE_VNI_GENERATED_vnl_info_TopicInfo_HXX_
#define INCLUDE_VNI_GENERATED_vnl_info_TopicInfo_HXX_

// AUTO GENERATED by virtual-network-interface codegen

#include <vnl/Hash64.h>
#include <vnl/Map.h>
#include <vnl/Topic.hxx>
#include <vnl/Value.hxx>

#include <vnl/Type.hxx>


namespace vnl {
namespace info {

class TopicInfo : public vnl::Value {
public:
	static const uint32_t VNI_HASH = 0x1e3eb783;
	static const uint32_t NUM_FIELDS = 7;
	
	
	vnl::Topic topic;
	vnl::Map<vnl::Hash64, int64_t > publishers;
	vnl::Map<vnl::Hash64, int64_t > subscribers;
	int64_t send_counter;
	int64_t receive_counter;
	int64_t first_time;
	int64_t last_time;
	
	TopicInfo();
	
	static TopicInfo* create();
	virtual TopicInfo* clone() const;
	virtual bool assign(const vnl::Value& _value);
	virtual void raise() const { throw *this; }
	virtual void destroy();
	
	virtual void serialize(vnl::io::TypeOutput& _out) const;
	virtual void deserialize(vnl::io::TypeInput& _in, int _size);
	
	virtual uint32_t get_vni_hash() const { return VNI_HASH; }
	virtual const char* get_type_name() const { return "vnl.info.TopicInfo"; }
	
	virtual int get_type_size() const { return sizeof(TopicInfo); }
	virtual int get_num_fields() const { return NUM_FIELDS; }
	virtual int get_field_index(vnl::Hash32 _hash) const;
	virtual const char* get_field_name(int _index) const;
	virtual void get_field(int _index, vnl::String& _str) const;
	virtual void set_field(int _index, const vnl::String& _str);
	virtual void get_field(int _index, vnl::io::TypeOutput& _out) const;
	virtual void set_field(int _index, vnl::io::TypeInput& _in);
	virtual void get_field(int _index, vnl::Var& _var) const;
	virtual void set_field(int _index, const vnl::Var& _var);
	
	
};

} // namespace
} // namespace

#endif // INCLUDE_VNI_GENERATED_vnl_info_TopicInfo_HXX_
