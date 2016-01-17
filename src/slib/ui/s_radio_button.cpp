#include "../../../inc/slib/ui/s_radio_button.h"
#include "../../../inc/slib/graphics/util.h"

SLIB_UI_NAMESPACE_BEGIN
class _SRadioButton_Icon : public Drawable
{
public:
	Ref<Pen> m_penBorder;
	Ref<Brush> m_brushBack;
	Ref<Brush> m_brushCheck;
	
	_SRadioButton_Icon(const Ref<Pen>& penBorder, const Color& backColor, const Color& checkColor)
	{
		m_penBorder = penBorder;
		if (backColor.getAlpha() > 0) {
			m_brushBack = Brush::createSolidBrush(backColor);
		}
		if (checkColor.getAlpha() > 0) {
			m_brushCheck = Brush::createSolidBrush(checkColor);
		}
	}
	
	sl_real getDrawableWidth()
	{
		return 1;
	}
	
	sl_real getDrawableHeight()
	{
		return 1;
	}
	
	void onDraw(Canvas* canvas, const Rectangle& _rectDst, const Rectangle& rectSrc)
	{
		Rectangle rectDst = GraphicsUtil::transformRectangle(_rectDst, rectSrc, Rectangle(0, 0, 1, 1));
		canvas->drawEllipse(rectDst, m_penBorder, m_brushBack);
		if (m_brushCheck.isNotNull()) {
			Rectangle rcCheck;
			rcCheck.setLeftTop(rectDst.getCenter());
			sl_real w = rectDst.getWidth() / 2;
			sl_real h = rectDst.getHeight() / 2;
			rcCheck.left -= w / 2;
			rcCheck.top -= h / 2;
			rcCheck.right = rcCheck.left + w;
			rcCheck.bottom = rcCheck.top + h;
			canvas->fillEllipse(rcCheck, m_brushCheck);
		}
	}
	
};

class _SRadioButton_Categories
{
public:
	SButtonCategory categories[2];
	
	_SRadioButton_Categories()
	{
		Color colorBackNormal = Color::white();
		Color colorBackHover = Color::white();
		Color colorBackDown(220, 230, 255);
		Color colorBackDisabled = Color::gray(220);
		Ref<Pen> penNormal = Pen::createSolidPen(1, Color::black());
		Ref<Pen> penHover = Pen::createSolidPen(1, Color(0, 80, 200));
		Ref<Pen> penDown = penHover;
		Ref<Pen> penDisabled = Pen::createSolidPen(1, Color::gray(90));
		Color colorCheckNormal = Color::black();
		Color colorCheckDisabled = Color::gray(90);
		Color colorCheckHover = Color(0, 80, 200);
		Color colorCheckDown = colorCheckHover;
		categories[0].properties[buttonState_Normal].textColor = Color::black();
		categories[0].properties[buttonState_Normal].icon = new _SRadioButton_Icon(penNormal, colorBackNormal, Color::zero());
		categories[0].properties[buttonState_Disabled].textColor = Color::gray(90);
		categories[0].properties[buttonState_Disabled].icon = new _SRadioButton_Icon(penDisabled, colorBackDisabled, Color::zero());
		categories[0].properties[buttonState_Hover].icon = new _SRadioButton_Icon(penHover, colorBackHover, Color::zero());
		categories[0].properties[buttonState_Down].icon = new _SRadioButton_Icon(penDown, colorBackDown, Color::zero());
		
		categories[1] = categories[0];
		categories[1].properties[buttonState_Normal].icon = new _SRadioButton_Icon(penNormal, colorBackNormal, colorCheckNormal);
		categories[1].properties[buttonState_Disabled].icon = new _SRadioButton_Icon(penDisabled, colorBackDisabled, colorCheckDisabled);
		categories[1].properties[buttonState_Hover].icon = new _SRadioButton_Icon(penHover, colorBackHover, colorCheckHover);
		categories[1].properties[buttonState_Down].icon = new _SRadioButton_Icon(penDown, colorBackDown, colorCheckDown);
	}
	
	static _SRadioButton_Categories* get()
	{
		SLIB_SAFE_STATIC(_SRadioButton_Categories, ret);
		return &ret;
	}
};

SRadioButton::SRadioButton() : SCheckBox(2, _SRadioButton_Categories::get()->categories)
{
}

SLIB_UI_NAMESPACE_END
