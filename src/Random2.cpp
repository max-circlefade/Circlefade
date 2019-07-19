#include "Circlefade.hpp"
#include <stdio.h>
#include<iostream>
using namespace std;
struct Random2 : Module {
	enum ParamIds {
		TEMP_PARAM,
		DENSITY_PARAM,
		DUR_PARAM,
		LOOP_PARAM,
		SCALE_PARAM,
		TEMP_POT,
		DENSITY_POT,
		DUR_POT,
		NUM_PARAMS
	};
	enum InputIds {
		DENSITY_INPUT,
		TEMP_INPUT,
		CLK_INPUT,
		DUR_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		TRIG_OUTPUT,
		PITCH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(MODEL_LIGHT, 3 * 2),
		NUM_LIGHTS
	};

	int counter = 0;
	int interval = 0;
	int lastcounter =0;
	float buffer[16*4];
	int ratcheting_steps =0;
	float MV = 1/12.0;
	int pulse_out=0;
	bool clk_received = false;
	int origin=0;
	float note =0;

	//[C,Db,D,Eb,E,F,Gb,G,Ab,A,Bb,B];

	int Allnotes[12]=  {1,1,1,1,1,1,1,1,1,1,1,1};
	int Ionian[12]=    {1,0,1,0,1,1,0,1,0,1,0,1};
	int Locrian[12]=   {1,1,0,1,0,1,1,0,1,0,1,0};
	int Dorian[12]=    {1,0,1,1,0,1,0,1,0,1,1,0};
	int Lydian[12]=    {1,0,1,0,1,0,1,1,0,1,0,1};
	int Phygrian[12]=  {1,1,0,1,0,1,0,1,1,0,1,0};
	int Mixolydian[12]={1,0,1,0,1,1,0,1,0,1,1,0};
	int Aeolian[12]=   {1,0,1,1,0,1,0,1,1,0,1,0};

	//int Scales_L[8][12]={Allnotes,Ionian,Dorian,Phygrian,Lydian,Mixolydian,Aeolian,Locrian};
	//int Scales_L[8][12]={{1,1,1,1,1,1,1,1,1,1,1,1},{1,0,1,0,1,1,0,1,0,1,0,1},{1,0,1,1,0,1,0,1,0,1,1,0},{1,1,0,1,0,1,0,1,1,0,1,0},{1,0,1,0,1,0,1,1,0,1,0,1},{1,0,1,0,1,1,0,1,0,1,1,0},{1,0,1,1,0,1,0,1,1,0,1,0},{1,1,0,1,0,1,1,0,1,0,1,0}};

	int Scales_L[8][12]={{1,1,1,1,1,1,1,1,1,1,1,1},{1,0,1,0,1,1,0,1,0,1,0,1},{1,1,0,1,0,1,1,0,1,0,1,0},{1,0,1,1,0,1,0,1,0,1,1,0},{1,0,1,0,1,0,1,1,0,1,0,1},{1,1,0,1,0,1,0,1,1,0,1,0},{1,0,1,0,1,1,0,1,0,1,1,0},{1,0,1,1,0,1,0,1,1,0,1,0}};



	Random2() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);	
		configParam(LOOP_PARAM, 0.0, 1.0, 0.0,"4 / 8 / 16 Beats Loop");
		configParam(SCALE_PARAM, 0.0, 1.0, 0.0);
		configParam(Random2::TEMP_POT, -1.0, 1.0, 0.0);
		configParam(Random2::DENSITY_POT, -1.0, 1.0, 0.0);
		configParam(Random2::DUR_POT, -1.0, 1.0, 0.0);		
		configParam(Random2::TEMP_PARAM, 0.0, 100.0, 0.0, "TEMPERATURE");
		configParam(Random2::DENSITY_PARAM, 0.0, 100.0, 20.0, "DENSITY");
		configParam(Random2::DUR_PARAM, 1.0, 5.0, 1.0, "RATCHETING");}

	dsp::SchmittTrigger loop_pro;
	int loop = 0;
	int loop_size=64;
	dsp::SchmittTrigger clk;
	dsp::PulseGenerator Pulse;
	bool run_pulse = false;
	bool init = false;
	bool ratchting = true;
	dsp::SchmittTrigger scale_pro;	
	int scale_mode=0;



	void process(const ProcessArgs &args) override;
};


