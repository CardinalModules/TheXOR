#include "common.hpp"
#include "demplex.hpp"

void Dmplex::on_loaded()
{
	load();
}

void Dmplex::load()
{
	set_output(0);
}

void Dmplex::set_output(int n)
{
	cur_sel = n;
	for(int k = 0; k < NUM_DEMULTIPLEX_OUTPUTS; k++)
	{
		lights[LED_1 + k].value = k == cur_sel ? LVL_ON : LVL_OFF;
	}
}

void Dmplex::process_keys()
{
	if(outInc.process(params[OUTPUT_INC].value))
	{
		int n = roundf(num_outputs_f);
		if(n < NUM_DEMULTIPLEX_OUTPUTS)
		{
			n += 1;
			num_outputs_f = n;
		}
	} else if(outDec.process(params[OUTPUT_DEC].value))
	{
		int n = roundf(num_outputs_f);
		if(n > 1)
		{
			n -= 1;
			num_outputs_f = n;
		}
	}
}

void Dmplex::process(const ProcessArgs &args)
{
	process_keys();
	int num_outputs = roundf(num_outputs_f);

	if(reset.process(inputs[RESET].value))
	{
		set_output( 0);
	} else if(random.process(inputs[RANDOM].value))
	{
		set_output(getRand(num_outputs_f));
	} else if(upTrigger.process(params[BTDN].value + inputs[INDN].value))
	{
		if(++cur_sel >= num_outputs)
			cur_sel = 0;
		set_output(cur_sel);
	} else if(dnTrigger.process(params[BTUP].value + inputs[INUP].value))
	{
		if(--cur_sel < 0)
			cur_sel = num_outputs-1;
		set_output(cur_sel);
	}

	outputs[OUT_1+cur_sel].value = inputs[IN_1].value;
}

DmplexWidget::DmplexWidget(Dmplex *module) : ModuleWidget()
{
	setModule(module);

	box.size = Vec(10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SvgPanel *panel = new SvgPanel();
		panel->box.size = box.size;
		panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/modules/dmplex.svg")));		
		addChild(panel);
	}

	addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addParam(createParam<BefacoPushBig>(Vec(mm2px(15.267), yncscape(85.436, 8.999)), module, Dmplex::BTUP));
	addParam(createParam<BefacoPushBig>(Vec(mm2px(15.267), yncscape(33.452, 8.999)), module, Dmplex::BTDN));
	addInput(createInput<PJ301BPort>(Vec(mm2px(15.640), yncscape(71.230, 8.255)), module, Dmplex::INUP));
	addInput(createInput<PJ301BPort>(Vec(mm2px(15.640), yncscape(49.014, 8.255)), module, Dmplex::INDN));
	
	addInput(createInput<PJ301GRPort>(Vec(mm2px(3.558), yncscape(60.122, 8.255)), module, Dmplex::IN_1));

	addInput(createInput<PJ301YPort>(Vec(mm2px(3.558), yncscape(92.529,8.255)), module, Dmplex::RESET));
	addInput(createInput<PJ301BPort>(Vec(mm2px(3.558), yncscape(27.716, 8.255)), module, Dmplex::RANDOM));

	addParam(createParam<UPSWITCH>(Vec(mm2px(3.127), yncscape(10.727,4.115)), module, Dmplex::OUTPUT_INC));
	addParam(createParam<DNSWITCH>(Vec(mm2px(3.127), yncscape(5.926, 4.115)), module, Dmplex::OUTPUT_DEC));

	SigDisplayWidget *display = new SigDisplayWidget(1, 0);
	display->box.size = Vec(15, 22);
	display->box.pos = Vec(mm2px(8.495), yncscape(6.439, px2mm(display->box.size.y)));
	if(module != NULL)
		display->value = &module->num_outputs_f;
	addChild(display);

	float y = 105.068;
	float x = 40.045;
	float led_x = 36.110;
	float y_offs = y - 108.108;
	float delta_y = 92.529 - 105.068;
	for(int k = 0; k < NUM_DEMULTIPLEX_OUTPUTS; k++)
	{
		addOutput(createOutput<PJ301GPort>(Vec(mm2px(x), yncscape(y, 8.255)), module, Dmplex::OUT_1 + k));
		addChild(createLight<SmallLight<RedLight>>(Vec(mm2px(led_x), yncscape(y-y_offs, 2.176)), module, Dmplex::LED_1 + k));
		y += delta_y;
		if(k == 3)
			y -= 2.117;
	}
}
