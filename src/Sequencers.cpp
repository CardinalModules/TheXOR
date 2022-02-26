#include "../include/common.hpp"
#include "../include/Klee.hpp"
#include "../include/M581.hpp"
#include "../include/Z8K.hpp"
#include "../include/Renato.hpp"
#include "../include/SpiraloneModule.hpp"
#include "../include/pwmClock.hpp"
#include "../include/quantizer.hpp"
#include "../include/burst.hpp"
#include "../include/uncert.hpp"
#include "../include/attenuator.hpp"
#include "../include/boole.hpp"
#include "../include/mplex.hpp"
#include "../include/demplex.hpp"
#include "../include/switch.hpp"
#include "../include/counter.hpp"
#include "../include/nag.hpp"
#include "../include/empty.hpp"
#include "../include/ascii.hpp"
#include "../include/quattro.hpp"
#include "../include/flop.hpp"
#include "../include/o88o.hpp"
#include "../include/volt.hpp"
#include "../include/Nordschleife.hpp"
#include "../include/kexp.hpp"
#include "../include/rr.hpp"
#include "../include/z8expV.hpp"
#include "../include/z8expC.hpp"
#include "../include/z8expT.hpp"
#include "../include/z8expM.hpp"
#include "../include/z8expX.hpp"
#include "../include/chords.hpp"
#include "../include/c2v.hpp"
#include "../include/c3v.hpp"
#include "../include/m581expT.hpp"
#include "../include/m581expC.hpp"
#include "../include/m581expM.hpp"
#include "../include/m581expV.hpp"
#include "../include/dice.hpp"
#include "../include/gate.hpp"
#include "../include/pwc.hpp"
#include "../include/cclk.hpp"

namespace TheXOR {
	Model* modelKlee = createModel<Klee, KleeWidget>("Klee");
	Model* modelkExp = createModel<kExp, kExpWidget>("kExp");
	Model* modelM581 = createModel<M581, M581Widget>("M581");
	Model* modelM581TExp = createModel<m581expT, m581expTWidget>("M581TExp");
	Model* modelM581CExp = createModel<m581expC, m581expCWidget>("M581CExp");
	Model* modelM581MExp = createModel<m581expM, m581expMWidget>("M581MExp");
	Model* modelM581VExp = createModel<m581expV, m581expVWidget>("M581VExp");
	Model* modelZ8K = createModel<Z8K, Z8KWidget>("Z8K");
	Model* modelZ8KVExp = createModel<z8expV, z8expVWidget>("Z8KVExp");
	Model* modelZ8KCExp = createModel<z8expC, z8expCWidget>("Z8KCExp");
	Model* modelZ8KMExp = createModel<z8expM, z8expMWidget>("Z8KMExp");
	Model* modelZ8KTExp = createModel<z8expT, z8expTWidget>("Z8KTExp");
	Model* modelZ8KXExp = createModel<z8expX, z8expXWidget>("Z8KXExp");
	Model* modelRenato = createModel<Renato, RenatoWidget>("Renato");
	Model* modelSpiralone = createModel<Spiralone, SpiraloneWidget>("Spiralone");
	Model* modelBurst = createModel<Burst, BurstWidget>("Burst");
	Model* modelUncertain = createModel<Uncertain, UncertainWidget>("Uncertain");
	Model* modelPWMClock = createModel<PwmClock, PwmClockWidget>("PWMClock");
	Model* modelQuantizer = createModel<Quantizer, QuantizerWidget>("Quantizer");
	Model* modelAttAmp = createModel<Attenuator, AttenuatorWidget>("AttAmp");
	Model* modelXSwitch = createModel<XSwitch, SwitchWidget>("XSwitch");
	Model* modelBoole = createModel<Boole, BooleWidget>("Boole");
	Model* modelMplex = createModel<Mplex, MplexWidget>("Mplex");
	Model* modelDmplex = createModel<Dmplex, DmplexWidget>("Dmplex");
	Model* modelCounter = createModel<Counter, CounterWidget>("Counter");
	Model* modelmidyQuant = createModel<midyQuant, midyQuantWidget>("midyQuant");
	Model* modelnag = createModel<nag, nagWidget>("nag");
	Model* modelempty = createModel<empty, emptyWidget>("empty");
	Model* modelascii = createModel<ascii, asciiWidget>("ascii");
	Model* modelquattro = createModel<quattro, quattroWidget>("quattro");
	Model* modelflop = createModel<flop, flopWidget>("flop");
	Model* modelo88o = createModel<o88o, o88oWidget>("o88o");
	Model* modelvolt = createModel<volt, voltWidget>("volt");
	Model* modelNordschleife = createModel<Nordschleife, NordschleifeWidget>("Nordschleife");
	Model* modelRR = createModel<RR, RRWidget>("RR");
	Model* modelchords = createModel<chords, chordsWidget>("chords");
	Model* modelc2v = createModel<c2v, c2vWidget>("c2v");
	Model* modelc3v = createModel<c3v, c3vWidget>("c3v");
	Model* modeldice = createModel<dice, diceWidget>("dice");
	Model* modelgate = createModel<gate, gateWidget>("gate");
	Model* modelPWC = createModel<pwc, pwcWidget>("PWC");
	Model* modelcclk = createModel<cclk, cclkWidget>("cclk");
}
