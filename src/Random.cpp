#include "Circlefade.hpp"
#include <stdio.h>
#include<iostream>
using namespace std;
struct Random : Module {
	enum ParamIds {
		TEMP_PARAM,
		DENSITY_PARAM,
		DUR_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		DENSITY_INPUT,
		TEMP_INPUT,
		DUR_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		TRIG_OUTPUT,
		PITCH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	int counter = 0.0;
	float lastDensity = 0.0;
	float densityScaled = 0.0;
	float threshold = 0.0;
	float lastTemp = 0.0;
	float tempScaled = 0.0;	
	float duration=7000.0;
	int length=0;
	float noiseValue=2;
	float lastnoise=0;
	float lastPitch=0;
	float lastDur=0;	
	float iter = 0;


	Random() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu
};


void Random::step() {

	float densityInput = params[DENSITY_PARAM].value + inputs[DENSITY_INPUT].value;
	float tempInput = params[TEMP_PARAM].value + inputs[TEMP_INPUT].value;
	float durInput = params[DUR_PARAM].value + inputs[DUR_INPUT].value/4;
	
	if(lastDur != durInput)
	{
		duration = clampf(durInput, 0.0f, 5.0f) / 5.0f *10000+1000;
		lastDur = durInput;
	}	

	if(lastDensity != densityInput)
	{
		densityScaled = clampf(densityInput, 0.0f, 3.0f) / 3.0f;
		threshold = densityScaled/5000.0f;
		lastDensity = densityInput;
	}
	if(lastTemp != tempInput)
	{
		tempScaled = clampf(tempInput, 0.0f, 3.0f) / 3.0f;
		lastTemp = tempInput;
	}


	if (lastnoise < threshold && length < duration)
	{
		length=length+1;	

		if (length/1000%2 ==0)
			{
				outputs[PITCH_OUTPUT].value = lastPitch;
				outputs[TRIG_OUTPUT].value = 5.0f;
			}
		else
			{
				outputs[PITCH_OUTPUT].value = lastPitch;
				outputs[TRIG_OUTPUT].value = 0.0f;
			}			
		if (length+1000>duration)
			{
				outputs[PITCH_OUTPUT].value = lastPitch;
				outputs[TRIG_OUTPUT].value = 0.0f;
			}

											
	}
	else
	{
		outputs[TRIG_OUTPUT].value = 0.0;
		length=0.0;
		float noiseValue = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float pitchValue = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);	
		pitchValue=round(pitchValue*12.0f*tempScaled)/2;			
		lastnoise=noiseValue;
		lastPitch=pitchValue;
	}


}


struct RandomWidget : ModuleWidget {
	RandomWidget(Random *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Random.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<Rogan3PWhite>(Vec(67, 182), module, Random::TEMP_PARAM, 0.0, 3.0, 0.0));
		addParam(ParamWidget::create<Rogan3PWhite>(Vec(67, 116), module, Random::DENSITY_PARAM, 0.0, 3.0, 0.0));
		addParam(ParamWidget::create<Rogan3PWhite>(Vec(67, 248), module, Random::DUR_PARAM, 0.0, 5.0, 0.0));

		addInput(Port::create<PJ301MPort>(Vec(14, 191), Port::INPUT, module, Random::DENSITY_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(14, 124), Port::INPUT, module, Random::TEMP_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(14, 258), Port::INPUT, module, Random::DUR_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(28, 320), Port::OUTPUT, module, Random::PITCH_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(71, 320), Port::OUTPUT, module, Random::TRIG_OUTPUT));

	
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelRandom = Model::create<Random, RandomWidget>("Circlefade", "Random", "Random", OSCILLATOR_TAG);
