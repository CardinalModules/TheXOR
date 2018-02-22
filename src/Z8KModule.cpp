#include "Z8K.hpp"
#include "z8kSequencer.hpp"
#include <sstream>

struct Z8K : Module
{
	enum ParamIds
	{
		VOLTAGE_1,
		NUM_PARAMS = VOLTAGE_1 + 16
	};

	enum InputIds
	{
		RESET_1,
		RESET_A = RESET_1 + 4,
		RESET_VERT = RESET_A + 4,
		RESET_HORIZ,

		DIR_1,
		DIR_A = DIR_1 + 4,
		DIR_VERT = DIR_A + 4,
		DIR_HORIZ,

		CLOCK_1,
		CLOCK_A = CLOCK_1 + 4,
		CLOCK_VERT = CLOCK_A + 4,
		CLOCK_HORIZ,

		NUM_INPUTS
	};

	enum OutputIds
	{
		CV_1,
		CV_A = CV_1 + 4,
		CV_VERT = CV_A + 4,
		CV_HORIZ,
		NUM_OUTPUTS
	};

	enum LightIds
	{
		LED_ROW,
		LED_COL = LED_ROW + 16,
		LED_VERT = LED_COL + 16,
		LED_HORIZ = LED_VERT + 16,
		NUM_LIGHTS = LED_HORIZ + 16
	};

	enum SequencerIds
	{
		SEQ_1,
		SEQ_A = SEQ_1 + 4,
		SEQ_VERT = SEQ_A + 4,
		SEQ_HORIZ,
		NUM_SEQUENCERS
	};

	Z8K() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		#ifdef LAUNCHPAD
		drv = new LaunchpadBindingDriver(Scene4, 1);
		drv->SetAutoPageKey(LaunchpadKey::SESSION, 0);
		#endif
		#ifdef OSCTEST_MODULE
		#define MY_SCENE    4
		oscDrv = new OSCDriver(MY_SCENE);
		#endif
		on_loaded();
	}

	#ifdef DIGITAL_EXT
	~Z8K()
	{
		#if defined(LAUNCHPAD)
		delete drv;
		#endif
		#if defined(OSCTEST_MODULE)
		delete oscDrv;
		#endif
	}
	#endif

	void step() override;
	void reset() override { load(); }

	void fromJson(json_t *root) override { Module::fromJson(root); on_loaded(); }
	json_t *toJson() override
	{
		json_t *rootJ = json_object();
		return rootJ;
	}

	#ifdef DIGITAL_EXT
	float connected;
	#endif
	#ifdef LAUNCHPAD
	LaunchpadBindingDriver *drv;
	#endif
	#if defined(OSCTEST_MODULE)
	OSCDriver *oscDrv;
	#endif

private:
	void on_loaded();
	void load();
	z8kSequencer seq[10];
};

void Z8K::on_loaded()
{
	#ifdef DIGITAL_EXT
	connected = 0;
	#endif
	load();
}

void Z8K::load()
{
	// sequencer 1-4
	for(int k = 0; k < 4; k++)
	{
		int base = VOLTAGE_1 + 4 * k;
		std::vector<int> steps = {base, base + 1, base + 2, base + 3};
		seq[SEQ_1 + k].Init(&inputs[RESET_1 + k], &inputs[DIR_1 + k], &inputs[CLOCK_1 + k], &outputs[CV_1 + k], &lights[LED_ROW], params, steps);
	}
	// sequencer A-D
	for(int k = 0; k < 4; k++)
	{
		std::vector<int> steps = {k, k + 4, k + 8, k + 12};
		seq[SEQ_A + k].Init(&inputs[RESET_A + k], &inputs[DIR_A + k], &inputs[CLOCK_A + k], &outputs[CV_A + k], &lights[LED_COL], params, steps);
	}
	// horiz
	std::vector<int> steps_h = {0,1,2,3,7,6,5,4,8,9,10,11,15,14,13,12};
	seq[SEQ_HORIZ].Init(&inputs[RESET_HORIZ], &inputs[DIR_HORIZ], &inputs[CLOCK_HORIZ], &outputs[CV_HORIZ], &lights[LED_HORIZ], params, steps_h);
	//vert
	std::vector<int> steps_v = {0,4,8,12,13,9,5,1,2,6,10,14,15,11,7,3};
	seq[SEQ_VERT].Init(&inputs[RESET_VERT], &inputs[DIR_VERT], &inputs[CLOCK_VERT], &outputs[CV_VERT], &lights[LED_VERT], params, steps_v);
}

