#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "UTFCheck.hpp"
#include "version.h"
#include <unordered_map>

#define SCOREBOARD_LEFT 537
#define BLUE_BOTTOM 77
#define ORANGE_TOP 32
#define BANNER_DISTANCE 57
#define IMAGE_WIDTH 150
#define IMAGE_HEIGHT 100
#define CENTER_X 960
#define CENTER_Y 540
#define SCOREBOARD_HEIGHT 548
#define SCOREBOARD_WIDTH 1033
#define IMBALANCE_SHIFT 32
#define MUTATOR_SIZE 478
#define SKIP_TICK_SHIFT 67

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class Mojilla: public BakkesMod::Plugin::BakkesModPlugin/*, public BakkesMod::Plugin::PluginSettingsWindow*//*, public BakkesMod::Plugin::PluginWindow*/
{

	//std::shared_ptr<bool> enabled;

	//Boilerplate
	virtual void onLoad();
	virtual void onUnload();
	void render(CanvasWrapper canvas);
	void openScoreboard(std::string eventName);
	void closeScoreboard(std::string eventName);
	void teamUpdate(std::string eventName);
	void scoreUpdate();
	std::string getKey(PriWrapper);
	void removeNonActive();
	void sortLeaderboard();
	void clear();
	void initScoreboard();
	// Inherited via PluginSettingsWindow
	/*
	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;
	*/

	// Inherited via PluginWindow
	/*

	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "Mojilla";

	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
	
	*/
private:
	Vector2 canvas_size;
	struct pri {
		std::string uid;
		int score;
		unsigned char team;//isblue
		bool isUTF;
		std::string rawName;
	};
	std::vector<pri> leaderboard;
	std::unordered_map<std::string,std::vector<std::shared_ptr<ImageWrapper>>> imgPointers;
	std::unordered_map<std::string, std::vector<bool>> nameUTFTable;
	std::unordered_map<std::string, std::vector<std::string>> namesMap;
	float scale = 1.0f;
	float uiScale = 1.0f;
	bool isFirst = true;
	int blueteamNum = 0;
};

