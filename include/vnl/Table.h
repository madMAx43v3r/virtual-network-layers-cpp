/*
 * Table.h
 *
 *  Created on: Nov 30, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_TABLE_H_
#define INCLUDE_VNL_TABLE_H_

#include <vnl/TableSupport.hxx>
#include <vnl/DuplicateKeyException.hxx>
#include <vnl/NoSuchKeyException.hxx>
#include <vnl/NoSuchFieldException.hxx>


namespace vnl {

template<typename T>
class View {
public:
	virtual ~View() {}
	
	/*
	 * Called before row is inserted, memory address is temporary. Should throw.
	 */
	virtual void check_insert(const T& row) = 0;
	
	/*
	 * Called after row was inserted, memory address is valid forever. Cannot throw.
	 */
	virtual void insert(const T& row) = 0;
	
	/*
	 * Called before the field is updated. Should throw.
	 */
	virtual void check_update(const T& row, int field, const Binary& value) = 0;
	
	/*
	 * Called before the field is updated. Cannot throw.
	 */
	virtual void update(const T& row, int field, const Binary& value) = 0;
	
	/*
	 * Called before the row is deleted. Cannot throw.
	 */
	virtual void remove(const T& row) = 0;
	
};


template<typename T>
class Table : public TableBase<T> {
public:
	Table(const vnl::String& domain_, const vnl::String& topic_)
		:	TableBase<T>::TableBase(domain_, topic_),
			in(&buffer)
	{
	}
	
protected:
	T get(const Hash64& id) const {
		return *get_row(id);
	}
	
	Array<T> get_all(const Array<Hash64>& ids) const {
		Array<T> result;
		for(Hash64 id : ids) {
			const T* row = table.find(id);
			if(row) {
				result.push_back(*row);
			}
		}
		return result;
	}
	
	void insert(const T& row) {
		if(table.find(row.id)) {
			throw DuplicateKeyException();
		}
		for(View<T>* view : views) {
			view->check_insert(row);
		}
		T& ref = table[row.id];
		ref = row;
		for(View<T>* view : views) {
			view->insert(ref);
		}
	}
	
	void update(const Hash64& id, const Hash32& field_name, const Binary& value) {
		T* row = get_row(id);
		int index = row->field_index(field_name);
		if(index < 0) {
			throw NoSuchFieldException();
		}
		for(View<T>* view : views) {
			view->check_update(*row, index, value);
		}
		for(View<T>* view : views) {
			view->update(*row, index, value);
		}
		buffer.wrap(value.data, value.size);
		in.reset();
		row->set_field(index, in);
		if(in.error()) {
			throw IOException();
		}
	}
	
	void remove(const vnl::Hash64& id) {
		const T* row = get_row(id);
		if(row) {
			for(View<T>* view : views) {
				view->remove(*row);
			}
			table.erase(id);
		}
	}
	
	void add_view(View<T>* view) {
		views.push_back(view);
	}
	
private:
	T* get_row(Hash64 id) {
		T* row = table.find(id);
		if(!row) {
			throw NoSuchKeyException();
		}
		return row;
	}
	
	const T* get_row(Hash64 id) const {
		const T* row = table.find(id);
		if(!row) {
			throw NoSuchKeyException();
		}
		return row;
	}
	
private:
	Map<Hash64, T> table;
	Array<View<T>*> views;
	vnl::io::ByteBuffer buffer;
	vnl::io::TypeInput in;
	
};


#define VNL_UNIQUE_INDEX(T, K, field_name, index_name) \
class index_name_t : public vnl::View<T> { \
public: \
	index_name_t() : in(&buffer) {} \
	const T* get(const K& key) const { \
		const T* const* ptr = index.find(key); \
		return ptr ? *ptr : 0; \
	} \
	void check_insert(const T& row) { \
		if(index.find(row.field_name)) { \
			throw vnl::DuplicateKeyException(); \
		} \
	} \
	void insert(const T& row) { \
		index[row.field_name] = &row; \
		if(our_field < 0) { \
			our_field = row.field_index(#field_name); \
		} \
	} \
	void check_update(const T& row, int field, const vnl::Binary& value) { \
		if(field != our_field) { \
			return; \
		} \
		buffer.wrap(value.data, value.size); \
		in.reset(); \
		vnl::read(in, tmp_key); \
		if(in.error()) { \
			throw vnl::IOException(); \
		} \
		if(index.find(tmp_key)) { \
			throw vnl::DuplicateKeyException(); \
		} \
	} \
	void update(const T& row, int field, const vnl::Binary& value) { \
		if(field != our_field) { \
			return; \
		} \
		index.erase(row.field_name); \
		index[tmp_key] = &row; \
	} \
	void remove(const T& row) { \
		index.erase(row.field_name); \
	} \
private: \
	vnl::Map<K, const T*> index; \
	vnl::io::ByteBuffer buffer; \
	vnl::io::TypeInput in; \
	int our_field = -1; \
	K tmp_key; \
} index_name;


} // vnl

#endif /* INCLUDE_VNL_TABLE_H_ */
