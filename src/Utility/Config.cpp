#include "Config.h"
#include "COMException.h"

Config::Config()
{
	std::fstream cfg_file;
	cfg_file.open("config.cfg", std::ios::in);

	if (cfg_file.is_open())
	{
		std::string line;
		CFG_Group group = CFG_Group::UNDEFINED;
		
		while (std::getline(cfg_file, line))
		{
			if (line[0] == '[')
			{
				if (line.compare("[DISPLAY]") == 0)
				{
					group = CFG_Group::DISPLAY;
				}
				else if (line.compare("[SOUND]") == 0)
				{
					group = CFG_Group::SOUND;
				}
				else if (line.compare("[CONTROL]") == 0)
				{
					group = CFG_Group::CONTROL;
				}
				else
				{
					group = CFG_Group::UNDEFINED;
				}
			}
			else if (group != CFG_Group::UNDEFINED)
			{
				std::string key;
				std::string value;

				size_t deli = line.find('=');

				if (deli != -1)
				{
					key = line.substr(0, deli);
					deli++;
					value = line.substr(deli, line.length());

					m_config.insert(std::pair<CFG_Key, std::string>(CFG_Key(group, key), value));
				}
			}
			else
			{
				ErrorLogger::Log("Malformed configuration file.");
			}
		}

		cfg_file.close();
	}
}

Config::~Config()
{
	if (m_is_pending)
	{
		Flush();
	}

	if (!m_config.empty())
	{
		std::fstream cfg_file;
		cfg_file.open("config.cfg", std::ios::out);

		if (cfg_file.is_open())
		{
			CFG_Group group = CFG_Group::UNDEFINED;
			for (const auto &cfg_pair : m_config)
			{
				if (cfg_pair.first.group != group)
				{
					group = cfg_pair.first.group;
					switch (group)
					{
					case CFG_Group::DISPLAY:
						cfg_file << "[DISPLAY]\n";
						break;
					case CFG_Group::SOUND:
						cfg_file << "[SOUND]\n";
						break;
					case CFG_Group::CONTROL:
						cfg_file << "[CONTROL]\n";
						break;
					}
				}
				cfg_file << cfg_pair.first.key << "=" << cfg_pair.second << "\n";
			}
			cfg_file.close();
		}
	}
}

void Config::Flush()
{
	while (!m_pending.empty())
	{
		auto pair = m_pending.back();

		m_config[pair.first] = pair.second;

		m_pending.pop_back();
	}

	m_is_pending = false;
}

void Config::RevertChanges()
{
	m_pending.clear();

	m_is_pending = false;
}

void Config::ChangeConfigValue(CFG_Key key, std::string value)
{
	m_pending.push_back(std::pair<CFG_Key, std::string>(key, value));

	m_is_pending = true;
}

bool Config::GetConfigValue(CFG_Key key, std::string& value)
{
	auto it = m_config.find(key);
	if (it != m_config.end())
	{
		value = it->second;
		return true;
	}
	else
	{
		return false;
	}
}
