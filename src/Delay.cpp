#include "Circlefade.hpp"
#include <stdio.h>
#include<iostream>
using namespace std;
struct Delay : Module {
	enum ParamIds {
		DW_PARAM,
		LENGTH_PARAM,
		FEEDBACK_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FEEDBACK_INPUT,
		IN_INPUT,
		DW_INPUT,
		LENGTH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		TRIG_OUTPUT,
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	int counter = 0.0;

	// Declaring the buffer
	int buffer_size=44000*20;
	//float* buffer = new float[buffer_size];
	// Can't use new because can't remove it afterwards
	float buffer[44000*20];
	float delayed=0;
	float delayed1=0;
	float delayed2=0;
	float delayed3=0;
	float delayed4=0;
	float delayed5=0;
	float delayed6=0;
	float delayed7=0;
	float out=0;
	float init = true;
	float trig = 0;

	Delay() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);	
		configParam(Delay::LENGTH_PARAM, 0.0, 1.0, 0.0, "LENGTH");
		configParam(Delay::FEEDBACK_PARAM, 0.0, 1.0, 0.3, "FEEDBACK");
		configParam(Delay::DW_PARAM, 0.0, 1.0, 0.5, "DRY/WET");}

	void process(const ProcessArgs &args) override;
};


void Delay::process(const ProcessArgs &args) {

	// Initialisation of the buffer
	if (init) {
		for ( int i=0 ; i < buffer_size ; ++i) buffer[i]=0;
		init=!init;
	}

	// Calcuating the inputs
	float Input = inputs[IN_INPUT].getVoltage();
	int delay_steps=clamp((params[LENGTH_PARAM].getValue() + clamp(inputs[LENGTH_INPUT].getVoltage(),0.0f,5.0f)/5),0.0f,1.0f)*44000;
	float dry = clamp(1-(params[DW_PARAM].getValue() + clamp(inputs[DW_INPUT].getVoltage(),0.0f,5.0f)/5.0),0.0f,1.0f);
	float feedback=clamp(params[FEEDBACK_PARAM].getValue() + clamp(inputs[FEEDBACK_INPUT].getVoltage(),0.0f,5.0f)/5.0,0.0f,1.0f);


	// Managing the counter
	counter+=1;
	if (counter >= buffer_size) counter =0;

	// Populating the buffer with the Input
	buffer[counter]=Input;

	// Calculating the delayed elements
	if (counter-delay_steps>=0) delayed=buffer[counter-delay_steps];
	else delayed=buffer[counter-delay_steps+buffer_size];

	if (counter-2*delay_steps>=0) delayed1=buffer[counter-2*delay_steps];
	else delayed1=buffer[counter-2*delay_steps+buffer_size];

	if (counter-3*delay_steps>=0) delayed2=buffer[counter-3*delay_steps];
	else delayed2=buffer[counter-3*delay_steps+buffer_size];

	if (counter-4*delay_steps>=0) delayed3=buffer[counter-4*delay_steps];
	else delayed3=buffer[counter-4*delay_steps+buffer_size];

	if (counter-5*delay_steps>=0) delayed4=buffer[counter-5*delay_steps];
	else delayed4=buffer[counter-5*delay_steps+buffer_size];

	if (counter-6*delay_steps>=0) delayed5=buffer[counter-6*delay_steps];
	else delayed5=buffer[counter-6*delay_steps+buffer_size];

	if (counter-7*delay_steps>=0) delayed6=buffer[counter-7*delay_steps];
	else delayed6=buffer[counter-7*delay_steps+buffer_size];

	if (counter-8*delay_steps>=0) delayed7=buffer[counter-8*delay_steps];
	else delayed7=buffer[counter-8*delay_steps+buffer_size];


	// Outputing the sum of all elements
	if (delay_steps==0) out = dry*Input;
	else out=dry*Input+(1-dry)*(delayed+(0.7*delayed1+0.4*delayed2+0.2*delayed3+0.1*delayed4+0.05*delayed5+0.025*delayed6+0.0125*delayed7))*feedback;
	outputs[OUT_OUTPUT].setVoltage(out);

	if (out >0.25) trig=5.0;
	else trig=0.0;
	outputs[TRIG_OUTPUT].setVoltage(trig);
}


struct DelayWidget : ModuleWidget {
	DelayWidget(Delay *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Delay.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<Rogan3PWhite>(Vec(67, 182), module, Delay::FEEDBACK_PARAM));
		addParam(createParam<Rogan3PWhite>(Vec(67, 116), module, Delay::LENGTH_PARAM));
		addParam(createParam<Rogan3PWhite>(Vec(67, 248), module, Delay::DW_PARAM));

		addInput(createInput<PJ301MPort>(Vec(14, 191), module, Delay::FEEDBACK_INPUT));
		addInput(createInput<PJ301MPort>(Vec(14, 124), module, Delay::LENGTH_INPUT));
		addInput(createInput<PJ301MPort>(Vec(14, 258), module, Delay::DW_INPUT));
		addInput(createInput<PJ301MPort>(Vec(10, 320), module, Delay::IN_INPUT));

		addOutput(createOutput<PJ301MPort>(Vec(46, 320), module, Delay::TRIG_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(82, 320), module, Delay::OUT_OUTPUT));

	
	}
};


Model *modelDelay = createModel<Delay, DelayWidget>("Delay");
