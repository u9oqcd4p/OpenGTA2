#include "opengta_cl.h"
#include "opengta_sv.h"
#include "perfhud.h"

Perfomance_Display perfHUD;

TexID spr = BAD_ID;

void Perfomance_Display::Render() {
	if (Convar.GetInt("perf_fps",0))		perfHUD.DoFPSStats();
	if (Convar.GetInt("perf_font",0))		perfHUD.DoFontTest();
	if (Convar.GetInt("perf_net",1))		perfHUD.DoNetworkStats();
	if (Convar.GetInt("perf_stats",0))		perfHUD.DoGeneralStats();
	if (Convar.GetInt("perf_vbo",0))		perfHUD.DoMapVBOStats();
	if (Convar.GetInt("perf_ped",1))		perfHUD.DoPedTest();
	if (Convar.GetInt("perf_mem",0))		perfHUD.DoMemoryStats();

	if (Convar.GetInt("fps",1)) {
		Fonts.printf(Fonts.Message,Vector2f(Screen.Width-96.0f,0),"FPS: %.0f",Timer.FPS());
	}
}

void Perfomance_Display::DoNetworkStats() {
	Fonts.SetAlign(FONTALIGN_ABOVERIGHT);
	if (Network.IsServer) {
		int activeCount = 0;
		for (uint i = 0; i < Network.Connections.Count; i++) 
			if (Network[i]->socketHandle >= 0) activeCount++;

		Fonts.printf(Fonts.Message,Vector2f(Screen.Width-8.0f,Screen.Height-8.0f),
			"Network (server)\n"
			"Total: send %d (%db)  recv %d (%db)\n"
			"Connections: %d/%d out of %d\n",
			Network.numPacketsSent,Network.numBytesSent,
			Network.numPacketsReceived,Network.numBytesReceived,
			activeCount,Network.Connections.Count,Network.Connections.AllocCount);
	} else if (Network.IsConnected) {
		Fonts.printf(Fonts.Message,Vector2f(Screen.Width-8.0f,Screen.Height-8.0f),
			"Network (client, ping %.2f ms, server %.2f fps)\n"
			"Total: send %d (%db)  recv %d (%db)\n",
			Network.packetLatency*1000.0f,Network.remoteServerFPS,
			Network.numPacketsSent,Network.numBytesSent,
			Network.numPacketsReceived,Network.numBytesReceived);

		for (uint i = 0; i < Peds.pedPool.Count; i++) {
			Draw.Line3D((*Peds.pedPool[i])->Position,
						(*Peds.pedPool[i])->netPosition,Vector4f(0.9f,0.1f,0.1f,1.0f));

			Draw.Line3D((*Peds.pedPool[i])->netPosition+Vector3f(-0.1f,-0.1f,0.0f),
						(*Peds.pedPool[i])->netPosition+Vector3f(0.1f,0.1f,0.0f),Vector4f(0.9f,0.1f,0.1f,1.0f));
			Draw.Line3D((*Peds.pedPool[i])->netPosition+Vector3f(0.1f,-0.1f,0.0f),
						(*Peds.pedPool[i])->netPosition+Vector3f(-0.1f,0.1f,0.0f),Vector4f(0.9f,0.1f,0.1f,1.0f));

			Draw.Line3D((*Peds.pedPool[i])->Position+Vector3f(-0.1f,-0.1f,0.0f),
						(*Peds.pedPool[i])->Position+Vector3f(0.1f,0.1f,0.0f),Vector4f(0.1f,0.9f,0.1f,1.0f));
			Draw.Line3D((*Peds.pedPool[i])->Position+Vector3f(0.1f,-0.1f,0.0f),
						(*Peds.pedPool[i])->Position+Vector3f(-0.1f,0.1f,0.0f),Vector4f(0.1f,0.9f,0.1f,1.0f));
		}
	} else {
		Fonts.printf(Fonts.Message,Vector2f(Screen.Width-8.0f,Screen.Height-8.0f),
			"Network (not connected)");
	}
	Fonts.Reset();

	float fontY = 16.0f;
	float fontX = 16.0f;

	Fonts.printf(Fonts.Message,Vector2f(fontX,fontY),"Clients table:");
	fontY += Fonts.TextHeight(Fonts.Message,"W");

	for (uint i = 0; i < Clients.List.AllocCount; i++) {
		if (Clients[i]->ClientName[0]) {
			Fonts.printf(Fonts.Message,Vector2f(fontX,fontY),"[%d] %s (pedID %d)",
				i,Clients[i]->ClientName,Clients[i]->PlayerPed);
			fontY += Fonts.TextHeight(Fonts.Message,"W");
		}
	}
	Fonts.Reset();
}

