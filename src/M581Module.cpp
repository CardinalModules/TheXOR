#include "M581.hpp"
#include <sstream>

void M581::on_loaded()
{
	#ifdef DIGITAL_EXT
	connected = 0;
	#endif
	load();
}

void M581::load()
{
	stepCounter.Set(&getter);
	cvControl.Set(&getter);
	gateControl.Set(&getter);
	getter.Set(this);
	_reset();
}

void M581::_reset()
{
	cvControl.Reset();
	gateControl.Reset();
	stepCounter.Reset(&Timer);
	showCurStep(0, 0);
}

void M581::randrandrand()
{
	if (theRandomizer & M581Widget::RANDOMIZE_PITCH)
		randrandrand(0);

	if(theRandomizer & M581Widget::RANDOMIZE_COUNTER)
		randrandrand(1);

	if (theRandomizer & M581Widget::RANDOMIZE_MODE)
		randrandrand(2);

	if (theRandomizer & M581Widget::RANDOMIZE_ENABLE)
		randrandrand(3);

	if (theRandomizer & M581Widget::RANDOMIZE_LAQUALUNQUE)
	{
		randrandrand(int(random::uniform() * 4));
	}
}

void M581::randrandrand(int action)
{
	switch (action)
	{	
		case 0:
			pWidget->std_randomize(M581::STEP_NOTES, M581::STEP_NOTES + 8); 
			break;

		case 1:
			pWidget->std_randomize(M581::COUNTER_SWITCH, M581::COUNTER_SWITCH + 8);
			break;

		case 2:
			pWidget->std_randomize(M581::GATE_SWITCH, M581::GATE_SWITCH + 8);
			break;

		case 3:
			pWidget->std_randomize(M581::STEP_ENABLE, M581::STEP_ENABLE + 8); 
			break;
	}
}

void M581::process(const ProcessArgs &args)
{
	if(resetTrigger.process(inputs[RESET].value) || masterReset.process(params[M_RESET].value))
	{
		_reset();
	} else
	{
		if(pWidget != NULL && rndTrigger.process(inputs[RANDOMIZONE].value))
			randrandrand();

		Timer.Step();

		if(clockTrigger.process(inputs[CLOCK].value) && any())
			beginNewStep();

		outputs[CV].value = cvControl.Play(Timer.Elapsed());
		outputs[GATE].value = gateControl.Play(&Timer, stepCounter.PulseCounter());
	}

	#ifdef DIGITAL_EXT
	bool dig_connected = false;

	#ifdef LAUNCHPAD
	if(drv->Connected())
		dig_connected = true;
	drv->ProcessLaunchpad();
	#endif

	#if defined(OSCTEST_MODULE)
	if(oscDrv->Connected())
		dig_connected = true;
	oscDrv->ProcessOSC();
	#endif	
	connected = dig_connected ? 1.0 : 0.0;
	#endif
}

void M581::beginNewStep()
{
	int cur_step;
	if(stepCounter.Play(&Timer, &cur_step)) // inizia un nuovo step?
	{
		gateControl.Begin(cur_step);
		cvControl.Begin(cur_step);	// 	glide note increment in 1/10 di msec. param = new note value
	}

	showCurStep(cur_step, stepCounter.PulseCounter());
}

void M581::showCurStep(int cur_step, int sub_div)
{
	int lled = cur_step;
	int sled = sub_div;
	for(int k = 0; k < 8; k++)
	{
		lights[LED_STEP + k].value = k == lled ? LED_ON : LED_OFF;
		lights[LED_SUBDIV + k].value = k == sled ? LED_ON : LED_OFF;
	}
}

bool M581::any()
{
	for(int k = 0; k < 8; k++)
	{
		if(getter.IsEnabled(k))  // step on?
			return true;
	}

	return false;
}

