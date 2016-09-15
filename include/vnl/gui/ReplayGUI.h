/*
 * ReplayGUI.h
 *
 *  Created on: Sep 15, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_GUI_REPLAYGUI_H_
#define INCLUDE_VNL_GUI_REPLAYGUI_H_

#include <vnl/PlayerSupport.hxx>
#include <vnl/gui/ReplayGUISupport.hxx>


namespace vnl {
namespace gui {

class ReplayGUI : public ReplayGUIBase {
public:
	ReplayGUI(const vnl::String& domain)
		:	ReplayGUIBase(domain, "ReplayGUI"),
			callback(vnl::Address(domain, "Player"))
	{
	}
	
	class Callback {
	public:
		Callback(vnl::Address target) {
			player.set_fail(true);
			player.set_address(target);
			player.connect(&engine);
		}
		
		static void play(Fl_Widget* obj, void* ptr) {
			((Callback*)ptr)->player.play();
		}
		
	private:
		vnl::ThreadEngine engine;
		vnl::PlayerClient player;
	};
	
protected:
	void setup() {
		
	}
	
	void handle(const vnl::info::PlayerStatus& status) {
		
	}
	
private:
	Callback callback;
	
};


}}

#endif /* INCLUDE_VNL_GUI_REPLAYGUI_H_ */
