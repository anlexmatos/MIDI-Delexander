/*
Super MIDI 64 : Midi to 4x16ch CV with MPE and regular Polyphonic modes
Copyright (C) 2020 Anthony Lexander Matos

Based on MIDIpolyMPE, Copyright (C) 2019 Pablo Delaloza

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https:www.gnu.org/licenses/>.
*/
#include "plugin.hpp"

struct SuperMIDI64 : Module {
	enum ParamIds {
		MINUSONE_PARAM,
		PLUSONE_PARAM,
		SUSTHOLD_PARAM,
		RETRIG_PARAM,
		DATAKNOB_PARAM,
		BENDPITCH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(X_OUTPUT, 4),
		ENUMS(Y_OUTPUT, 4),
		ENUMS(Z_OUTPUT, 4),
		ENUMS(VEL_OUTPUT, 4),
		ENUMS(RVEL_OUTPUT, 4),
		ENUMS(GATE_OUTPUT, 4),
		PBEND_OUTPUT,
		ENUMS(MM_OUTPUT, 20),
		NUM_OUTPUTS
	};
	enum LightIds {
		RESETMIDI_LIGHT,
		ENUMS(CH_LIGHT, 64),
		SUSTHOLD_LIGHT,
		NUM_LIGHTS
	};
////MIDI
	midi::InputQueue midiInput;
	int MPEmasterCh = 0;// 0 ~ 15
	int midiActivity = 0;
	bool resetMidi = false;
	int mdriverJx = -1;
	int mchannelJx = -1;
	std::string mdeviceJx = "";
/////
	enum PolyMode {
		MPE_MODE,
		MPEPLUS_MODE,
		ROTATE_MODE,
    	ROTATE_OUT_MODE,
		REUSE_MODE,
		RESET_MODE,
		REASSIGN_MODE,
		UNISON_MODE,
		UNISONLWR_MODE,
		UNISONUPR_MODE,
		NUM_MODES
	};
	int polyModeIx = ROTATE_MODE;
	/////
	const float POLYMODE_ROW_HEIGHT = 13.3f;
	enum LcdSelectorIds {
		MIDI_LCD,
		POLYMODE_LCD,
		POLYMODE_SELECTOR,
		VOICES_SELECTOR,
		OUTS_SELECTOR,
		ENUMS(NOTE_RANGE_SELECTOR, 2),
		ENUMS(VEL_RANGE_SELECTOR, 2),
		Y_LCD,
		Z_LCD,
		RELVEL_LCD,
		TRNSP_LCD,
		ENUMS(PBEND_LCD, 2),
		ENUMS(CC_LCD, 20),
		NUM_LCDS_SELECTORS
	};
	Vec coords[NUM_LCDS_SELECTORS] = {  //Selector coordinates are relative to LCD
		Vec(9.375f, 20.f),																	//MIDI_LCD
		Vec(9.375f, 62.f),																	//POLYMODE_LCD
		Vec(1.f,		coords[POLYMODE_LCD].y),											//POLYMODE_SELECTOR
		Vec(1.f,		coords[POLYMODE_LCD].y + POLYMODE_ROW_HEIGHT),						//VOICES_SELECTOR
		Vec(67.f, 	coords[POLYMODE_LCD].y + POLYMODE_ROW_HEIGHT),							//OUTS_SELECTOR
		Vec(19.f, 	coords[POLYMODE_LCD].y + POLYMODE_ROW_HEIGHT * 2),						//NOTE_RANGE_SELECTOR 1
		Vec(48.f, 	coords[POLYMODE_LCD].y + POLYMODE_ROW_HEIGHT * 2),						//NOTE_RANGE_SELECTOR 2
		Vec(93.f, 	coords[POLYMODE_LCD].y + POLYMODE_ROW_HEIGHT * 2),						//VEL_RANGE_SELECTOR 1
		Vec(113.f, 	coords[POLYMODE_LCD].y + POLYMODE_ROW_HEIGHT * 2),						//VEL_RANGE_SELECTOR 2
		Vec(206.836f +.5f, 67.135f +.5f),		// +.5 to SVG object						//Y_LCD
		Vec(coords[Y_LCD].x, coords[Y_LCD].y + 43.9f),										//Z_LCD
		Vec(coords[Y_LCD].x, coords[Y_LCD].y + 117.859),									//RELVEL_LCD
		Vec(163.387f +.5f, 161.425f +.5f),	// +.5 to SVG object							//TRNSP_LCD
		Vec(coords[TRNSP_LCD].x + 30.1f, coords[TRNSP_LCD].y),								//PBEND_LCD 1
		Vec(coords[TRNSP_LCD].x + 55.1f, coords[TRNSP_LCD].y),								//PBEND_LCD 2
		Vec(12.161f +.5f, 163.477f +.5f),		// +.5 to SVG object						//CC_LCD 1
		Vec(coords[CC_LCD+0].x + 33.f, 	coords[CC_LCD].y),									//CC_LCD 2
		Vec(coords[CC_LCD+1].x + 33.f, 	coords[CC_LCD].y),									//CC_LCD 3
		Vec(coords[CC_LCD+2].x + 33.f, 	coords[CC_LCD].y),									//CC_LCD 4
		Vec(coords[CC_LCD+0].x,					coords[CC_LCD].y + 40.f),					//CC_LCD 5
		Vec(coords[CC_LCD+0].x + 33.f, 	coords[CC_LCD].y + 40.f),							//CC_LCD 6
		Vec(coords[CC_LCD+1].x + 33.f, 	coords[CC_LCD].y + 40.f),							//CC_LCD 7
		Vec(coords[CC_LCD+2].x + 33.f, 	coords[CC_LCD].y + 40.f),							//CC_LCD 8
		Vec(coords[CC_LCD+0].x, 				coords[CC_LCD+4].y + 40.f),					//CC_LCD 9
		Vec(coords[CC_LCD+0].x + 33.f, 	coords[CC_LCD+4].y + 40.f),							//CC_LCD 10
		Vec(coords[CC_LCD+1].x + 33.f, 	coords[CC_LCD+4].y + 40.f),							//CC_LCD 11
		Vec(coords[CC_LCD+2].x + 33.f, 	coords[CC_LCD+4].y + 40.f),							//CC_LCD 12
		Vec(coords[CC_LCD+0].x, 				coords[CC_LCD+8].y + 40.f),					//CC_LCD 13
		Vec(coords[CC_LCD+0].x + 33.f, 	coords[CC_LCD+8].y + 40.f),							//CC_LCD 14
		Vec(coords[CC_LCD+1].x + 33.f, 	coords[CC_LCD+8].y + 40.f),							//CC_LCD 15
		Vec(coords[CC_LCD+2].x + 33.f, 	coords[CC_LCD+8].y + 40.f),							//CC_LCD 16
		Vec(coords[CC_LCD+0].x, 				coords[CC_LCD+12].y + 40.f),				//CC_LCD 17
		Vec(coords[CC_LCD+0].x + 33.f, 	coords[CC_LCD+12].y + 40.f),						//CC_LCD 18
		Vec(coords[CC_LCD+1].x + 33.f, 	coords[CC_LCD+12].y + 40.f),						//CC_LCD 19
		Vec(coords[CC_LCD+2].x + 33.f, 	coords[CC_LCD+12].y + 40.f),						//CC_LCD 20
	};

	/////
	struct NoteData {
		uint8_t velocity = 0;
		uint8_t aftertouch = 0;
	};
	NoteData noteData[128];

	std::vector<uint8_t> cachedNotes;// Stolen notes (UNISON_MODE and REASSIGN_MODE cache all played)
	std::vector<uint8_t> cachedMPE[16];// MPE stolen notes

	uint8_t notes[64] = {0};
	uint8_t vels[64] = {0};
	uint8_t rvels[64] = {0};
	int16_t mpex[16] = {0};
	uint16_t mpey[16] = {0};
	uint16_t mpez[16] = {0};
	uint8_t mpePlusLB[16] = {0};
	uint8_t chAfTch = 0;
	int16_t mPBnd = 0;
	uint8_t midiCCsVal[20] = {0};

	int midiCCs[20] = {128,1,4,7,10,11,12,13,64,70,71,74,16,17,18,19,80,81,82,83};
	bool gates[64] = {false};

	float xpitch[16] = {0.f};
	float drift[64] = {0.f};
	bool pedalgates[64] = {false}; // gates set to TRUE by pedal if current gate. FALSE by pedal.
	bool pedal = false;
	int rotateIndex = 0;
	int stealIndex = 0;
	int numVOch = 1;
	int numVOout = 1;
	int numVOper = 16;
	int numVo = numVOout * numVOper;
	int pbMainDwn = -2;
	int pbMainUp = 2;
	int pbMPE = 96;
	int driftcents = 10;
	int noteMin = 0;
	int noteMax = 127;
	int velMin = 1;
	int velMax = 127;
	int trnsps = 0;
	int mpeYcc = 74; //cc74 (default MPE Y)
	int mpeZcc = 128; //128 = ChannelAfterTouch (default MPE Z)
	int displayYcc = 74;
	int displayZcc = 128;
	int learnCC = -1;
	int learnNote = -1;
	int cursorIx = -1;
	bool MPEmode = false;
	bool mpePbOut = true;
	float dataKnob = 0.f;
	int frameData = 0;
	int autoFocusOff = 0;
	//uint8_t MPEchMap[16];
	//std::vector<uint8_t> dynMPEch;

	dsp::ExponentialFilter MPExFilter[16];
	dsp::ExponentialFilter MPEyFilter[16];
	dsp::ExponentialFilter MPEzFilter[16];
	dsp::ExponentialFilter MCCsFilter[20];
	dsp::ExponentialFilter mPBndFilter;
	dsp::PulseGenerator reTrigger[64];	// retrigger for stolen notes
	dsp::SchmittTrigger PlusOneTrigger;
	dsp::SchmittTrigger MinusOneTrigger;

	SuperMIDI64() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PLUSONE_PARAM, 0.f, 1.f, 0.f);
		configParam(MINUSONE_PARAM, 0.f, 1.f, 0.f);
		configParam(SUSTHOLD_PARAM, 0.f, 1.f, 1.f);
		configParam(RETRIG_PARAM, 0.f, 1.f, 1.f);
		configParam(DATAKNOB_PARAM, -1.f, 1.f, 0.f);
		configParam(BENDPITCH_PARAM, 0.f, 1.f, 1.f);
		//onReset();
	}