void Random2::process(const ProcessArgs &args) {

	// Getting the knob and cv ins value
	float densityInput = clamp(params[DENSITY_PARAM].getValue() + 10*clamp(inputs[DENSITY_INPUT].getVoltage(),-10.0,10.0)*params[DENSITY_POT].getValue(),0.0,100.0);
	float tempInput = clamp(params[TEMP_PARAM].getValue() + 10*clamp(inputs[TEMP_INPUT].getVoltage(),-10.0,10.0)*params[TEMP_POT].getValue(),0.0,100.0);
	float durInput = clamp(params[DUR_PARAM].getValue() + clamp(inputs[DUR_INPUT].getVoltage()/2,-4.0,4.0)*params[DUR_POT].getValue(),0.0,5.0);


	//Init of lights
	if (init) {
		init = true;
		lights[MODEL_LIGHT].setBrightness(0.f);
		lights[MODEL_LIGHT + 1].setBrightness(0.f);
		lights[MODEL_LIGHT + 2].setBrightness(0.f);
		lights[MODEL_LIGHT + 3].setBrightness(0.f);
		lights[MODEL_LIGHT + 4].setBrightness(0.f);
		lights[MODEL_LIGHT + 5].setBrightness(0.f);
	}

	// Receiving clock ticks
	if (inputs[CLK_INPUT].isConnected()) {
		if (inputs[CLK_INPUT].getVoltage()>=5.0 && !clk_received) {
			clk_received = true;
			if (counter>=63) counter=0;
			else counter+=1;
			interval=0;
		}
		if (inputs[CLK_INPUT].getVoltage()<5.0 ) clk_received=false;
	}
	// Internal clocking
	else {
		interval +=1;
		if (interval==6000) {
			if (counter>=63) counter=0;
			else counter+=1;
			interval=0;
		}
	}

	// Loop processing
	if (loop_pro.process(params[LOOP_PARAM].getValue())) {
		loop +=1;
		loop=loop%4;

		if (loop==0) loop_size=64;
		if (loop==1) {loop_size=16;
			origin=counter;
		}
		if (loop==2) loop_size=32;
		if (loop==3) loop_size=64;

		lights[MODEL_LIGHT + 5].setBrightness((loop==1) ? 1.f : 0.f);
		lights[MODEL_LIGHT + 3].setBrightness((loop==2) ? 1.f : 0.f);
		lights[MODEL_LIGHT + 1].setBrightness((loop==3) ? 1.f : 0.f);
		lights[MODEL_LIGHT + 0].setBrightness(0.f);
		lights[MODEL_LIGHT + 2].setBrightness(0.f);
		lights[MODEL_LIGHT + 4].setBrightness(0.f);

	}

	// Scales processing
	if (scale_pro.process(params[SCALE_PARAM].getValue())) {
		scale_mode +=1;
		scale_mode=scale_mode%8;
		lights[MODEL_LIGHT + 0].setBrightness((scale_mode % 2==1) ? 1.0f : 0.f);
		lights[MODEL_LIGHT + 2].setBrightness((scale_mode==2 or scale_mode==3 or scale_mode==6 or scale_mode==7 ) ? 1.0f : 0.f);
		lights[MODEL_LIGHT + 4].setBrightness((scale_mode >= 4) ? 1.0f : 0.f);
		lights[MODEL_LIGHT + 1].setBrightness(0.f);
		lights[MODEL_LIGHT + 3].setBrightness(0.f);
		lights[MODEL_LIGHT + 5].setBrightness(0.f);
		
	}

	// Processing the random triggers
	if (counter !=lastcounter) {
		lastcounter=counter;

		// Adding a trig to the buffer if no ratcheting is currently being played
		if (loop==0) {
			if (ratcheting_steps==0) 
				{
				buffer[counter]=0;
				float random1=rand()%100;
				if (random1<=densityInput) {
					float random2=rand()%100+1;
					buffer[counter]=random2;
				}
			}
		}

		// Ratcheting steps, sending a pulse, pitch stays the same
		if (ratcheting_steps>0) {
			ratcheting_steps-=1;
			pulse_out=2500;
		}
		
		// Trig that is not a ratcheting step, calculating the note to send
		else if (buffer[(  (loop==0) ? counter :  (counter%loop_size + origin - loop_size + 64)%64  )]>0) {
			// pitch between 0 and 10000, accounting for 3 octaves
			float scale=36;
			float n = 10000.0/scale;
			int pitch =  floor(buffer[(  (loop==0) ? counter : (counter%loop_size + origin - loop_size + 64)%64   )]*tempInput/n);
			// First note is C3 = 2.250mV?
			if (Scales_L[scale_mode][pitch%12]==1) {
				 note = (pitch*MV +0.250) ; 				
			}
			else  note = ((pitch+1)*MV +0.250) ; 
			outputs[PITCH_OUTPUT].setVoltage(note);	
			pulse_out=2500;
			ratcheting_steps=floor(durInput-1);
			}
	}

	// Sending the step
	if (pulse_out>0) {
		outputs[TRIG_OUTPUT].setVoltage(10.0f);
		pulse_out+=-1;
		}
	else outputs[TRIG_OUTPUT].setVoltage(0.0f);

}


