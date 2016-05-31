#include "../../../inc/slib/core/definition.h"

#if defined(SLIB_PLATFORM_IS_WIN32)

#include "../../../inc/slib/ui/scroll_view.h"
#include "../../../inc/slib/ui/core.h"

#include "view_win32.h"

#define _SCROLL_LINE_SIZE 20
#define _SCROLL_WHEEL_SIZE 40

SLIB_UI_NAMESPACE_BEGIN
    
class _Win32_ScrollViewInstance : public Win32_ViewInstance
{
public:
	Color m_backgroundColor;

public:
    // override
	sl_bool processWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)
	{
		HWND handle = getHandle();
		if (msg == WM_ERASEBKGND) {
			Color color = m_backgroundColor;
			if (color.a > 0) {
				HBRUSH hbr = ::CreateSolidBrush(RGB(color.r, color.g, color.b));
				if (hbr) {
					HDC hDC = (HDC)(wParam);
					RECT rc;
					::GetClientRect(handle, &rc);
					::FillRect(hDC, &rc, hbr);
					::DeleteObject(hbr);
					result = TRUE;
					return sl_true;
				}
			}
		} else if (Windows::processWindowScrollEvents(handle, msg, wParam, lParam, _SCROLL_LINE_SIZE, _SCROLL_WHEEL_SIZE)) {
			Ref<View> view = getView();
			if (ScrollView::checkInstance(view.ptr)) {
				__refreshContentPosition((ScrollView*)(view.ptr));
			}
			return sl_true;
		}
		return sl_false;
	}

    // override
	sl_bool processCommand(SHORT code, LRESULT& result)
	{
		return sl_false;
	}

    // override
	sl_bool processNotify(NMHDR* nmhdr, LRESULT& result)
	{
		return sl_false;
	}

	void __setContentView(const Ref<View>& viewChild, ScrollView* viewParent)
	{
		if (viewChild.isNotNull()) {
			Ref<ViewInstance> instance = viewChild->attachToNewInstance(this);
			if (instance.isNotNull()) {
				__refreshContentSize(viewParent);
				__refreshContentPosition(viewParent);
			}
		}
	}

	void __refreshContentSize(ScrollView* view)
	{
		HWND handle = getHandle();
		if (handle) {
			Ref<View> viewContent = view->getContentView();
			Sizei sizeContent;
			if (viewContent.isNotNull()) {
				sizeContent = viewContent->getSize();
			} else {
				sizeContent = Sizei::zero();
			}
			Sizei sizeParent = view->getSize();
			Windows::setWindowHorizontalScrollParam(handle, 0, sizeContent.x, sizeParent.x);
			Windows::setWindowVerticalScrollParam(handle, 0, sizeContent.y, sizeParent.y);
			__refreshContentPosition(view);
		}
	}

	void __refreshContentPosition(ScrollView* view)
	{
		HWND handle = getHandle();
		if (handle) {
			Ref<View> viewContent = view->getContentView();
			if (viewContent.isNotNull()) {
				SCROLLINFO si;
				Base::zeroMemory(&si, sizeof(si));
				si.fMask = SIF_POS;
				::GetScrollInfo(handle, SB_HORZ, &si);
				int x = si.nPos;
				::GetScrollInfo(handle, SB_VERT, &si);
				int y = si.nPos;
				viewContent->setPosition(-(sl_real)x, -(sl_real)y);
			}
		}
	}
};

