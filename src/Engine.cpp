/*
 * Engine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include <assert.h>
#include <string.h>

#include "vnl/Engine.h"
#include "vnl/Module.h"
#include "vnl/Registry.h"
#include "vnl/Random.h"


namespace vnl {

Engine::Engine()
	:	queue(memory), buffer(memory)
{
	mac = Random64::global_rand();
}

void Engine::exec(Module* object) {
	object->exec(this);
	Registry::finished_t msg(object);
	send(this, &msg, Registry::instance);
}





}