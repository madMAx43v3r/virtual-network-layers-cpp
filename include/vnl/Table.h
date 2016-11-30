/*
 * Table.h
 *
 *  Created on: Nov 30, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_TABLE_H_
#define INCLUDE_VNL_TABLE_H_

#include <vnl/io.h>
#include <vnl/TableSupport.hxx>
#include <vnl/Exception.hxx>


namespace vnl {

template<typename T>
class Table : public TableBase<T> {
public:
	Table(const vnl::String& domain_, const vnl::String& topic_)
		:	TableBase<T>::TableBase(domain_, topic_)
	{
	}
	
protected:
	void insert(const T& row) {
		
	}
	
private:
	
};


} // vnl

#endif /* INCLUDE_VNL_TABLE_H_ */
