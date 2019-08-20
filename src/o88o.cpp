#include "../include/o88o.hpp"
#include "../include/o88oDisplay.hpp"
#include "../include/o88oData.hpp"


void o88o::process_keys()
{
	if(pWidget != NULL)
	{
		if(btninc.process(params[PTN_INC].value))
		{
			curPtn = clamp(curPtn + 1, 0, NUM_PATTERNS - 1);
			pWidget->SetPattern(curPtn);
		} else if(btndec.process(params[PTN_DEC].value))
		{
			curPtn = clamp(curPtn - 1, 0, NUM_PATTERNS - 1);
			pWidget->SetPattern(curPtn);
		}
	}
}

void o88o::process(const ProcessArgs &args)
{
	process_keys();
	getPatternLimits();
	if(masterReset.process(params[M_RESET].value + inputs[RESET_IN].value))
	{
		reset();
	} else
	{
		invert_active = getSwitch(SWITCH_INVERT, SWVERT_IN);
		if(inputs[PATTERN_IN].isConnected())
			curPtn = clamp((int)rescale(inputs[PATTERN_IN].getNormalVoltage(0.0), LVL_OFF, LVL_MAX, 0, NUM_PATTERNS - 1), 0, NUM_PATTERNS - 1);
		else
			curPtn = params[PATTERN].value - 1;

		if(curPtn == 0 && (rndTrigger.process(inputs[RANDOMIZE_IN].value) || rndBtnTrig.process(params[RANDOMIZE].value)))
			randPattrn();
		
		int clk = clockTrigger.process(inputs[CLOCK_IN].value); // 1=rise, -1=fall
		bool gate = (params[GATE].value > 0.5 || inputs[GATE_IN].getNormalVoltage(0.0) >= 1.0);
		if(clk == 1 && gate)
		{
			next_step();
			open_gate();
		} else if(clk == -1)
			close_gate();
	}
}

void o88o::randPattrn()
{
	for(int r = 0; r < NUM_o88o_RECT; r++)
		for(int c = 0; c < NUM_o88o_RECT; c++)
			TheMatrix[0][r][c] = int(random::uniform() * 2);
}

void o88o::getPatternLimits()
{
	int a, b;
	if(inputs[FIRSTROW_IN].isConnected())
		a = clamp((int)roundf(inputs[FIRSTROW_IN].getNormalVoltage(0.0)), 0, NUM_o88o_RECT-1);
	else
		a = params[FIRSTROW].value - 1;
	if(inputs[LASTROW_IN].isConnected())
		b = clamp((int)roundf(inputs[LASTROW_IN].getNormalVoltage(0.0)), 0, NUM_o88o_RECT - 1);
	else
		b = params[LASTROW].value - 1;
	firstRow = std::min(a, b);
	lastRow = std::max(a, b);

	if(inputs[FIRSTCOL_IN].isConnected())
		a = clamp((int)roundf(inputs[FIRSTCOL_IN].getNormalVoltage(0.0)), 0, NUM_o88o_RECT - 1);
	else
		a = params[FIRSTCOL].value - 1;
	if(inputs[LASTCOL_IN].isConnected())
		b = clamp((int)roundf(inputs[LASTCOL_IN].getNormalVoltage(0.0)), 0, NUM_o88o_RECT - 1);
	else
		b = params[LASTCOL].value - 1;
	firstCol = std::min(a, b);
	lastCol = std::max(a, b);
}

void o88o::reset()
{
	bool back = getSwitch(SWITCH_BACKW, SWBACK_IN);
	bool loop = getSwitch(SWITCH_LOOP, SWLOOP_IN);
	if(back)
	{
		curCol = loop ? lastCol : NUM_o88o_RECT - 1;
		curRow = loop ? lastRow : NUM_o88o_RECT - 1;
	} else
	{
		curCol = loop ? firstCol : 0;
		curRow = loop ? firstRow : 0;
	}
	close_gate();
}

void o88o::out_position()
{ 
	outputs[CURROW_OUT].value = curRow;
	outputs[CURCOL_OUT].value = curCol;
}

void o88o::open_gate()
{
	if(getCell(curRow, curCol) && isCellEnabled(curRow, curCol))
	{
		outputs[GATE_OUT].value = LVL_ON;
		params[LED_GATE].value = LVL_ON;
	}
	out_position();
}

void o88o::close_gate()
{
	outputs[GATE_OUT].value = LVL_OFF;
	params[LED_GATE].value = LVL_OFF;
	out_position();
}

