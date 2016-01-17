#include "../../../inc/slib/graphics/canvas.h"
#include "../../../inc/slib/graphics/context.h"
#include "../../../inc/slib/graphics/util.h"
#include "../../../inc/slib/math/transform2d.h"

SLIB_GRAPHICS_NAMESPACE_BEGIN

Ref<GraphicsContext> Canvas::getGraphicsContext()
{
	return m_context;
}

void Canvas::clipToRoundRect(const Rectangle& rect, const Size& radius)
{
	Ref<GraphicsPath> path = GraphicsPath::create();
	if (path.isNotNull()) {
		path->addRoundRect(rect, radius);
		clipToPath(path);
	}
}

void Canvas::clipToEllipse(const Rectangle& rect)
{
	Ref<GraphicsPath> path = GraphicsPath::create();
	if (path.isNotNull()) {
		path->addEllipse(rect);
		clipToPath(path);
	}
}

void Canvas::translate(sl_real dx, sl_real dy)
{
	Matrix3 mat;
	Transform2::setTranslation(mat,dx, dy);
	concatMatrix(mat);
}

void Canvas::rotate(sl_real radians)
{
	Matrix3 mat;
	Transform2::setRotation(mat, radians);
	concatMatrix(mat);
}

void Canvas::rotate(sl_real cx, sl_real cy, sl_real radians)
{
	Matrix3 mat;
	Transform2::setRotation(mat, cx, cy, radians);
	concatMatrix(mat);
}

void Canvas::scale(sl_real sx, sl_real sy)
{
	Matrix3 mat;
	Transform2::setScaling(mat, sx, sy);
	concatMatrix(mat);
}

void Canvas::draw(const Rectangle& rectDst, const Ref<Drawable>& src, const Rectangle& rectSrc)
{
	if (src.isNotNull()) {
		src->onDraw(this, rectDst, rectSrc);
	}
}

void Canvas::draw(const Rectangle& rectDst, const Ref<Drawable>& src)
{
	if (src.isNotNull()) {
		Rectangle rectSrc(0, 0, src->getDrawableWidth(), src->getDrawableHeight());
		src->onDraw(this, rectDst, rectSrc);
	}
}

void Canvas::draw(sl_real xDst, sl_real yDst, sl_real widthDst, sl_real heightDst
					  , const Ref<Drawable>& src, sl_real xSrc, sl_real ySrc, sl_real widthSrc, sl_real heightSrc)
{
	if (src.isNotNull()) {
		Rectangle rectDst(xDst, yDst, xDst + widthDst, yDst + heightDst);
		Rectangle rectSrc(xSrc, ySrc, xSrc + widthSrc, ySrc + heightSrc);
		src->onDraw(this, rectDst, rectSrc);
	}
}

void Canvas::draw(sl_real xDst, sl_real yDst, sl_real widthDst, sl_real heightDst, const Ref<Drawable>& src)
{
	if (src.isNotNull()) {
		Rectangle rectDst(xDst, yDst, xDst + widthDst, yDst + heightDst);
		Rectangle rectSrc(0, 0, src->getDrawableWidth(), src->getDrawableHeight());
		src->onDraw(this, rectDst, rectSrc);
	}
}

void Canvas::draw(sl_real xDst, sl_real yDst, const Ref<Drawable>& src)
{
	if (src.isNotNull()) {
		sl_real w = src->getDrawableWidth();
		sl_real h = src->getDrawableHeight();
		Rectangle rectDst(xDst, yDst, xDst + w, yDst + h);
		Rectangle rectSrc(0, 0, w, h);
		src->onDraw(this, rectDst, rectSrc);
	}
}

Size Canvas::getTextSize(const Ref<Font>& font, const String &text)
{
	Ref<GraphicsContext> context = m_context;
	if (context.isNotNull()) {
		return context->getFontTextSize(font, text);
	} else {
		return Size::zero();
	}
}

void Canvas::drawText(const String& text, const Rectangle& rcDst, const Ref<Font>& _font, const Color& color, Alignment align)
{
	Ref<Font> font = _font;
	if (font.isNull()) {
		font = Font::getDefault();
		if (font.isNull()) {
			return;
		}
	}
	Size size = getTextSize(font, text);
	Point pt = GraphicsUtil::calculateAlignPosition(rcDst, size.x, size.y, align);
	drawText(text, pt.x, pt.y, font, color);
}

void Canvas::draw(const Rectangle& rectDst, const Ref<Drawable>& source, ScaleMode scaleMode, Alignment alignment)
{
	Canvas* canvas = this;
	if (source.isNotNull()) {
		sl_real dw = rectDst.getWidth();
		sl_real dh = rectDst.getHeight();
		sl_real sw = source->getDrawableWidth();
		sl_real sh = source->getDrawableHeight();
		if (dw > 0 && dh > 0 && sw > 0 && sh > 0) {
			Rectangle rectSrc;
			Rectangle rectDraw;
			if (scaleMode == scaleMode_Stretch) {
				rectSrc.left = 0;
				rectSrc.top = 0;
				rectSrc.right = sw;
				rectSrc.bottom = sh;
				rectDraw = rectDst;
			} else if (scaleMode == scaleMode_Contain) {
				sl_real fw = dw / sw;
				sl_real fh = dh / sh;
				sl_real tw, th;
				if (fw > fh) {
					th = dh;
					tw = sw * fh;
				} else {
					tw = dw;
					th = sh * fw;
				}
				rectSrc.left = 0;
				rectSrc.top = 0;
				rectSrc.right = sw;
				rectSrc.bottom = sh;
				Point pt = GraphicsUtil::calculateAlignPosition(rectDst, tw, th, alignment);
				rectDraw.left = pt.x;
				rectDraw.top = pt.y;
				rectDraw.right = rectDraw.left + tw;
				rectDraw.bottom = rectDraw.top + th;
			} else if (scaleMode == scaleMode_Cover) {
				sl_real fw = sw / dw;
				sl_real fh = sh / dh;
				sl_real tw, th;
				if (fw > fh) {
					th = sh;
					tw = dw * fh;
				} else {
					tw = sw;
					th = dh * fw;
				}
				Point pt = GraphicsUtil::calculateAlignPosition(rectSrc, tw, th, alignment);
				rectSrc.left = pt.x;
				rectSrc.top = pt.y;
				rectSrc.right = rectSrc.left + tw;
				rectSrc.bottom = rectSrc.top + th;
				rectDraw = rectDst;
				rectDraw.bottom = dh;
			} else {
				rectSrc.left = 0;
				rectSrc.top = 0;
				rectSrc.right = sw;
				rectSrc.bottom = sh;
				Point pt = GraphicsUtil::calculateAlignPosition(rectSrc, sw, sh, alignment);
				rectDraw.left = pt.x;
				rectDraw.top = pt.y;
				rectDraw.right = rectDraw.left + sw;
				rectDraw.bottom = rectDraw.top + sh;
			}
			canvas->draw(rectDraw, source, rectSrc);
		}
	}
}

CanvasStatusScope::CanvasStatusScope(const Ref<Canvas>& canvas)
{
	m_canvas = canvas;
	if (m_canvas.isNotNull()) {
		m_canvas->save();
	}
}

CanvasStatusScope::~CanvasStatusScope()
{
	Ref<Canvas> canvas = m_canvas;
	if (canvas.isNotNull()) {
		canvas->restore();
	}
}

SLIB_GRAPHICS_NAMESPACE_END
