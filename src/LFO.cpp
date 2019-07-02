#include "Circlefade.hpp"
#include <stdio.h>
#include<iostream>
using namespace std;
struct LFO : Module {
	enum ParamIds {
		BUT_PARAM,
		POL_PARAM,
		FREQ_PARAM,
		PHASE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		RESET_INPUT,
		FREQ_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LFO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT1,
		BLINK_LIGHT2,
		BLINK_LIGHT3,
		BLINK_LIGHT4,
		NUM_LIGHTS
	};

	float phase=0;
	int reseted=0;
	float tri=0;
	int inititalphaseadded=0;
	float lastphase=0;

	LFO() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);	
		configParam(BUT_PARAM, 0.0, 1.0, 0.0);
		configParam(POL_PARAM, 0.0, 1.0, 0.0);
		configParam(LFO::FREQ_PARAM, 0.0, 1.0, 0.1, "FREQUENCY");
		configParam(LFO::PHASE_PARAM, 0, 1.0, 0.0, "PHASE");}


	dsp::SchmittTrigger signal_shape_pro;
	float sine_shape = true;
	dsp::SchmittTrigger pol_choice_but;
	float pol_choice = true;

	void process(const ProcessArgs &args) override;

};


void LFO::process(const ProcessArgs &args) {

	float freqInput = params[FREQ_PARAM].getValue()*5 + clamp(inputs[FREQ_INPUT].getVoltage(),0.0f,5.0f)/2;
	float phaseInput = params[PHASE_PARAM].getValue();
	float resetInput = inputs[RESET_INPUT].getVoltage();

	// LEDs processing
	if (signal_shape_pro.process(params[BUT_PARAM].getValue())) {
		sine_shape = !sine_shape;
	}
	lights[BLINK_LIGHT1].value = sine_shape ? 1.0f : 0.0f;
	lights[BLINK_LIGHT2].value = sine_shape ? 0.0f : 1.0f;

	if (pol_choice_but.process(params[POL_PARAM].getValue())) {
		pol_choice = !pol_choice;
	}
	lights[BLINK_LIGHT3].value = pol_choice ? 1.0f : 0.0f;
	lights[BLINK_LIGHT4].value = pol_choice ? 0.0f : 1.0f;


	// Aggregating the phase at each step
	phase += freqInput * args.sampleTime;

	// Adding the initial phase once
	if (inititalphaseadded==0){		
		phase+=phaseInput;
		inititalphaseadded=1;}

	// Modifying the phase when needed
	if (lastphase != phaseInput){
			phase+=phaseInput-lastphase;
			lastphase=phaseInput;}

	// making sure 0 < phase < 1
	while (phase>= 1.f)
		phase -= 1.f;

	// Reseting the phase once when the Input is > 3V
	if (resetInput>3 && reseted==0) {
		phase=0;
		reseted=1;
	}
	// Phase can t reset again until the voltage goes back under 3V
	if (resetInput<3 && reseted==1) reseted=0;


	// Outputing either a sine or triangle signal
	if (sine_shape){	
		float sine = std::sin(2.f * M_PI * phase);
		sine = pol_choice ? (sine+1)/2.0f : sine;
		outputs[LFO_OUTPUT].setVoltage(5.f * sine);}

	// Formaula for a saw signal (unused)
	//float saw=10*phase-5;
	//outputs[LFO_OUTPUT].setVoltage(saw);
		
	else{
		if (phase<=0.5)
			tri=20*phase-5;
		else
			tri=5-20*(phase-0.5);
		tri = pol_choice ? (tri+5)/2.0f : tri;
		outputs[LFO_OUTPUT].setVoltage(tri);}

}


struct LFOWidget : ModuleWidget {
	LFOWidget(LFO *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LFO.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<Rogan3PWhite>(Vec(67, 182), module, LFO::FREQ_PARAM));
		addParam(createParam<Rogan3PWhite>(Vec(67, 248), module, LFO::PHASE_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(4, 43)), module, LFO::BUT_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(24, 43)), module, LFO::POL_PARAM));

		addChild(createLight<MediumLight<RedLight>>(mm2px(Vec(10, 42)), module, LFO::BLINK_LIGHT1));
		addChild(createLight<MediumLight<RedLight>>(mm2px(Vec(10, 46)), module, LFO::BLINK_LIGHT2));
		addChild(createLight<MediumLight<RedLight>>(mm2px(Vec(30, 42)), module, LFO::BLINK_LIGHT3));
		addChild(createLight<MediumLight<RedLight>>(mm2px(Vec(30, 46)), module, LFO::BLINK_LIGHT4));


		addInput(createInput<PJ301MPort>(Vec(14, 191), module, LFO::FREQ_INPUT));
		addInput(createInput<PJ301MPort>(Vec(14, 258), module, LFO::RESET_INPUT));

		addOutput(createOutput<PJ301MPort>(Vec(50, 320), module, LFO::LFO_OUTPUT));


	
	}
};

Model *modelLFO = createModel<LFO, LFOWidget>("LFO");