void Perfomance_Display::DoFontTest() {
	if (Convar.GetInt("perf_font") == 1) {
		Fonts.SetScale(1.0f);

		Fonts.SetAlign(FONTALIGN_LEFT);
		Fonts.printf(Fonts.Message,Vector2f(0,0),"EDGE");

		Fonts.SetAlign(FONTALIGN_ABOVELEFT);
		Fonts.printf(Fonts.Message,Vector2f(0,Screen.Height),"EDGE");

		Fonts.SetAlign(FONTALIGN_RIGHT);
		Fonts.printf(Fonts.Message,Vector2f(Screen.Width,0),"EDGE");

		Fonts.SetAlign(FONTALIGN_ABOVERIGHT);
		Fonts.printf(Fonts.Message,Vector2f(Screen.Width,Screen.Height),"EDGE");

		Fonts.SetAlign(FONTALIGN_CENTER);
		Fonts.printf(Fonts.Message,Vector2f(Screen.Width/2,Screen.Height/2),"CENTER");

		Fonts.SetAlign(FONTALIGN_CENTER);
		Fonts.printf(Fonts.Message,Vector2f(Screen.Width/2,0),"SIDE");

		Fonts.SetAlign(FONTALIGN_ABOVE);
		Fonts.printf(Fonts.Message,Vector2f(Screen.Width/2,Screen.Height),"SIDE");

		Fonts.Reset();
	} else if (Convar.GetInt("perf_font") == 2) {
		Fonts.Reset();
		for (float s = 5.0f; s >= 0.2f; s -= 0.2f) {
			Fonts.SetScale(s);
			Fonts.printf(Fonts.Message,Vector2f(16.0f,64.0f+s*s*32.0f),"SCALE");
		}
	} else if (Convar.GetInt("perf_font") == 3) {
		Fonts.Reset();
		float fontY = 32.0f;
		for (uint i = 0; i < Fonts.fontEntries.Count; i++) {
			//Fonts.printf(i,Vector2f(16.0f,fontY),"a quick brown fox jumps over a lazy dog");
			//fontY += Fonts.TextHeight(i,"a quick brown fox jumps over a lazy dog");
			Fonts.printf(i,Vector2f(16.0f,fontY),"THE QUICK BROWN FOX JUMPS OVER A LAZY DOG 1234567890");
			fontY += Fonts.TextHeight(i,"THE QUICK BROWN FOX JUMPS OVER A LAZY DOG 1234567890");
		}
	} else {
		Fonts.Reset();
		if (Fonts.Message == BAD_ID) return; //No message font - no printing (cause we access all fonts directly)


		Fonts.BeginFastRender();
		Screen.Start2D();
			float fontY = 16.0f;
			float fontX = 16.0f;
			for (uint i = 1; i < 0xFFFF; i++) {
				char buf[256];
				UTF8_PUT_CHAR(buf,i);
				buf[UTF8_CHAR_LENGTH(i)] = 0;
	
				if (Fonts.fontEntries[Fonts.Message]->getCharacter(i) != BAD_ID) {
					Fonts.printf(Fonts.Message,Vector2f(fontX,fontY),"%.4X: %s",i,buf);
					fontY += Fonts.TextHeight(Fonts.Message,"X");
					if (fontY > Screen.Height - 32.0f) {
						fontY = 16.0f;
						fontX += 96.0f;
					}
				}
			}

			Draw.FlushSprites();
		Screen.End2D();
		Fonts.EndFastRender();
	}
}