///////////////////////////////////////////////////////////////////////////////////////
	json_t* miditoJson() {//saves last valid driver/device/chn
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "driver", json_integer(mdriverJx));
		json_object_set_new(rootJ, "deviceName", json_string(mdeviceJx.c_str()));
		json_object_set_new(rootJ, "channel", json_integer(mchannelJx));
		return rootJ;
	}
	///////////////////////////////////////////////////////////////////////////////////////
	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "midi", miditoJson());
		json_object_set_new(rootJ, "polyModeIx", json_integer(polyModeIx));
		json_object_set_new(rootJ, "pbMainDwn", json_integer(pbMainDwn));
		json_object_set_new(rootJ, "pbMainUp", json_integer(pbMainUp));
		json_object_set_new(rootJ, "pbMPE", json_integer(pbMPE));
		json_object_set_new(rootJ, "mpePbOut", json_integer(mpePbOut ? 1 : 0));
		json_object_set_new(rootJ, "numVo", json_integer(numVo));
		json_object_set_new(rootJ, "numVOper", json_integer(numVOper));
		json_object_set_new(rootJ, "numVOout", json_integer(numVOout));
		json_object_set_new(rootJ, "MPEmasterCh", json_integer(MPEmasterCh));
		json_object_set_new(rootJ, "midiAcc", json_integer(midiCCs[0]));
		json_object_set_new(rootJ, "midiBcc", json_integer(midiCCs[1]));
		json_object_set_new(rootJ, "midiCcc", json_integer(midiCCs[2]));
		json_object_set_new(rootJ, "midiDcc", json_integer(midiCCs[3]));
		json_object_set_new(rootJ, "midiEcc", json_integer(midiCCs[4]));
		json_object_set_new(rootJ, "midiFcc", json_integer(midiCCs[5]));
		json_object_set_new(rootJ, "midiGcc", json_integer(midiCCs[6]));
		json_object_set_new(rootJ, "midiHcc", json_integer(midiCCs[7]));
		json_object_set_new(rootJ, "midiIcc", json_integer(midiCCs[8]));
		json_object_set_new(rootJ, "midiJcc", json_integer(midiCCs[9]));
		json_object_set_new(rootJ, "midiKcc", json_integer(midiCCs[10]));
		json_object_set_new(rootJ, "midiLcc", json_integer(midiCCs[11]));
		json_object_set_new(rootJ, "midiMcc", json_integer(midiCCs[12]));
		json_object_set_new(rootJ, "midiNcc", json_integer(midiCCs[13]));
		json_object_set_new(rootJ, "midiOcc", json_integer(midiCCs[14]));
		json_object_set_new(rootJ, "midiPcc", json_integer(midiCCs[15]));
		json_object_set_new(rootJ, "midiQcc", json_integer(midiCCs[16]));
		json_object_set_new(rootJ, "midiRcc", json_integer(midiCCs[17]));
		json_object_set_new(rootJ, "midiScc", json_integer(midiCCs[18]));
		json_object_set_new(rootJ, "midiTcc", json_integer(midiCCs[19]));
		json_object_set_new(rootJ, "mpeYcc", json_integer(mpeYcc));
		json_object_set_new(rootJ, "mpeZcc", json_integer(mpeZcc));
		json_object_set_new(rootJ, "driftcents", json_integer(driftcents));
		json_object_set_new(rootJ, "trnsps", json_integer(trnsps));
		json_object_set_new(rootJ, "noteMin", json_integer(noteMin));
		json_object_set_new(rootJ, "noteMax", json_integer(noteMax));
		json_object_set_new(rootJ, "velMin", json_integer(velMin));
		json_object_set_new(rootJ, "velMax", json_integer(velMax));
		return rootJ;
	}
///////////////////////////////////////////////////////////////////////////////////////
	void dataFromJson(json_t *rootJ) override {
		json_t *midiJ = json_object_get(rootJ, "midi");
		if (midiJ)	{
			json_t* driverJ = json_object_get(midiJ, "driver");
			if (driverJ) mdriverJx = json_integer_value(driverJ);
			json_t* deviceNameJ = json_object_get(midiJ, "deviceName");
			if (deviceNameJ) mdeviceJx = json_string_value(deviceNameJ);
			json_t* channelJ = json_object_get(midiJ, "channel");
			if (channelJ) mchannelJx = json_integer_value(channelJ);
			midiInput.fromJson(midiJ);
		}
		json_t *polyModeIxJ = json_object_get(rootJ, "polyModeIx");
		if (polyModeIxJ) polyModeIx = json_integer_value(polyModeIxJ);
		MPEmode = (polyModeIx < ROTATE_MODE);
		json_t *pbMainDwnJ = json_object_get(rootJ, "pbMainDwn");
		if (pbMainDwnJ) pbMainDwn = json_integer_value(pbMainDwnJ);
		json_t *pbMainUpJ = json_object_get(rootJ, "pbMainUp");
		if (pbMainUpJ) pbMainUp = json_integer_value(pbMainUpJ);
		json_t *pbMPEJ = json_object_get(rootJ, "pbMPE");
		if (pbMPEJ) pbMPE = json_integer_value(pbMPEJ);
		json_t *mpePbOutJ = json_object_get(rootJ, "mpePbOut");
		if (mpePbOutJ) mpePbOut = static_cast<bool>(json_integer_value(mpePbOutJ));
		json_t *numVoJ = json_object_get(rootJ, "numVo");
		if (numVoJ) numVo = json_integer_value(numVoJ);
		json_t *numVOperJ = json_object_get(rootJ, "numVOper");
		if (numVOperJ) numVOper = json_integer_value(numVOperJ);
		json_t *numVOoutJ = json_object_get(rootJ, "numVOout");
		if (numVOoutJ) numVOout = json_integer_value(numVOoutJ);
		json_t *MPEmasterChJ = json_object_get(rootJ, "MPEmasterCh");
		if (MPEmasterChJ) MPEmasterCh = json_integer_value(MPEmasterChJ);
		json_t *midiAccJ = json_object_get(rootJ, "midiAcc");
		if (midiAccJ) midiCCs[0] = json_integer_value(midiAccJ);
		json_t *midiBccJ = json_object_get(rootJ, "midiBcc");
		if (midiBccJ) midiCCs[1] = json_integer_value(midiBccJ);
		json_t *midiCccJ = json_object_get(rootJ, "midiCcc");
		if (midiCccJ) midiCCs[2] = json_integer_value(midiCccJ);
		json_t *midiDccJ = json_object_get(rootJ, "midiDcc");
		if (midiDccJ) midiCCs[3] = json_integer_value(midiDccJ);
		json_t *midiEccJ = json_object_get(rootJ, "midiEcc");
		if (midiEccJ) midiCCs[4] = json_integer_value(midiEccJ);
		json_t *midiFccJ = json_object_get(rootJ, "midiFcc");
		if (midiFccJ) midiCCs[5] = json_integer_value(midiFccJ);
		json_t *midiGccJ = json_object_get(rootJ, "midiGcc");
		if (midiGccJ) midiCCs[6] = json_integer_value(midiGccJ);
		json_t *midiHccJ = json_object_get(rootJ, "midiHcc");
		if (midiHccJ) midiCCs[7] = json_integer_value(midiHccJ);
		json_t *midiIccJ = json_object_get(rootJ, "midiIcc");
		if (midiIccJ) midiCCs[8] = json_integer_value(midiIccJ);
		json_t *midiJccJ = json_object_get(rootJ, "midiJcc");
		if (midiJccJ) midiCCs[9] = json_integer_value(midiJccJ);
		json_t *midiKccJ = json_object_get(rootJ, "midiKcc");
		if (midiKccJ) midiCCs[10] = json_integer_value(midiKccJ);
		json_t *midiLccJ = json_object_get(rootJ, "midiLcc");
		if (midiLccJ) midiCCs[11] = json_integer_value(midiLccJ);
		json_t *midiMccJ = json_object_get(rootJ, "midiMcc");
		if (midiMccJ) midiCCs[12] = json_integer_value(midiMccJ);
		json_t *midiNccJ = json_object_get(rootJ, "midiNcc");
		if (midiNccJ) midiCCs[13] = json_integer_value(midiNccJ);
		json_t *midiOccJ = json_object_get(rootJ, "midiOcc");
		if (midiOccJ) midiCCs[14] = json_integer_value(midiOccJ);
		json_t *midiPccJ = json_object_get(rootJ, "midiPcc");
		if (midiPccJ) midiCCs[15] = json_integer_value(midiPccJ);
		json_t *midiQccJ = json_object_get(rootJ, "midiQcc");
		if (midiQccJ) midiCCs[16] = json_integer_value(midiQccJ);
		json_t *midiRccJ = json_object_get(rootJ, "midiRcc");
		if (midiRccJ) midiCCs[17] = json_integer_value(midiRccJ);
		json_t *midiSccJ = json_object_get(rootJ, "midiScc");
		if (midiSccJ) midiCCs[18] = json_integer_value(midiSccJ);
		json_t *midiTccJ = json_object_get(rootJ, "midiTcc");
		if (midiTccJ) midiCCs[19] = json_integer_value(midiTccJ);
		json_t *mpeYccJ = json_object_get(rootJ, "mpeYcc");
		if (mpeYccJ) mpeYcc = json_integer_value(mpeYccJ);
		json_t *mpeZccJ = json_object_get(rootJ, "mpeZcc");
		if (mpeZccJ) mpeZcc = json_integer_value(mpeZccJ);
		json_t *driftcentsJ = json_object_get(rootJ, "driftcents");
		if (driftcentsJ) driftcents = json_integer_value(driftcentsJ);
		json_t *trnspsJ = json_object_get(rootJ, "trnsps");
		if (trnspsJ) trnsps = json_integer_value(trnspsJ);
		json_t *noteMinJ = json_object_get(rootJ, "noteMin");
		if (noteMinJ) noteMin = json_integer_value(noteMinJ);
		json_t *noteMaxJ = json_object_get(rootJ, "noteMax");
		if (noteMaxJ) noteMax = json_integer_value(noteMaxJ);
		json_t *velMinJ = json_object_get(rootJ, "velMin");
		if (velMinJ) velMin = json_integer_value(velMinJ);
		json_t *velMaxJ = json_object_get(rootJ, "velMax");
		if (velMaxJ) velMax = json_integer_value(velMaxJ);
	}
