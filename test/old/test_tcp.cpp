/*
 * test_tcp.cpp
 *
 *  Created on: Jan 2, 2016
 *      Author: mad
 */

#include <assert.h>
#include <io/poll/PollServer.h>

#include "io/Socket.h"
#include "phy/FiberEngine.h"
#include "phy/ThreadEngine.h"
#include "phy/Processor.h"
using namespace vnl::phy;
using namespace vnl::io;

static const bool verify = false;

class EchoServer : public vnl::io::Socket {
public:
	EchoServer(Object* parent, int port) : Socket(parent) {
		launch(std::bind(&EchoServer::server_loop, this, port));
	}
protected:
	void server_loop(int port) {
		if(create() < 0) {
			printf("Error: EchoServer::create() failed.\n"); return;
		}
		if(bind(port) < 0) {
			printf("Error: EchoServer::bind() failed.\n"); return;
		}
		if(listen() < 0) {
			printf("Error: EchoServer::listen() failed.\n"); return;
		}
		printf("Server ready.\n");
		while(true) {
			Socket* client = accept();
			if(client) {
				printf("Accepted new Client %lu\n", client->getMAC());
				launch(std::bind(&EchoServer::echo_loop, this, client));
			} else {
				printf("Server::accept() error.\n");
				break;
			}
		}
	}
	int counter = 0;
	uint64_t last = vnl::System::currentTimeMillis();
	void echo_loop(Socket* sock) {
		unsigned char buf[1024];
		while(true) {
			int n = sock->read(buf, sizeof(buf));
			if(n <= 0) {
				printf("Server::read() error.\n");
				break;
			}
			//printf("Server::read() got %d bytes.\n", n);
			for(int i = 0; i < n && verify; ++i) {
				if(buf[i] > 0 && i > 0) {
					assert(buf[i-1] == buf[i]-1);
				}
			}
			if(!sock->write(buf, n)) {
				printf("Server::write() error.\n");
				break;
			}
			counter += n;
			if(counter >= 100*1024*1024) {
				uint64_t now = vnl::System::currentTimeMillis();
				std::cout << (now-last) << " EchoServer " << getMAC() << std::endl;
				last = now;
				counter = 0;
			}
			yield();
		}
	}
};

class Client : public vnl::io::Socket {
public:
	Client(Object* parent) : Socket(parent) {}
	void start(std::string addr, int port) {
		launch(std::bind(&Client::setup, this, addr, port));
	}
protected:
	void setup(std::string addr, int port) {
		sleep(100 + std::rand() % 1000);
		if(create() < 0) {
			printf("Error: Client::create() failed.\n"); return;
		}
		if(connect(addr, port) < 0) {
			printf("Error: Client::connect() failed.\n"); return;
		}
		printf("Client connected.\n");
		launch(std::bind(&Client::writer, this));
		launch(std::bind(&Client::reader, this));
	}
	void writer() {
		unsigned char buf[1024];
		for(int i = 0; i < sizeof(buf); ++i) {
			buf[i] = i;
		}
		while(true) {
			int n = std::rand() % sizeof(buf);
			if(!write(buf, n)) {
				printf("Client::write() error.\n");
				break;
			}
			yield();
		}
	}
	void reader() {
		unsigned char buf[1024];
		while(true) {
			int n = read(buf, sizeof(buf));
			if(n > 0) {
				for(int i = 0; i < n && verify; ++i) {
					if(buf[i] > 0 && i > 0) {
						assert(buf[i-1] == buf[i]-1);
					}
				}
			} else {
				printf("Client::read() error.\n");
				break;
			}
			yield();
		}
	}
};

int main() {
	
	const int debug = 0;
	const int N = 10;
	
	socket::Server::instance = new poll::Server(new FiberEngine(10));
	socket::Server::instance->start(0);
	
	Processor* linkA = new Processor(new FiberEngine(10));
	Processor* linkB = new Processor(new FiberEngine(10));
	
	EchoServer* echo = new EchoServer(linkA, 1337);
	
	Client** clients = new Client*[N];
	for(int i = 0; i < N; ++i) {
		clients[i] = new Client(linkB);
		clients[i]->start("localhost", 1337);
	}
	
	linkA->start(0);
	linkB->start(2);
	
	std::this_thread::sleep_for(std::chrono::seconds(1*60));
	
	linkA->stop();
	linkB->stop();
	socket::Server::instance->stop();
	
	for(int i = 0; i < N; ++i) {
		delete clients[i];
	}
	delete echo;
	delete linkA;
	delete linkB;
	delete socket::Server::instance;
	
}