void Z8K::step()
{
	for(int k = 0; k < NUM_SEQUENCERS; k++)
		seq[k].Step();

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

Z8KWidget::Z8KWidget()
{
	#ifdef OSCTEST_MODULE
	char name[60];
	#endif

	Z8K *module = new Z8K();
	setModule(module);
	box.size = Vec(28 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/Z8KModule.svg")));
	addChild(panel);
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));

	int x = 10;
	int y = 30;
	int dist_v = 38;
	int dist_h = 32;
	for(int k = 0; k < 4; k++)
	{
		addInput(createInput<PJ301YPort>(Vec(x, y + k * dist_v), module, Z8K::RESET_1 + k));
		addInput(createInput<PJ301WPort>(Vec(x + dist_h, y + k * dist_v), module, Z8K::DIR_1 + k));
		addInput(createInput<PJ301RPort>(Vec(x + 2 * dist_h, y + k * dist_v), module, Z8K::CLOCK_1 + k));
	}

	y += 5 * dist_v;
	for(int k = 0; k < 4; k++)
	{
		addInput(createInput<PJ301YPort>(Vec(x, y + k * dist_v), module, Z8K::RESET_A + k));
		addInput(createInput<PJ301WPort>(Vec(x + dist_h, y + k * dist_v), module, Z8K::DIR_A + k));
		addInput(createInput<PJ301RPort>(Vec(x + 2 * dist_h, y + k * dist_v), module, Z8K::CLOCK_A + k));
	}

	y = 35;
	x += 2 * dist_h + 40;
	dist_h = 64;
	dist_v = 65;
	for(int r = 0; r < 4; r++)
	{
		for(int c = 0; c < 4; c++)
		{
			int n = c + r * 4;
			ParamWidget *pctrl = createParam<Davies1900hBlackKnob>(Vec(x + dist_h * c, y + dist_v * r), module, Z8K::VOLTAGE_1 + n, 0.0, 1.0, 0.5);
			#ifdef OSCTEST_MODULE
			sprintf(name, "/Knob%i", n + 1);
			oscControl *oc = new oscControl(name);
			module->oscDrv->Add(oc, pctrl);
			#endif
			addParam(pctrl);

			ModuleLightWidget *plight = createLight<SmallLight<RedLight>>(Vec(x + 2 * dist_h / 3 + c * dist_h - 5, y + 2 * dist_v / 3 + dist_v * r - 5), module, Z8K::LED_ROW + n);
			#ifdef OSCTEST_MODULE
			sprintf(name, "/LedR%i", n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, plight);
			#endif
			addChild(plight);

			plight = createLight<SmallLight<GreenLight>>(Vec(x + 2 * dist_h / 3 + c * dist_h + 5, y + 2 * dist_v / 3 + dist_v * r - 5), module, Z8K::LED_COL + n);
			#ifdef OSCTEST_MODULE
			sprintf(name, "/LedC%i", n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, plight);
			#endif
			addChild(plight);

			plight = createLight<SmallLight<YellowLight>>(Vec(x + 2 * dist_h / 3 + c * dist_h - 5, y + 2 * dist_v / 3 + dist_v * r + 5), module, Z8K::LED_VERT + n);
			#ifdef OSCTEST_MODULE
			sprintf(name, "/LedV%i", n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, plight);
			#endif
			addChild(plight);

			plight = createLight<SmallLight<BlueLight>>(Vec(x + 2 * dist_h / 3 + c * dist_h + 5, y + 2 * dist_v / 3 + dist_v * r + 5), module, Z8K::LED_HORIZ + n);
			#ifdef OSCTEST_MODULE
			sprintf(name, "/LedH%i", n + 1);
			oc = new oscControl(name);
			module->oscDrv->Add(oc, plight);
			#endif
			addChild(plight);

			if(r == 3)
				addOutput(createOutput<PJ301GPort>(Vec(x + dist_h * c + 7, y + dist_v * 4 - dist_v / 3), module, Z8K::CV_A + c));
		}
		addOutput(createOutput<PJ301GPort>(Vec(box.size.x - 40, y + 5 + dist_v * r), module, Z8K::CV_1 + r));
	}

	y += dist_v * 4 + 40;
	dist_h /= 2;
	dist_v = 20;
	for(int k = 0; k < 2; k++)
	{
		int px = 7 + x + k * 4 * dist_h;
		addInput(createInput<PJ301YPort>(Vec(px, y), module, Z8K::RESET_VERT + k));
		addInput(createInput<PJ301WPort>(Vec(px + dist_h, y - dist_v), module, Z8K::DIR_VERT + k));
		addInput(createInput<PJ301RPort>(Vec(px + 2 * dist_h, y), module, Z8K::CLOCK_VERT + k));
		addOutput(createOutput<PJ301GPort>(Vec(px + 3 * dist_h, y - dist_v), module, Z8K::CV_VERT + k));
	}

	#ifdef DIGITAL_EXT
	addChild(new DigitalLed((box.size.x - 24) / 2, 5, &module->connected));
	#endif
}