///////////////////////////////////////////////////////////////////////////////////////
	void resetVoices(){
		float lambdaf = 100.f * APP->engine->getSampleTime();
		pedal = false;
		lights[SUSTHOLD_LIGHT].value = 0.f;
		int OUTcount = 0, VOcount = 0; //Iterating across 4 outputs using single 64-index array
		for (int i = 0; i < 64; i++) { //64 for non-MPE
			notes[i] = 60;
			gates[i] = false;
			pedalgates[i] = false;
			vels[i] = 0;
			rvels[i] = 0;
			lights[CH_LIGHT+ i].value = 0.f;
			outputs[GATE_OUTPUT+ OUTcount].setVoltage(0.f, i - OUTcount*16);
			if (++ VOcount == 16) { //Increment VOcount. If incrementation corresponds to next output, adjust to match
				OUTcount ++;
				VOcount = 0;
			}
		}
		for (int i = 0; i < 16; i++) { //16 for MPE-only
			xpitch[i] = {0.f};
			mpex[i] = 0;
			mpey[i] = 0;
			mpez[i] = 0;
			cachedMPE[i].clear();
			MPExFilter[i].lambda = lambdaf;
			MPEyFilter[i].lambda = lambdaf;
			MPEzFilter[i].lambda = lambdaf;
			mpePlusLB[i] = 0;
			}
		rotateIndex = ((polyModeIx == ROTATE_OUT_MODE)? -numVOper : -1);  //For "Output Rotation", ensure that first index is 0 by setting rotateIndex to e.g. -16
		cachedNotes.clear();
		if (polyModeIx < ROTATE_MODE) {
			if (polyModeIx > 0){// Haken MPE Plus
				displayYcc = 131;
				displayZcc = 132;
			}else{
				displayYcc = mpeYcc;
				displayZcc = mpeZcc;
			}
			numVOch = 1;
			//dynMPEch.clear();
		}else {
			displayYcc = 130;
			displayZcc = 129;
			numVOch = numVo;
		}
		learnCC = -1;
		learnNote = -1;
		for (int i=0; i < 20; i++){
			MCCsFilter[i].lambda = lambdaf;
			midiCCsVal[i] = 0;
		}
		mPBndFilter.lambda = lambdaf;
		midiActivity = 96;
		resetMidi = false;
	}
	///////////////////////////////////////////////////////////////////////////////////////
	void onAdd() override{
		resetVoices();
	}
///////////////////////////////////////////////////////////////////////////////////////
	void onReset() override{
		resetVoices();
		//default midi CCs
		midiCCs[0] = 128;
		midiCCs[1] = 1;
		midiCCs[2] = 4;
		midiCCs[3] = 7;
		midiCCs[4] = 10;
		midiCCs[5] = 11;
		midiCCs[6] = 12;
		midiCCs[7] = 13;
		midiCCs[8] = 64;
		midiCCs[9] = 70;
		midiCCs[10] = 71;
		midiCCs[11] = 74;
		midiCCs[12] = 16;
		midiCCs[13] = 17;
		midiCCs[14] = 18;
		midiCCs[15] = 19;
		midiCCs[16] = 80;
		midiCCs[17] = 81;
		midiCCs[18] = 82;
		midiCCs[19] = 83;
		numVOper = 16;
		if (numVOout != 1) {  //When resetting from a state with more than 1 active output, reset the channels of the extra outputs then output count to 1
				for (int i = numVOout; i > 1; i--) {
					outputs[X_OUTPUT+ i- 1].setChannels(0);
					outputs[Y_OUTPUT+ i- 1].setChannels(0);
					outputs[Z_OUTPUT+ i- 1].setChannels(0);
					outputs[VEL_OUTPUT+ i- 1].setChannels(0);
					outputs[RVEL_OUTPUT+ i- 1].setChannels(0);
					outputs[GATE_OUTPUT+ i- 1].setChannels(0);
				}
				numVOout = 1;
		}
		numVo = numVOout * numVOper;  //The number of voices = the number of active outputs multiplied by the number of voices per output
		trnsps = 0;
		pbMainDwn = -12;
		pbMainUp = 12;
		pbMPE = 96;
		mpePbOut = true;
		driftcents = 10;
		noteMin = 0;
		noteMax = 127;
		velMin = 1;
		velMax = 127;
		mpeYcc = 74; //cc74 (default MPE Y)
		mpeZcc = 128; //128 = ChannelAfterTouch (default MPE Z)
		MPEmasterCh = 0;// 0 ~ 15
		displayYcc = 74;
		displayZcc = 128;
		cursorIx = -1;
		learnCC = -1;
		learnNote = -1;
		polyModeIx = ROTATE_MODE;
		MPEmode = false;
	 	rotateIndex = 0;
	 	stealIndex = 0;
	}
///////////////////////////////////////////////////////////////////////////////////////
	int getPolyIndex(int nowIndex) {
		for (int i = 0; i < numVo; i++) {
			nowIndex++;
			if (nowIndex > (numVo - 1))
				nowIndex = 0;
			if (!(gates[nowIndex] || pedalgates[nowIndex])) {
				stealIndex = nowIndex;
				return nowIndex;
			}
		}
		// All taken = steal (rotates)
		stealIndex++;
		if (stealIndex > (numVo - 1))
			stealIndex = 0;
		if ((polyModeIx < REASSIGN_MODE) && (gates[stealIndex]))//&&(polyMode > MPE_MODE).cannot reach here if MPE mode true
			cachedNotes.push_back(notes[stealIndex]);
		return stealIndex;
	}
///////////////////////////////////////////////////////////////////////////////////////
	int getAltPolyIndex(int nowIndex) {  //This alternate function rotates the index across all active outputs, e.g. A[1] -> B[1] -> C[1] -> D[1] -> A[2]...
		for (int i = 0; i < numVo; i++) {
			nowIndex += numVOper;
			if (nowIndex >= numVo)
				nowIndex = (((nowIndex - numVo + 1) == numVOper)? 0 : nowIndex - numVo + 1);  //If we have reached the last channel of the last active output, advance to the first channel of the first output (0)
			if (!(gates[nowIndex] || pedalgates[nowIndex])) {
				stealIndex = nowIndex;
				return nowIndex;
			}
		}
		// All taken = steal (rotates)
		stealIndex += numVOper;
		if (stealIndex > (numVo - 1))
			stealIndex = (((stealIndex - numVo + 1) == numVOper)? 0 : stealIndex - numVo + 1);
		if ((polyModeIx < REASSIGN_MODE) && (gates[stealIndex]))//&&(polyMode > MPE_MODE).cannot reach here if MPE mode true
			cachedNotes.push_back(notes[stealIndex]);
		return stealIndex;
	}
///////////////////////////////////////////////////////////////////////////////////////
	void pressNote(uint8_t channel, uint8_t note, uint8_t vel) {
		switch (learnNote){
			case -1:
				break;
			case NOTE_RANGE_SELECTOR:{
				noteMin = note;
				learnNote = -1;
				cursorIx = -1;
				}break;
			case NOTE_RANGE_SELECTOR + 1:{
				noteMax = note;
				learnNote = -1;
				cursorIx = -1;
			}break;
			case VEL_RANGE_SELECTOR:{
				velMin = vel;
				learnNote = -1;
				cursorIx = -1;
			}break;
			case VEL_RANGE_SELECTOR + 1:{
				velMax = vel;
				learnNote = -1;
				cursorIx = -1;
			}break;
		}

		if (noteMin <= noteMax) {
			if (note < noteMin) return;
			if (note > noteMax) return;
		}
		else if (note < noteMin && note > noteMax) return;

		if (velMin <= velMax) {
			if (vel < velMin) return;
			if (vel > velMax) return;
		}
		else if (vel < velMin && vel > velMax) return;

		// Set notes and gates
		switch (polyModeIx) {
			case MPE_MODE:
			case MPEPLUS_MODE:{
				if (channel == MPEmasterCh) return; /////  R E T U R N !!!!!!!
				//uint8_t ixch;
				if (channel + 1 > numVOch) numVOch = channel + 1;
				rotateIndex = channel; // ASSIGN VOICE Index
				if (gates[channel]) cachedMPE[channel].push_back(notes[channel]);///if gate push note to mpe_buffer
//				std::vector<uint8_t>::iterator it = std::find(dynMPEch.begin(), dynMPEch.end(), channel);
//				if (it != dynMPEch.end()) {//found = get the index of the channel
//					 ixch = std::distance(dynMPEch.begin(), it);
//				}else {// not found = get index (old size) and add
//					ixch = dynMPEch.size();
//					dynMPEch.push_back(channel);
//					numVOch = ixch + 1; // grow dynamic voices
//				}
//				rotateIndex = ixch; // ASSIGN VOICE Index
//				if (gates[ixch]) cachedMPE[ixch].push_back(notes[ixch]);///if gate push note to mpe_buffer
//				MPEchMap[channel] = ixch; //map channel for chPB Y Z and note off
			} break;
			case ROTATE_MODE: {
				rotateIndex = getPolyIndex(rotateIndex);
			} break;
			case ROTATE_OUT_MODE: {  //Added mode for rotating first-across-then-within outputs
				rotateIndex = getAltPolyIndex(rotateIndex);
			} break;
			case REUSE_MODE: {
				bool reuse = false;
				for (int i = 0; i < numVo; i++) {
					if (notes[i] == note) {
						rotateIndex = i;
						reuse = true;
						break;
					}
				}
				if (!reuse)
					rotateIndex = getPolyIndex(rotateIndex);
			} break;
			case RESET_MODE: {
				rotateIndex = getPolyIndex(-1);
			} break;
			case REASSIGN_MODE: {
				cachedNotes.push_back(note);
				rotateIndex = getPolyIndex(-1);
			} break;
			case UNISON_MODE: {
				cachedNotes.push_back(note);
				bool retrignow = static_cast<bool>(params[RETRIG_PARAM].getValue());
				for (int i = 0; i < numVo; i++) {
					notes[i] = note;
					vels[i] = vel;
					gates[i] = true;
					pedalgates[i] = pedal;
					drift[i] = static_cast<float>(rand() % 200  - 100) * static_cast<float>(driftcents) / 120000.f;
					if (retrignow) reTrigger[i].trigger(1e-3);
				}
				return;/////  R E T U R N !!!!!!!
			} break;
			case UNISONLWR_MODE: {
				cachedNotes.push_back(note);
				uint8_t lnote = *min_element(cachedNotes.begin(),cachedNotes.end());
				bool retrignow = static_cast<bool>(params[RETRIG_PARAM].getValue()) && (lnote < notes[0]);
				for (int i = 0; i < numVo; i++) {
					notes[i] = lnote;
					vels[i] = vel;
					gates[i] = true;
					pedalgates[i] = pedal;
					drift[i] = static_cast<float>(rand() % 200  - 100) * static_cast<float>(driftcents) / 120000.f;
					if (retrignow) reTrigger[i].trigger(1e-3);
				}
				return;/////  R E T U R N !!!!!!!
			} break;
			case UNISONUPR_MODE:{
				cachedNotes.push_back(note);
				uint8_t unote = *max_element(cachedNotes.begin(),cachedNotes.end());
				bool retrignow = static_cast<bool>(params[RETRIG_PARAM].getValue()) && (unote > notes[0]);
				for (int i = 0; i < numVo; i++) {
					notes[i] = unote;
					vels[i] = vel;
					gates[i] = true;
					pedalgates[i] = pedal;
					drift[i] = static_cast<float>(rand() % 200  - 100) * static_cast<float>(driftcents) / 120000.f;
					if (retrignow) reTrigger[i].trigger(1e-3);
				}
				return;/////  R E T U R N !!!!!!!
			} break;
			default: break;
		}
		// Set notes and gates
		if (static_cast<bool>(params[RETRIG_PARAM].getValue()) && (gates[rotateIndex] || pedalgates[rotateIndex]))
			reTrigger[rotateIndex].trigger(1e-3);
		notes[rotateIndex] = note;
		vels[rotateIndex] = vel;
		gates[rotateIndex] = true;
		pedalgates[rotateIndex] = pedal;
		drift[rotateIndex] = static_cast<float>((rand() % 1000 - 500) * driftcents) / 1200000.f;
		midiActivity = vel;
	}
