/*
 * Pipe.h
 *
 *  Created on: May 19, 2016
 *      Author: mad
 */

#ifndef INCLUDE_PHY_PIPE_H_
#define INCLUDE_PHY_PIPE_H_

#include "phy/Node.h"
#include "phy/Reference.h"


namespace vnl { namespace phy {


class Pipe : public Reactor {
public:
	Pipe(Engine* engine, const vnl::String& dst_name) : dst(engine, dst_name) {}
	
	
	
protected:
	
	
private:
	Reference<Object> dst;
	
};


}}

#endif /* INCLUDE_PHY_PIPE_H_ */
