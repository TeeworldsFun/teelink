/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_SHARED_CONFIG_VARIABLES_H
#define ENGINE_SHARED_CONFIG_VARIABLES_H
#undef ENGINE_SHARED_CONFIG_VARIABLES_H // this file will be included several times

// TODO: remove this
#include "././game/variables.h"


MACRO_CONFIG_STR(PlayerName, player_name, 16, "nameless tee", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Name of the player")
MACRO_CONFIG_STR(PlayerClan, player_clan, 12, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Clan of the player")
MACRO_CONFIG_INT(PlayerCountry, player_country, -1, -1, 1000, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Country of the player")
MACRO_CONFIG_STR(Password, password, 32, "", CFGFLAG_CLIENT|CFGFLAG_SERVER, "Password to the server")
MACRO_CONFIG_STR(Logfile, logfile, 128, "", CFGFLAG_SAVE|CFGFLAG_CLIENT|CFGFLAG_SERVER, "Filename to log all output to")
MACRO_CONFIG_INT(ConsoleOutputLevel, console_output_level, 0, 0, 2, CFGFLAG_CLIENT|CFGFLAG_SERVER, "Adjusts the amount of information in the console")

MACRO_CONFIG_INT(ClCpuThrottle, cl_cpu_throttle, 0, 0, 100, CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(ClEditor, cl_editor, 0, 0, 1, CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(ClLoadCountryFlags, cl_load_country_flags, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Load and show country flags")

MACRO_CONFIG_INT(ClAutoDemoRecord, cl_auto_demo_record, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Automatically record demos")
MACRO_CONFIG_INT(ClAutoDemoMax, cl_auto_demo_max, 10, 0, 1000, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Maximum number of automatically recorded demos (0 = no limit)")
MACRO_CONFIG_INT(ClAutoScreenshot, cl_auto_screenshot, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Automatically take game over screenshot")
MACRO_CONFIG_INT(ClAutoScreenshotMax, cl_auto_screenshot_max, 10, 0, 1000, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Maximum number of automatically created screenshots (0 = no limit)")

MACRO_CONFIG_INT(ClEventthread, cl_eventthread, 0, 0, 1, CFGFLAG_CLIENT, "Enables the usage of a thread to pump the events")

MACRO_CONFIG_INT(InpGrab, inp_grab, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Use forceful input grabbing method")

MACRO_CONFIG_STR(BrFilterString, br_filter_string, 25, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Server browser filtering string")
MACRO_CONFIG_INT(BrFilterFull, br_filter_full, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Filter out full server in browser")
MACRO_CONFIG_INT(BrFilterEmpty, br_filter_empty, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Filter out empty server in browser")
MACRO_CONFIG_INT(BrFilterSpectators, br_filter_spectators, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Filter out spectators from player numbers")
MACRO_CONFIG_INT(BrFilterFriends, br_filter_friends, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Filter out servers with no friends")
MACRO_CONFIG_INT(BrFilterCountry, br_filter_country, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Filter out servers with non-matching player country")
MACRO_CONFIG_INT(BrFilterCountryIndex, br_filter_country_index, -1, -1, 999, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Player country to filter by in the server browser")
MACRO_CONFIG_INT(BrFilterPw, br_filter_pw, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Filter out password protected servers in browser")
MACRO_CONFIG_INT(BrFilterPing, br_filter_ping, 999, 0, 999, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Ping to filter by in the server browser")
MACRO_CONFIG_STR(BrFilterGametype, br_filter_gametype, 128, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Game types to filter")
MACRO_CONFIG_INT(BrFilterGametypeStrict, br_filter_gametype_strict, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Strict gametype filter")
MACRO_CONFIG_STR(BrFilterServerAddress, br_filter_serveraddress, 128, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Server address to filter")
MACRO_CONFIG_INT(BrFilterPure, br_filter_pure, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Filter out non-standard servers in browser")
MACRO_CONFIG_INT(BrFilterPureMap, br_filter_pure_map, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Filter out non-standard maps in browser")
MACRO_CONFIG_INT(BrFilterCompatversion, br_filter_compatversion, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Filter out non-compatible servers in browser")

MACRO_CONFIG_INT(BrSort, br_sort, 0, 0, 256, CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(BrSortOrder, br_sort_order, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(BrMaxRequests, br_max_requests, 25, 0, 1000, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Number of requests to use when refreshing server browser")

MACRO_CONFIG_INT(SndBufferSize, snd_buffer_size, 512, 128, 32768, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Sound buffer size")
MACRO_CONFIG_INT(SndRate, snd_rate, 48000, 0, 0, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Sound mixing rate")
MACRO_CONFIG_INT(SndEnable, snd_enable, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Sound enable")
MACRO_CONFIG_INT(SndMusic, snd_enable_music, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Play background music")
MACRO_CONFIG_INT(SndVolume, snd_volume, 100, 0, 100, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Sound volume")
MACRO_CONFIG_INT(SndDevice, snd_device, -1, 0, 0, CFGFLAG_SAVE|CFGFLAG_CLIENT, "(deprecated) Sound device to use")

MACRO_CONFIG_INT(SndNonactiveMute, snd_nonactive_mute, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_INT(GfxScreenWidth, gfx_screen_width, 0, 0, 0, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Screen resolution width")
MACRO_CONFIG_INT(GfxScreenHeight, gfx_screen_height, 0, 0, 0, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Screen resolution height")
MACRO_CONFIG_INT(GfxBorderless, gfx_borderless, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Borderless window (not to be used with fullscreen)")
MACRO_CONFIG_INT(GfxFullscreen, gfx_fullscreen, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Fullscreen")
MACRO_CONFIG_INT(GfxAlphabits, gfx_alphabits, 0, 0, 0, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Alpha bits for framebuffer (fullscreen only)")
MACRO_CONFIG_INT(GfxColorDepth, gfx_color_depth, 24, 16, 24, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Colors bits for framebuffer (fullscreen only)")
MACRO_CONFIG_INT(GfxClear, gfx_clear, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Clear screen before rendering")
MACRO_CONFIG_INT(GfxVsync, gfx_vsync, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Vertical sync")
MACRO_CONFIG_INT(GfxDisplayAllModes, gfx_display_all_modes, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(GfxTextureCompression, gfx_texture_compression, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Use texture compression")
MACRO_CONFIG_INT(GfxHighDetail, gfx_high_detail, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "High detail")
MACRO_CONFIG_INT(GfxTextureQuality, gfx_texture_quality, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(GfxFsaaSamples, gfx_fsaa_samples, 0, 0, 16, CFGFLAG_SAVE|CFGFLAG_CLIENT, "FSAA Samples")
MACRO_CONFIG_INT(GfxRefreshRate, gfx_refresh_rate, 0, 0, 0, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Screen refresh rate")
MACRO_CONFIG_INT(GfxFinish, gfx_finish, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(GfxAsyncRender, gfx_asyncrender, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Do rendering async from the the update")

MACRO_CONFIG_INT(GfxThreaded, gfx_threaded, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Use the threaded graphics backend")

MACRO_CONFIG_INT(InpMousesens, inp_mousesens, 100, 5, 100000, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Mouse sensitivity")

MACRO_CONFIG_STR(SvName, sv_name, 128, "unnamed server", CFGFLAG_SERVER, "Server name")
MACRO_CONFIG_STR(Bindaddr, bindaddr, 128, "", CFGFLAG_CLIENT|CFGFLAG_SERVER|CFGFLAG_MASTER, "Address to bind the client/server to")
MACRO_CONFIG_INT(SvPort, sv_port, 8303, 0, 0, CFGFLAG_SERVER, "Port to use for the server")
MACRO_CONFIG_INT(SvExternalPort, sv_external_port, 0, 0, 0, CFGFLAG_SERVER, "External port to report to the master servers")
MACRO_CONFIG_STR(SvMap, sv_map, 128, "dm1", CFGFLAG_SERVER, "Map to use on the server")
MACRO_CONFIG_INT(SvMaxClients, sv_max_clients, 8, 1, MAX_CLIENTS, CFGFLAG_SERVER, "Maximum number of clients that are allowed on a server")
MACRO_CONFIG_INT(SvMaxClientsPerIP, sv_max_clients_per_ip, 4, 1, MAX_CLIENTS, CFGFLAG_SERVER, "Maximum number of clients with the same IP that can connect to the server")
MACRO_CONFIG_INT(SvHighBandwidth, sv_high_bandwidth, 0, 0, 1, CFGFLAG_SERVER, "Use high bandwidth mode. Doubles the bandwidth required for the server. LAN use only")
MACRO_CONFIG_INT(SvRegister, sv_register, 1, 0, 1, CFGFLAG_SERVER, "Register server with master server for public listing")
MACRO_CONFIG_STR(SvRconPassword, sv_rcon_password, 32, "", CFGFLAG_SERVER, "Remote console password (full access)")
MACRO_CONFIG_STR(SvRconModPassword, sv_rcon_mod_password, 32, "", CFGFLAG_SERVER, "Remote console password for moderators (limited access)")
MACRO_CONFIG_INT(SvRconMaxTries, sv_rcon_max_tries, 3, 0, 100, CFGFLAG_SERVER, "Maximum number of tries for remote console authentication")
MACRO_CONFIG_INT(SvRconBantime, sv_rcon_bantime, 5, 0, 1440, CFGFLAG_SERVER, "The time a client gets banned if remote console authentication fails. 0 makes it just use kick")
MACRO_CONFIG_INT(SvAutoDemoRecord, sv_auto_demo_record, 0, 0, 1, CFGFLAG_SERVER, "Automatically record demos")
MACRO_CONFIG_INT(SvAutoDemoMax, sv_auto_demo_max, 10, 0, 1000, CFGFLAG_SERVER, "Maximum number of automatically recorded demos (0 = no limit)")

MACRO_CONFIG_STR(EcBindaddr, ec_bindaddr, 128, "localhost", CFGFLAG_ECON, "Address to bind the external console to. Anything but 'localhost' is dangerous")
MACRO_CONFIG_INT(EcPort, ec_port, 0, 0, 0, CFGFLAG_ECON, "Port to use for the external console")
MACRO_CONFIG_STR(EcPassword, ec_password, 32, "", CFGFLAG_ECON, "External console password")
MACRO_CONFIG_INT(EcBantime, ec_bantime, 0, 0, 1440, CFGFLAG_ECON, "The time a client gets banned if econ authentication fails. 0 just closes the connection")
MACRO_CONFIG_INT(EcAuthTimeout, ec_auth_timeout, 30, 1, 120, CFGFLAG_ECON, "Time in seconds before the the econ authentification times out")
MACRO_CONFIG_INT(EcOutputLevel, ec_output_level, 1, 0, 2, CFGFLAG_ECON, "Adjusts the amount of information in the external console")

MACRO_CONFIG_INT(Debug, debug, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SERVER, "Debug mode")
MACRO_CONFIG_INT(DbgStress, dbg_stress, 0, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SERVER, "Stress systems")
MACRO_CONFIG_INT(DbgStressNetwork, dbg_stress_network, 0, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SERVER, "Stress network")
MACRO_CONFIG_INT(DbgPref, dbg_pref, 0, 0, 1, CFGFLAG_SERVER, "Performance outputs")
MACRO_CONFIG_INT(DbgGraphs, dbg_graphs, 0, 0, 1, CFGFLAG_CLIENT, "Performance graphs")
MACRO_CONFIG_INT(DbgHitch, dbg_hitch, 0, 0, 0, CFGFLAG_SERVER, "Hitch warnings")
MACRO_CONFIG_STR(DbgStressServer, dbg_stress_server, 32, "localhost", CFGFLAG_CLIENT, "Server to stress")
MACRO_CONFIG_INT(DbgResizable, dbg_resizable, 0, 0, 0, CFGFLAG_CLIENT, "Enables window resizing")

/** H-Client **/
//MACRO_CONFIG_INT(hc3DRender, hc_3d_render, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enables/Disable 3D Render")
/*
1 - 3.5
2 - 3.5.1
3 - 3.5.2
4 - 3.5.3
5 - 3.6
*/
MACRO_CONFIG_INT(hcVersionCode, hc_version_code, 5, 1, 99, CFGFLAG_SAVE|CFGFLAG_CLIENT, "H-Client Version Code")
MACRO_CONFIG_INT(hcAutoUpdate, hc_auto_update, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Auto-Update")
MACRO_CONFIG_INT(hcAutoDownloadSkins, hc_auto_download_skins, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Auto-Download Skins from DDNet Database")
MACRO_CONFIG_STR(hcAutoDownloadSkinsSpeed, hc_auto_download_skins_speed, 5, "1", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Auto-Download Skins Speed (KiBps)")
MACRO_CONFIG_INT(hcUseHUD, hc_use_hud, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Uses H-Client HUD")
MACRO_CONFIG_INT(hcColorClan, hc_color_clan, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "HighLight Clan Members")
MACRO_CONFIG_INT(hcChatEmoticons, hc_chat_emoticons, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable chat emoticons")
MACRO_CONFIG_INT(hcChatColours, hc_chat_colours, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable chat colours")
MACRO_CONFIG_INT(hcChatTeamColors, hc_chat_team_colors, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable chat team colours")
MACRO_CONFIG_INT(hcGoreStyle, hc_gore_style, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable Gore Style!")
MACRO_CONFIG_INT(hcGoreStyleTeeColors, hc_gore_style_tee_colors, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable Tee Colors in Gore Style!")

MACRO_CONFIG_INT(hcLaserCustomColor, hc_laser_custom_color, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable Custom Lase Color")
MACRO_CONFIG_INT(hcLaserColorHue, hc_laser_color_hue, 127, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser color hue")
MACRO_CONFIG_INT(hcLaserColorSat, hc_laser_color_sat, 127, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser color saturation")
MACRO_CONFIG_INT(hcLaserColorLht, hc_laser_color_lht, 127, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser color lightness")
MACRO_CONFIG_INT(hcLaserColorAlpha, hc_laser_color_alpha, 190, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser alpha")

MACRO_CONFIG_INT(hcDisableChatSoundNotification, hc_disable_chat_sound_notification, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Disable Chat Sound Notification")
MACRO_CONFIG_INT(hcPlayerInfo, hc_player_info, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show Player Info")
MACRO_CONFIG_INT(hcShowPreviewMap, hc_show_preview_map, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show Preview Map")

MACRO_CONFIG_STR(hcEyesSelectorTime, hc_eyes_selector_time, 4, "3", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Eyes Selector Time")

MACRO_CONFIG_INT(ddrShowHiddenWays, ddrace_show_hidden_ways, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Show hidden ways")
MACRO_CONFIG_INT(ddrShowTeeDirection, ddrace_show_tee_direction, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "View Tee Direcion")
MACRO_CONFIG_INT(ddrPreventPrediction, ddr_prevent_prediction, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Prevent Predection When Freeze")
MACRO_CONFIG_STR(ddrTimeoutHash, ddrace_timeout_hash, 16, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "DDRace Timeout Hash")

// Editor Vars
MACRO_CONFIG_INT(hcEditorDrawRouteAccuracy, hc_editor_draw_route_accuracy, 5, 0, 100, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Editor Draw Route Accuracy")

// Theme Vars
MACRO_CONFIG_STR(hcTheme, theme, 25, "default", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_STR(hcListHeaderBackgroundColor, list_header_background_color, 9, "FFFFFF44", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcListHeaderTextColor, list_header_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcListFooterBackgroundColor, list_footer_background_color, 9, "FFFFFF44", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcListFooterTextColor, list_footer_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcListBackgroundColor, list_background_color, 9, "00000099", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcListTextColor, list_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcListItemSelectedColor, list_item_selected_color, 9, "FFFFFF99", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcListItemOddColor, list_item_odd_color, 9, "FFFFFF33", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcListColumnSelectedColor, list_column_selected_color, 9, "FFFFFF99", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_STR(hcServerbrowserListGroupHeaderBackgroundColor, serverbrowser_list_group_header_background_color, 9, "0078FAA6", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcServerbrowserListGroupHeaderTextColor, serverbrowser_list_group_header_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcServerbrowserListExtraInfoBackgroundColor, serverbrowser_list_extra_info_background_color, 9, "FFFFFF99", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcServerbrowserListExtraInfoTextColor, serverbrowser_list_extra_info_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_STR(hcContainerHeaderBackgroundColor, container_header_background_color, 9, "FFFFFF99", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcContainerHeaderTextColor, container_header_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcContainerBackgroundColor, container_background_color, 9, "FFFFFF99", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcContainerTextColor, container_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_STR(hcSubcontainerHeaderBackgroundColor, subcontainer_header_background_color, 9, "FFFFFF99", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcSubcontainerHeaderTextColor, subcontainer_header_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcSubcontainerBackgroundColor, subcontainer_background_color, 9, "FFFFFF33", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcSubcontainerTextColor, subcontainer_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_STR(hcPopupHeaderBackgroundColor, popup_header_background_color, 9, "FFFFFF55", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcPopupHeaderTextColor, popup_header_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcPopupBackgroundColor, popup_background_color, 9, "00000055", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcPopupTextColor, popup_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_STR(hcEditboxBackgroundColor, editbox_background_color, 9, "FFFFFF99", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcEditboxTextColor, editbox_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_STR(hcButtonBackgroundColor, button_background_color, 9, "FFFFFF99", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcButtonTextColor, button_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_STR(hcTrackbarBackgroundColor, trackbar_background_color, 9, "99999999", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcTrackbarSliderBackgroundColor, trackbar_slider_background_color, 9, "FFFFFF99", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_STR(hcProgressbarBackgroundColor, progressbar_background_color, 9, "00000099", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcProgressbarSliderBackgroundColor, progressbar_slider_background_color, 9, "FFFFFF99", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_STR(hcMainmenuTextColor, mainmenu_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcMainmenuBackgroundTopColor, mainmenu_background_top_color, 9, "3360A7FF", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcMainmenuBackgroundBottomColor, mainmenu_background_bottom_color, 9, "B8D8F0FF", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_STR(hcPaneltabSelectedBackgroundColor, paneltab_selected_background_color, 9, "00000099", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcPaneltabSelectedTextColor, paneltab_selected_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcPaneltabBackgroundColor, paneltab_background_color, 9, "00000044", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcPaneltabTextColor, paneltab_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")

MACRO_CONFIG_STR(hcSettingsPaneltabSelectedBackgroundColor, settings_paneltab_selected_background_color, 9, "FFFFFF99", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcSettingsPaneltabSelectedTextColor, settings_paneltab_selected_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcSettingsPaneltabBackgroundColor, settings_paneltab_background_color, 9, "FFFFFF44", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
MACRO_CONFIG_STR(hcSettingsPaneltabTextColor, settings_paneltab_text_color, 9, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "")
#endif
