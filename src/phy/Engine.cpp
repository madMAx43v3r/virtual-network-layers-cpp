/*
 * Engine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include <assert.h>
#include <string.h>

#include "vnl/phy/Engine.h"
#include "vnl/phy/Stream.h"
#include "vnl/phy/Object.h"
#include "vnl/phy/Registry.h"
#include "vnl/phy/Random.h"


namespace vnl { namespace phy {

thread_local Engine* Engine::local = 0;

Engine::Engine()
	:	queue(memory), buffer(memory)
{
	assert(Engine::local == 0);
	Engine::local = this;
	mac = Random64::global_rand();
}

Engine::~Engine() {
	Engine::local = 0;
}

void Engine::exec(Object* object) {
	assert(Engine::local == this);
	object->exec(this);
	Registry::finished_t msg(object);
	send(this, &msg, Registry::instance);
}





}}