M581Widget::M581Widget(M581 *module) : SequencerWidget(module)
{
	#ifdef OSCTEST_MODULE
	char name[60];
	#endif
	if(module != NULL)
		module->setWidget(this);

	box.size = Vec(29 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	SvgPanel *panel = new SvgPanel();
	panel->box.size = box.size;
	panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/modules/M581Module.svg")));
	addChild(panel);
	addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2*RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));
	addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2*RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));

	float dist_h = 11.893;
	for(int k = 0; k < 8; k++)
	{
		// page #0 (Session): step enable/disable; gate mode
			  // step enable
		ParamWidget *pwdg = createParam<CKSSThreeFix>(Vec(mm2px(14.151+k*dist_h), yncscape(11.744,10.0)), module, M581::STEP_ENABLE + k);
		addParam(pwdg);
		#ifdef LAUNCHPAD
		if(module != NULL)
		{
			module->drv->Add(new LaunchpadRadio(0, ILaunchpadPro::RC2Key(5, k), 3, LaunchpadLed::Color(43), LaunchpadLed::Color(32)), pwdg);
		}
		#endif
		#ifdef OSCTEST_MODULE
		if(module != NULL)
		{
			sprintf(name, "/Enable%i", k + 1);
			module->oscDrv->Add(new oscControl(name), pwdg);
		}
		#endif

		// Gate switches
		pwdg = createParam<VerticalSwitch>(Vec(mm2px(14.930 + k*dist_h), yncscape(39.306, 13.2)), module, M581::GATE_SWITCH + k);
		addParam(pwdg);
		#ifdef LAUNCHPAD
		if(module != NULL)
		{
			module->drv->Add(new LaunchpadRadio(0, ILaunchpadPro::RC2Key(1, k), 4, LaunchpadLed::Color(11), LaunchpadLed::Color(17)), pwdg);
		}
		#endif
		#ifdef OSCTEST_MODULE
		if(module != NULL)
		{
			sprintf(name, "/GateMode%i", k + 1);
			module->oscDrv->Add(new oscControl(name), pwdg);
		}
		#endif

		// page #1 (Note): Notes
		// step notes
		pwdg = createParam<BefacoSlidePotFix>(Vec(mm2px(14.943 + k*dist_h), yncscape(95.822, 27.517)), module, M581::STEP_NOTES + k);
		addParam(pwdg);
	
		#ifdef OSCTEST_MODULE
		if(module != NULL)
		{
			sprintf(name, "/Knob%i", k + 1);
			module->oscDrv->Add(new oscControl(name), pwdg);
		}
		#endif

		//page #2 (Device): Counters
		// Counter switches
		pwdg = createParam<CounterSwitch>(Vec(mm2px(14.93 + k*dist_h), yncscape(60.897, 24.0)), module, M581::COUNTER_SWITCH + k);
		addParam(pwdg);
		#ifdef LAUNCHPAD
		if(module != NULL)
		{
			module->drv->Add(new LaunchpadRadio(1, ILaunchpadPro::RC2Key(0, k), 8, LaunchpadLed::Color(1), LaunchpadLed::Color(58)), pwdg);
		}
		#endif
		#ifdef OSCTEST_MODULE
		if(module != NULL)
		{
			sprintf(name, "/Count%i", k + 1);
			module->oscDrv->Add(new oscControl(name), pwdg);
		}
		#endif

		// step leds (all pages)
		ModuleLightWidget *plight = createLight<LargeLight<RedLight>>(Vec(mm2px(13.491 + k*dist_h), yncscape(27.412, 5.179)), module, M581::LED_STEP + k);
		addChild(plight);
		#ifdef LAUNCHPAD
		if(module != NULL)
		{
			LaunchpadLight *ld1 = new LaunchpadLight(launchpadDriver::ALL_PAGES, ILaunchpadPro::RC2Key(0, k), LaunchpadLed::Off(), LaunchpadLed::Color(5));
			module->drv->Add(ld1, plight);
		}
		#endif
		#ifdef OSCTEST_MODULE
		if(module != NULL)
		{
			sprintf(name, "/Led%i", k + 1);
			module->oscDrv->Add(new oscControl(name), plight);
		}
		#endif

		// subdiv leds (all pages)
		const float dv = 3.029;
		plight = createLight<TinyLight<RedLight>>(Vec(mm2px(11.642), yncscape(61.75+k*dv+0.272, 1.088)), module, M581::LED_SUBDIV + k);
		addChild(plight);
		#ifdef LAUNCHPAD
		if(module != NULL)
		{
			// colonna PLAY
			module->drv->Add(new LaunchpadLight(launchpadDriver::ALL_PAGES, ILaunchpadPro::RC2Key(8, k), LaunchpadLed::Off(), LaunchpadLed::Color(5)), plight);
		}
		#endif
		#ifdef OSCTEST_MODULE
		if(module != NULL)
		{
			sprintf(name, "/SubLed%i", k + 1);
			module->oscDrv->Add(new oscControl(name), plight);
		}
		#endif
	}

	// Gate time
	ParamWidget *pwdg = createParam<Davies1900hFixWhiteKnob>(Vec(mm2px(121.032), yncscape(112.942, 9.525)), module, M581::GATE_TIME);
	#ifdef OSCTEST_MODULE
	if(module != NULL)
	{
		module->oscDrv->Add(new oscControl("/GateTime"), pwdg);
	}
	#endif
	addParam(pwdg);    // in sec

	// Slide time
	pwdg = createParam<Davies1900hFixWhiteKnob>(Vec(mm2px(121.032), yncscape(95.480, 9.525)), module, M581::SLIDE_TIME);
	addParam(pwdg); // in sec
	#ifdef OSCTEST_MODULE
	if(module != NULL)
	{
		module->oscDrv->Add(new oscControl("/SlideTime"), pwdg);
	}
	#endif

	// volt fondo scala
	pwdg = createParam<CKSSFix>(Vec(mm2px(5.489), yncscape(114.224, 5.460)), module, M581::MAXVOLTS);
	addParam(pwdg);
	#ifdef LAUNCHPAD
	if (module != NULL)
	{
		module->drv->Add(new LaunchpadRadio(2, ILaunchpadPro::RC2Key(6, 0), 2, LaunchpadLed::Color(43), LaunchpadLed::Color(32)), pwdg);
	}
	#endif

	#ifdef OSCTEST_MODULE
	if(module != NULL)
	{
		module->oscDrv->Add(new oscControl("/Voltage"), pwdg);
	}
	#endif

	// step div
	pwdg = createParam<VerticalSwitch>(Vec(mm2px(123.494), yncscape(75.482, 13.2)), module, M581::STEP_DIV);
	addParam(pwdg);
	#ifdef LAUNCHPAD
	if (module != NULL)
	{
		module->drv->Add(new LaunchpadRadio(2, ILaunchpadPro::RC2Key(4, 3), 4, LaunchpadLed::Color(43), LaunchpadLed::Color(32)), pwdg);
	}
	#endif
	#ifdef OSCTEST_MODULE
	if(module != NULL)
	{
		module->oscDrv->Add(new oscControl("/StepDiv"), pwdg);
	}
	#endif

	// input
	addInput(createInput<PJ301RPort>(Vec(mm2px(113.864), yncscape(22.128, 8.255)), module, M581::CLOCK));
	addInput(createInput<PJ301YPort>(Vec(mm2px(124.178), yncscape(22.128, 8.255)), module, M581::RESET));
	addChild(createParam<BefacoPushBig>(Vec(mm2px(134.427), yncscape(21.756, 8.999)), module, M581::M_RESET));

	addInput(createInput<PJ301HPort>(Vec(mm2px(106.516), yncscape(114.960, 8.255)), module, M581::RANDOMIZONE));

	// OUTPUTS
	addOutput(createOutput<PJ301GPort>(Vec(mm2px(113.864), yncscape(7.228, 8.255)), module, M581::CV));
	addOutput(createOutput<PJ301WPort>(Vec(mm2px(129.469), yncscape(7.228, 8.255)), module, M581::GATE));

	// # STEPS
	SigDisplayWidget *display2 = new SigDisplayWidget(2);
	display2->box.pos = Vec(mm2px(127.229), yncscape(37.851, 9.525));
	display2->box.size = Vec(30, 20);
	if(module != NULL)
	{
		display2->value = module->getAddress(1);
	}
	addChild(display2);
	pwdg = createParam<Davies1900hFixRedKnob>(Vec(mm2px(113.229), yncscape(38.851, 9.525)), module, M581::NUM_STEPS);
	((Davies1900hKnob *)pwdg)->snap = true;
	addParam(pwdg);

	// run mode
	RunModeDisplay *display = new RunModeDisplay();
	display->box.pos = Vec(mm2px(127.229), yncscape(57.259, 9.525));
	display->box.size = Vec(42, 20);
	if(module != NULL)
	{
		display->mode = module->getAddress(0);
	}
	addChild(display);
	pwdg = createParam<Davies1900hFixWhiteKnob>(Vec(mm2px(113.229), yncscape(58.259,9.525)), module, M581::RUN_MODE);
	((Davies1900hKnob *)pwdg)->snap = true;
	addParam(pwdg);
	#ifdef LAUNCHPAD
	if (module != NULL)
	{
		module->drv->Add(new LaunchpadRadio(2, ILaunchpadPro::RC2Key(6, 1), 5, LaunchpadLed::Color(11), LaunchpadLed::Color(14)), pwdg);
	}
	#endif
	#ifdef OSCTEST_MODULE
	if(module != NULL)
	{
		module->oscDrv->Add(new oscControl("/RunMode"), pwdg);
	}
	#endif

	#ifdef DIGITAL_EXT
	if(module != NULL)
		addChild(new DigitalLed(mm2px(92.540), yncscape(2.322,3.867), &module->connected));
	#endif
}

