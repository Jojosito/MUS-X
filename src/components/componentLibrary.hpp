#pragma once

#include <rack.hpp>

namespace musx {

using namespace rack;

class RoundBlackKnobWithArc : public RoundBlackKnob {
private:
	static constexpr float arcThickness = 1.75f;
	static constexpr float TOP_ANGLE = 3.0f / 2.0f * M_PI;

	NVGcolor arcColor = nvgRGB(0, 255, 0);
	bool bipolar = false;

public:
	void setBipolar(bool b)
	{
		bipolar = b;
	}

	void setArcColor(NVGcolor color)
	{
		arcColor = color;
	}

	void draw(const DrawArgs& args) override
	{
		RoundBlackKnob::draw(args);

		auto paramQuantity = getParamQuantity();

		float a0 = TOP_ANGLE;
		if (!bipolar)
		{
			a0 += minAngle;
		}
		float a1 = TOP_ANGLE + math::rescale(paramQuantity->getValue(), paramQuantity->getMinValue(), paramQuantity->getMaxValue(), minAngle, maxAngle);

		drawArc(args, a0, a1);
	}


private:
	void drawArc(const DrawArgs &args, float a0, float a1) {

		int dir = a1 > a0 ? NVG_CW : NVG_CCW;
		Vec cVec = box.size.div(2.0f);
		float r = (box.size.x * 1.2f) / 2.0f; // arc radius

		// bg
		nvgBeginPath(args.vg);
		nvgLineCap(args.vg, NVG_ROUND);
		nvgArc(args.vg, cVec.x, cVec.y, r, TOP_ANGLE + minAngle, TOP_ANGLE + maxAngle, NVG_CW);
		nvgStrokeWidth(args.vg, arcThickness * 3.f);
		nvgStrokeColor(args.vg, nvgRGB(32, 32, 32));
		nvgStroke(args.vg);

		// color
		nvgBeginPath(args.vg);
		nvgLineCap(args.vg, NVG_ROUND);
		nvgArc(args.vg, cVec.x, cVec.y, r, a0, a1, dir);
		nvgStrokeWidth(args.vg, arcThickness);
		nvgStrokeColor(args.vg, arcColor);
		nvgStroke(args.vg);
	}

};

}
