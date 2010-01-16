#ifndef PERFHUD_H
#define PERFHUD_H

struct Perfomance_Display {
	void Render();

	void DoNetworkStats();
	void DoFontTest();

	void DoTestSprites();
	void DoGeneralStats();
	void DoMapVBOStats();
	void DoFPSStats();
	void DoMapInfo();
	void DoDrawSateliteImage();
	void DoPedTest();

	void DoMemoryStats();
};

extern Perfomance_Display perfHUD;

#endif