#include "guis/GuiDateTimeUtil.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>

#include "ApiSystem.h"
#include "Scripting.h"
#include "Window.h"
#include "components/OptionListComponent.h"
#include "components/SwitchComponent.h"
#include "components/DateTimeEditComponent.h"
#include "guis/GuiMsgBox.h"
#include "utils/StringUtil.h"
#include "utils/TimeUtil.h"
#include "platform.h"
#include "Log.h"
#include "SystemConf.h"

void GuiDateTimeUtil::show(Window* window)
{
	window->pushGui(new GuiDateTimeUtil(window));
}

GuiDateTimeUtil::GuiDateTimeUtil(Window* window)
 : GuiSettings(window, "DATE/TIME SETTINGS")
{

	auto emuelec_timezones = std::make_shared<OptionListComponent<std::string> >(mWindow, _("TIMEZONE"), false);
	std::string currentTimezone = SystemConf::getInstance()->get("system.timezone");
	if (currentTimezone.empty())
		currentTimezone = std::string(getShOutput(R"(/usr/bin/timeinfo current_timezone)"));
	std::string a;
	for(std::stringstream ss(getShOutput(R"(/usr/bin/timeinfo timezones)")); getline(ss, a, ','); ) {
		emuelec_timezones->add(a, a, currentTimezone == a); // emuelec
	}
	addWithLabel(_("TIMEZONE"), emuelec_timezones);
	addSaveFunc([emuelec_timezones] {
		if (emuelec_timezones->changed()) {
			std::string selectedTimezone = emuelec_timezones->getSelected();
			runSystemCommand("ln -sf /usr/share/zoneinfo/" + selectedTimezone + " $(readlink /etc/localtime)", "", nullptr);
		}
		SystemConf::getInstance()->set("system.timezone", emuelec_timezones->getSelected());
	});

	// language choice
	/*
	auto language_choice = std::make_shared<OptionListComponent<std::string> >(window, _("LANGUAGE"), false);

	std::string language = SystemConf::getInstance()->get("system.language");
	if (language.empty())
		language = "en_US";

	language_choice->add("ARABIC",               "ar_YE", language == "ar_YE");
	language_choice->add("CATALÀ",               "ca_ES", language == "ca_ES");
	language_choice->add("CYMRAEG",              "cy_GB", language == "cy_GB");
	language_choice->add("DEUTSCH", 	     "de_DE", language == "de_DE");
	language_choice->add("GREEK",                "el_GR", language == "el_GR");
	language_choice->add("ENGLISH", 	     "en_US", language == "en_US" || language == "en");
	language_choice->add("ESPAÑOL", 	     "es_ES", language == "es_ES" || language == "es");
	language_choice->add("ESPAÑOL MEXICANO",     "es_MX", language == "es_MX");
	language_choice->add("BASQUE",               "eu_ES", language == "eu_ES");
	language_choice->add("FRANÇAIS",             "fr_FR", language == "fr_FR" || language == "fr");
	language_choice->add("עברית",                "he_IL", language == "he_IL");
	language_choice->add("HUNGARIAN",            "hu_HU", language == "hu_HU");
	language_choice->add("ITALIANO",             "it_IT", language == "it_IT");
	language_choice->add("JAPANESE", 	     "ja_JP", language == "ja_JP");
	language_choice->add("KOREAN",   	     "ko_KR", language == "ko_KR" || language == "ko");
	language_choice->add("NORWEGIAN BOKMAL",     "nb_NO", language == "nb_NO");
	language_choice->add("DUTCH",                "nl_NL", language == "nl_NL");
	language_choice->add("NORWEGIAN",            "nn_NO", language == "nn_NO");
	language_choice->add("OCCITAN",              "oc_FR", language == "oc_FR");
	language_choice->add("POLISH",               "pl_PL", language == "pl_PL");
	language_choice->add("PORTUGUES BRASILEIRO", "pt_BR", language == "pt_BR");
	language_choice->add("PORTUGUES PORTUGAL",   "pt_PT", language == "pt_PT");
	language_choice->add("РУССКИЙ",              "ru_RU", language == "ru_RU");
	language_choice->add("SVENSKA", 	     "sv_SE", language == "sv_SE");
	language_choice->add("TÜRKÇE",  	     "tr_TR", language == "tr_TR");
	language_choice->add("Українська",           "uk_UA", language == "uk_UA");
	language_choice->add("简体中文", 	     "zh_CN", language == "zh_CN");
	language_choice->add("正體中文", 	     "zh_TW", language == "zh_TW");
	s->addWithLabel(_("LANGUAGE"), language_choice);
	*/

#if !defined(_ENABLEEMUELEC)
	// Timezone
#if !defined(WIN32) || defined(_DEBUG)
	auto availableTimezones = ApiSystem::getInstance()->getTimezones();
	if (availableTimezones.size() > 0)
	{
		std::string currentTZ = ApiSystem::getInstance()->getCurrentTimezone();

		bool valid_tz = false;
		for (auto list_tz : availableTimezones){
			if (currentTZ == list_tz) {
				valid_tz = true;
			}
		}
		if (!valid_tz)
			currentTZ = "Europe/Paris";

		auto tzChoices= std::make_shared<OptionListComponent<std::string> >(mWindow, _("SELECT YOUR TIME ZONE"), false);

		for (auto tz : availableTimezones)
			tzChoices->add(_(Utils::String::toUpper(tz).c_str()), tz, currentTZ == tz);

		addWithLabel(_("TIME ZONE"), tzChoices);
		addSaveFunc([tzChoices] {
			SystemConf::getInstance()->set("system.timezone", tzChoices->getSelected());
			ApiSystem::getInstance()->setTimezone(tzChoices->getSelected());
		});
	}
#endif
#endif

  	auto currentDateTime = Utils::Time::DateTime::now();

	auto tmDate = std::make_shared<DateTimeEditComponent>(mWindow, DateTimeEditComponent::DISP_DATE_TIME);
	tmDate->setValue(currentDateTime);
	addWithLabel(_("SET DATE/TIME"), tmDate);
	addSaveFunc([this, tmDate] {
		if (tmDate->changed()) {

			auto new_dt = Utils::Time::stringToTime(tmDate->getValue(), "%Y%m%dT%H%M%S");
			auto formatted_dt = Utils::Time::timeToString(new_dt, "%Y-%m-%d %H:%M:%S");

			std::string set_dt_cmd = "date -s \"" + formatted_dt + "\"";

			std::string msg = "Set the following date and time?\n" +
				formatted_dt + "\n" +
				"Note: This will have no effect when connected\n"
				"to a network with network time sync";
			mWindow->pushGui(new GuiMsgBox(mWindow, msg, _("YES"), [set_dt_cmd] {
			runSystemCommand(set_dt_cmd, "", nullptr);
			runSystemCommand("hwclock -w -u", "", nullptr);
			}, "NO",nullptr));
		}
	});

  	// Clock time format (14:42 or 2:42 pm)
	auto tmFormat = std::make_shared<SwitchComponent>(mWindow);
	tmFormat->setState(Settings::getInstance()->getBool("ClockMode12"));
	addWithLabel(_("SHOW CLOCK IN 12-HOUR FORMAT"), tmFormat);
	addSaveFunc([tmFormat] { if (tmFormat->changed()) Settings::getInstance()->setBool("ClockMode12", tmFormat->getState()); });

}
