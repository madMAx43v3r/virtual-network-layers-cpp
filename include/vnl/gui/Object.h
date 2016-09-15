/*
 * Object.h
 *
 *  Created on: Sep 14, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_GUI_OBJECT_H_
#define INCLUDE_VNL_GUI_OBJECT_H_

#include <vnl/gui/ObjectSupport.hxx>
#include <vnl/gui/Main.h>
#include <vnl/ThreadEngine.h>


namespace vnl {
namespace gui {

class Object : public ObjectBase {
public:
	Object(const vnl::String& domain, const vnl::String& topic)
		:	ObjectBase(domain, topic), window(0)
	{
		window_title = topic;
	}
	
protected:
	void main() {
		Fl::lock();
		char buf[1024];
		window_title.to_string(buf, sizeof(buf));
		Fl_Window win_(window_width, window_height, buf);
		window = &win_;
		window->begin();
		setup();
		window->end();
		Fl::unlock();
		show_window_t show(window);
		send(&show, vnl_gui_main);
		run();
	}
	
	virtual void setup() = 0;
	
protected:
	Fl_Window* window;
	
};


}}

#endif /* INCLUDE_VNL_GUI_OBJECT_H_ */