Menu *M581Widget::addContextMenu(Menu *menu)
{
	menu->addChild(new RandomizeItem(module));

	menu->addChild(new SeqMenuItem<M581Widget>("Randomize Pitch", this, RANDOMIZE_PITCH));
	menu->addChild(new SeqMenuItem<M581Widget>("Randomize Counters", this, RANDOMIZE_COUNTER));
	menu->addChild(new SeqMenuItem<M581Widget>("Randomize Modes", this, RANDOMIZE_MODE));
	menu->addChild(new SeqMenuItem<M581Widget>("Randomize Enable/Slide", this, RANDOMIZE_ENABLE));
	return menu;
}

void M581Widget::onMenu(int action)
{
	switch(action)
	{
	case RANDOMIZE_COUNTER: std_randomize(M581::COUNTER_SWITCH, M581::COUNTER_SWITCH + 8); break;
	case RANDOMIZE_PITCH: std_randomize(M581::STEP_NOTES, M581::STEP_NOTES + 8); break;
	case RANDOMIZE_MODE: std_randomize(M581::GATE_SWITCH, M581::GATE_SWITCH + 8); break;
	case RANDOMIZE_ENABLE: std_randomize(M581::STEP_ENABLE, M581::STEP_ENABLE + 8); break;
	}
}

