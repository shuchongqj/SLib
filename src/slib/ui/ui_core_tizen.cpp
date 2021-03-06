/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "slib/core/definition.h"

#if defined(SLIB_PLATFORM_IS_TIZEN)

#include "ui_core_common.h"

#include "slib/ui/mobile_app.h"
#include "slib/ui/screen.h"
#include "slib/ui/window.h"
#include "slib/ui/platform.h"

#include "slib/core/platform_tizen.h"
#include "slib/core/log.h"
#include "slib/core/safe_static.h"
#include "slib/core/scoped.h"

#include <app.h>
#include <Ecore.h>
#include <system_info.h>
#include <Elementary.h>

#include <pthread.h>

namespace slib
{

	pthread_t _g_main_thread = 0;

	class _Tizen_Screen : public Screen
	{
	public:
		int m_width;
		int m_height;

	public:
		static Ref<_Tizen_Screen> create()
		{
			Ref<_Tizen_Screen> ret = new _Tizen_Screen();
			if (ret.isNotNull()) {
				ret->m_width = 0;
				::system_info_get_platform_int("http://tizen.org/feature/screen.width", &(ret->m_width));
				ret->m_height = 0;
				::system_info_get_platform_int("http://tizen.org/feature/screen.height", &(ret->m_height));
				return ret;
			}
			return sl_null;
		}

	public:
		// override
		UIRect getRegion()
		{
			int rotation = 0;

			Evas_Object* win = UIPlatform::getMainWindow();
			if (win) {
				rotation = ::elm_win_rotation_get(win);
			}

			List<int> orientations = UI::getAvailableScreenOrientations();
			if(orientations.getCount() > 0) {
				if(orientations.indexOf(rotation) == -1) {
					rotation = orientations.getValueAt(0);
				}
			}

			UIRect ret;
			ret.left = 0;
			ret.top = 0;
			if (rotation == 90 || rotation == 270) {
				ret.right = (sl_ui_pos)m_height;
				ret.bottom = (sl_ui_pos)m_width;
			} else {
				ret.right = (sl_ui_pos)m_width;
				ret.bottom = (sl_ui_pos)m_height;
			}
			return ret;
		}
	};

	Ref<Screen> UI::getPrimaryScreen()
	{
		SLIB_STATIC_ZERO_INITIALIZED(AtomicRef<Screen>, ret)
		if (SLIB_SAFE_STATIC_CHECK_FREED(ret)) {
			return sl_null;
		}
		if (ret.isNull()) {
			ret = _Tizen_Screen::create();
		}
		return ret;
	}

	Ref<Screen> UI::getFocusedScreen()
	{
		return UI::getPrimaryScreen();
	}

	List< Ref<Screen> > UI::getScreens()
	{
		List< Ref<Screen> > ret;
		Ref<Screen> screen = UI::getPrimaryScreen();
		if (screen.isNotNull()) {
			ret.add_NoLock(screen);
		}
		return ret;
	}

	void UI::openUrl(const String& url) {
		app_control_h app_control;
		if (0 == ::app_control_create(&app_control)) {
			::app_control_set_operation(app_control, APP_CONTROL_OPERATION_DEFAULT);
			::app_control_set_app_id(app_control, "com.samsung.browser");
			::app_control_set_uri(app_control, url.getData());
			::app_control_send_launch_request(app_control, NULL, NULL);
			::app_control_destroy(app_control);
		}
	}

	sl_bool UI::isUiThread()
	{
		return _g_main_thread == ::pthread_self();
	}
	
	static void _ui_dispatch_callback(void* data)
	{
		_UIDispatcher::processCallbacks();
	}

	static Eina_Bool _ui_dispatch_timer_callback(void* data)
	{
		Callable<void()>* callable = reinterpret_cast<Callable<void()>*>(data);
		callable->invoke();
		callable->decreaseReference();
		return ECORE_CALLBACK_CANCEL;
	}

	void UI::dispatchToUiThread(const Function<void()>& callback, sl_uint32 delayMillis)
	{
		if (callback.isNull()) {
			return;
		}
		if (delayMillis == 0) {
			if (_UIDispatcher::addCallback(callback)) {
				::ecore_main_loop_thread_safe_call_async(_ui_dispatch_callback, sl_null);
			}
		} else {
			Callable<void()>* callable = callback.ref.get();
			callable->increaseReference();
			::ecore_timer_loop_add((double)delayMillis / 1000.0, _ui_dispatch_timer_callback, callable);
		}
	}


	sl_int32 _g_main_loop_level = 0;
	void UIPlatform::runLoop(sl_uint32 level)
	{
		sl_int32 loopLevel = Base::interlockedIncrement32(&_g_main_loop_level);
		while (loopLevel == _g_main_loop_level) {
			::ecore_main_loop_iterate();
		}
	}

	static void _ui_quit_callback(void* data)
	{
	}

	void UIPlatform::quitLoop()
	{
		Base::interlockedDecrement(&_g_main_loop_level);
		::ecore_main_loop_thread_safe_call_async(_ui_quit_callback, sl_null);
	}

	static bool _ui_callback_app_create(void* data)
	{
		Log("App", "Create");
		::elm_config_accel_preference_set("opengl");
		UIApp::dispatchStartToApp();
		MobileApp::dispatchCreateActivityToApp();
		return true;
	}

	static void _ui_callback_app_resume(void* data)
	{
		Log("App", "Resume");
		MobileApp::dispatchResumeToApp();
	}

	static void _ui_callback_app_pause(void* data)
	{
		Log("App", "Pause");
		MobileApp::dispatchPauseToApp();
	}

	static void _ui_callback_app_terminate(void* data)
	{
		Log("App", "Terminate");
		MobileApp::dispatchDestroyActivityToApp();
		UIApp::dispatchExitToApp();
	}

	void UIPlatform::runApp()
	{
		_g_main_thread = ::pthread_self();

		Ref<Application> app = Application::getApp();
		if (app.isNull()) {
			return;
		}

		ListLocker<String> args = app->getArguments();
		sl_uint32 n = (sl_uint32)(args.count);
		SLIB_SCOPED_BUFFER(char*, 64, p, n)
		for (sl_uint32 i = 0; i < n; i++) {
			p[i] = args[i].getData();
		}

		ui_app_lifecycle_callback_s event_callback = {0,};
		event_callback.create = _ui_callback_app_create;
		event_callback.resume = _ui_callback_app_resume;
		event_callback.pause = _ui_callback_app_pause;
		event_callback.terminate = _ui_callback_app_terminate;

		::ui_app_main(n, p, &event_callback, sl_null);

	}

	void UIPlatform::quitApp()
	{
		::ui_app_exit();
	}

}

#endif
