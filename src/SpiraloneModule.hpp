#pragma once

struct Spiralone : Module
{
    enum ParamIds
    {
        VOLTAGE_1,
        MODE_1 = VOLTAGE_1 + TOTAL_STEPS,
        LENGHT_1 = MODE_1 + NUM_SEQUENCERS,
        STRIDE_1 = LENGHT_1 + NUM_SEQUENCERS,
        XPOSE_1 = STRIDE_1 + NUM_SEQUENCERS,
        NUM_PARAMS = XPOSE_1 + NUM_SEQUENCERS
    };

    enum InputIds
    {
        RESET_1,
        INLENGHT_1 = RESET_1 + NUM_SEQUENCERS,
        INSTRIDE_1 = INLENGHT_1 + NUM_SEQUENCERS,
        INXPOSE_1 = INSTRIDE_1 + NUM_SEQUENCERS,
        CLOCK_1 = INXPOSE_1 + NUM_SEQUENCERS,
        NUM_INPUTS = CLOCK_1 + NUM_SEQUENCERS
    };

    enum OutputIds
    {
        CV_1,
        GATE_1 = CV_1 + NUM_SEQUENCERS,
        NUM_OUTPUTS = GATE_1 + NUM_SEQUENCERS
    };

    enum LightIds
    {
        LED_SEQUENCE_1,
		LED_GATE_1 = (LED_SEQUENCE_1 + TOTAL_STEPS) * NUM_SEQUENCERS,
        NUM_LIGHTS = LED_GATE_1 + NUM_SEQUENCERS
    };


    Spiralone() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
    {
        #ifdef LAUNCHPAD
        drv = new LaunchpadBindingDriver(Scene5, 3);
        drv->SetAutoPageKey(LaunchpadKey::SESSION, 0);
        drv->SetAutoPageKey(LaunchpadKey::NOTE, 1);
        drv->SetAutoPageKey(LaunchpadKey::DEVICE, 2);
        #endif
        on_loaded();
    }

    #ifdef LAUNCHPAD
    ~Spiralone()
    {
        delete drv;
    }
    #endif

    void step() override;
    void reset() override {load();}

    void fromJson(json_t *root) override {Module::fromJson(root); on_loaded();}
    json_t *toJson() override
    {
		json_t *rootJ = json_object();
		return rootJ;
	}

	#ifdef LAUNCHPAD
    LaunchpadBindingDriver *drv;
    float connected;
    #endif

private:
    void on_loaded();
    void load();

    spiraloneSequencer sequencer[NUM_SEQUENCERS];

};
