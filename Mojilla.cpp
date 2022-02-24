#include "pch.h"
#include "Mojilla.h"
#include <sstream>
#include <bitset>
#include <vector>
#include <map>

BAKKESMOD_PLUGIN(Mojilla, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void Mojilla::onLoad()
{
	_globalCvarManager = cvarManager;
	cvarManager->registerNotifier("kazuryu_test", [this](std::vector < std::string > commands) {
		if (!gameWrapper->IsInOnlineGame())
			return;
		ServerWrapper server = gameWrapper->GetOnlineGame();
		ArrayWrapper<PriWrapper> pls = server.GetPRIs();
		for (int i = 0; i < pls.Count(); i++) {
			PriWrapper priw = pls.Get(i);
			cvarManager->log(priw.GetOldName().ToString());
			cvarManager->log(std::to_string(priw.GetOldName().ToString().length()));
			UTFCheck utf;
			_name = utf.get(priw.GetOldName().ToString());
			cvarManager->log(priw.GetUniqueIdWrapper().GetIdString());
			cvarManager->log(std::to_string(priw.GetPlayerID()));
			cvarManager->log("--------");
		}

		gameWrapper->HookEvent("Function TAGame.GFxData_GameEvent_TA.OnOpenScoreboard", std::bind(&Mojilla::openScoreboard, this, std::placeholders::_1));
		gameWrapper->HookEvent("Function TAGame.GFxData_GameEvent_TA.OnCloseScoreboard", std::bind(&Mojilla::closeScoreboard, this, std::placeholders::_1));
		}, "test", PERMISSION_ALL);
	cvarManager->executeCommand("kazuryu_test");
	//cvarManager->log("Plugin loaded!");

	//cvarManager->registerNotifier("my_aweseome_notifier", [&](std::vector<std::string> args) {
	//	cvarManager->log("Hello notifier!");
	//}, "", 0);

	//auto cvar = cvarManager->registerCvar("template_cvar", "hello-cvar", "just a example of a cvar");
	//auto cvar2 = cvarManager->registerCvar("template_cvar2", "0", "just a example of a cvar with more settings", true, true, -10, true, 10 );

	//cvar.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) {
	//	cvarManager->log("the cvar with name: " + cvarName + " changed");
	//	cvarManager->log("the new value is:" + newCvar.getStringValue());
	//});

	//cvar2.addOnValueChanged(std::bind(&Mojilla::YourPluginMethod, this, _1, _2));

	// enabled decleared in the header
	//enabled = std::make_shared<bool>(false);
	//cvarManager->registerCvar("TEMPLATE_Enabled", "0", "Enable the TEMPLATE plugin", true, true, 0, true, 1).bindTo(enabled);

	//cvarManager->registerNotifier("NOTIFIER", [this](std::vector<std::string> params){FUNCTION();}, "DESCRIPTION", PERMISSION_ALL);
	//cvarManager->registerCvar("CVAR", "DEFAULTVALUE", "DESCRIPTION", true, true, MINVAL, true, MAXVAL);//.bindTo(CVARVARIABLE);
	//gameWrapper->HookEvent("FUNCTIONNAME", std::bind(&TEMPLATE::FUNCTION, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("FUNCTIONNAME", std::bind(&Mojilla::FUNCTION, this, _1, _2, _3));
	//gameWrapper->RegisterDrawable(bind(&TEMPLATE::Render, this, std::placeholders::_1));


	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", [this](std::string eventName) {
	//	cvarManager->log("Your hook got called and the ball went POOF");
	//});
	// You could also use std::bind here
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&Mojilla::YourPluginMethod, this);
}

void Mojilla::onUnload()
{
}

void Mojilla::render(CanvasWrapper canvas) {
	canvas_size = gameWrapper->GetScreenSize();
	if (float(canvas_size.X) / float(canvas_size.Y) > 1.5f) scale = 0.507f * canvas_size.Y / SCOREBOARD_HEIGHT;
	else scale = 0.615f * canvas_size.X / SCOREBOARD_WIDTH;
	uiScale = gameWrapper->GetUIScale();
	Vector2F center = Vector2F{ float(canvas_size.X) / 2, float(canvas_size.Y) / 2 };
	float mutators_center = canvas_size.X - 1005.0f * scale * uiScale;
	std::filesystem::path dataFolder = gameWrapper->GetDataFolderW();
	dataFolder = dataFolder / "assets";
	canvas.SetPosition(Vector2F{center.X - SCOREBOARD_WIDTH/2.0f,center.Y});
	canvas.DrawTexture(std::make_shared<ImageWrapper>(dataFolder / "png" / "3042.png",true).get(),0.5f);
	canvas.SetColor(255, 255, 255, 255);
	canvas.SetPosition(Vector2{ 0,0 });
	for (int i = 0; i < _name.size(); i++) {
		cvarManager->log(_name[i]);
		canvas.SetPosition(Vector2{ i * (52 - 10) + 500,500 });
		auto pic = std::make_shared<ImageWrapper>(dataFolder / "png" / (_name[i] + ".png"), true);
		canvas.DrawTexture(pic.get(), 0.5f);
	}
}

void Mojilla::openScoreboard(std::string eventName) {
	//-----Black Magic------------
	gameWrapper->UnregisterDrawables();
	gameWrapper->RegisterDrawable(std::bind(&Mojilla::render, this, std::placeholders::_1));
}

void Mojilla::closeScoreboard(std::string eventName) {
	gameWrapper->UnregisterDrawables();
}