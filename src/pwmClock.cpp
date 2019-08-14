#include "../include/pwmClock.hpp"

void PwmClock::on_loaded()
{
	bpm = 0;
	swing = 0;
	_reset();
	load();
}

void PwmClock::_reset()
{
	for(int k = 0; k < OUT_SOCKETS; k++)
	{
		sa_timer[k].Reset();
		odd_beat[k] = false;
	}
	midiClock.reset();
}

void PwmClock::load()
{
	updateBpm(false);
}

void PwmClock::updateBpm(bool externalMidiClock)
{
	bool updated = false;
	float new_bpm;
	if(externalMidiClock)
	{
		new_bpm = midiClock.getBpm(inputs[MIDI_CLOCK].value);
	} else
	{
		if(inputs[EXT_BPM].isConnected())
			new_bpm = rescale(inputs[EXT_BPM].value, LVL_OFF, LVL_ON, BPM_MINVALUE, BPM_MAXVALUE);
		else
			new_bpm = (roundf(params[BPMDEC].value) + 10 * bpm_integer) / 10.0;
	}

	if(bpm != new_bpm)
	{
		updated = true;
		bpm = new_bpm;
		duration[0] = 240.0 / bpm;	// 1/1
		duration[1] = duration[0] + duration[0] / 2.0;
		duration[2] = 2.0 * duration[0] / 3.0;

		for(int k = 1; k < 7; k++)
		{
			duration[3 * k] = duration[3 * (k - 1)] / 2.0;
			duration[3 * k + 1] = duration[3 * (k - 1) + 1] / 2.0;
			duration[3 * k + 2] = duration[3 * (k - 1) + 2] / 2.0;
		}
	}
	float new_swing = getSwing();
	if(updated || new_swing != swing)
	{
		swing = new_swing;
		for(int k = 0; k < OUT_SOCKETS; k++)
			swingAmt[k] = duration[k] + duration[k] * swing;
	}
}

void PwmClock::process_keys()
{
	if(btnup.process(params[BPM_INC].value))
	{
		if(bpm_integer < BPM_MAXVALUE)
		{
			bpm_integer += 1;
			pWidget->SetBpm(bpm_integer);
		}
	} else if(btndwn.process(params[BPM_DEC].value))
	{
		if(bpm_integer > BPM_MINVALUE)
		{
			bpm_integer -= 1;
			pWidget->SetBpm(bpm_integer);
		}
	}
}

void PwmClock::process_active(const ProcessArgs &args)
{
	onStopPulse.reset();
	onManualStep.reset();
	lights[ACTIVE].value = LVL_ON;
	if(resetTrigger.process(inputs[RESET].value))
	{
		_reset();
	} else
	{
		for(int k = 0; k < OUT_SOCKETS; k++)
		{
			if(outputs[OUT_1 + k].isConnected())
			{
				float gate_len = getDuration(k) * getPwm();
				sa_timer[k].Step();
				float elps = sa_timer[k].Elapsed();
				if(elps >= getDuration(k))
				{
					elps = sa_timer[k].Reset();
					odd_beat[k] = !odd_beat[k];
				}
				if(elps <= gate_len)
					outputs[OUT_1 + k].value = LVL_ON;
				else
					outputs[OUT_1 + k].value = LVL_OFF;
			}
		}
	}
}

void PwmClock::process_inactive(const ProcessArgs &args)
{
	float deltaTime = 1.0 / args.sampleRate;

	// il led on e' usato per svagare la transizione on -> off
	if(lights[ACTIVE].value == LED_ON && !onStopPulse.process(deltaTime))
		onStopPulse.trigger(pulseTime);

	if((manualTrigger.process(params[PULSE].value) || pulseTrigger.process(inputs[PULSE_IN].value)) && !onManualStep.process(deltaTime))
		onManualStep.trigger(pulseTime);

	outputs[ONSTOP].value = onStopPulse.process(deltaTime) ? LVL_ON : LVL_OFF;

	for(int k = 0; k < OUT_SOCKETS; k++)
		outputs[OUT_1 + k].value = onManualStep.process(deltaTime) ? LVL_ON : LVL_OFF;

	lights[ACTIVE].value = onManualStep.process(deltaTime) ? LED_ON : LED_OFF;
}

bool PwmClock::isGeneratorActive()
{
	bool active = false;
	if(inputs[REMOTE_IN].isConnected()) // priorita; prioritaria
	{
		active = inputs[REMOTE_IN].getNormalVoltage(0.0) > 0.5;
		if(active && (params[OFFON].value < 0.5))
		{
			pWidget->params[OFFON]->dirtyValue = params[OFFON].value = 1.0;
		} else if(!active && (params[OFFON].value > 0.5))
		{
			pWidget->params[OFFON]->dirtyValue = params[OFFON].value = 0.0;
		}

	} else if(offTrigger.process(inputs[MIDI_STOP].value))
	{
		pWidget->params[OFFON]->dirtyValue = params[OFFON].value = 0.0;
		active = false;
	} else if(onTrigger.process(inputs[MIDI_START].value + inputs[MIDI_CONTINUE].value))
	{
		pWidget->params[OFFON]->dirtyValue = params[OFFON].value = 1.0;
		active = true;
	} else
		active = params[OFFON].value > 0.5;

	return active;
}

void PwmClock::process(const ProcessArgs &args)
{
	if(pWidget == NULL)
		return;

	bool active = isGeneratorActive();
	bool externalMidiClock = inputs[MIDI_CLOCK].isConnected();
	if(!externalMidiClock)
	{
		process_keys();
		bpm_integer = roundf(params[BPM].value);
	}

	updateBpm(externalMidiClock);

	if(active)
	{
		process_active(args);
	} else
	{
		process_inactive(args);
	}
}