void Perfomance_Display::DoTestSprites() {
	/*if (spr == BAD_ID) {
		spr = Graphics.GetTextureID("GTA2_77");
	} else {
		Screen.Start2D();
			Draw.AnimSprite2D(&spr,Vector2f(Screen.Width/2,Screen.Height/2),4.0f,ALIGN_CENTER);
			Draw.FlushSprites();
		Screen.End2D();
	}*/
	/*Vector2f pos = Vector2f(0,0);
	for (int i = 0; i < 1024; i++) {
		int texID = i+2048;
		texture_entry* texEnt = Graphics.GetTextureEntryByID(texID);

		Draw.Sprite2D(texID,pos);
		pos.x += texEnt->Width;
		if (pos.x > Screen.Width) {
			pos.x = 0;
			pos.y += texEnt->Height;
		}
		//Draw.Sprite2D(Random.Int(0,1000),Vector2f(Random.Float(0,Screen.Width),Random.Float(0,Screen.Height)),Random.Float(0.0f,1.0f));
	}*/
}

void Perfomance_Display::DoDrawSateliteImage() {
	Vector2f spos = Vector2f(0.0f,0.0f);
	Vector2f tpos = Vector2f(0.0f,0.0f);//Camera.Position.x,Camera.Position.y);
	float scale = 1.0f/(Camera.Position.z / 16.0f);

	Screen.Start2D();
		rmp_cityscape* cityscape;
		int cx = -1;
		int cy = -1;
		TexID baseTex;
		
		while (spos.y < Screen.Height) {
			float z = Map.GetZ(tpos,true);
			TexID blockTex = Map.GetBlock(Vector3f(tpos.x,tpos.y,z))->tex_lid;
			int tx = (int)tpos.x / 64;
			int ty = (int)tpos.y / 64;
			if ((cx != tx) || (cy != ty)) {
				cityscape = Map.GetCityscape(tpos);

				char texName[256];
				if (cityscape->GraphicsName[0]) {
					snprintf(texName,256,"%s_0",cityscape->GraphicsName[0]);
					baseTex = Graphics.GetTextureID(texName);
				} else baseTex = BAD_ID;

				cx = tx;
				cy = ty;
			}
		
			if (baseTex != BAD_ID) Draw.Sprite2D(baseTex+blockTex,spos,scale);
			spos.x += 64.0f*scale;
			tpos.x += 1;
			if (spos.x > Screen.Width) {
				spos.x = 0;
				spos.y += 64.0f*scale;

				tpos.x = 0;//Camera.Position.x;
				tpos.y += 1;
			}
		}
		Draw.FlushSprites();
	Screen.End2D();
}

