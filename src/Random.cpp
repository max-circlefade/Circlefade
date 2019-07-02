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


	Random() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);	
		configParam(Random::TEMP_PARAM, 0.0, 3.0, 0.0, "TEMPERATURE");
		configParam(Random::DENSITY_PARAM, 0.0, 3.0, 0.0, "DENSITY");
		configParam(Random::DUR_PARAM, 0.0, 5.0, 0.30, "RATCHETING");}

	void process(const ProcessArgs &args) override;
};


void Random::process(const ProcessArgs &args) {

	// Getting the knob and cv ins value
	float densityInput = params[DENSITY_PARAM].getValue() + inputs[DENSITY_INPUT].getVoltage();
	float tempInput = params[TEMP_PARAM].getValue() + inputs[TEMP_INPUT].getVoltage();
	float durInput = params[DUR_PARAM].getValue() + inputs[DUR_INPUT].getVoltage()/4;
	
	// Updating the value only if it changed from last step
	if(lastDur != durInput)
	{
		duration = clamp(durInput, 0.0f, 5.0f) / 5.0f *10000+1000;
		lastDur = durInput;
	}	


	if(lastDensity != densityInput)
	{
		densityScaled = clamp(densityInput, 0.0f, 3.0f) / 3.0f;
		threshold = densityScaled/5000.0f;
		lastDensity = densityInput;
	}
	if(lastTemp != tempInput)
	{
		tempScaled = clamp(tempInput, 0.0f, 3.0f) / 3.0f;
		lastTemp = tempInput;
	}

	// checking if the random is triggered and if so, that it has not reached max length
	if (lastnoise < threshold && length < duration)
	{
		length=length+1;	

		// Checking if the gate will be on or off (due to the ratcheting effect)
		if (length/2000%2 ==0)
			{
				outputs[PITCH_OUTPUT].setVoltage(lastPitch);
				outputs[TRIG_OUTPUT].setVoltage(5.0f);
			}
		else
			{
				outputs[PITCH_OUTPUT].setVoltage(lastPitch);
				outputs[TRIG_OUTPUT].setVoltage(0.0f);
			}	

		// the last 1000 steps are always off		
		if (length+1000>duration)
			{
				outputs[PITCH_OUTPUT].setVoltage(lastPitch);
				outputs[TRIG_OUTPUT].setVoltage(0.0f);
			}

											
	}

	// if not: recalculate a random number to try and fit under the threshold
	else
	{
		outputs[TRIG_OUTPUT].setVoltage(0.0);
		length=0.0;
		float noiseValue = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float pitchValue = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);	
		pitchValue=round(pitchValue*12.0f*tempScaled)/2;			
		lastnoise=noiseValue;
		lastPitch=pitchValue;
	}


}


struct RandomWidget : ModuleWidget {
	RandomWidget(Random *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Random2.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<Rogan3PWhite>(Vec(67, 182), module, Random::TEMP_PARAM));
		addParam(createParam<Rogan3PWhite>(Vec(67, 116), module, Random::DENSITY_PARAM));
		addParam(createParam<Rogan3PWhite>(Vec(67, 248), module, Random::DUR_PARAM));

		addInput(createInput<PJ301MPort>(Vec(14, 191), module, Random::TEMP_INPUT));
		addInput(createInput<PJ301MPort>(Vec(14, 124), module, Random::DENSITY_INPUT));
		addInput(createInput<PJ301MPort>(Vec(14, 258), module, Random::DUR_INPUT));

		addOutput(createOutput<PJ301MPort>(Vec(28, 320), module, Random::PITCH_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(71, 320), module, Random::TRIG_OUTPUT));

	
	}
};


Model *modelRandom = createModel<Random, RandomWidget>("Random");
