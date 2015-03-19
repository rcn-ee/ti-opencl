#include <list>

#include "object.h"
#include "cpu/device.h"
#include "dsp/device.h"

using namespace Coal;

std::list<Object *> known_objects;

Coal::CPUDevice cpudevice;

Coal::DSPDevice dsp0device(0);
Coal::DSPDevice dsp1device(1);
Coal::DSPDevice dsp2device(2);
Coal::DSPDevice dsp3device(3);
