#include "static_neuron_colors.h"

QColor static_neuron_color(int ind)
{
	int r0=(ind*541)%60;
	int g0=(ind*229)%200;
	int b0=(ind*281)%200;
	return QColor(r0,g0,b0);
}