void o88o::next_step()
{
	bool loop = getSwitch(SWITCH_LOOP, SWLOOP_IN);
	bool vert = getSwitch(SWITCH_VERT, SWVERT_IN);
	bool back = getSwitch(SWITCH_BACKW, SWBACK_IN);
	if(vert)
		next_row(vert, back, loop);
	else
		next_column(vert, back, loop);
}

void o88o::next_column(bool vert, bool back, bool loop)
{
	if(back)
	{
		curCol--;
		if(curCol < 0 || (loop && curCol < firstCol))
		{
			curCol = loop ? lastCol : NUM_o88o_RECT - 1;
			if(!vert)
				next_row(vert, back, loop);
		}
	} else
	{
		curCol++;
		if(curCol >= NUM_o88o_RECT || (loop && curCol > lastCol))
		{
			curCol = loop ? firstCol : 0;
			if(!vert)
				next_row(vert, back, loop);
		}
	}
}

void o88o::next_row(bool vert, bool back, bool loop)
{
	if(back)
	{
		curRow--;
		if(curRow < 0 || (loop && curRow < firstRow))
		{
			curRow = loop ? lastRow : NUM_o88o_RECT - 1;
			if(vert)
				next_column(vert, back, loop);
		}
	} else
	{
		curRow++;
		if(curRow >= NUM_o88o_RECT || (loop && curRow > lastRow))
		{
			curRow = loop ? firstRow : 0;
			if(vert)
				next_column(vert, back, loop);
		}
	}
}

NVGcolor o88o::getCellColor(int r, int c)
{
	bool cell = getCell(r, c);
	if(r == curRow && c == curCol)
	{
		if(cell)
		{
			if(isCellEnabled(r, c))
				return cellColors[CURRENT];
			else
				return cellColors[CURRENT_DISABLED];

		}
		return cellColors[CURRENT_OFF];

	} else
	{
		if(cell)
		{
			if(isCellEnabled(r, c))
				return cellColors[ENABLED];
			else
				return cellColors[DISABLED];

		}
	}

	return cellColors[OFF];
}