void Perfomance_Display::DoMapInfo() {
	float z = Map.GetZ(Camera.Position);
	TexID blockTex = Map.GetBlock(Vector3f(Camera.Position.x,Camera.Position.y,z))->tex_lid;
	rmp_cityscape* cityscape = Map.GetCityscape(Camera.Position);

	Fonts.printf(Fonts.Message,Vector2f(640.0f,16.0f),
		"X %d\n"
		"Y %d\n"
		"Z %.2f\n"
		"CX %d\n"
		"CY %d\n"
		"Lid texture %d (type %d)\n"
		"District %c%c%c%c\n"
		"Graphics %s\n",
		(int)Camera.Position.x - 64*((int)Camera.Position.x / 64),
		(int)Camera.Position.y - 64*((int)Camera.Position.y / 64),
		z,
		(int)Camera.Position.x / 64,
		(int)Camera.Position.y / 64,
		blockTex,Map.GetBlock(Vector3f(Camera.Position.x,Camera.Position.y,z))->block_type >> 4,
		cityscape->District[0],
		cityscape->District[1],
		cityscape->District[2],
		cityscape->District[3],
		cityscape->GraphicsName[0]);

	if (cityscape->GraphicsName[0]) {
		char texName[256];
		snprintf(texName,256,"%s_0",cityscape->GraphicsName[0]);
		TexID base = Graphics.GetTextureID(texName);
	
		Screen.Start2D();
			Draw.Sprite2D(base,Vector2f(760.0f,64.0f),1.02f,ALIGN_CENTER);
			Draw.Sprite2D(base+blockTex,Vector2f(760.0f,64.0f),1.0f,ALIGN_CENTER);
			Draw.FlushSprites();
		Screen.End2D();
	}
}

void Perfomance_Display::DoMemoryStats() {
	int stringpool_size = 0;
	int stringpool_used = 0;
	int stringpool_lost = 0;
	for (int i = 0; i <= Memory.malloc_curstringpool; i++) { 
		stringpool_size += Memory.malloc_stringpoolsz[i];
		stringpool_used += Memory.malloc_curstringpoolsz[i];
		stringpool_lost += Memory.malloc_stringpoolsz[i] - Memory.malloc_curstringpoolsz[i];
	}

	Fonts.SetAlign(FONTALIGN_ABOVE);
	Fonts.printf(Fonts.Message,Vector2f(Screen.Width/2,Screen.Height-8.0f),
		"Memory usage: %d KB (%d bytes)\n"
		"Peak memory usage: %d KB (%d bytes)\n"
		"Allocations: %d (peak: %d)\n"
		"String pool usage: %d out of %d bytes\n"
		"(%d percent, lost %d bytes, %d entries used)\n",
		Memory.malloc_memory / 1024,
		Memory.malloc_memory,
		Memory.malloc_max / 1024,
		Memory.malloc_max,
		Memory.malloc_cnt,
		Memory.malloc_cnt_max,
		stringpool_used,
		stringpool_size,
		(100*stringpool_used) / stringpool_size,
		stringpool_lost,
		Memory.malloc_curstringpool+1);
	Fonts.Reset();
}

void Perfomance_Display::DoGeneralStats() {
	Fonts.SetScale(min(1.0f,Screen.Height / 650.0f));
	Fonts.printf(Fonts.Message,Vector2f(16.0f,16.0f),
		"%.0f x %.0f (avg FPS %.2f)\n"
		"Client FPS %.2f\n"
		"Server FPS %.2f\n"
		"Server dT %.4f\n"
		"Client dT %.4f\n"
		"Client Time %.2f\n"
		"Server Time %.2f\n"
		"Frames %d\n"
		"Cycles %d\n"
		"\nDraw:\n"
		"VBO switches %d\n"
		"Sprites %d\n"
		"Textures used %d\n"
		"Polygons %d\n"
		"\nMemory:\n"
		"Used: %d KB\n"
		"\nCamera:\n"
		"X: %.2f\n"
		"Y: %.2f\n"
		"Z: %.2f\n"
		"\nVBOT %.3f\n",
		Screen.Width,
		Screen.Height,
		1.0f*Timer.totalFrames / Timer.Time(),
		Timer.FPS(),
		ServerTimer.FPS(),
		ServerTimer.dT(),
		Timer.dT(),
		Timer.Time(),
		ServerTimer.Time(),
		Timer.totalFrames,
		ServerTimer.totalFrames,
		Draw.statVBOSwitches,
		Draw.statSprites,
		Draw.statTextureBinds,
		Draw.statPolies,
		Memory.malloc_memory / 1024,
		Camera.Position.x,
		Camera.Position.y,
		Camera.Position.z,
		max(0,Map_Render.nextAnimationTime - Timer.Time()));
	Fonts.Reset();
}