///////////////////////////////////////////////////////////////////////////////////////
	void releaseNote(uint8_t channel, uint8_t note, uint8_t vel) {
		//bool backnote = false;
		if (polyModeIx > MPEPLUS_MODE) {
		// Remove the note
			//if (!cachedNotes.empty()) backnote = (note == cachedNotes.back());
			std::vector<uint8_t>::iterator it = std::find(cachedNotes.begin(), cachedNotes.end(), note);
			if (it != cachedNotes.end()) cachedNotes.erase(it);
		}else{
			if (channel == MPEmasterCh) return;
			//get channel from dynamic map
			std::vector<uint8_t>::iterator it = std::find(cachedMPE[channel].begin(), cachedMPE[channel].end(), note);
			if (it != cachedMPE[channel].end()) cachedMPE[channel].erase(it);
		}
		switch (polyModeIx) {
			case MPE_MODE:
			case MPEPLUS_MODE:{
				if (note == notes[channel]) {
					if (pedalgates[channel]) {
						gates[channel] = false;
					}
					/// check for cachednotes on MPE buffers...
					else if (!cachedMPE[channel].empty()) {
						notes[channel] = cachedMPE[channel].back();
						cachedMPE[channel].pop_back();
					}
					else {
						gates[channel] = false;
					}
					rvels[channel] = vel;
				}
			} break;
			case REASSIGN_MODE: {
				for (int i = 0; i < numVo; i++) {
					if (i < (int) cachedNotes.size()) {
						if (!pedalgates[i])
							notes[i] = cachedNotes[i];
						pedalgates[i] = pedal;
					}
					else {
						gates[i] = false;
						rvels[i] = vel;
					}
				}
			} break;
			case UNISON_MODE: {
				if (vel > 128) vel = 64;
				if (!cachedNotes.empty()) {
					uint8_t backnote = cachedNotes.back();
					bool retrignow = static_cast<bool>(params[RETRIG_PARAM].getValue()) && (backnote);
					for (int i = 0; i < numVo; i++) {
						notes[i] = backnote;
						gates[i] = true;
						rvels[i] = vel;
						if (retrignow) reTrigger[i].trigger(1e-3);
					}
				}
				else {
					for (int i = 0; i < numVo; i++) {
						gates[i] = false;
						rvels[i] = vel;
					}
				}
			} break;
			case UNISONLWR_MODE: {
				if (vel > 128) vel = 64;
				if (!cachedNotes.empty()) {
					uint8_t lnote = *min_element(cachedNotes.begin(),cachedNotes.end());
					for (int i = 0; i < numVo; i++) {
						notes[i] = lnote;
						gates[i] = true;
						rvels[i] = vel;
					}
				}
				else {
					for (int i = 0; i < numVo; i++) {
						gates[i] = false;
						rvels[i] = vel;
					}
				}
			} break;
			case UNISONUPR_MODE: {
				if (vel > 128) vel = 64;
				if (!cachedNotes.empty()) {
					uint8_t unote = *max_element(cachedNotes.begin(),cachedNotes.end());
					for (int i = 0; i < numVo; i++) {
						notes[i] = unote;
						gates[i] = true;
						rvels[i] = vel;
					}
				}
				else {
					for (int i = 0; i < numVo; i++) {
						gates[i] = false;
						rvels[i] = vel;
					}
				}
			} break;
			// default ROTATE_MODE REUSE_MODE RESET_MODE
			default: {
				for (int i = 0; i < numVo; i++) {
					if (notes[i] == note) {
						if (pedalgates[i]) {
							gates[i] = false;
						}
						else if (!cachedNotes.empty()) {
							notes[i] = cachedNotes.back();
							cachedNotes.pop_back();
						}
						else {
							gates[i] = false;
						}
						rvels[i] = vel;
					}
				}
			} break;
		}
		midiActivity = vel;
	}
///////////////////////////////////////////////////////////////////////////////////////
	void pressPedal() {
		pedal = true;
		lights[SUSTHOLD_LIGHT].value = params[SUSTHOLD_PARAM].getValue();
		if (polyModeIx == MPE_MODE) {
			for (int i = 0; i < numVOch; i++) {
				pedalgates[i] = gates[i];
			}
		}else {
			for (int i = 0; i < numVo; i++) {
				pedalgates[i] = gates[i];
			}
		}
	}
///////////////////////////////////////////////////////////////////////////////////////
	void releasePedal() {
		pedal = false;
		lights[SUSTHOLD_LIGHT].value = 0.f;
		// When pedal is off, recover notes for pressed keys (if any) after they were already being shut by pedal-sustained notes.
		if (polyModeIx < ROTATE_MODE) {
			for (int i = 0; i < numVOch; i++) {
				pedalgates[i] = false;
				if (!cachedMPE[i].empty()) {
						notes[i] = cachedMPE[i].back();
						cachedMPE[i].pop_back();
						gates[i] = true;
				}
			}
		}else{
			for (int i = 0; i < numVo; i++) {
				pedalgates[i] = false;
				if (!cachedNotes.empty()) {
					if  (polyModeIx < REASSIGN_MODE){
						notes[i] = cachedNotes.back();
						cachedNotes.pop_back();
						gates[i] = true;
					}
				}
			}
			if (polyModeIx == REASSIGN_MODE) {
				for (int i = 0; i < numVo; i++) {
					if (i < (int) cachedNotes.size()) {
						notes[i] = cachedNotes[i];
						gates[i] = true;
					}
					else {
						gates[i] = false;
					}
				}
			}
		}
	}
///////////////////////////////////////////////////////////////////////////////////////
	void processMessage(midi::Message msg) {
		switch (msg.getStatus()) {
				// note off
			case 0x8: {
				if ((polyModeIx < ROTATE_MODE) && (msg.getChannel() == MPEmasterCh)) return;
				releaseNote(msg.getChannel(), msg.getNote(), msg.getValue());
			} break;
				// note on
			case 0x9: {
				if ((polyModeIx < ROTATE_MODE) && (msg.getChannel() == MPEmasterCh)) return;
				if (msg.getValue() > 0) {
					pressNote(msg.getChannel(), msg.getNote(), msg.getValue());
				}
				else {
					releaseNote(msg.getChannel(), msg.getNote(), 64);//64 is the default rel velocity.
				}
			} break;
				// note (poly) aftertouch
			case 0xa: {
				if (polyModeIx < ROTATE_MODE) return;
				noteData[msg.getNote()].aftertouch = msg.getValue();
				midiActivity = msg.getValue();
			} break;
				// channel aftertouch
			case 0xd: {
				if (learnCC > -1) {// learn enabled ???
					midiCCs[learnCC] = 128;
					learnCC = -1;
					return;
				}////////////////////////////////////////
				else if (polyModeIx < ROTATE_MODE){
					uint8_t channel = msg.getChannel();
					if (channel == MPEmasterCh){
						chAfTch = msg.getNote();
					}else if (polyModeIx > 0){
						mpez[channel] =  msg.getNote() * 128 + mpePlusLB[channel];
						mpePlusLB[channel] = 0;
					}else {
						if (mpeZcc == 128)
							mpez[channel] = msg.getNote() * 128;
						if (mpeYcc == 128)
							mpey[channel] = msg.getNote() * 128;
					}
				}else{
					chAfTch = msg.getNote();
				}
				midiActivity = msg.getNote();
			} break;
				// pitch Bend
			case 0xe:{
				if (learnCC > -1) {// learn enabled ???
					midiCCs[learnCC] = 128;
					learnCC = -1;
					return;
				}////////////////////////////////////////
				else if (polyModeIx < ROTATE_MODE){
					uint8_t channel = msg.getChannel();
					if (channel == MPEmasterCh){
						mPBnd = msg.getValue() * 128 + msg.getNote()  - 8192;
					}else{
						mpex[channel] = msg.getValue() * 128 + msg.getNote()  - 8192;
					}
				}else{
					mPBnd = msg.getValue() * 128 + msg.getNote() - 8192; //14bit Pitch Bend
				}
				midiActivity = msg.getValue();
			} break;
				// cc
			case 0xb: {
				if (polyModeIx < ROTATE_MODE){
					uint8_t channel = msg.getChannel();
					if (channel == MPEmasterCh){
						if (learnCC > -1) {///////// LEARN CC MPE master
							midiCCs[learnCC] = msg.getNote();
							learnCC = -1;
							return;
						}else processCC(msg);
					}else if (polyModeIx == MPEPLUS_MODE){ //Continuum
						if (msg.getNote() == 87){
							mpePlusLB[channel] = msg.getValue();
						}else if (msg.getNote() == 74){
							mpey[channel] =  msg.getValue() * 128 + mpePlusLB[channel];
							mpePlusLB[channel] = 0;
						}
					}else if (msg.getNote() == mpeYcc){
						//cc74 0x4a default
						mpey[channel] = msg.getValue() * 128;
					}else if (msg.getNote() == mpeZcc){
						mpez[channel] = msg.getValue() * 128;
					}
				}else if (learnCC > -1) {///////// LEARN CC Poly
					midiCCs[learnCC] = msg.getNote();
					learnCC = -1;
					return;
				}else processCC(msg);
				midiActivity = msg.getValue();
			} break;
			default: break;
		}
	}