Ref<ViewInstance> ScrollView::createNativeWidget(ViewInstance* parent)
{
	Win32_UI_Shared* shared = Win32_UI_Shared::get();
	DWORD style = WS_HSCROLL | WS_VSCROLL;
	DWORD styleEx = WS_EX_CONTROLPARENT;
#if defined(_SLIB_UI_WIN32_USE_COMPOSITE_VIEWS)
	styleEx |= WS_EX_COMPOSITED;
#endif
#if defined(_SLIB_UI_WIN32_USE_CLIP_CHILDREN)
	style |= (WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
#endif
	if (isBorder()) {
		style |= WS_BORDER;
	}
	Ref<_Win32_ScrollViewInstance> ret = Win32_ViewInstance::create<_Win32_ScrollViewInstance>(this, parent, (LPCWSTR)((LONG_PTR)(shared->wndClassForView)), L"", style, styleEx);
	if (ret.isNotNull()) {
		ret->__setContentView(m_viewContent, this);
		ret->m_backgroundColor = getBackgroundColor();
	}
	return ret;
}

void ScrollView::_refreshContentSize_NW()
{
	Ref<ViewInstance> instance = getViewInstance();
	if (_Win32_ScrollViewInstance::checkInstance(instance.ptr)) {
		((_Win32_ScrollViewInstance*)(instance.ptr))->__refreshContentSize(this);
	}
}

void ScrollView::_setContentView_NW(const Ref<View>& view)
{
	Ref<ViewInstance> instance = getViewInstance();
	if (_Win32_ScrollViewInstance::checkInstance(instance.ptr)) {
		((_Win32_ScrollViewInstance*)(instance.ptr))->__setContentView(view, this);
	}
}

void ScrollView::_scrollTo_NW(sl_real x, sl_real y)
{
	HWND handle = UIPlatform::getViewHandle(this);
	if (handle) {
		SCROLLINFO si;
		Base::zeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		si.nPos = (int)x;
		::SetScrollInfo(handle, SB_HORZ, &si, TRUE);
		si.nPos = (int)y;
		::SetScrollInfo(handle, SB_VERT, &si, TRUE);
		Ref<ViewInstance> instance = getViewInstance();
		if (_Win32_ScrollViewInstance::checkInstance(instance.ptr)) {
			((_Win32_ScrollViewInstance*)(instance.ptr))->__refreshContentPosition(this);
		}
	}
}

Point ScrollView::_getScrollPosition_NW()
{
	HWND handle = UIPlatform::getViewHandle(this);
	if (handle) {
		SCROLLINFO si;
		Base::zeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		::GetScrollInfo(handle, SB_HORZ, &si);
		int x = si.nPos;
		::GetScrollInfo(handle, SB_VERT, &si);
		int y = si.nPos;
		return Point((sl_real)x, (sl_real)y);
	}
	return Point::zero();
}

Size ScrollView::_getScrollRange_NW()
{
	HWND handle = UIPlatform::getViewHandle(this);
	if (handle) {
		SCROLLINFO si;
		Base::zeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE;
		::GetScrollInfo(handle, SB_HORZ, &si);
		int w = si.nMax;
		::GetScrollInfo(handle, SB_VERT, &si);
		int h = si.nMax;
		if (w < 0) {
			w = 0;
		}
		if (h < 0) {
			h = 0;
		}
		return Size((sl_real)w, (sl_real)h);
	}
	return Size::zero();
}

void ScrollView::_setBorder_NW(sl_bool flag)
{
	HWND handle = UIPlatform::getViewHandle(this);
	if (handle) {
		LONG old = ::GetWindowLongW(handle, GWL_STYLE);
		if (flag) {
			::SetWindowLongW(handle, GWL_STYLE, old | WS_BORDER);
		} else {
			::SetWindowLongW(handle, GWL_STYLE, old & (~WS_BORDER));
		}
		::SetWindowPos(handle, NULL, 0, 0, 0, 0
			, SWP_FRAMECHANGED | SWP_NOREPOSITION | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);
	}
}

void ScrollView::_setBackgroundColor_NW(const Color& color)
{
	Ref<ViewInstance> instance = getViewInstance();
	if (_Win32_ScrollViewInstance::checkInstance(instance.ptr)) {
		((_Win32_ScrollViewInstance*)(instance.ptr))->m_backgroundColor = color;
	}
}

SLIB_UI_NAMESPACE_END

#endif
