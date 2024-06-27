#pragma once

#include <rack.hpp>

namespace musx {

using namespace rack;

struct BipolarColorParamQuantity : ParamQuantity
{
	NVGcolor color = nvgRGB(0, 255, 0);
	NVGcolor indicatorColor = nvgRGB(0, 255, 0);
	bool bipolar = false;
	bool disabled = false;
	bool indicator = false;

};

class RoundBlackKnobWithArc : public RoundBlackKnob {
private:
	static constexpr float arcThickness = 2.f;
	static constexpr float TOP_ANGLE = 3.0f / 2.0f * M_PI;

public:
	void draw(const DrawArgs& args) override
	{
		RoundBlackKnob::draw(args);

		BipolarColorParamQuantity* paramQuantity = dynamic_cast<BipolarColorParamQuantity*>(getParamQuantity());

		if (!paramQuantity)
		{
			return;
		}

		drawBg(args, nvgRGB(32, 32, 32));
	}

	void drawLayer(const DrawArgs& args, int layer) override
	{
		RoundBlackKnob::drawLayer(args, layer);

		BipolarColorParamQuantity* paramQuantity = dynamic_cast<BipolarColorParamQuantity*>(getParamQuantity());

		if (!paramQuantity)
		{
			return;
		}

		if (layer == 1) // lights layer
		{
			if (paramQuantity->disabled)
			{
				drawArc(args, TOP_ANGLE + minAngle, TOP_ANGLE + maxAngle, nvgRGB(128, 128, 128));
			}
			else
			{
				float a0 = TOP_ANGLE;
				if (!paramQuantity->bipolar)
				{
					a0 += minAngle;
				}
				float a1 = TOP_ANGLE + math::rescale(paramQuantity->getValue(), paramQuantity->getMinValue(), paramQuantity->getMaxValue(), minAngle, maxAngle);

				drawArc(args, a0, a1, paramQuantity->color);

				if (paramQuantity->indicator)
				{
					drawIndicator(args, paramQuantity->indicatorColor);
				}
			}
		}
	}


private:
	void drawBg(const DrawArgs &args, NVGcolor color) {

		Vec cVec = box.size.div(2.0f);
		float r = (box.size.x * 1.3f) / 2.0f; // arc radius

		// bg
		nvgBeginPath(args.vg);
		nvgLineCap(args.vg, NVG_ROUND);
		nvgArc(args.vg, cVec.x, cVec.y, r, TOP_ANGLE + minAngle, TOP_ANGLE + maxAngle, NVG_CW);
		nvgStrokeWidth(args.vg, arcThickness * 3.f);
		nvgStrokeColor(args.vg, color);
		nvgStroke(args.vg);

		// indicator bg
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, cVec.x, cVec.y + 0.5f * 1.3f * box.size.y, arcThickness * 1.5f);
		nvgFillColor(args.vg, color);
		nvgFill(args.vg);
	}

	void drawIndicator(const DrawArgs &args, NVGcolor color) {

		Vec cVec = box.size.div(2.0f);

		// indicator
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, cVec.x, cVec.y + 0.5f * 1.3f * box.size.y, arcThickness);
		nvgFillColor(args.vg, color);
		nvgFill(args.vg);
	}

	void drawArc(const DrawArgs &args, float a0, float a1, NVGcolor arcColor) {

		int dir = a1 > a0 ? NVG_CW : NVG_CCW;
		Vec cVec = box.size.div(2.0f);
		float r = (box.size.x * 1.3f) / 2.0f; // arc radius

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
