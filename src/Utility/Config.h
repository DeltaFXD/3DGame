#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "ErrorLogger.h"

enum class CFG_Group : unsigned char
{
	DISPLAY,
	SOUND,
	CONTROL,
	UNDEFINED
};

struct CFG_Key
{
	CFG_Group group;
	std::string key;

	CFG_Key(CFG_Group group = CFG_Group::UNDEFINED, std::string key = "") : group(group), key(key) {}

	constexpr bool operator()(const CFG_Key &lhs, const CFG_Key &rhs) const
	{
		return (lhs.group < rhs.group) || ((lhs.group == rhs.group) && (lhs.key < rhs.key));
	}
};

class Config
{
public:
	Config(Config const&) = delete;
	void operator=(Config const&) = delete;

	~Config();

	static Config& GetConfig()
	{
		static Config m_instance;

		return m_instance;
	}

	void Flush();
	void RevertChanges();
	void ChangeConfigValue(CFG_Key key, std::string value);

	bool GetConfigValue(CFG_Key key, std::string& value);
private:
	Config();

	bool m_is_pending = false;
	
	std::vector<std::pair<CFG_Key, std::string>> m_pending;
	std::map<CFG_Key, std::string, CFG_Key> m_config;
};