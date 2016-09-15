/*
 * Main.h
 *
 *  Created on: Sep 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_GUI_MAIN_H_
#define INCLUDE_VNL_GUI_MAIN_H_

#include <vnl/Object.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>


namespace vnl {
namespace gui {

typedef MessageType<Fl_Window*, 0x282408e5> show_window_t;

class Main : public vnl::Object {
public:
	Main(const vnl::String& domain, const vnl::String& topic)
		:	Object(domain, topic)
	{
	}
	
	void receive(vnl::Message* msg) {
		Object::receive(msg);
		Fl::lock();
		Fl::awake((void*)0);
		Fl::unlock();
	}
	
protected:
	void main() {
		Fl::lock();
		while(dorun && Fl::wait() > 0) {
			Fl::thread_message();
			poll(0);
		}
		Fl::unlock();
	}
	
	bool handle(vnl::Message* msg) {
		switch(msg->msg_id) {
		case show_window_t::MID:
			((show_window_t*)msg)->data->show();
			break;
		}
		return Object::handle(msg);
	}
	
};


}}

extern vnl::gui::Main* vnl_gui_main;


#endif /* INCLUDE_VNL_GUI_MAIN_H_ */
