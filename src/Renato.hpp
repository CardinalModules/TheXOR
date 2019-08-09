#include "common.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "rntSequencer.hpp"

////////////////////
// module widgets
////////////////////

struct Renato;
struct RenatoWidget : SequencerWidget
{	
public:
	void onMenu(int action);
	RenatoWidget(Renato * module);
	enum MENUACTIONS
	{
		RANDOMIZE_PITCH  = 0x01,
		RANDOMIZE_GATEX  = 0x02,
		RANDOMIZE_GATEY  = 0x04,
		RANDOMIZE_ACCESS = 0x08,
		RANDOMIZE_LAQUALUNQUE = 0x10

	};
private:

	struct RandomizeSubItemItem : MenuItem {
		RandomizeSubItemItem(Module *ren, const char *title, int action);

		int randomizeDest;
		Renato *renato;
		void onAction(const event::Action &e) override;
	};

	struct RandomizeItem : ui::MenuItem
	{
	public:
		RandomizeItem(Module *ren)
		{
			renato = ren;
			text = "Force the hand of chance";
			rightText = RIGHT_ARROW;
		};
		Menu *createChildMenu() override
		{
			Menu *sub_menu = new Menu;
			sub_menu->addChild(new RandomizeSubItemItem(renato, "Ov Pitch", RANDOMIZE_PITCH));
			sub_menu->addChild(new RandomizeSubItemItem(renato, "Ov Gate Ov X", RANDOMIZE_GATEX));
			sub_menu->addChild(new RandomizeSubItemItem(renato, "Ov Gate Ov Y", RANDOMIZE_GATEY));
			sub_menu->addChild(new RandomizeSubItemItem(renato, "Ov Access", RANDOMIZE_ACCESS));
			sub_menu->addChild(new RandomizeSubItemItem(renato, "Ov Power", RANDOMIZE_LAQUALUNQUE));
			return sub_menu;
		}

	private:
		Module *renato;
	};

	Menu *addContextMenu(Menu *menu) override;
};

struct Renato : Module
{
	enum ParamIds
	{
		COUNTMODE_X, COUNTMODE_Y,
		SEEKSLEEP,
		ACCESS_1,
		GATEX_1 = ACCESS_1 + 16,
		GATEY_1 = GATEX_1 + 16,
		VOLTAGE_1 = GATEY_1 + 16,
		 M_RESET = VOLTAGE_1 + 16,
		 NUM_PARAMS
	};

	enum InputIds
	{
		XCLK,
		YCLK,
		RESET,
		ACCESS_IN1,
		GATEX_IN1 = ACCESS_IN1 + 16,
		GATEY_IN1 = GATEX_IN1 + 16,
		RANDOMIZONE = GATEY_IN1+16,
		NUM_INPUTS
	};

	enum OutputIds
	{
		CV,
		XGATE, YGATE,
		CV_OUTSTEP1,
		NUM_OUTPUTS = CV_OUTSTEP1+16
	};

	enum LightIds
	{
		LED_GATEX, LED_GATEY,
		LED_1,
		NUM_LIGHTS = LED_1 + 16
	};
	
	void setWidget(RenatoWidget *pwdg) { pWidget = pwdg; }
	Renato() : Module()
	{
		theRandomizer = 0;
		pWidget = NULL;
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(Renato::COUNTMODE_Y, 0.0, 2.0, 0.0);
		configParam(Renato::COUNTMODE_X, 0.0, 2.0, 0.0);
		configParam(Renato::SEEKSLEEP, 0.0, 1.0, 0.0);
		for(int r = 0; r < 4; r++)
		{
			for(int c = 0; c < 4; c++)
			{
				int n = c + r * 4;
				configParam(Renato::ACCESS_1 + n, 0.0, 1.0, 1.0);
				configParam(Renato::GATEX_1 + n, 0.0, 1.0, 1.0);
				configParam(Renato::GATEY_1 + n, 0.0, 1.0, 1.0);
				configParam(Renato::VOLTAGE_1 + n, LVL_OFF, LVL_MAX, 1.0, "Voltage", "V");
			}
		}
		#ifdef LAUNCHPAD
		drv = new LaunchpadBindingDriver(this, Scene3, 2);
		drv->SetAutoPageKey(LaunchpadKey::SESSION, 0);
		drv->SetAutoPageKey(LaunchpadKey::NOTE, 1);
		#endif
		#ifdef OSCTEST_MODULE
		oscDrv = new OSCDriver(this, 3);
		#endif
		onReset();
	}

	#ifdef DIGITAL_EXT
	~Renato()
	{
		#if defined(LAUNCHPAD)
		delete drv;
		#endif
		#if defined(OSCTEST_MODULE)
		delete oscDrv;
		#endif
	}
	#endif

	void process(const ProcessArgs &args) override;
	void onReset() override { load(); }

	void dataFromJson(json_t *root) override
	{
		Module::dataFromJson(root);
		json_t *rndJson = json_object_get(root, "theRandomizer");
		if (rndJson)
			theRandomizer = json_integer_value(rndJson);
		on_loaded();
	}
	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();
		json_t *rndJson = json_integer(theRandomizer);
		json_object_set_new(rootJ, "theRandomizer", rndJson);
		return rootJ;
	}
	bool _accessX(int p) { return _access(xy(p, seqY.Position())); }
	bool _accessY(int p) { return _access(xy(seqX.Position(), p)); }

	#ifdef DIGITAL_EXT
	float connected;
	#endif
	#ifdef LAUNCHPAD
	LaunchpadBindingDriver *drv = NULL;
	#endif
	#if defined(OSCTEST_MODULE)
	OSCDriver *oscDrv = NULL;
	#endif
	int theRandomizer;

private:
	float getStatus(int pid, int iid)
	{
		return inputs[iid].getNormalVoltage(0.0) + params[pid].value;
	}
	void randrandrand();
	void randrandrand(int action);

private:
	RenatoWidget *pWidget;
	dsp::SchmittTrigger resetTrigger;
	dsp::SchmittTrigger accessRndTrigger;
	dsp::SchmittTrigger gatexRndTrigger;
	dsp::SchmittTrigger gateyRndTrigger;
	dsp::SchmittTrigger masterReset;
	void on_loaded();
	void load();
	void led(int n);
	void setOut(int n, bool on);
	int xy(int x, int y) { return 4 * y + x; }
	bool _access(int n) { return getStatus(ACCESS_1 + n, ACCESS_IN1 + n) > 0; }
	bool _gateX(int n) { return  getStatus(GATEX_1 + n, GATEX_IN1 + n) > 0; }
	bool _gateY(int n) { return  getStatus(GATEY_1 + n, GATEY_IN1 + n) > 0; }
	rntSequencer seqX;
	rntSequencer seqY;
};
