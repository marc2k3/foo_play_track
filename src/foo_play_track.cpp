#define _WIN32_WINNT _WIN32_WINNT_WIN7
#define WINVER _WIN32_WINNT_WIN7

#include <algorithm>
#include <string>

#include <SDK/foobar2000.h>

namespace
{
	static constexpr std::string_view component_name = "Play Track";
	static constexpr std::string_view prefix = "/play_track:";

	static constexpr GUID guid_main_menu_group = { 0xef2b8edd, 0xdad4, 0x4291, { 0xb2, 0x28, 0xa2, 0x6f, 0xbf, 0x8b, 0x5, 0x1d } };

	DECLARE_COMPONENT_VERSION(
		component_name,
		"1.0.0",
		"Copyright (C) 2023 marc2003\n\n"
		"Build: " __TIME__ ", " __DATE__
	);

	VALIDATE_COMPONENT_FILENAME("foo_play_track.dll");

	class MainMenu : public mainmenu_commands
	{
	public:
		GUID get_command(uint32_t index) final
		{
			if (index >= count) FB2K_BugCheck();

			auto api = hasher_md5::get();
			hasher_md5_state state;
			api->initialize(state);
			api->process(state, &index, sizeof(index));
			return api->get_result_guid(state);
		}

		GUID get_parent() final
		{
			return guid_main_menu_group;
		}

		bool get_description(uint32_t index, pfc::string_base& out) final
		{
			if (index >= count) FB2K_BugCheck();

			get_name(index, out);
			return true;
		}

		bool get_display(uint32_t index, pfc::string_base& out, uint32_t& flags) final
		{
			if (index >= count) FB2K_BugCheck();

			auto api = playlist_manager::get();
			const size_t item_count = api->activeplaylist_get_item_count();
		
			if (item_count == 0U || (index != last && index >= item_count))
			{
				flags = mainmenu_commands::flag_disabled;
			}

			get_name(index, out);
			return true;
		}

		uint32_t get_command_count() final
		{
			return count;
		}

		void execute(uint32_t index, service_ptr_t<service_base>) final
		{
			if (index >= count) FB2K_BugCheck();

			auto api = playlist_manager::get();
			const size_t item_count = api->activeplaylist_get_item_count();
			if (item_count == 0U) return;

			if (index == last)
			{
				api->activeplaylist_execute_default_action(item_count - 1U);
			}
			else if (index < item_count)
			{
				api->activeplaylist_execute_default_action(index);
			}
		}

		void get_name(uint32_t index, pfc::string_base& out) final
		{
			if (index >= count) FB2K_BugCheck();

			if (index == last)
			{
				out = "Last";
			}
			else
			{
				out = std::to_string(index + 1U);
			}
		}

	private:
		static constexpr uint32_t count = 31U;
		static constexpr uint32_t last = count - 1U;
	};

	class CommandLineHandler : public commandline_handler
	{
	public:
		result on_token(const char* token) final
		{
			const std::string s = token;

			if (s.starts_with(prefix))
			{
				auto api = playlist_manager::get();
				const size_t item_count = api->activeplaylist_get_item_count();

				if (item_count > 0U)
				{
					const std::string index_str = s.substr(prefix.length());

					if (pfc::string_is_numeric(index_str.c_str()))
					{
						const size_t index = std::clamp<size_t>(std::stoul(index_str), 1U, item_count) - 1U;
						api->activeplaylist_execute_default_action(index);
					}
				}

				return RESULT_PROCESSED;
			}

			return RESULT_NOT_OURS;
		}
	};

	static mainmenu_group_popup_factory g_mainmenu_group(guid_main_menu_group, mainmenu_groups::playback_controls, mainmenu_commands::sort_priority_base, component_name.data());
	FB2K_SERVICE_FACTORY(MainMenu);
	FB2K_SERVICE_FACTORY(CommandLineHandler)
}
