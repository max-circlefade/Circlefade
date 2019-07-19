#include "Circlefade.hpp"
#include <stdio.h>
#include<iostream>
using namespace std;
struct Clock : Module {
	enum ParamIds {
		BUT_PARAM,
		BPM_PARAM,
		LOOP_PARAM,
		V1_PARAM,
		V2_PARAM,
		PHASE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PLAY,
		BPM,
		LOOP,
		V1,
		V2,
		PHASE,
		NUM_INPUTS
	};
	enum OutputIds {
		V1_OUTPUT,
		V2_OUTPUT,
		PLAYING_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT1,
		BLINK_LIGHT2,
		NUM_LIGHTS
	};


	Clock() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);	
		configParam(BUT_PARAM, 0.0, 1.0, 0.0);
		configParam(Clock::BPM_PARAM, 30.0, 200.0, 120.0, "BPM");
		configParam(Clock::LOOP_PARAM, 1.0, 16.0, 4.0, "BEATS");
		configParam(Clock::V1_PARAM, 1.0, 64.0, 1.0, "STEPS");
		configParam(Clock::V2_PARAM, 1.0, 64.0, 1.0, "STEPS");		
		configParam(Clock::PHASE_PARAM, 0.0, 1.0, 0.0, "PHASE");}

	dsp::SchmittTrigger playing;

	float playing_bool = false;
	int counter1 =0;
	int counter2 =0;
	int counterloop=0;

	dsp::PulseGenerator Pulse1;
	bool run_pulse1 = false;
	dsp::PulseGenerator Pulse2;
	bool run_pulse2 = false;


	void process(const ProcessArgs &args) override;

};


void Clock::process(const ProcessArgs &args) {

	// Getting the knob values
	float BPMIn = clamp(params[BPM_PARAM].getValue()+170*clamp(inputs[BPM].getVoltage()/10,0.0,1.0),30.0,200.0);
	float LoopIn = clamp(params[LOOP_PARAM].getValue()+15*clamp(inputs[LOOP].getVoltage()/10,0.0,1.0),1.0,16.0);
	float V1In = clamp(params[V1_PARAM].getValue()+63*clamp(inputs[V1].getVoltage()/10,0.0,1.0),1.0,64.0);
	float V2In = clamp(params[V2_PARAM].getValue()+63*clamp(inputs[V2].getVoltage()/10,0.0,1.0),1.0,64.0);
	float PhaseIn = -60.0/BPMIn*args.sampleRate*clamp(params[PHASE_PARAM].getValue()+clamp(inputs[PHASE].getVoltage()/10,0.0,1.0),0.0,1.0);
	
	// Incrementing counters
	counter1 +=1;
	counter2 +=1;
	counterloop +=1;


	// LEDs and reset processing
	// Play button
	if (playing.process(params[BUT_PARAM].getValue())) {
		playing_bool = !playing_bool;
		counter1=0;
		counter2=PhaseIn;
		counterloop=0;
	}

	// Play / Reset CV
	if (inputs[PLAY].isConnected()){

		if (inputs[PLAY].getVoltage()>=3.0 && !playing_bool) {
			playing_bool = true;
			counter1=0;
			counter2=PhaseIn;
			counterloop=0;
		}
		if (inputs[PLAY].getVoltage()<3.0) {
			playing_bool = false;
		}		
	}

	// Playing output and LEDs processing
	outputs[PLAYING_OUTPUT].setVoltage(5.0*playing_bool);

	lights[BLINK_LIGHT1].value = playing_bool ? 1.0f : 0.0f;
	lights[BLINK_LIGHT2].value = playing_bool ? 0.0f : 1.0f;


	// Clock signals processing
	float interval1=60.0/BPMIn*args.sampleRate/V1In*4;
	float interval2=60.0/BPMIn*args.sampleRate/V2In*4;
	float loop=60.0/BPMIn*args.sampleRate*LoopIn;

	// Output 1
	if (counter1==0) {Pulse1.trigger(0.01f); }

	if (counter1 >=floor(interval1)){
		Pulse1.trigger(0.01f); 
		counter1=0;}

	// Output 2
	if (counter2==0) {Pulse2.trigger(0.01f); }

	if (counter2 >=floor(interval2)){
		Pulse2.trigger(0.01f); 
		counter2=0;}

	// Looping
	if (counterloop>=floor(loop)){
		counter1=-1;
		counter2=PhaseIn-1;
		counterloop=0;}

	// Sending the clock ticks
	run_pulse1 = Pulse1.process(0.4f / args.sampleRate);
	outputs[V1_OUTPUT].setVoltage((run_pulse1 && playing_bool ? 10.0f : 0.0f));

	run_pulse2 = Pulse2.process(0.4f / args.sampleRate);
	outputs[V2_OUTPUT].setVoltage((run_pulse2 && playing_bool ? 10.0f : 0.0f));

}


struct ClockWidget : ModuleWidget {
	ClockWidget(Clock *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Clock.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<TL1105>(mm2px(Vec(19, 44)), module, Clock::BUT_PARAM));

		addChild(createLight<MediumLight<RedLight>>(mm2px(Vec(25, 43)), module, Clock::BLINK_LIGHT1));
		addChild(createLight<MediumLight<RedLight>>(mm2px(Vec(25, 47)), module, Clock::BLINK_LIGHT2));

		float e=1.9;
		float f=2.4;
		float g=6;
		addInput(createInput<PJ301MPort>(Vec(14+f, 191+e), module, Clock::BPM));
		addInput(createInput<PJ301MPort>(Vec(14+f, 124+e), module, Clock::PLAY));
		addInput(createInput<PJ301MPort>(Vec(14+f, 258+e), module, Clock::LOOP));
		addInput(createInput<PJ301MPort>(Vec(127+g, 191+e), module, Clock::V2));
		addInput(createInput<PJ301MPort>(Vec(127+g, 124+e), module, Clock::V1));
		addInput(createInput<PJ301MPort>(Vec(127+g, 258+e), module, Clock::PHASE));


		addParam(createParam<Rogan3PWhite>(Vec(67, 182), module, Clock::BPM_PARAM));
		addParam(createParam<Rogan3PWhite>(Vec(67, 248), module, Clock::LOOP_PARAM));
		addParam(createParam<Rogan3PWhite>(Vec(180, 116), module, Clock::V1_PARAM));
		addParam(createParam<Rogan3PWhite>(Vec(180, 182), module, Clock::V2_PARAM));
		addParam(createParam<Rogan3PWhite>(Vec(180, 248), module, Clock::PHASE_PARAM));


		addOutput(createOutput<PJ301MPort>(Vec(140, 320), module, Clock::PLAYING_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(100, 320), module, Clock::V2_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(70, 320), module, Clock::V1_OUTPUT));
	
	}
};


Model *modelClock = createModel<Clock, ClockWidget>("Clock");
