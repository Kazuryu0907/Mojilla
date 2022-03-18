#include "pch.h"
#include "Mojilla.h"
#include <sstream>
#include <bitset>
#include <vector>
#include <map>
#include <algorithm>

BAKKESMOD_PLUGIN(Mojilla, "文字化けを回避するPlugin", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void Mojilla::onLoad()
{
	_globalCvarManager = cvarManager;
	cvarManager->registerNotifier("Mojilla", [this](std::vector < std::string > commands) {
		if (!gameWrapper->IsInOnlineGame())
			return;
		ServerWrapper server = gameWrapper->GetOnlineGame();
		
		gameWrapper->HookEvent("Function TAGame.GFxData_GameEvent_TA.OnOpenScoreboard", std::bind(&Mojilla::openScoreboard, this, std::placeholders::_1));
		gameWrapper->HookEvent("Function TAGame.GFxData_GameEvent_TA.OnCloseScoreboard", std::bind(&Mojilla::closeScoreboard, this, std::placeholders::_1));
		gameWrapper->HookEvent("Function TAGame.PRI_TA.OnTeamChanged", std::bind(&Mojilla::teamUpdate, this, std::placeholders::_1));//std::bind(&Mojilla::teamUpdate, this, std::placeholders::_1)
		//gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerAdded", [this](std::string eventName) {cvarManager->log("Add"); });
		//gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Active.StartRound",std::bind(&Mojilla::clear,this));
		//gameWrapper->HookEvent("Function OnlineGameJoinGame_X.JoiningBase.IsJoiningGame", [this](std::string eventName) {cvarManager->log("Join"); });
		}, "test", PERMISSION_ALL);
	cvarManager->executeCommand("Mojilla");
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
	gameWrapper->UnhookEvent("Function TAGame.GFxData_GameEvent_TA.OnOpenScoreboard");
	gameWrapper->UnhookEvent("Function TAGame.GFxData_GameEvent_TA.OnCloseScoreboard");
	gameWrapper->UnhookEvent("Function TAGame.PRI_TA.OnTeamChanged");
}

void Mojilla::clear() {
	leaderboard.clear();
	imgPointers.clear();
	nameUTFTable.clear();
	namesMap.clear();
	cvarManager->log("clear");
}
void Mojilla::teamUpdate(std::string) {
	ServerWrapper sw = gameWrapper->IsInOnlineGame() ? gameWrapper->GetOnlineGame() : gameWrapper->GetGameEventAsServer();
	if (sw.IsNull())return;
	ArrayWrapper<PriWrapper> pls = sw.GetPRIs();
	leaderboard.clear();
	blueteamNum = 0;
	for (int i = 0; i < pls.Count(); i++) {
		PriWrapper pl = pls.Get(i);
		if (pl.IsNull())continue;

		std::string rawName = pl.GetOldName().ToString();
		std::string nameKey = getKey(pl);
		UTFCheck utf;
		if (pl.GetTeamNum() == 255)continue;// spectate
		pri p = { nameKey,pl.GetMatchScore(),pl.GetTeamNum() == 0,utf.isUTF(),rawName };
		if (pl.GetTeamNum() == 0)blueteamNum++;
		leaderboard.push_back(p);
		if (namesMap.find(nameKey) == namesMap.end()) { //not exist
			std::vector<std::string> name = utf.get(rawName);
			std::filesystem::path dataFolder = gameWrapper->GetDataFolderW();
			dataFolder = dataFolder / "assets";
			for (std::string str : name) {
				bool isUTF = false;
				if (str.find("u\\") != std::string::npos) {//is exist
					isUTF = true;
					str = str.erase(0,2);
				}
				auto pic = std::make_shared<ImageWrapper>(dataFolder / "png" / (str + ".png"), true);
				imgPointers[nameKey].push_back(pic);	
				nameUTFTable[nameKey].push_back(isUTF);
			}
			namesMap[nameKey] = name;
		}
	}
	removeNonActive();
}

std::string Mojilla::getKey(PriWrapper pl) {
	UniqueIDWrapper uidw = pl.GetUniqueIdWrapper();
	std::string rawName = pl.GetOldName().ToString();
	bool isBot = pl.GetbBot();
	std::string nameKey = uidw.GetIdString();// name for key
	if (isBot)nameKey = rawName;			 // Bot hasn't uid
	return nameKey;
}

void Mojilla::scoreUpdate() {
	if (!isFirst)return;					// Run once
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
	for (int i = 0; i < leaderboard.size(); i++) {	
		auto key = leaderboard[i].uid;
		leaderboard[i].score = tempMap[key];
	}
}
void Mojilla::render(CanvasWrapper canvas) {
	canvas_size = gameWrapper->GetScreenSize();
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
		auto name = imgPointers[key];
		auto UTFTable = nameUTFTable[key];
		int i = 0;
		int offset = int((blueteamNum+2) * 57. -43)+3;
		int UTFoffset = 0;
		int stackPositionX = 1500;
		bool preUTF = false;
		for (auto pic:name) {
			UTFoffset = !UTFTable[i] ? 10 : 0;//isn't UTF
			//blue
			if(!preUTF && UTFTable[i])stackPositionX -= 10;
			if (preUTF && !UTFTable[i])stackPositionX += 5;
			if (stackPositionX > canvas_size.X)stackPositionX = 0;
			if (blueteamNum - 1 >= k)canvas.SetPosition(Vector2{ stackPositionX,int(canvas_size.Y / 2. - int(offset) + k * 57.) });//blue
			if(blueteamNum-1 < k)canvas.SetPosition(Vector2{ stackPositionX,int(canvas_size.Y / 2. + 22 + (static_cast<unsigned __int64>(k)-blueteamNum) * 57.) });//orange
			canvas.DrawTexture(pic.get(), 0.5f);
			stackPositionX += int(52 - 10 - UTFoffset);
			preUTF = UTFTable[i];
			i++;
			
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

	for (std::string key : leaderboardPls){
		auto fi = std::find_if(leaderboard.begin(), leaderboard.end(), [key](pri& p) {return(p.uid == key); });
		if (fi == leaderboard.end())continue;
		int index = std::distance(leaderboard.begin(),fi);
		leaderboard.erase(leaderboard.begin() + index);
		imgPointers.erase(key);
		if(isFirst)cvarManager->log("erase->"+key);
	}
}
void Mojilla::sortLeaderboard(){
	std::sort(leaderboard.begin(), leaderboard.end(), [](const pri& a, const pri& b) {return (a.score == b.score ? a.rawName < b.rawName : a.score > b.score); });
	std::sort(leaderboard.begin(), leaderboard.end(), [](const pri& a, const pri& b) {return (a.team > b.team); });
}
void Mojilla::openScoreboard(std::string eventName) {
	removeNonActive();
	scoreUpdate();
	sortLeaderboard();
	isFirst = false;
	//-----Black Magic------------
	gameWrapper->UnregisterDrawables();
	gameWrapper->RegisterDrawable(std::bind(&Mojilla::render, this, std::placeholders::_1));
}

void Mojilla::closeScoreboard(std::string eventName) {
	gameWrapper->UnregisterDrawables();
	isFirst = true;
}