struct Random2Widget : ModuleWidget {
	Random2Widget(Random2 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Random2_2.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<Rogan3PWhite>(Vec(6, 152), module, Random2::TEMP_PARAM));
		addParam(createParam<Rogan3PWhite>(Vec(40, 90), module, Random2::DENSITY_PARAM));
		addParam(createParam<Rogan3PWhite>(Vec(72, 152), module, Random2::DUR_PARAM));

		addInput(createInput<PJ301MPort>(Vec(10, 275), module, Random2::TEMP_INPUT));
		addInput(createInput<PJ301MPort>(Vec(47, 275), module, Random2::DENSITY_INPUT));
		addInput(createInput<PJ301MPort>(Vec(84, 275), module, Random2::DUR_INPUT));
		addInput(createInput<PJ301MPort>(Vec(10, 320), module, Random2::CLK_INPUT));

		addParam(createParam<Trimpot>(mm2px(Vec(4.5 ,79.60705)), module, Random2::TEMP_POT));
		addParam(createParam<Trimpot>(mm2px(Vec(17, 79.60705)), module, Random2::DENSITY_POT));
		addParam(createParam<Trimpot>(mm2px(Vec(29.5, 79.60705)), module, Random2::DUR_POT));



		addOutput(createOutput<PJ301MPort>(Vec(47, 320), module, Random2::PITCH_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(84, 320), module, Random2::TRIG_OUTPUT));
			
		addChild(createLight<MediumLight<GreenRedLight>>(mm2px(Vec(14.5, 21)), module, Random2::MODEL_LIGHT));
		addChild(createLight<MediumLight<GreenRedLight>>(mm2px(Vec(19.5, 21)), module, Random2::MODEL_LIGHT+2));
		addChild(createLight<MediumLight<GreenRedLight>>(mm2px(Vec(24.5, 21)), module, Random2::MODEL_LIGHT+4));


		addParam(createParam<TL1105>(mm2px(Vec(6, 20)), module, Random2::SCALE_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(31, 20)), module, Random2::LOOP_PARAM));

	
	}
};


Model *modelRandom2 = createModel<Random2, Random2Widget>("Random2");
