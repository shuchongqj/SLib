/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_UI_PROGRESS_BAR
#define CHECKHEADER_SLIB_UI_PROGRESS_BAR

#include "definition.h"

#include "view.h"

namespace slib
{

	class SLIB_EXPORT ProgressBar : public View
	{
		SLIB_DECLARE_OBJECT
		
	public:
		ProgressBar(LayoutOrientation orientation = LayoutOrientation::Horizontal);

		~ProgressBar();
		
	public:
		LayoutOrientation getOrientation();
		
		void setOrientation(LayoutOrientation orientation, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		sl_bool isVertical();
		
		void setVertical(UIUpdateMode mode = UIUpdateMode::Redraw);
		
		sl_bool isHorizontal();
		
		void setHorizontal(UIUpdateMode mode = UIUpdateMode::Redraw);
		
		float getMinimumValue();
		
		void setMinimumValue(float value, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		float getMaximumValue();
		
		void setMaximumValue(float value, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		float getRange();
		
		void setRange(float range, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		float getValue();
		
		void setValue(float value, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		float getSecondaryValue();
		
		void setSecondaryValue(float value, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		sl_bool isDualValues();
		
		void setDualValues(sl_bool flagDualValues, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		sl_bool isDiscrete();
		
		void setDiscrete(sl_bool flagDiscrete, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		float getStep();
		
		void setStep(float step, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		sl_bool isReversed();
		
		void setReversed(sl_bool flagReversed, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		
		Ref<Drawable> getTrackDrawable();
		
		void setTrackDrawable(const Ref<Drawable>& drawable, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		void setTrackColor(const Color& color, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		Ref<Drawable> getProgressDrawable();
		
		void setProgressDrawable(const Ref<Drawable>& drawable, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		void setProgressColor(const Color& color, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		Ref<Drawable> getSecondaryProgressDrawable();
		
		void setSecondaryProgressDrawable(const Ref<Drawable>& drawable, UIUpdateMode mode = UIUpdateMode::Redraw);
		
		void setSecondaryProgressColor(const Color& color, UIUpdateMode mode = UIUpdateMode::Redraw);
		
	protected:
		// override
		void onDraw(Canvas* canvas);
		
	protected:
		float _refineValue(float value);
		
		float _refineStep();
		
		sl_ui_pos _getPositionFromValue(float value);
		
		void _getProgressRegions(UIRect& outProgress, UIRect& outSecondaryProgress);
		
		virtual void dispatchChange(float value);
		
		virtual void dispatchChangeSecondary(float value);
		
	protected:
		LayoutOrientation m_orientation;
		float m_value_min;
		float m_value_max;
		float m_value;
		float m_value2;
		sl_bool m_flagDualValues;
		sl_bool m_flagDiscrete;
		float m_step;
		sl_bool m_flagReversed;
		
		AtomicRef<Drawable> m_track;
		AtomicRef<Drawable> m_progress;
		AtomicRef<Drawable> m_progress2;
		
	};

}

#endif
