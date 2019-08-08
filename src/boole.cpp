#include "common.hpp"
#include "boole.hpp"

void Boole::process(const ProcessArgs &args)
{
	bool hiz = params[HIZ].value > 0.1;
	bool compare = params[COMPAREMODE].value > 0.1;
	lights[LED_HIZ].value = hiz ? LED_ON : LED_OFF;

	for(int k = 0; k < NUM_BOOL_OP; k++)
	{
		if(outputs[OUT_1 + k].isConnected())
		{
			bool o = process(k, hiz, compare);
			if(params[INVERT_1 + k].value > 0.1)
				o = !o;
			lights[LED_OUT+k].value = o ? LED_ON : LED_OFF;
			outputs[OUT_1 + k].value = o ? LVL_ON : LVL_OFF;
		}
		else
		{
			lights[LED_X + k].value = lights[LED_OUT + k].value = LED_OFF;
			if(k>0)
				lights[LED_Y + k].value = LED_OFF;
		}
	}
}

float Boole::getVoltage(int index, int num_op, bool hiz)
{
	if (hiz && !inputs[index].isConnected())
	{
		if(random::uniform() > 0.6)
		{
			float n = random::normal();
			if (n > 2.1)
				return random::uniform() * 9.2;
			else if (n > 2.0)
				return random::uniform() * 7.0;
			else if (n > 1.0)
				return random::uniform() * 5.0;
			else if (n > 0.5)
				return random::uniform() * 2.5;
			else if (n < -1.0)
				return random::uniform() * 1.0;
		}
		return 0;
	} else
		return inputs[index].getNormalVoltage(0.0);
}

bool Boole::logicLevel(float v1, float v2, bool compare)
{	
	return compare ? fabs(v1 - v2) < std::numeric_limits<float>::epsilon() : v1 > v2;
}

bool Boole::process(int num_op, bool hiz, bool compare)
{
	bool x = logicLevel(getVoltage(IN_X + num_op, num_op, hiz), params[THRESH_X + num_op].value, compare);
	lights[LED_X + num_op].value = x ? LED_ON : LED_OFF;
	if(num_op == 0)	// not?
		return !x;
		
	bool y = logicLevel(getVoltage(IN_Y + num_op-1, num_op, hiz), params[THRESH_Y + num_op-1].value, compare);
	lights[LED_Y + num_op - 1].value = y ? LED_ON : LED_OFF;
		
	switch(num_op)
	{	
		case 1: return x && y;	//and
		case 2: return x || y;	//or
		case 3: return x ^ y;	//the xor
		case 4: return !x || y;	// implication
	}

	return false;
}

BooleWidget::BooleWidget(Boole *module) : ModuleWidget()
{
	CREATE_PANEL(module, this, 14, "res/modules/boole.svg");

	float in_x = mm2px(5.170);
	float in_led_x = mm2px(15.778);
	float out_led_x = mm2px(53.878);
	float pot_x = mm2px(20.561);
	float out_x = mm2px(58.091);
	float y = 112.349;
	float yout = 112.349;
	float ypot = 112.477;
	float yled = 115.389;
	float yinv = 97.892;
	float yled_out = 115.389;
	float delta_y =- 14.771;
	float sub_dy = -11.92;
	float out_dy = -26.691;
	
	for(int k = 0; k < NUM_BOOL_OP; k++)
	{
		// X
		addInput(createInput<PJ301GRPort>(Vec(in_x, yncscape(y, 8.255)), module, Boole::IN_X + k));
		addParam(createParam<Davies1900hFixWhiteKnobSmall>(Vec(pot_x, yncscape(ypot, 8.0)), module, Boole::THRESH_X + k));
		addChild(createLight<SmallLight<RedLight>>(Vec(in_led_x, yncscape(yled, 2.176)), module, Boole::LED_X +k));

		// Y
		if(k > 0)
		{
			y += sub_dy;
			ypot += sub_dy;
			yled += sub_dy;
			addInput(createInput<PJ301GRPort>(Vec(in_x, yncscape(y, 8.255)), module, Boole::IN_Y + k-1));
			addParam(createParam<Davies1900hFixWhiteKnobSmall>(Vec(pot_x, yncscape(ypot, 8.0) ), module, Boole::THRESH_Y + k-1));
			addChild(createLight<SmallLight<RedLight>>(Vec(in_led_x, yncscape(yled, 2.176)), module, Boole::LED_Y + k-1 ));
		}
		
		// OUT
		addOutput(createOutput<PJ301WPort>(Vec(out_x, yncscape(yout, 8.255)), module, Boole::OUT_1+k));
		addChild(createLight<SmallLight<WhiteLight>>(Vec(out_led_x, yncscape(yled_out, 2.176)), module, Boole::LED_OUT+k));

		if(k == 0)
		{
			addParam(createParam<CKSSFix>(Vec(mm2px(53.116), yncscape(118.714, 5.460)), module, Boole::INVERT_1 + k));
			yled_out -= 20.731;
			yout -= 20.731;
		} else
		{
			addParam(createParam<CKSSFix>(Vec(mm2px(53.116), yncscape(yinv, 5.460)), module, Boole::INVERT_1 + k));
			yled_out += out_dy;
			yout += out_dy;
			yinv += out_dy;
		}

		y += delta_y;
		ypot += delta_y;
		yled += delta_y;
	}

	addParam(createParam<CKSSFixH>(Vec(mm2px(21.831), yncscape(1.436, 3.704)), module, Boole::COMPAREMODE));
	addParam(createParam<CKSSFixH>(Vec(mm2px(43.084), yncscape(1.436, 3.704)), module, Boole::HIZ));
	addChild(createLight<SmallLight<WhiteLight>>(Vec(mm2px(49.404), yncscape(2.2, 2.176)), module, Boole::LED_HIZ ));
}

