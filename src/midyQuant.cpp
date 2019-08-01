#include "common.hpp"
#include "quantizer.hpp"

void midyQuant::process(const ProcessArgs &args)
{
	if(resetTrigger.process(inputs[RESET].value))
	{
		midiOutput.reset();
	} else if(inputs[CV].isConnected())
	{
		int clk = gate.process(inputs[GATE].value); // 1=rise, -1=fall
		if(clk != 0)
		{
			float v = inputs[CV].getVoltage();
			int vel = (int)rescale(inputs[VEL].getNormalVoltage(0.5), 0.0, 1.0, 0, 127);
			int octave = int(v);	// 1v/octave
			float notef = quantize(v, octave) + octave;
			int note = clamp(std::round(notef * 12.0 + 60.0), 0, 127);
			midiOutput.sendNote(clk == 1, note, vel);
		}
	}
}

void midyQuant::on_loaded()
{
	load();
	calcScale();
}

Menu *midyQuantWidget::addContextMenu(Menu *menu)
{
	menu->addChild(new SeqMenuItem<midyQuantWidget>("MIDI Panic", this, MIDIPANIC));
	return menu;
}

void midyQuantWidget::onMenu(int action)
{
	switch(action)
	{
	case MIDIPANIC: ((midyQuant *)module)->midiOutput.panic();
	break;
	}
}


midyQuantWidget::midyQuantWidget(midyQuant *module) : ModuleWidget()
{
	display = NULL;
	midiDisplay = NULL;
	if(module != NULL)
		module->setWidget(this);

	setModule(module);
	box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	{
		SvgPanel *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/modules/midyQuant.svg")));
		addChild(panel);
	}

	addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	display = createWidget<qtzrDisplay>(mm2px(Vec(3.41891, 14.0)));
	display->box.size = mm2px(Vec(33.840, 19));
	if(module != NULL)
		display->CreateInterface(module);
	addChild(display);
	
	midiDisplay = createWidget<qtzrMidiDisplay>(mm2px(Vec(3.41891, 38.00)));
	midiDisplay->box.size = mm2px(Vec(33.840, 28));
	if(module != NULL)
		midiDisplay->CreateInterface(module);
	addChild(midiDisplay);

	addInput(createInput<PJ301GPort>(Vec(mm2px(2.669), yncscape(9.432, 8.255)), module, midyQuant::CV));
	addInput(createInput<PJ301GRPort>(Vec(mm2px(15.928), yncscape(9.432, 8.255)), module, midyQuant::GATE));
	addInput(createInput<PJ301BPort>(Vec(mm2px(29.686), yncscape(9.432, 8.255)), module, midyQuant::VEL));
	addInput(createInput<PJ301YPort>(Vec(mm2px(15.928), yncscape(29.540, 8.255)), module, midyQuant::RESET));
}