float PwmClock::getPwm()
{
	float offs = inputs[PWM_IN].isConnected() ? rescale(inputs[PWM_IN].value, LVL_OFF, LVL_ON, PWM_MINVALUE, PWM_MAXVALUE) : 0.0;
	return clamp(offs + params[PWM].value, PWM_MINVALUE, PWM_MAXVALUE);
}

float PwmClock::getSwing()
{
	float offs = inputs[SWING_IN].isConnected() ? rescale(inputs[SWING_IN].value, LVL_OFF, LVL_ON, SWING_MINVALUE, SWING_MAXVALUE) : 0.0;
	return clamp(offs + params[SWING].value, SWING_MINVALUE, SWING_MAXVALUE);
}

PwmClockWidget::PwmClockWidget(PwmClock *module) : SequencerWidget()
{
	if(module != NULL)
		module->setWidget(this);

	CREATE_PANEL(module, this, 15, "res/modules/PwmClock.svg");

	addParam(createParam<UPSWITCH>(Vec(mm2px(14.452), yncscape(104.588 + 4.762, 4.115)), module, PwmClock::BPM_INC));
	addParam(createParam<DNSWITCH>(Vec(mm2px(14.452), yncscape(99.788 + 4.762, 4.115)), module, PwmClock::BPM_DEC));

	SigDisplayWidget *display = new SigDisplayWidget(4, 1);
	display->box.pos = Vec(mm2px(22), RACK_GRID_HEIGHT - mm2px(108 + 4.762));
	display->box.size = Vec(30 + 43, 20);
	if(module != NULL)
		display->value = &module->bpm;
	addChild(display);

	addChild(createParam<BefacoPushBig>(Vec(mm2px(2.937), yncscape(109.841, 8.999)), module, PwmClock::PULSE));
	addInput(createInput<PJ301BPort>(Vec(mm2px(3.309), yncscape(99.175, 8.255)), module, PwmClock::PULSE_IN));

	ParamWidget *pw = createParam<Davies1900hFixWhiteKnobSmall>(Vec(mm2px(50.364), yncscape(100.245 + 4.762, 8)), module, PwmClock::BPMDEC);
	((Davies1900hKnob *)pw)->snap = true;
	addParam(pw);
	pw = createParam<Davies1900hFixWhiteKnob>(Vec(mm2px(62.528), yncscape(99.483 + 4.762, 9.525)), module, PwmClock::BPM);
	((Davies1900hKnob *)pw)->snap = true;
	addParam(pw);
	addInput(createInput<PJ301BPort>(Vec(mm2px(63.162f), yncscape(94.395, 8.255)), module, PwmClock::EXT_BPM));
	addInput(createInput<PJ301YPort>(Vec(mm2px(35.392f), yncscape(86.857, 8.255)), module, PwmClock::RESET));

	addParam(createParam<NKK1>(Vec(mm2px(7.769), yncscape(87.34, 9.488)), module, PwmClock::OFFON));
	addChild(createLight<SmallLight<RedLight>>(Vec(mm2px(3.539), yncscape(89.897, 2.176)), module, PwmClock::ACTIVE));
	addInput(createInput<PJ301BPort>(Vec(mm2px(21.633), yncscape(86.857, 8.255)), module, PwmClock::REMOTE_IN));

	addInput(createInput<PJ301BPort>(Vec(mm2px(49.145), yncscape(72.372, 8.255)), module, PwmClock::MIDI_CLOCK));
	addInput(createInput<PJ301BPort>(Vec(mm2px(63.162), yncscape(72.372, 8.255)), module, PwmClock::MIDI_START));
	addInput(createInput<PJ301BPort>(Vec(mm2px(49.145), yncscape(59.672, 8.255)), module, PwmClock::MIDI_STOP));
	addInput(createInput<PJ301BPort>(Vec(mm2px(63.162), yncscape(59.672, 8.255)), module, PwmClock::MIDI_CONTINUE));

	addParam(createParam<Davies1900hFixRedKnob>(Vec(mm2px(48.511), yncscape(33.782, 9.525)), module, PwmClock::SWING));
	addInput(createInput<PJ301BPort>(Vec(mm2px(63.162), yncscape(34.417, 8.255)), module, PwmClock::SWING_IN));

	addParam(createParam<Davies1900hFixBlackKnob>(Vec(mm2px(48.511), yncscape(17.603, 9.525)), module, PwmClock::PWM));
	addInput(createInput<PJ301BPort>(Vec(mm2px(63.162), yncscape(18.238, 8.255)), module, PwmClock::PWM_IN));

	float col_x[3] = {7.875, 21.633, 35.392};
	float pos_y = yncscape(70.175, 8.255);
	int col = 0;
	for(int k = 0; k < OUT_SOCKETS; k++)
	{
		addOutput(createOutput<PJ301OPort>(Vec(mm2px(col_x[col]), pos_y), module, PwmClock::OUT_1 + k));
		if(++col >= 3)
		{
			col = 0;
			pos_y += mm2px(11);
		}
	}
	addOutput(createOutput<PJ301BLUPort>(Vec(mm2px(49.145), yncscape(4.175, 8.255)), module, PwmClock::ONSTOP));
}

void PwmClockWidget::SetBpm(float bpm_integer)
{
	int index = getParamIndex(PwmClock::BPM);
	if(index >= 0)
	{
		// VCV interface update is ahem.... migliorabile....
		Davies1900hFixWhiteKnob *pKnob = (Davies1900hFixWhiteKnob *)params[index];
		bool smooth = pKnob->smooth;
		pKnob->smooth = false;
		params[index]->paramQuantity->setValue((double)bpm_integer);
		pKnob->smooth = smooth;
	}
}