///////////////////////////////////////////////////////////////////////////////////////
	void processCC(midi::Message msg) {
		if (msg.getNote() ==  0x40) { //internal sust pedal
			if (msg.getValue() >= 64)
				pressPedal();
			else
				releasePedal();
		}
		for (int i = 0; i < 20; i++){
			if (midiCCs[i] == msg.getNote()){
				midiCCsVal[i] = msg.getValue();
				return;
			}
		}
	}
///////////////////////////////////////////////////////////////////////////////////////
	void dataPlus(){
		switch (cursorIx){
			case -1: {
			}break;
			case POLYMODE_SELECTOR: {
				if (numVo == 1) {
					switch (polyModeIx){
						case MPEPLUS_MODE:
							polyModeIx = UNISON_MODE;
							break;
						case UNISONUPR_MODE:
							polyModeIx = MPE_MODE;
							break;
						default:
							polyModeIx++;
							break;
					}
				}else{
					if (polyModeIx < UNISONUPR_MODE) polyModeIx ++;
					else polyModeIx = MPE_MODE;
				}
				MPEmode = (polyModeIx < ROTATE_MODE);
				resetVoices();
			}break;
			case VOICES_SELECTOR: {
				if (polyModeIx < ROTATE_MODE) {
					if (pbMPE < 96) pbMPE ++;
				} else {
					if (numVOper < 16) {  //In non-MPE modes, dataPlus here increments the number of voices-per-output rather than the total number of voices
						numVOper ++;
						numVo = numVOout * numVOper;  //Update total number of voices to match
					}
					resetVoices();
				}
			}break;
			case OUTS_SELECTOR: {  //Added incrementable setting for number of active outputs
				if (numVOout < 4) {
					numVOout ++;
					numVo = numVOout * numVOper;
				}
				resetVoices();
			}break;
			case NOTE_RANGE_SELECTOR: {
				if (noteMin < 127) noteMin ++;
			}break;
			case NOTE_RANGE_SELECTOR + 1: {
				if (noteMax < 127) noteMax ++;
			}break;
			case VEL_RANGE_SELECTOR: {
				if (velMin < 127) velMin ++;
			}break;
			case VEL_RANGE_SELECTOR + 1: {
				if (velMax < 127) velMax ++;
			}break;
			case Y_LCD: {
				if (polyModeIx == MPE_MODE) {
					if (mpeYcc <128) mpeYcc ++;
					displayYcc = mpeYcc;
				}else if (driftcents < 99) driftcents ++;
			}break;
			case Z_LCD: {
				if (mpeZcc < 128) mpeZcc ++;
				else mpeZcc = 0;
				displayZcc = mpeZcc;
			}break;
			case RELVEL_LCD: {
				mpePbOut = !mpePbOut ;
			}break;
			case TRNSP_LCD: {
				if (trnsps < 48) trnsps ++;
			}break;
			case PBEND_LCD: {
				if (pbMainDwn < 0) pbMainDwn ++;
			}break;
			case PBEND_LCD + 1: {
				if (pbMainUp < 96) pbMainUp ++;
			}break;
			default: {
				if (midiCCs[cursorIx - 15] < 128)  //CC_LCD == 15
					midiCCs[cursorIx - 15]  ++;
				else midiCCs[cursorIx - 15] = 0;
			}break;
		}
		autoFocusOff = 10 * APP->engine->getSampleRate();
		return;
	}
///////////////////////////////////////////////////////////////////////////////////////
	void dataMinus(){
		switch (cursorIx){
			case -1: {
			}break;
			case POLYMODE_SELECTOR: {
				if (numVo == 1) {
					switch (polyModeIx){
						case MPE_MODE:
							polyModeIx = UNISONUPR_MODE;
							break;
						case UNISON_MODE:
							polyModeIx = MPEPLUS_MODE;
							break;
						default:
							polyModeIx--;
							break;
					}
				}else{
					if (polyModeIx > MPE_MODE) polyModeIx --;
					else polyModeIx = UNISONUPR_MODE;
				}
				MPEmode = (polyModeIx < ROTATE_MODE);
				resetVoices();
			}break;
			case VOICES_SELECTOR: {
				if (polyModeIx < ROTATE_MODE) {
					if (pbMPE > 0) pbMPE --;
				} else {
					if (numVOper > 1) {  //In non-MPE modes, dataPlus here increments the number of voices-per-output rather than the total number of voices
						numVOper --;
						numVo = numVOout * numVOper;  //Update total number of voices to match
						if (numVo == 1) polyModeIx = UNISON_MODE;
					}
					resetVoices();
				}
			}break;
			case OUTS_SELECTOR: {
				if (numVOout > 1) {  //Added decrementable setting for number of active outputs
					numVOout --;
					numVo = numVOper * numVOout;
					//Reset channels on the now-deactivated output
					outputs[X_OUTPUT+ numVOout].setChannels(0);
					outputs[Y_OUTPUT+ numVOout].setChannels(0);
					outputs[Z_OUTPUT+ numVOout].setChannels(0);
					outputs[VEL_OUTPUT+ numVOout].setChannels(0);
					outputs[RVEL_OUTPUT+ numVOout].setChannels(0);
					outputs[GATE_OUTPUT+ numVOout].setChannels(0);
          if (numVOout < 2 && polyModeIx == ROTATE_OUT_MODE) polyModeIx = ROTATE_MODE;  //If using ROTATE OUT mode and the number of outputs is reduced to 1, revert to basic ROTATE mode
				}
				resetVoices();
			}break;
			case NOTE_RANGE_SELECTOR: {
				if (noteMin > 0) noteMin --;
			}break;
			case NOTE_RANGE_SELECTOR + 1: {
				if (noteMax > 0) noteMax --;
			}break;
			case VEL_RANGE_SELECTOR: {
				if (velMin > 1) velMin --;
			}break;
			case VEL_RANGE_SELECTOR + 1: {
				if (velMax > 1) velMax --;
			}break;
			case Y_LCD: {
				if (polyModeIx == MPE_MODE) {
					if (mpeYcc > 0) mpeYcc --;
					displayYcc = mpeYcc;
				}else if (driftcents > 0) driftcents --;
			}break;
			case Z_LCD: {
				if (mpeZcc > 0) mpeZcc --;
				else mpeZcc = 128;
				displayZcc = mpeZcc;
			}break;
			case RELVEL_LCD: {
				mpePbOut = !mpePbOut ;
			}break;
			case TRNSP_LCD: {
				if (trnsps > -48) trnsps --;
			}break;
			case PBEND_LCD: {
				if (pbMainDwn > -96) pbMainDwn --;
			}break;
			case PBEND_LCD + 1: {
				if (pbMainUp > 0) pbMainUp --;
			}break;
			default: {
				if (midiCCs[cursorIx - 15] > 0)  //CC_LCD == 15
					midiCCs[cursorIx - 15] --;
				else midiCCs[cursorIx - 15] = 128;
			}break;
		}
		autoFocusOff = 10 * APP->engine->getSampleRate();
		return;
	}