void Perfomance_Display::DoMapVBOStats() {
	Fonts.SetAlign(FONTALIGN_CENTER);
	for (uint i = 0; i < Map_Render.VBOEntry.Count; i++)
		Fonts.printf(Fonts.Message,Vector2f(Screen.Width/2,16.0f+i*24.0f),"VBO%d %d (%s)",i,
				Map_Render.VBOEntry[i]->VBO.vertexCount / 3,
				Map_Render.VBOEntry[i]->GraphicsName);
	Fonts.Reset();
}

float FPSHistory[600][3];

void Perfomance_Display::DoFPSStats() {
	for (int i = 0; i < 599; i++) {
		FPSHistory[i][0] = FPSHistory[i+1][0];
		FPSHistory[i][1] = FPSHistory[i+1][1];
		FPSHistory[i][2] = FPSHistory[i+1][2];
	}
	FPSHistory[599][0] = Timer.FPS();
	FPSHistory[599][1] = ServerTimer.FPS();
	FPSHistory[599][2] = Network.packetLatency*1000.0f;

	Screen.Start2D();
		Vector2f prevPos1,prevPos2,prevPos3,nextPos1,nextPos2,nextPos3;
		for (int i = 0; i < 599; i++) {
			nextPos1 = Vector2f(Screen.Width-600.0f+i/1.0f,max(0.0f,128.0f-FPSHistory[i][0]*0.5f));
			nextPos2 = Vector2f(Screen.Width-600.0f+i/1.0f,max(0.0f,128.0f-FPSHistory[i][1]*0.5f));
			nextPos3 = Vector2f(Screen.Width-600.0f+i/1.0f,max(0.0f,128.0f-FPSHistory[i][2]*0.5f));

			if (i > 0) {
				Draw.Line2D(nextPos1,prevPos1,Vector4f(1,0,0,1));
				Draw.Line2D(nextPos2,prevPos2,Vector4f(0,1,0,1));
				if (Network.IsConnected) {
					Draw.Line2D(nextPos3,prevPos3,Vector4f(0,0,1,1));
				}
			}

			prevPos1 = nextPos1;
			prevPos2 = nextPos2;
			prevPos3 = nextPos3;
		}
	Screen.End2D();
}