o88oWidget::o88oWidget(o88o *module)
{
	CREATE_PANEL(module, this, 28, "res/modules/o88o.svg");

	if(module != NULL)
	{
		module->setWidget(this);
		o88oDisplay *display = createWidget<o88oDisplay>(mm2px(Vec(20.273f, 11.883f)));
		display->box.size = mm2px(Vec(90.f, 90.f));
		display->SetModule(module);
		addChild(display);
	}
	ParamWidget *pwdg = createParam<BefacoPushBig>(Vec(mm2px(5.988), yncscape(105.884, 8.999)), module, o88o::M_RESET);
	addParam(pwdg);
	#ifdef OSCTEST_MODULE
	if(module != NULL)
	{
		module->oscDrv->Add(new oscControl("/Reset"), pwdg);
	}
	#endif	
	addInput(createInput<PJ301RPort>(Vec(mm2px(6.361), yncscape(77.482, 8.255)), module, o88o::CLOCK_IN));
	addInput(createInput<PJ301BPort>(Vec(mm2px(6.361), yncscape(95.204, 8.255)), module, o88o::RESET_IN));
	addInput(createInput<PJ301BPort>(Vec(mm2px(6.361), yncscape(58.330, 8.255)), module, o88o::GATE_IN));

	addParam(createParam<TL1105HSw>(Vec(mm2px(7.185), yncscape(47.082, 4.477)), module, o88o::GATE));
	
	addInput(createInput<PJ301BPort>(Vec(mm2px(36.642), yncscape(10.017, 8.255)), module, o88o::SWLOOP_IN));
	addInput(createInput<PJ301BPort>(Vec(mm2px(53.383), yncscape(10.017, 8.255)), module, o88o::SWVERT_IN));
	addInput(createInput<PJ301BPort>(Vec(mm2px(70.124), yncscape(10.017, 8.255)), module, o88o::SWBACK_IN));
	addInput(createInput<PJ301BPort>(Vec(mm2px(86.864), yncscape(10.017, 8.255)), module, o88o::SWINVERT_IN));

	addParam(createParam<TL1105Sw>(Vec(mm2px(79.437), yncscape(10.379f, 6.607)), module, o88o::SWITCH_BACKW));
	addParam(createParam<TL1105Sw>(Vec(mm2px(62.697), yncscape(10.379f, 6.607)), module, o88o::SWITCH_VERT));
	addParam(createParam<TL1105Sw>(Vec(mm2px(45.956), yncscape(10.379f, 6.607)), module, o88o::SWITCH_LOOP));
	addParam(createParam<TL1105Sw>(Vec(mm2px(96.178), yncscape(10.379f, 6.607)), module, o88o::SWITCH_INVERT));

	addOutput(createOutput<PJ301WPort>(Vec(mm2px(20.881), yncscape(10.017, 8.255)), module, o88o::GATE_OUT));

	addInput(createInput<PJ301BPort>(Vec(mm2px(128.294), yncscape(106.256, 8.255)), module, o88o::FIRSTROW_IN));
	pwdg = createParam<Davies1900hFixWhiteKnobSmall>(Vec(mm2px(115.900), yncscape(106.383, 8.0)), module, o88o::FIRSTROW);
	((Davies1900hFixRedKnobSmall *)pwdg)->snap = true;
	addParam(pwdg);
	#ifdef OSCTEST_MODULE
	if(module != NULL)
	{
		module->oscDrv->Add(new oscControl("/FirstRow"), pwdg);
	}
	#endif

	addInput(createInput<PJ301BPort>(Vec(mm2px(128.294), yncscape(89.851, 8.255)), module, o88o::LASTROW_IN));
	pwdg = createParam<Davies1900hFixWhiteKnobSmall>(Vec(mm2px(115.900), yncscape(89.979, 8.0)), module, o88o::LASTROW);
	((Davies1900hFixRedKnobSmall *)pwdg)->snap = true;
	addParam(pwdg);
	#ifdef OSCTEST_MODULE
	if(module != NULL)
	{
		module->oscDrv->Add(new oscControl("/LastRow"), pwdg);
	}
	#endif

	addOutput(createOutput<PJ301WPort>(Vec(mm2px(128.294), yncscape(74.820, 8.255)), module, o88o::CURROW_OUT));

	addInput(createInput<PJ301BPort>(Vec(mm2px(128.294), yncscape(58.101, 8.255)), module, o88o::FIRSTCOL_IN));
	pwdg = createParam<Davies1900hFixBlackKnobSmall>(Vec(mm2px(115.900), yncscape(58.229, 8.0)), module, o88o::FIRSTCOL);
	((Davies1900hFixRedKnobSmall *)pwdg)->snap = true;
	addParam(pwdg);
	#ifdef OSCTEST_MODULE
	if(module != NULL)
	{
		module->oscDrv->Add(new oscControl("/FirstCol"), pwdg);
	}
	#endif

	addInput(createInput<PJ301BPort>(Vec(mm2px(128.294), yncscape(41.697, 8.255)), module, o88o::LASTCOL_IN));
	pwdg = createParam<Davies1900hFixBlackKnobSmall>(Vec(mm2px(115.900), yncscape(41.825, 8.0)), module, o88o::LASTCOL);
	((Davies1900hFixRedKnobSmall *)pwdg)->snap = true;
	addParam(pwdg);
	#ifdef OSCTEST_MODULE
	if(module != NULL)
	{
		module->oscDrv->Add(new oscControl("/LastCol"), pwdg);
	}
	#endif

	addOutput(createOutput<PJ301WPort>(Vec(mm2px(128.294), yncscape(26.666, 8.255)), module, o88o::CURCOL_OUT));

	addInput(createInput<PJ301BPort>(Vec(mm2px(128.294), yncscape(10.017, 8.255)), module, o88o::PATTERN_IN));
	pwdg = createParam<Davies1900hFixRedKnobSmall>(Vec(mm2px(115.900), yncscape(10.144, 8.0)), module, o88o::PATTERN);
	((Davies1900hFixRedKnobSmall *)pwdg)->snap = true;
	addParam(pwdg);
	#ifdef OSCTEST_MODULE
	if(module != NULL)
	{
		module->oscDrv->Add(new oscControl("/Pattern"), pwdg);
	}
	#endif

	addChild(new o88o7Segm(module != NULL ? module : NULL, 103.832, 10.320));
	addParam(createParam<RIGHTSWITCH>(Vec(mm2px(109.255), yncscape(4.358, 4.627)), module, o88o::PTN_INC));
	addParam(createParam<LEFTSWITCH>(Vec(mm2px(103.832), yncscape(4.358, 4.627)), module, o88o::PTN_DEC));
	addParam(createParam<HiddenButton>(Vec(mm2px(0), yncscape(0, 5.08)), module, o88o::RANDOMIZE));
	addInput(createInput<PJ301HPort>(Vec(mm2px(6.361), yncscape(29.018, 8.255)), module, o88o::RANDOMIZE_IN));
	addParam(createParam<TheXORBtn>(Vec(mm2px(6.489), yncscape(10.144, 8)), module, o88o::LED_GATE));
}

void o88oWidget::SetPattern(int ptn)
{
	int index = getParamIndex(o88o::PATTERN);
	if(index >= 0)
		params[index]->paramQuantity->setValue(ptn+1);
}
