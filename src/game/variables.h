/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_VARIABLES_H
#define GAME_VARIABLES_H
#undef GAME_VARIABLES_H // this file will be included several times


// client
MACRO_CONFIG_INT(ClPredict, cl_predict, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Predict client movements")
MACRO_CONFIG_INT(ClNameplates, cl_nameplates, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show name plates")
MACRO_CONFIG_INT(ClNameplatesAlways, cl_nameplates_always, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Always show name plates disregarding of distance")
MACRO_CONFIG_INT(ClNameplatesTeamcolors, cl_nameplates_teamcolors, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Use team colors for name plates")
MACRO_CONFIG_INT(ClNameplatesSize, cl_nameplates_size, 50, 0, 100, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Size of the name plates from 0 to 100%")
MACRO_CONFIG_INT(ClAutoswitchWeapons, cl_autoswitch_weapons, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Auto switch weapon on pickup")

MACRO_CONFIG_INT(ClShowhud, cl_showhud, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show ingame HUD")
MACRO_CONFIG_INT(ClShowChatFriends, cl_show_chat_friends, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show only chat messages from friends")
MACRO_CONFIG_INT(ClShowfps, cl_showfps, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show ingame FPS counter")

MACRO_CONFIG_INT(ClAirjumpindicator, cl_airjumpindicator, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(ClThreadsoundloading, cl_threadsoundloading, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Load sound files threaded")

MACRO_CONFIG_INT(ClWarningTeambalance, cl_warning_teambalance, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Warn about team balance")

MACRO_CONFIG_INT(ClMouseDeadzone, cl_mouse_deadzone, 300, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(ClMouseFollowfactor, cl_mouse_followfactor, 60, 0, 200, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(ClMouseMaxDistance, cl_mouse_max_distance, 800, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(EdShowkeys, ed_showkeys, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

//MACRO_CONFIG_INT(ClFlow, cl_flow, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")

MACRO_CONFIG_INT(ClShowWelcome, cl_show_welcome, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "")
MACRO_CONFIG_INT(ClMotdTime, cl_motd_time, 10, 0, 100, CFGFLAG_CLIENT|CFGFLAG_SAVE, "How long to show the server message of the day")

MACRO_CONFIG_STR(ClVersionServer, cl_version_server, 100, "version.teeworlds.com", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Server to use to check for new versions")

MACRO_CONFIG_STR(ClLanguagefile, cl_languagefile, 255, "", CFGFLAG_CLIENT|CFGFLAG_SAVE, "What language file to use")

MACRO_CONFIG_INT(PlayerUseCustomColor, player_use_custom_color, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toggles usage of custom colors")
MACRO_CONFIG_INT(PlayerColorBody, player_color_body, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player body color")
MACRO_CONFIG_INT(PlayerColorFeet, player_color_feet, 65408, 0, 0xFFFFFF, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player feet color")
MACRO_CONFIG_STR(PlayerSkin, player_skin, 24, "default", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Player skin")

MACRO_CONFIG_INT(UiPage, ui_page, 6, 0, 12, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface page")
MACRO_CONFIG_INT(UiToolboxPage, ui_toolbox_page, 0, 0, 2, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Toolbox page")
MACRO_CONFIG_STR(UiServerAddress, ui_server_address, 64, "localhost:8303", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface server address")
MACRO_CONFIG_INT(UiScale, ui_scale, 100, 50, 150, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface scale")
MACRO_CONFIG_INT(UiMousesens, ui_mousesens, 100, 5, 100000, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Mouse sensitivity for menus/editor")

MACRO_CONFIG_INT(UiColorHue, ui_color_hue, 160, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color hue")
MACRO_CONFIG_INT(UiColorSat, ui_color_sat, 70, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color saturation")
MACRO_CONFIG_INT(UiColorLht, ui_color_lht, 175, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface color lightness")
MACRO_CONFIG_INT(UiColorAlpha, ui_color_alpha, 228, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface alpha")

MACRO_CONFIG_INT(GfxNoclip, gfx_noclip, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Disable clipping")

// server
MACRO_CONFIG_INT(SvWarmup, sv_warmup, 0, 0, 0, CFGFLAG_SERVER, "Number of seconds to do warmup before round starts")
MACRO_CONFIG_STR(SvMotd, sv_motd, 900, "", CFGFLAG_SERVER, "Message of the day to display for the clients")
MACRO_CONFIG_INT(SvTeamdamage, sv_teamdamage, 0, 0, 1, CFGFLAG_SERVER, "Team damage")
MACRO_CONFIG_STR(SvMaprotation, sv_maprotation, 768, "", CFGFLAG_SERVER, "Maps to rotate between")
MACRO_CONFIG_INT(SvRoundsPerMap, sv_rounds_per_map, 1, 1, 100, CFGFLAG_SERVER, "Number of rounds on each map before rotating")
MACRO_CONFIG_INT(SvRoundSwap, sv_round_swap, 1, 0, 1, CFGFLAG_SERVER, "Swap teams between rounds")
MACRO_CONFIG_INT(SvPowerups, sv_powerups, 1, 0, 1, CFGFLAG_SERVER, "Allow powerups like ninja")
MACRO_CONFIG_INT(SvScorelimit, sv_scorelimit, 20, 0, 1000, CFGFLAG_SERVER, "Score limit (0 disables)")
MACRO_CONFIG_INT(SvTimelimit, sv_timelimit, 0, 0, 1000, CFGFLAG_SERVER, "Time limit in minutes (0 disables)")
MACRO_CONFIG_STR(SvGametype, sv_gametype, 32, "dm", CFGFLAG_SERVER, "Game type (dm, tdm, ctf)")
MACRO_CONFIG_INT(SvTournamentMode, sv_tournament_mode, 0, 0, 1, CFGFLAG_SERVER, "Tournament mode. When enabled, players joins the server as spectator")
MACRO_CONFIG_INT(SvSpamprotection, sv_spamprotection, 1, 0, 1, CFGFLAG_SERVER, "Spam protection")

MACRO_CONFIG_INT(SvRespawnDelayTDM, sv_respawn_delay_tdm, 3, 0, 10, CFGFLAG_SERVER, "Time needed to respawn after death in tdm gametype")

MACRO_CONFIG_INT(SvSpectatorSlots, sv_spectator_slots, 0, 0, MAX_CLIENTS, CFGFLAG_SERVER, "Number of slots to reserve for spectators")
MACRO_CONFIG_INT(SvTeambalanceTime, sv_teambalance_time, 1, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before autobalancing teams")
MACRO_CONFIG_INT(SvInactiveKickTime, sv_inactivekick_time, 3, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before taking care of inactive players")
MACRO_CONFIG_INT(SvInactiveKick, sv_inactivekick, 1, 0, 2, CFGFLAG_SERVER, "How to deal with inactive players (0=move to spectator, 1=move to free spectator slot/kick, 2=kick)")

MACRO_CONFIG_INT(SvStrictSpectateMode, sv_strict_spectate_mode, 0, 0, 1, CFGFLAG_SERVER, "Restricts information in spectator mode")
MACRO_CONFIG_INT(SvVoteSpectate, sv_vote_spectate, 1, 0, 1, CFGFLAG_SERVER, "Allow voting to move players to spectators")
MACRO_CONFIG_INT(SvVoteSpectateRejoindelay, sv_vote_spectate_rejoindelay, 3, 0, 1000, CFGFLAG_SERVER, "How many minutes to wait before a player can rejoin after being moved to spectators by vote")
MACRO_CONFIG_INT(SvVoteKick, sv_vote_kick, 1, 0, 1, CFGFLAG_SERVER, "Allow voting to kick players")
MACRO_CONFIG_INT(SvVoteKickMin, sv_vote_kick_min, 0, 0, MAX_CLIENTS, CFGFLAG_SERVER, "Minimum number of players required to start a kick vote")
MACRO_CONFIG_INT(SvVoteKickBantime, sv_vote_kick_bantime, 5, 0, 1440, CFGFLAG_SERVER, "The time to ban a player if kicked by vote. 0 makes it just use kick")

// debug
#ifdef CONF_DEBUG // this one can crash the server if not used correctly
	MACRO_CONFIG_INT(DbgDummies, dbg_dummies, 0, 0, 15, CFGFLAG_SERVER, "")
#endif

MACRO_CONFIG_INT(DbgFocus, dbg_focus, 0, 0, 1, CFGFLAG_CLIENT, "")
MACRO_CONFIG_INT(DbgTuning, dbg_tuning, 0, 0, 1, CFGFLAG_CLIENT, "")

/** H-Client **/
#ifdef CONF_FAMILY_WINDOWS
	MACRO_CONFIG_STR(hcRecordVideoFFMPEG, hc_record_video_ffmpeg, 512, "c:\\ffmpeg\\ffmpeg.exe", CFGFLAG_SAVE|CFGFLAG_CLIENT, "ffmpeg fullpath")
#else
	MACRO_CONFIG_STR(hcRecordVideoFFMPEG, hc_record_video_ffmpeg, 512, "/usr/bin/ffmpeg", CFGFLAG_SAVE|CFGFLAG_CLIENT, "ffmpeg fullpath")
#endif

//MACRO_CONFIG_INT(hc3DRender, hc_3d_render, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enables/Disable 3D Render")
MACRO_CONFIG_INT(hcAutoUpdate, hc_auto_update, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Auto-Update")
MACRO_CONFIG_INT(hcAutoDownloadSkins, hc_auto_download_skins, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Auto-Download Skins from DDNet Database")
MACRO_CONFIG_STR(hcAutoDownloadSkinsSpeed, hc_auto_download_skins_speed, 5, "1", CFGFLAG_SAVE|CFGFLAG_CLIENT, "Auto-Download Skins Speed (KiBps)")
MACRO_CONFIG_INT(hcUseHUD, hc_use_hud, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Uses H-Client HUD")
MACRO_CONFIG_INT(hcColorClan, hc_color_clan, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "HighLight Clan Members")
MACRO_CONFIG_INT(hcChatEmoticons, hc_chat_emoticons, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable chat emoticons")
MACRO_CONFIG_INT(hcChatColours, hc_chat_colours, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable chat colors")
MACRO_CONFIG_INT(hcChatTeamColors, hc_chat_team_colors, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable chat team colors")
MACRO_CONFIG_INT(hcGoreStyle, hc_gore_style, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable Gore Style!")
MACRO_CONFIG_INT(hcGoreStyleTeeColors, hc_gore_style_tee_colors, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable Tee Colors in Gore Style!")
MACRO_CONFIG_INT(hcGoreStyleDropWeapons, hc_gore_style_drop_weapons, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable Drop Weapons in Gore Style!")

MACRO_CONFIG_INT(hcLaserCustomColor, hc_laser_custom_color, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable Custom Laser Color")
MACRO_CONFIG_INT(hcLaserColorHue, hc_laser_color_hue, 127, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser color hue")
MACRO_CONFIG_INT(hcLaserColorSat, hc_laser_color_sat, 127, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser color saturation")
MACRO_CONFIG_INT(hcLaserColorLht, hc_laser_color_lht, 127, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser color lightness")
MACRO_CONFIG_INT(hcLaserColorAlpha, hc_laser_color_alpha, 190, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Laser alpha")

MACRO_CONFIG_INT(hcSmokeCustomColor, hc_smoke_custom_color, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Enable Custom Smoke Color")
MACRO_CONFIG_INT(hcSmokeColorHue, hc_smoke_color_hue, 247, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Smoke color hue")
MACRO_CONFIG_INT(hcSmokeColorSat, hc_smoke_color_sat, 148, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Smoke color saturation")
MACRO_CONFIG_INT(hcSmokeColorLht, hc_smoke_color_lht, 0, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Smoke color lightness")
MACRO_CONFIG_INT(hcSmokeColorAlpha, hc_smoke_color_alpha, 255, 0, 255, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Smoke alpha")

MACRO_CONFIG_INT(hcDisableChatSoundNotification, hc_disable_chat_sound_notification, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Disable Chat Sound Notification")
MACRO_CONFIG_INT(hcPlayerInfo, hc_player_info, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show Player Info")
MACRO_CONFIG_INT(hcShowOffScreenPlayers, hc_show_offscreen_players, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show Off-Screen Players")
MACRO_CONFIG_INT(hcShowPreviewMap, hc_show_preview_map, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show Preview Map")
MACRO_CONFIG_INT(hcPreviewMapMaxWidth, hc_preview_map_max_width, 600, 1, 999999, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Preview Map Max Width")
MACRO_CONFIG_INT(hcPreviewMapMaxHeight, hc_preview_map_max_height, 600, 1, 999999, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Preview Map Max Height")

MACRO_CONFIG_STR(hcEyesSelectorTime, hc_eyes_selector_time, 4, "3", CFGFLAG_CLIENT|CFGFLAG_SAVE, "Eyes Selector Time")

MACRO_CONFIG_INT(hcAutoVoteNoAction, hc_auto_vote_no_action, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Auto Vote NO if the target are you")
MACRO_CONFIG_INT(hcMarkVoteTarget, hc_mark_vote_target, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Highlight target of vote")

MACRO_CONFIG_INT(hcRaceGhost, hc_race_ghost, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Enable ghost")
MACRO_CONFIG_INT(hcRaceShowGhost, hc_race_show_ghost, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Show ghost")
MACRO_CONFIG_INT(hcRaceSaveGhost, hc_race_save_ghost, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Save ghost")

MACRO_CONFIG_INT(AntiPing, anti_ping, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Anti-Ping")

MACRO_CONFIG_INT(ddrMapSounds, ddrace_map_sounds, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Play DDRace Map Sounds")
MACRO_CONFIG_INT(ddrMapsFromHttp, ddrace_maps_from_http, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Try Download Maps From DDRaceNet HTTP Servers")
MACRO_CONFIG_INT(ddrShowOthers, ddrace_show_others, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Show Others")
MACRO_CONFIG_INT(ddrShowHiddenWays, ddrace_show_hidden_ways, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Show hidden ways")
MACRO_CONFIG_INT(ddrShowTeeDirection, ddrace_show_tee_direction, 1, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "View Tee Direcion")
MACRO_CONFIG_INT(ddrPreventPrediction, ddr_prevent_prediction, 0, 0, 1, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Prevent Predection When Freeze")
MACRO_CONFIG_STR(ddrTimeoutHash, ddrace_timeout_hash, 16, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "DDRace Timeout Hash")

MACRO_CONFIG_INT(UiSubPage, ui_subpage, 13, 0, 13, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Interface Subpage") //H-Client

MACRO_CONFIG_INT(hcNotificationTime, hc_notification_time, 5000, 0, 99999, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Notification Time")

// Editor
MACRO_CONFIG_INT(hcEditorDrawRouteAccuracy, hc_editor_draw_route_accuracy, 5, 0, 100, CFGFLAG_SAVE|CFGFLAG_CLIENT, "Editor Draw Route Accuracy")

// Irc
MACRO_CONFIG_STR(IrcNick, irc_nick, 15, "", CFGFLAG_SAVE|CFGFLAG_CLIENT, "IRC Nick")
MACRO_CONFIG_STR(IrcServer, irc_server, 124, "port80c.se.quakenet.org", CFGFLAG_SAVE|CFGFLAG_CLIENT, "IRC Server")
MACRO_CONFIG_INT(IrcPort, irc_port, 6667, 1111, 9999, CFGFLAG_SAVE|CFGFLAG_CLIENT, "IRC Port")

// Theme
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
