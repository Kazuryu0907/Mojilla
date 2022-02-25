#include "pch.h"
#include "Mojilla.h"
#include <sstream>
#include <bitset>
#include <vector>
#include <map>
#include <algorithm>

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
		gameWrapper->HookEvent("Function TAGame.PRI_TA.OnTeamChanged", std::bind(&Mojilla::teamUpdate, this, std::placeholders::_1));
		gameWrapper->HookEvent("Function TAGame.PRI_TA.GetBotName", [this](std::string eventName) {cvarManager->log("BotName"); });
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

void Mojilla::teamUpdate(std::string) {
	ServerWrapper sw = gameWrapper->IsInOnlineGame() ? gameWrapper->GetOnlineGame() : gameWrapper->GetGameEventAsServer();
	if (sw.IsNull())return;
	ArrayWrapper<PriWrapper> pls = sw.GetPRIs();
	std::vector<std::string> currentPlsName;
	leaderboard.clear();
	for (int i = 0; i < pls.Count(); i++) {

		PriWrapper pl = pls.Get(i);
		if (pl.IsNull())continue;

		std::string rawName = pl.GetOldName().ToString();
		std::string nameKey = getKey(pl);

		UTFCheck utf;
		currentPlsName.push_back(getKey(pl));
		pri p = { nameKey,pl.GetMatchScore(),pl.GetTeamNum() == 0,utf.isUTF() };
		leaderboard.push_back(p);
		if (namesMap.find(nameKey) == namesMap.end()) { //not exist
			std::vector<std::string> name = utf.get(rawName);
			namesMap[nameKey] = name;
		}
	}
	/*
	for (pri p : leaderboard) {
		cvarManager->log(p.uid);
		cvarManager->log(std::to_string(p.score));
		cvarManager->log(std::to_string(p.team));
		cvarManager->log(p.isUTF ? "UTF" : "not UTF");
		cvarManager->log("-----------------------");
	}*/
}
std::string Mojilla::getKey(PriWrapper pl) {
	UniqueIDWrapper uidw = pl.GetUniqueIdWrapper();
	std::string rawName = pl.GetOldName().ToString();
	bool isBot = pl.GetbBot();
	std::string nameKey = uidw.GetIdString();// name for key

	if (isBot)nameKey = rawName;			 // Bot‚Íuid‚È‚¢‚©‚ç
	return nameKey;
}
void Mojilla::scoreUpdate() {
	if (!isFirst)return;
	ServerWrapper sw = gameWrapper->IsInOnlineGame() ? gameWrapper->GetOnlineGame() : gameWrapper->GetGameEventAsServer();
	if (sw.IsNull())return;
	ArrayWrapper<PriWrapper> pls = sw.GetPRIs();
	std::map<std::string, int> tempMap;
	std::vector<std::string> erase;
	for (int i = 0; i < pls.Count(); i++) {
		auto pl = pls.Get(i);
		if (pl.IsNull())continue;
		auto keyName = getKey(pl);
		int score = pl.GetMatchScore();
		tempMap[keyName] = score;
	}
	for (int i = 0; i < pls.Count(); i++) {	//leaderboardÁ‚·ˆ—‚Ü‚¾
		auto key = leaderboard[i].uid;
		leaderboard[i].score = tempMap[key];
	}
}
void Mojilla::render(CanvasWrapper canvas) {
	/*
	canvas_size = gameWrapper->GetScreenSize();
	if (float(canvas_size.X) / float(canvas_size.Y) > 1.5f) scale = 0.507f * canvas_size.Y / SCOREBOARD_HEIGHT;
	else scale = 0.615f * canvas_size.X / SCOREBOARD_WIDTH;
	uiScale = gameWrapper->GetUIScale();
	Vector2F center = Vector2F{ float(canvas_size.X) / 2, float(canvas_size.Y) / 2 };
	float mutators_center = canvas_size.X - 1005.0f * scale * uiScale;
	std::filesystem::path dataFolder = gameWrapper->GetDataFolderW();
	dataFolder = dataFolder / "assets";
	canvas.SetPosition(Vector2F{center.X + SCOREBOARD_WIDTH / 2.f,center.Y});
	canvas.DrawTexture(std::make_shared<ImageWrapper>(dataFolder / "png" / "3042.png",true).get(),0.5f);
	*/
	std::filesystem::path dataFolder = gameWrapper->GetDataFolderW();
	dataFolder = dataFolder / "assets";
	canvas.SetColor(255, 255, 255, 255);
	canvas.SetPosition(Vector2{ 0,0 });

	for (int k = 0; k < leaderboard.size();k++) {
		auto key = leaderboard[k].uid;
		std::vector<std::string> name = namesMap[key];
		for (int i = 0; i < name.size(); i++) {
			//cvarManager->log(_name[i]);
			canvas.SetPosition(Vector2{ i * (52 - 10) + 500,500 + k*100});
			auto pic = std::make_shared<ImageWrapper>(dataFolder / "png" / (name[i] + ".png"), true);
			canvas.DrawTexture(pic.get(), 0.5f);
		}
	}
}

void Mojilla::removeNonActive() {
	if (!isFirst)return;
	ServerWrapper sw = gameWrapper->IsInOnlineGame() ? gameWrapper->GetOnlineGame() : gameWrapper->GetGameEventAsServer();
	if (sw.IsNull())return;
	ArrayWrapper<PriWrapper> pls = sw.GetPRIs();
	std::vector<std::string> leaderboardPls;
	for (pri p : leaderboard) {
		leaderboardPls.push_back(p.uid);
	}
	for (int i = 0; i < pls.Count(); i++) {
		PriWrapper pl = pls.Get(i);
		if (pl.IsNull()) {
			cvarManager->log("pl is null");
			continue;
		}
		auto itr = std::find(leaderboardPls.begin(), leaderboardPls.end(), getKey(pl));
		if (itr == leaderboardPls.end())continue;
		int index = std::distance(leaderboardPls.begin(),itr);
		leaderboardPls.erase(leaderboardPls.begin() + index);
	}
	cvarManager->log("len->"+std::to_string(leaderboard.size()));
	for (std::string key : leaderboardPls){
		auto fi = std::find_if(leaderboard.begin(), leaderboard.end(), [key](pri& p) {return(p.uid == key); });
		if (fi == leaderboard.end())continue;
		int index = std::distance(leaderboard.begin(),fi);
		leaderboard.erase(leaderboard.begin() + index);
		cvarManager->log("erase->"+key);
	}
	for (pri p : leaderboard) {
		cvarManager->log(p.uid);
	}
}
void Mojilla::openScoreboard(std::string eventName) {
	removeNonActive();
	//scoreUpdate();
	isFirst = false;
	//-----Black Magic------------
	gameWrapper->UnregisterDrawables();
	//gameWrapper->RegisterDrawable(std::bind(&Mojilla::render, this, std::placeholders::_1));
}

void Mojilla::closeScoreboard(std::string eventName) {
	gameWrapper->UnregisterDrawables();
	isFirst = true;
}