///////////////////////////////////////////////////////////////////////////////////////
	void onSampleRateChange() override {
		resetVoices();
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////
//////   STEP START
///////////////////////
	void process(const ProcessArgs &args) override {
		for (int i = 0; i < numVOout; i++) {  //For each active output, set channels to number of voices-per-output
			outputs[X_OUTPUT+ i].setChannels(numVOper);
			outputs[Y_OUTPUT+ i].setChannels(numVOper);
			outputs[Z_OUTPUT+ i].setChannels(numVOper);
			outputs[VEL_OUTPUT+ i].setChannels(numVOper);
			outputs[RVEL_OUTPUT+ i].setChannels(numVOper);
			outputs[GATE_OUTPUT+ i].setChannels(numVOper);
		}

		midi::Message msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}
		float pbVo = 0.f, pbVoice = 0.f;
		if (mPBnd < 0){
			pbVo = mPBndFilter.process(1.f ,rescale(mPBnd, -8192, 0, -5.f, 0.f));
			pbVoice = -1.f * pbVo * pbMainDwn / 60.f;
		} else {
			pbVo = mPBndFilter.process(1.f ,rescale(mPBnd, 0, 8191, 0.f, 5.f));
			pbVoice = pbVo * pbMainUp / 60.f;
		}
		outputs[PBEND_OUTPUT].setVoltage(pbVo);
		bool sustainHold = (params[SUSTHOLD_PARAM].getValue() > .5 );
		if (polyModeIx > MPEPLUS_MODE){
			int OUTcount = 0, VOcount = 0;  //Iterating across 4 outputs using single 64-index array
			for (int i = 0; i < numVo; i++) {
				float lastGate = ((gates[i] || (sustainHold && pedalgates[i])) && (!(reTrigger[i].process(args.sampleTime))))? 10.f : 0.f;
				float thispitch = ((notes[i] - 60 + trnsps) / 12.f) + (params[BENDPITCH_PARAM].getValue() == 1.f ? pbVoice : 0);
				outputs[GATE_OUTPUT+ OUTcount].setVoltage(lastGate, i - OUTcount*numVOper);
				outputs[X_OUTPUT+ OUTcount].setVoltage(thispitch, i - OUTcount*numVOper);
				outputs[Y_OUTPUT+ OUTcount].setVoltage(thispitch + drift[i] , i - OUTcount*numVOper);	//drifted out
				outputs[VEL_OUTPUT+ OUTcount].setVoltage(rescale(vels[i], 0, 127, 0.f, 10.f), i - OUTcount*numVOper);
				outputs[RVEL_OUTPUT+ OUTcount].setVoltage(rescale(rvels[i], 0, 127, 0.f, 10.f), i - OUTcount*numVOper);
				outputs[Z_OUTPUT+ OUTcount].setVoltage(rescale(noteData[notes[i]].aftertouch, 0, 127, 0.f, 10.f), i - OUTcount*numVOper);
				lights[CH_LIGHT+ i].value = ((i == rotateIndex)? 0.2f : 0.f) + (lastGate * .08f);
				if (++ VOcount == numVOper) {  //Increment VOcount. If incrementation corresponds to next output, adjust to match
					OUTcount ++;
					VOcount = 0;
				}
			}
		} else {/// MPE MODE!!!
			for (int i = 0; i < numVOch; i++) {
					float lastGate = ((gates[i] || (sustainHold && pedalgates[i])) && (!(reTrigger[i].process(args.sampleTime))))? 10.f : 0.f;
					outputs[GATE_OUTPUT].setVoltage(lastGate, i);
					if (mpex[i] < 0) xpitch[i] = (MPExFilter[i].process(1.f ,rescale(mpex[i], -8192, 0, -5.f, 0.f)));
					else xpitch[i] = (MPExFilter[i].process(1.f ,rescale(mpex[i], 0, 8191, 0.f, 5.f)));
					outputs[X_OUTPUT].setVoltage(xpitch[i]  * pbMPE / 60.f + ((notes[i] - 60) / 12.f) + pbVoice, i);
					outputs[VEL_OUTPUT].setVoltage(rescale(vels[i], 0, 127, 0.f, 10.f), i);
					if (mpePbOut || (polyModeIx > MPE_MODE)) outputs[RVEL_OUTPUT].setVoltage(xpitch[i], i);
					else outputs[RVEL_OUTPUT].setVoltage(rescale(rvels[i], 0, 127, 0.f, 10.f), i);
					outputs[Y_OUTPUT].setVoltage(MPEyFilter[i].process(1.f ,rescale(mpey[i], 0, 16383, 0.f, 10.f)), i);
					outputs[Z_OUTPUT].setVoltage(MPEzFilter[i].process(1.f ,rescale(mpez[i], 0, 16383, 0.f, 10.f)), i);
					lights[CH_LIGHT + i].value = ((i == rotateIndex)? 0.2f : 0.f) + (lastGate * .08f);
			}
		}
		for (int i = 0; i < 20; i++){
			if (midiCCs[i] == 128)
				outputs[MM_OUTPUT + i].setVoltage(MCCsFilter[i].process(1.f ,rescale(chAfTch, 0, 127, 0.f, 10.f)));
			else
				outputs[MM_OUTPUT + i].setVoltage(MCCsFilter[i].process(1.f ,rescale(midiCCsVal[i], 0, 127, 0.f, 10.f)));
		}
		if (resetMidi) resetVoices();// resetMidi from MIDI widget;
		if (autoFocusOff > 0){
			autoFocusOff --;
			if (autoFocusOff < 1){
				autoFocusOff = 0;
				learnCC = -1;
				learnNote = -1;
				cursorIx = -1;
			}
		}
		//// PANEL KNOB AND BUTTONS
		dataKnob = params[DATAKNOB_PARAM].getValue();
		if ( dataKnob > 0.07f){
			if (learnCC + learnNote > -1) return;
			int knobInterval = static_cast<int>(0.03 * args.sampleRate / dataKnob);
			if (frameData ++ > knobInterval){
				frameData = 0;
				dataPlus();
			}
		}else if(dataKnob < -0.07f){
			if (learnCC + learnNote > -1) return;
			int knobInterval = static_cast<int>(0.03 * args.sampleRate / -dataKnob);
			if (frameData ++ > knobInterval){
				frameData = 0;
				dataMinus();
			}
		}
		if (MinusOneTrigger.process(params[MINUSONE_PARAM].getValue())) {
			if (learnCC + learnNote > -1) return;
			dataMinus();
			return;
		}
		if (PlusOneTrigger.process(params[PLUSONE_PARAM].getValue())) {
			if (learnCC + learnNote > -1) return;
			dataPlus();
			return;
		}
	}
///////////////////////
//////   STEP END
///////////////////////
};
// Main Display///////////////////////////////////////////////////////////////////////////////////////
struct PolyModeDisplayC : TransparentWidget {
	PolyModeDisplayC(){
		font = APP->window->loadFont(mFONT_FILE);
	}
	SuperMIDI64 *module;
	float mdfontSize = 11.f;
	std::string sMode = "";
	std::string sVo = "";
	std::string sOut = "";  //Added string for number of active outputs
	std::string sPBM = "";
	std::string snoteMin = "";
	std::string snoteMax = "";
	std::string svelMin = "";
	std::string svelMax = "";
	std::string yyDisplay = "";
	std::string zzDisplay = "";
	std::shared_ptr<Font> font;
	std::string polyModeStr[10] = {
		"M. P. E.",
		"M. P. E. Plus",
		"R O T A T E",
    	"R O T A T E Out",
		"R E U S E",
		"R E S E T",
		"R E A S S I G N",
		"U N I S O N",
		"U N I S O N Lower",
		"U N I S O N Upper",
	};
	std::string noteName[12] = {"C","C#","D","Eb","E","F","F#","G","Ab","A","Bb","B"};
	int drawFrame = 0;
	int cursorIxI = 0;
	int flashFocus = 0;
	unsigned char lrnflash = 0x55;
	const unsigned char rgbx = 0xdd;
	unsigned char rgblrn, gb1, gb2, gb3, gb4, gb1f, gb2f, gb3f, gb4f;
	unsigned char rgbf1 = 0xdd;
	unsigned char rgbf2 = 0xdd;
	unsigned char rgbf3 = 0xdd;