M581Widget::RandomizeSubItemItem::RandomizeSubItemItem(Module *m, const char *title, int action)
{
	md = (M581 *)m;
	text = title;
	randomizeDest = action;
	rightText = CHECKMARK((md->theRandomizer & randomizeDest) != 0);
}

void M581Widget::RandomizeSubItemItem::onAction(const event::Action &e)
{
	md->theRandomizer ^= randomizeDest;
}

bool ParamGetter::IsEnabled(int numstep) { return pModule->params[M581::STEP_ENABLE + numstep].value > 0.0; }
bool ParamGetter::IsSlide(int numstep) { return pModule->params[M581::STEP_ENABLE + numstep].value > 1.0; }
int ParamGetter::GateMode(int numstep) { return std::round(pModule->params[M581::GATE_SWITCH + numstep].value); }
int ParamGetter::PulseCount(int numstep) { return std::round(pModule->params[M581::COUNTER_SWITCH + numstep].value); }
float ParamGetter::Note(int numstep) { return clamp(pModule->params[M581::STEP_NOTES + numstep].value * (pModule->params[M581::MAXVOLTS].value > 0 ? LVL_ON : LVL_ON/2), LVL_OFF, LVL_ON); }
int ParamGetter::RunMode() { return std::round(pModule->params[M581::RUN_MODE].value); }
int ParamGetter::NumSteps() { return std::round(pModule->params[M581::NUM_STEPS].value); }
float ParamGetter::SlideTime() { return pModule->params[M581::SLIDE_TIME].value; }
float ParamGetter::GateTime() { return pModule->params[M581::GATE_TIME].value; }
int ParamGetter::StepDivision() { return std::round(pModule->params[M581::STEP_DIV].value) + 1; }
