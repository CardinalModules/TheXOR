#include "../include/common.hpp"


float SequencerWidget::quantizePitch(int idx, float value, const cvMicroStrip &cvs)
{
	value = cvs.Value(value);
	value = cvs.Reverse(NearestSemitone(value));
	int index = getParamIndex(idx);
	if(index >= 0)
	{
// 		params[index]->paramQuantity->setValue(value);
	}

	return value;
}

bool testaCroce() { return random::uniform() > 0.5; }

bool voltNear(float v1, float v2)
{
	static const float limitenum = 0.0005;
	return fabs(v1 - v2) <= limitenum;
}

bool isSwitchOn(Module *pm, int paramId)
{
	return (pm->params[paramId].getValue() > SWITCH_ON);
}

bool isLevelOn(Module *pm, int inputId)
{
	return (pm->inputs[inputId].getNormalVoltage(0.f) > SWITCH_ON);
}

bool getModulableSwitch(Module *pm, int paramId, int inputId)
{
	return (isLevelOn(pm, inputId) || isSwitchOn(pm, paramId));
}

float getModulableParam(Module *pm, int paramId, int inputId, float minValue, float maxValue)
{
	float v = (pm->inputs[inputId].getNormalVoltage(0.0)/LVL_MAX) * maxValue;
	return clamp(v+ pm->params[paramId].getValue(), minValue, maxValue);
}

bool IsExpansion(Module *pm, float *dest, int expansionID, int inputID, int ledID)
{
	if(pm->inputs[inputID].isConnected())
	{
		*dest = pm->inputs[inputID].getVoltage();
		uint8_t *addr = (uint8_t *)dest;
		if((*(addr+3) & 0xf0) == expansionID)
		{
			pm->lights[ledID].value = LED_ON;
			return true;
		}

	}
	pm->lights[ledID].value = LED_OFF;
	return false;
}