void Perfomance_Display::DoPedTest() {
	Vector3f collisionPoint;
	if (Clients.LocalClient == BAD_ID) return;

	int p = Clients[Clients.LocalClient]->PlayerPed;
	if (!Peds[p]) return;

	Map.traceSegment(
		Vector3f(Peds[p]->Position.x,Peds[p]->Position.y,Peds[p]->Position.z+0.1f),
		Vector3f(Peds[p]->Position.x,Peds[p]->Position.y,Peds[p]->Position.z-0.1f),&collisionPoint);

	Fonts.SetAlign(FONTALIGN_RIGHT);
	Fonts.printf(Fonts.Message,Vector2f(Screen.Width-8.0f,16.0f),
		"Player ped %d\n"
		"Camera ped %d\n"
		"X %.2f\n"
		"Y %.2f\n"
		"Z %.2f\n"
		"Collides %d\n"
		"Anim %d\n"
		"Z under me %f\n",
		Clients[Clients.LocalClient]->PlayerPed,
		Clients[Clients.LocalClient]->PlayerPed,
		Peds[p]->Position.x,
		Peds[p]->Position.y,
		Peds[p]->Position.z,
		Map.isSolid(Peds[p]->Position),
		Peds[p]->currentAnimation,
		collisionPoint.z);
	Fonts.Reset();

	if (Convar.GetInt("perf_ped_enablekeys",1)) {
		if (glfwGetKey('Q')) Peds[p]->Velocity.z = 2.0f;
		if (glfwGetKey('Z')) Peds[p]->Velocity.z = -2.0f;
		if (glfwGetKey('W')) Peds[p]->Velocity.y = -16.0f;
		if (glfwGetKey('S')) Peds[p]->Velocity.y =  16.0f;
		if (glfwGetKey('A')) Peds[p]->Velocity.x = -16.0f;
		if (glfwGetKey('D')) Peds[p]->Velocity.x =  16.0f;
	}

	if (Convar.GetInt("perf_ped_enabletrace",0)) {
		if (Map.traceSegment(
			Vector3f(Peds[p]->Position.x,Peds[p]->Position.y     ,Peds[p]->Position.z+0.1f),
			Vector3f(Peds[p]->Position.x,Peds[p]->Position.y+1.0f,Peds[p]->Position.z+0.1f),&collisionPoint)) {
			Draw.Line3D(Peds[p]->Position,collisionPoint,Vector4f(0.0f,0.9f,0.1f,1.0f));
		} else {
			Draw.Line3D(Peds[p]->Position,Peds[p]->Position+Vector3f(0.0f,1.0f,0.1f),Vector4f(0.9f,0.0f,0.1f,1.0f));
		}

		if (Map.traceSegment(
			Vector3f(Peds[p]->Position.x,Peds[p]->Position.y     ,Peds[p]->Position.z+0.1f),
			Vector3f(Peds[p]->Position.x,Peds[p]->Position.y-1.0f,Peds[p]->Position.z+0.1f),&collisionPoint)) {
			Draw.Line3D(Peds[p]->Position,collisionPoint,Vector4f(0.0f,0.9f,0.1f,1.0f));
		} else {
			Draw.Line3D(Peds[p]->Position,Peds[p]->Position+Vector3f(0.0f,-1.0f,0.1f),Vector4f(0.9f,0.0f,0.1f,1.0f));
		}

		if (Map.traceSegment(
			Vector3f(Peds[p]->Position.x,Peds[p]->Position.y     ,Peds[p]->Position.z+0.1f),
			Vector3f(Peds[p]->Position.x+1.0f,Peds[p]->Position.y,Peds[p]->Position.z+0.1f),&collisionPoint)) {
			Draw.Line3D(Peds[p]->Position,collisionPoint,Vector4f(0.0f,0.9f,0.1f,1.0f));
		} else {
			Draw.Line3D(Peds[p]->Position,Peds[p]->Position+Vector3f(1.0f,0.0f,0.1f),Vector4f(0.9f,0.0f,0.1f,1.0f));
		}

		if (Map.traceSegment(
			Vector3f(Peds[p]->Position.x,Peds[p]->Position.y     ,Peds[p]->Position.z+0.1f),
			Vector3f(Peds[p]->Position.x-1.0f,Peds[p]->Position.y,Peds[p]->Position.z+0.1f),&collisionPoint)) {
			Draw.Line3D(Peds[p]->Position,collisionPoint,Vector4f(0.0f,0.9f,0.1f,1.0f));
		} else {
			Draw.Line3D(Peds[p]->Position,Peds[p]->Position+Vector3f(-1.0f,0.0f,0.1f),Vector4f(0.9f,0.0f,0.1f,1.0f));
		}
	}

	/*if (Map.traceSegment(
		Vector3f(Peds[p]->Position.x,Peds[p]->Position.y     ,Peds[p]->Position.z+0.5f),
		Vector3f(Peds[p]->Position.x+1.0f,Peds[p]->Position.y,Peds[p]->Position.z+0.1f),&collisionPoint)) {
		Draw.Line3D(Peds[p]->Position+Vector3f(1.0f,0.0f,0.5f),collisionPoint,Vector4f(0.0f,0.9f,0.1f,1.0f));
	} else {
		Draw.Line3D(Peds[p]->Position+Vector3f(1.0f,0.0f,0.5f),Peds[p]->Position+Vector3f(1.0f,0.0f,0.1f),Vector4f(0.9f,0.0f,0.1f,1.0f));
	}*/
};
