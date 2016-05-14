/*
 * Engine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: mad
 */

#include <assert.h>
#include <string.h>

#include "phy/Engine.h"
#include "phy/Stream.h"
#include "phy/Object.h"
#include "phy/Registry.h"
#include "phy/Random.h"


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