	void draw(const DrawArgs &args) override {
		if (cursorIxI != module->cursorIx){
			cursorIxI = module->cursorIx;
			flashFocus = 64;
		}
		sMode = polyModeStr[module->polyModeIx];
		if (module->polyModeIx < 2){
			sVo =  "Vo chnl PB: " + std::to_string(module->pbMPE);
			sOut = "";
		}else{
			sVo = "Vo per Out: " + std::to_string(module->numVOper);
			sOut = "Active Outs: " + std::to_string(module->numVOout);
		}
		snoteMin = noteName[module->noteMin % 12] + std::to_string((module->noteMin / 12) - 2);
		snoteMax = noteName[module->noteMax % 12] + std::to_string((module->noteMax / 12) - 2);
		svelMin = std::to_string(module->velMin);
		svelMax = std::to_string(module->velMax);
		gb1 = rgbx;
		gb2 = rgbx;
		gb3 = rgbx;
		gb4 = rgbx;
		rgbf1 = rgbx;
		rgbf2 = rgbx;
		rgbf3 = rgbx;
		gb1f = rgbx;
		gb2f = rgbx;
		gb3f = rgbx;
		gb4f = rgbx;
		rgblrn =  0x64;
		switch (cursorIxI){
			case -1: {
				goto noFocus;
			}break;
			case SuperMIDI64::POLYMODE_SELECTOR:{ // PolyMode
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 1.f, 1.f, 134.f, 12.f, 3.f);
				lrnflash = 0x64;
				rgbf1 = 0;
			}break;
			case SuperMIDI64::VOICES_SELECTOR:{ //numVoicesPerOutput/PB MPE
				nvgBeginPath(args.vg);
				if (module->polyModeIx < 2) {  //In MPE modes, react as full-width button
					nvgRoundedRect(args.vg, 1.f, 14.f, 134.f, 12.f, 3.f);
				}else{  //In non-MPE modes, react as half-width button on left side
					nvgRoundedRect(args.vg, 1.f, 14.f, 67.f, 12.f, 3.f);
				}
				lrnflash = 0x64;
				rgbf2 = 0;
			}break;
			case SuperMIDI64::OUTS_SELECTOR: { //numVoiceOutputs. This case is only reachable in non-MPE modes
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 67.f, 14.f, 67.f, 12.f, 3.f);
				lrnflash = 0x64;
				rgbf3 = 0;
			}break;
			case SuperMIDI64::NOTE_RANGE_SELECTOR:{ //minNote
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 19.f, 28.f, 29.f, 12.f, 3.f);
				gb1f = 0;
				if (module->learnNote == SuperMIDI64::NOTE_RANGE_SELECTOR) {
					snoteMin = "LRN";
					//if ((lrnflash += 16) > 255) lrnflash = 0;
					gb1 = 0;
					rgblrn = 0;
				}else{
					lrnflash = 0x64;
				}
			}break;
			case SuperMIDI64::NOTE_RANGE_SELECTOR + 1:{ //maxNote
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 48.f, 28.f, 29.f, 12.f, 3.f);
				gb2f = 0;
				if (module->learnNote == SuperMIDI64::NOTE_RANGE_SELECTOR + 1) {
					snoteMax = "LRN";
					//if ((lrnflash += 16) > 255) lrnflash = 0;
					gb2 = 0;
					rgblrn = 0;
				}else{
					lrnflash = 0x64;
				}
			}break;
			case SuperMIDI64::VEL_RANGE_SELECTOR:{ //minVel
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 93.f, 28.f, 20.f, 12.f, 3.f);
				gb3f = 0;
				if (module->learnNote == SuperMIDI64::VEL_RANGE_SELECTOR) {
					svelMin = "LRN";
					//if ((lrnflash += 16) > 255) lrnflash = 0;
					gb3 = 0;
					rgblrn = 0;
				}else{
					lrnflash = 0x64;
				}
			}break;
			case SuperMIDI64::VEL_RANGE_SELECTOR + 1:{ //maxVel
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 113.f, 28.f, 20.f, 12.f, 3.f);
				gb4f = 0;
				if (module->learnNote == SuperMIDI64::VEL_RANGE_SELECTOR + 1) {
					svelMax = "LRN";
					//if ((lrnflash += 16) > 255) lrnflash = 0;
					gb4 = 0;
					rgblrn = 0;
				}else{
					lrnflash = 0x64;
				}
			}break;
			default:{
				goto noFocus;
			}
		}
		if (flashFocus > 0)	flashFocus --;
		nvgFillColor(args.vg, nvgRGBA(lrnflash, rgblrn, 0, 0xc0 + flashFocus)); //SELECTED
		nvgFill(args.vg);
		//nvgGlobalCompositeBlendFunc(args.vg,  NVG_ONE , NVG_ONE);
	noFocus:
		nvgFontSize(args.vg, mdfontSize);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
		nvgFillColor(args.vg, nvgRGB(rgbx, rgbx, rgbf1));
		nvgTextBox(args.vg, 1.f, 10.5f, 134.f, sMode.c_str(), NULL);
		if (module->polyModeIx < 2) {  //One button in MPE modes
			nvgFillColor(args.vg, nvgRGB(rgbx, rgbx, rgbf2));
			nvgTextBox(args.vg, 1.f, 23.5f, 134.f, sVo.c_str(), NULL);
		}else{  //Two buttons in non-MPE modes
			nvgFillColor(args.vg, nvgRGB(rgbx, rgbx, rgbf2));
			nvgTextBox(args.vg, 1.f, 23.5f, 67.f, sVo.c_str(), NULL);
			nvgFillColor(args.vg, nvgRGB(rgbx, rgbx, rgbf3));
			nvgTextBox(args.vg, 67.f, 23.5f, 67.f, sOut.c_str(), NULL);
		}
		nvgFillColor(args.vg, nvgRGB(rgbx, gb1 & gb2, gb1f & gb2f));
		nvgTextBox(args.vg, 1.f, 36.5f, 18.f, "nte:", NULL);
		nvgFillColor(args.vg, nvgRGB(rgbx, gb1, gb1f));
		nvgTextBox(args.vg, 19.f, 36.5f, 29.f, snoteMin.c_str(), NULL);
		nvgFillColor(args.vg, nvgRGB(rgbx, gb2, gb2f));
		nvgTextBox(args.vg, 48.f, 36.5f, 29.f, snoteMax.c_str(), NULL);
		nvgFillColor(args.vg, nvgRGB(rgbx, gb3 & gb4, gb3f & gb4f));
		nvgTextBox(args.vg, 77.f, 36.5f, 16.f, "vel:", NULL);
		nvgFillColor(args.vg, nvgRGB(rgbx, gb3, gb3f));
		nvgTextBox(args.vg, 93.f, 36.5f, 20.f, svelMin.c_str(), NULL);
		nvgFillColor(args.vg, nvgRGB(rgbx, gb4, gb4f));
		nvgTextBox(args.vg, 113.f, 36.5f, 20.f, svelMax.c_str(), NULL);
	}
	void onButton(const event::Button &e) override {
		if ((e.button == GLFW_MOUSE_BUTTON_LEFT) && (e.action == GLFW_PRESS)){
			int i = -1;
			int row = static_cast<int>(e.pos.y / module->POLYMODE_ROW_HEIGHT);
			if (row > 1) {
				if (e.pos.x < 77.f) {		//position of "vel:" text
					if (e.pos.x > module->coords[SuperMIDI64::NOTE_RANGE_SELECTOR + 1].x) i = SuperMIDI64::NOTE_RANGE_SELECTOR + 1;
					else if (e.pos.x > module->coords[SuperMIDI64::NOTE_RANGE_SELECTOR].x) i = SuperMIDI64::NOTE_RANGE_SELECTOR;
				}
				else if (e.pos.x > module->coords[SuperMIDI64::VEL_RANGE_SELECTOR].x) {
					if (e.pos.x > module->coords[SuperMIDI64::VEL_RANGE_SELECTOR + 1].x) i = SuperMIDI64::VEL_RANGE_SELECTOR + 1;
					else i = SuperMIDI64::VEL_RANGE_SELECTOR;
				}
				if (module->cursorIx != i){
					module->cursorIx = i;
					module->learnNote = i;
					module->autoFocusOff = 10 * APP->engine->getSampleRate();
				} else if (module->learnNote == i)	{
					module->learnNote = -1;
					module->autoFocusOff = 10 * APP->engine->getSampleRate();
				}else {
					module->learnNote = -1;
					module->cursorIx = -1;
					module->autoFocusOff = 0;
				}
			} else {
				module->learnNote = -1;
				if (row == 1) {
					if (module->polyModeIx > 1) {
						if (e.pos.x > 67.f) i = SuperMIDI64::OUTS_SELECTOR;
						else i = SuperMIDI64::VOICES_SELECTOR;
					}
					else i = SuperMIDI64::VOICES_SELECTOR;
				}
				else i = SuperMIDI64::POLYMODE_SELECTOR;
				if (module->cursorIx != i)	{
					module->cursorIx = i;
					module->autoFocusOff = 10 * APP->engine->getSampleRate();
				}else {
					module->cursorIx = -1;
					module->autoFocusOff = 0;
				}
			}
			module->learnCC = -1;
		}
	}
};
///////////////////////////////////////////////////////////////////////////////////////
struct MidiccDisplayC : OpaqueWidget {
	MidiccDisplayC(){
	font = APP->window->loadFont(mFONT_FILE);
	}
	SuperMIDI64 *module;
	float mdfontSize = 11.f;
	std::string sDisplay = "";
	int displayID = -1;
	int ccNumber = -1;
	int pbDwn = 222;
	int pbUp = 222;
	int trnsps = 222;
	int driftcents = 0;
	bool learnOn = false;
	int mymode = 0;
	bool focusOn = false;
	int flashFocus = 0;
	bool canlearn = true;
	bool canedit = true;
	int polychanged = -1;
	int lrncol = 0;
	std::shared_ptr<Font> font;
	void draw(const DrawArgs &args) override{
			switch (displayID){
				case 0:{
					if (module->polyModeIx < SuperMIDI64::ROTATE_MODE) {
						if ((ccNumber != module->displayYcc) || (polychanged != module->polyModeIx)) {
							ccNumber = module->displayYcc;
							polychanged = module->polyModeIx;
							displayedCC();
						}
						canedit = (module->polyModeIx == SuperMIDI64::MPE_MODE);
					}else{
						if ((driftcents != module->driftcents) || (polychanged != module->polyModeIx)) {
							driftcents = module->driftcents;
							polychanged = module->polyModeIx;
							sDisplay = "+-" + std::to_string(module->driftcents) + "cnt";
						}
						canedit = true;
					}
					canlearn = false;
				}break;
				case 1:{
					if (ccNumber != module->displayZcc) {
						ccNumber = module->displayZcc;
						//mymode = 0;
						displayedCC();
					}
					canedit = (module->polyModeIx == SuperMIDI64::MPE_MODE);
					canlearn = false;
				}break;
				case 2:{
					switch (module->polyModeIx) {
						case SuperMIDI64::MPE_MODE: {
							if (module->mpePbOut) sDisplay = "chnPB";
							else sDisplay = "relVel";
							canedit = true;
						}break;
						case SuperMIDI64::MPEPLUS_MODE: {
							sDisplay = "chnPB";
							canedit = false;
						}break;
						default: {
							sDisplay = "relVel";
							canedit = false;
						}break;
					}
					canlearn = false;
				}break;
				case 3:{
					if (trnsps != module->trnsps){
						trnsps = module->trnsps;
						if (trnsps > 0) sDisplay = "t+" + std::to_string(trnsps);
						else if (trnsps < 0) sDisplay = "t" + std::to_string(trnsps);
						else sDisplay = "t 0";
					}
					canedit = true;
					canlearn = false;
				}break;
				case 4:{
					if (pbDwn != module->pbMainDwn){
						pbDwn = module->pbMainDwn;
						if (pbDwn > 0) sDisplay = "d+" + std::to_string(pbDwn);
						else if (pbDwn < 0) sDisplay = "d" + std::to_string(pbDwn);
						else sDisplay = "d 0";
					};
					canlearn = false;
				}break;
				case 5:{
					if (pbUp != module->pbMainUp){
						pbUp = module->pbMainUp;
						if (pbUp > 0) sDisplay = "u+" + std::to_string(pbUp);
						else if (pbUp > 0) sDisplay = "u" + std::to_string(pbUp);
						else sDisplay = "u 0";
					}
					canlearn = false;
				}break;
				default:{
					if (ccNumber != module->midiCCs[displayID - 6]) {
						ccNumber = module->midiCCs[displayID - 6];
						displayedCC();
					}
					canlearn = true;
					if ((mymode==2) && (module->learnCC == -1)) {
						mymode = 0;
						displayedCC();
					}
				}break;
			}
			if (focusOn && (displayID != (module->cursorIx - SuperMIDI64::Y_LCD))){
				focusOn = false;
				if (mymode == 2) {
					displayedCC();
				}
				mymode = 0;
			}
			nvgFontSize(args.vg, mdfontSize);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
			switch(mymode){
				case 0:{
					unsigned char strgb = (canedit)? 0xdd : 0x99;
					nvgFillColor(args.vg, nvgRGB(strgb, strgb, strgb));
				}break;
				case 1:{
					nvgBeginPath(args.vg);
					nvgRoundedRect(args.vg, 0.f, 0.f, box.size.x, box.size.y,3.f);
					if (flashFocus > 0) flashFocus -= 2;
					nvgFillColor(args.vg, nvgRGBA(0x64, 0x64, 0, 0xc0 + flashFocus)); //SELECTED
					nvgFill(args.vg);
					nvgGlobalCompositeBlendFunc(args.vg,  NVG_ONE , NVG_ONE);
					nvgFillColor(args.vg, nvgRGB(0xdd, 0xdd, 0));
				}break;
				case 2:{
					if ((lrncol += 16) > 255) lrncol = 0;
					nvgBeginPath(args.vg);
					nvgRoundedRect(args.vg, 0.f, 0.f, box.size.x, box.size.y,3.f);
					nvgFillColor(args.vg, nvgRGBA(lrncol, 0, 0, 0xc0));
					nvgFill(args.vg);
					nvgGlobalCompositeBlendFunc(args.vg,  NVG_ONE , NVG_ONE);
					nvgFillColor(args.vg, nvgRGB(0xdd , 0, 0));//LEARN
				}break;
			}
		nvgTextBox(args.vg, 0.f, 10.f,box.size.x, sDisplay.c_str(), NULL);
	}
	void mymodeAction(){
		switch (mymode){
			case 0:{
				focusOn = false;
				module->cursorIx = -1;
				module->learnCC = -1;
				module->autoFocusOff = 0;
			}break;
			case 1:{
				if (canlearn) displayedCC();
				flashFocus = 64;
				focusOn = true;
				module->learnCC = -1;
				module->cursorIx = SuperMIDI64::Y_LCD + displayID;
				module->autoFocusOff = 10 * APP->engine->getSampleRate();
			}break;
			case 2:{
				sDisplay = "LRN";
				focusOn = true;
				module->cursorIx = SuperMIDI64::Y_LCD + displayID;
				module->learnCC = displayID - 6;
				module->autoFocusOff = 10 * APP->engine->getSampleRate();
			}break;
		}
	}
	void displayedCC(){
		switch (ccNumber) {
			case 1 :{
				sDisplay = "Mod";
			}break;
			case 2 :{
				sDisplay = "BrC";
			}break;
			case 4 :{
				sDisplay = "Foot";
			}break;
			case 7 :{
				sDisplay = "Vol";
			}break;
			case 8 :{
				sDisplay = "Bal";
			}break;
			case 10 :{
				sDisplay = "Pan";
			}break;
			case 11 :{
				sDisplay = "Expr";
			}break;
			case 12 :{
				sDisplay = "FX 1";
			}break;
			case 13 :{
				sDisplay = "FX 2";
			}break;
			case 16 :{
				sDisplay = "Ctrl 1";
			}break;
			case 17 :{
				sDisplay = "Ctrl 2";
			}break;
			case 18 :{
				sDisplay = "Ctrl 3";
			}break;
			case 19 :{
				sDisplay = "Ctrl 4";
			}break;
			case 64 :{
				sDisplay = "Sust";
			}break;
			case 66 :{
				sDisplay = "Sost";
			}break;
			case 67 :{
				sDisplay = "Soft";
			}break;
			case 68 :{
				sDisplay = "Lgto";
			}break;
			case 69 :{
				sDisplay = "Hold";
			}break;
			case 70 :{
				sDisplay = "Vari";
			}break;
			case 71 :{
				sDisplay = "Tmbr";
			}break;
			case 72 :{
				sDisplay = "Rel";
			}break;
			case 73 :{
				sDisplay = "Atk";
			}break;
			case 74 :{
				sDisplay = "Brite";
			}break;
			case 80 :{
				sDisplay = "Ctrl 5";
			}break;
			case 81 :{
				sDisplay = "Ctrl 6";
			}break;
			case 82 :{
				sDisplay = "Ctrl 7";
			}break;
			case 83 :{
				sDisplay = "Ctrl 8";
			}break;
			case 128 :{
				sDisplay = "chAT";
			}break;
			case 129 :{
				sDisplay = "noteAT";
			}break;
			case 130 :{
				sDisplay = "Dtn";
			}break;
			case 131 :{//HiRes MPE Y
				sDisplay = "cc74+";
			}break;
			case 132 :{//HiRes MPE Z
				sDisplay = "chAT+";
			}break;
			default :{
				sDisplay = "cc" + std::to_string(ccNumber);
			}
		}
	}
	void onButton(const event::Button &e) override {
		if ((e.button == GLFW_MOUSE_BUTTON_LEFT) && (canedit)){
			//int dispLine = static_cast<int>((e.pos.y - 2.f)/ 13.f) ;
			//bool valpress = (e.pos.x >  box.size.x / 2.f);
			if (e.action == GLFW_PRESS){
				if (mymode < 1) {
					if (canlearn) mymode = 2;
					else mymode = 1;
				}
				else mymode --;
				mymodeAction();
				module->learnNote = -1;
			};
		}
	}
};
///////////////////////////////////////////////////////////////////////////////////////
struct springDataKnobC : SvgKnob {
	springDataKnobC() {
		minAngle = -0.75*M_PI;
		maxAngle = 0.75*M_PI;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DLXdataKnobB.svg")));
		shadow->opacity = 0.f;
	}
	void randomize() override{
	}
	void onDragEnd(const event::DragEnd& e) override{
		this->reset();
		Knob::onDragEnd(e);
	}
};
///////////////////////////////////////////////////////////////////////////////////////
/////// MODULE WIDGET ////////
//////////////////////////////
struct SuperMIDI64Widget : ModuleWidget {
	SuperMIDI64Widget(SuperMIDI64 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance,"res/SuperMIDI64.svg")));
		//Screws
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		int dispID = 0;
		if (module) {
			//MIDI
			MIDIscreen *dDisplay = createWidget<MIDIscreen>(module->coords[SuperMIDI64::MIDI_LCD]);
			dDisplay->box.size = {136.f, 40.f};
			dDisplay->setMidiPort (&module->midiInput, &module->MPEmode, &module->MPEmasterCh, &module->midiActivity, &module->mdriverJx, &module->mdeviceJx, &module->mchannelJx, &module->resetMidi);
			addChild(dDisplay);
			//PolyModes LCD
			PolyModeDisplayC *polyModeDisplay = createWidget<PolyModeDisplayC>(module->coords[SuperMIDI64::POLYMODE_LCD]);
			polyModeDisplay->box.size = {136.f, 40.f};
			polyModeDisplay->module = module;
			addChild(polyModeDisplay);
			//  Y Z LCDs
			for ( int i = 0; i < 2; i++){
				MidiccDisplayC *MccDisplay = createWidget<MidiccDisplayC>(module->coords[SuperMIDI64::Y_LCD + i]);
				MccDisplay->box.size = {40.f, 13.f};
				MccDisplay->displayID =  dispID ++;
				MccDisplay->module = (module ? module : NULL);
				addChild(MccDisplay);
			}
			// RelVel / chPbend LCD
			MidiccDisplayC *rvelDisplay = createWidget<MidiccDisplayC>(module->coords[SuperMIDI64::RELVEL_LCD]);
			rvelDisplay->box.size = {40.f, 13.f};
			rvelDisplay->displayID =  dispID ++;
			rvelDisplay->module = module;
			addChild(rvelDisplay);
			// trnsp LCD
			MidiccDisplayC *MccDisplay = createWidget<MidiccDisplayC>(module->coords[SuperMIDI64::TRNSP_LCD]);
			MccDisplay->box.size = {25.f, 13.f};
			MccDisplay->displayID =  dispID ++;
			MccDisplay->module = module;
			addChild(MccDisplay);
			// Pitch Bend LCD
			for ( int i = 0; i < 2; i++){
				MidiccDisplayC *MccDisplay = createWidget<MidiccDisplayC>(module->coords[SuperMIDI64::PBEND_LCD + i]);
				MccDisplay->box.size = {25.f, 13.f};
				MccDisplay->displayID =  dispID ++;
				MccDisplay->module = module;
				addChild(MccDisplay);
			}
		}
		// ch Leds x 64
		float xPos = 8.052f;
		float yPos = 148.735f;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 32; j++) {
				addChild(createLight<TinyLight<TPurpleLight>>(Vec(xPos + j * 4.349f, yPos + i * 3.862f), module, SuperMIDI64::CH_LIGHT + j + i*32));
			}
		}
		////DATA KNOB + -
		xPos = 59.195f;
		yPos = 108.624f;
		addParam(createParam<springDataKnobC>(Vec(xPos, yPos), module, SuperMIDI64::DATAKNOB_PARAM));
		xPos = 40.104f;
		yPos = 124.568f - 3.265f;
		addParam(createParam<minusButtonB>(Vec(xPos, yPos), module, SuperMIDI64::MINUSONE_PARAM));
		xPos = 103.646f;
		yPos = 121.124;
		addParam(createParam<plusButtonB>(Vec(xPos, yPos), module, SuperMIDI64::PLUSONE_PARAM));
		// X Outs
		xPos = 173.140f;
		yPos = 41.022f;
		for (int i = 0; i < 4; i++)
			addOutput(createOutput<DLXPortPoly>(Vec(xPos + i * 29.201f, yPos),  module, SuperMIDI64::X_OUTPUT + i));
		// Y Outs
		yPos = 84.911f;
		for (int i = 0; i < 4; i++)
			addOutput(createOutput<DLXPortPoly>(Vec(xPos + i * 29.192f, yPos),  module, SuperMIDI64::Y_OUTPUT + i));
		// Z Outs
		yPos = 128.797f;
		for (int i = 0; i < 4; i++)
			addOutput(createOutput<DLXPortPoly>(Vec(xPos + i * 29.192f, yPos),  module, SuperMIDI64::Z_OUTPUT + i));
		//RVel Outs
		yPos = 202.770f;
		for (int i = 0; i < 4; i++)
			addOutput(createOutput<DLXPortPoly>(Vec(xPos + i * 29.192f, yPos),  module, SuperMIDI64::RVEL_OUTPUT + i));
		//Vel Outs
		yPos = 246.661f;
		for (int i = 0; i < 4; i++)
			addOutput(createOutput<DLXPortPoly>(Vec(xPos + i * 29.192f, yPos),  module, SuperMIDI64::VEL_OUTPUT + i));
		//Gate Outs
		yPos = 290.551f;
		for (int i = 0; i < 4; i++)
			addOutput(createOutput<DLXPortPoly>(Vec(xPos + i * 29.192f, yPos),  module, SuperMIDI64::GATE_OUTPUT + i));
		///Sustain hold notes switch
		xPos = 200.708f;
		yPos = 334.864f;
		addParam(createParam<DLXSwitchLed>(Vec(xPos, yPos), module, SuperMIDI64::SUSTHOLD_PARAM));
		if (module) addChild(createLight<TranspOffRedLight>(Vec(xPos, yPos), module, SuperMIDI64::SUSTHOLD_LIGHT));
		//Retrig
		xPos = 222.267f;
		addParam(createParam<DLXSwitchLed>(Vec(xPos, yPos), module, SuperMIDI64::RETRIG_PARAM));
		//Bend Pitch switch
		xPos = 243.825f;
		addParam(createParam<DLXSwitchLed>(Vec(xPos, yPos), module, SuperMIDI64::BENDPITCH_PARAM));
		// PBend Out
		xPos = 273.063f;
		yPos = 159.275f;
		addOutput(createOutput<DLXPortG>(Vec(xPos, yPos), module, SuperMIDI64::PBEND_OUTPUT));
		// CC's x 20
		xPos = 18.570f;
		yPos = 179.275f;
		for ( int r = 0; r < 5; r++){
			for ( int i = 0; i < 4; i++){
				if (module){
					MidiccDisplayC *MccDisplay = createWidget<MidiccDisplayC>(module->coords[SuperMIDI64::CC_LCD + i + 4*r]);
					MccDisplay->box.size = {30.f, 13.f};
					MccDisplay->displayID = dispID ++;
					MccDisplay->module = module;
					addChild(MccDisplay);
				}
				addOutput(createOutput<DLXPortG>(Vec(xPos + i*32.921f, yPos + r*39.98f),  module, SuperMIDI64::MM_OUTPUT + i + r * 4));
			}
		}
	}
};

Model *modelSuperMIDI64 = createModel<SuperMIDI64, SuperMIDI64Widget>("SuperMIDI64");
