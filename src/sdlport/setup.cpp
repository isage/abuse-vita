/*
 *  Abuse - dark 2D side-scrolling platform game
 *  Copyright (c) 2001 Anthony Kruize <trandor@labyrinth.net.au>
 *  Copyright (c) 2005-2011 Sam Hocevar <sam@hocevar.net>
 *  Copyright (c) 2016 Antonio Radojkovic <antonior.software@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 */

#if defined HAVE_CONFIG_H
#   include "config.h"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <string>
#include "SDL.h"

#include "specs.h"
#include "keys.h"
#include "setup.h"
#include "errorui.h"

//AR
#include <fstream>
#include <sstream>

#include <psp2/io/fcntl.h>

extern Settings settings;
//

extern int xres, yres;					//video.cpp
extern int sfx_volume, music_volume;	//loader.cpp
unsigned int scale;						//AR was static, removed for external

//AR tmp, until I figure out compiling stuff and cmake...
int AR_ToInt(std::string value)
{
	int n = 1;

	std::stringstream stream(value);
	stream >> n;

	return n;
}

bool AR_ToBool(std::string value)
{
	bool n = false;

	std::stringstream stream(value);
	stream >> n;

	return n;
}

bool AR_GetAttr(std::string line, std::string &attr, std::string &value)
{
	attr = value = "";

	std::size_t found = line.find("=");

	//no "="
	if(found==std::string::npos || found==line.size()-1) return false;
	
	attr = line.substr(0,found);
	value = line.substr(found+1,line.size()-1);

	//empty attribute or value
	if(attr.empty() || value.empty()) return false;
	
	return true;
}
//

Settings::Settings()
{
	//screen
	this->fullscreen		= 1;		// start in window
	this->borderless		= false;
	this->vsync				= true;
	this->xres				= 340;		// default window width
	this->yres				= 200;		// default window height
	this->scale				= 1;		// default window scale
	this->linear_filter		= false;    // don't "anti-alias"	
	this->hires				= 0;	

	//sound
	this->mono				= false;	// disable stereo sound
	this->no_sound			= false;	// disable sound
	this->no_music			= false;	// disable music	
	this->volume_sound		= 127;
	this->volume_music		= 87;

	//random
	this->local_save		= false;
	this->grab_input		= false;	// don't grab the input
	this->editor			= false;	// disable editor mode
	this->physics_update	= 65;		// original 65ms/15 FPS
	this->mouse_scale		= 0;		// match desktop
	this->big_font			= false;
	//
	this->bullet_time		= false;
	this->bullet_time_add	= 1.2f;

	this->player_touching_console = false;

	this->cheat_god = cheat_bullettime = false;
	
	//player controls
	this->up		= key_value("w");
    this->down		= key_value("s");
    this->left		= key_value("a");
    this->right		= key_value("d");
    this->up_2		= key_value("UP");
    this->down_2	= key_value("DOWN");
    this->left_2	= key_value("LEFT");
    this->right_2	= key_value("RIGHT");
	this->b1		= key_value("SHIFT_L");	//special
	this->b2		= key_value("f");		//fire
    this->b3		= key_value("q");		//weapons
    this->b4		= key_value("e");
	this->bt		= key_value("CTRL_L");	//special2, bulettime

	//controller settings
	this->ctr_aim			= true;	// controller overide disabled
	this->ctr_aim_correctx	= 50;
	this->ctr_cd			= 120;
	this->ctr_rst_s			= 10;
	this->ctr_rst_dz		= 5000;		// aiming
	this->ctr_lst_dzx		= 10000;	// move left right
	this->ctr_lst_dzy		= 25000;	// up/jump, down/use
	this->ctr_aim_x			= 0;
	this->ctr_aim_y			= 0;
	this->ctr_mouse_x		= 0;
	this->ctr_mouse_y		= 0;

	//controller buttons
	this->ctr_a = "b1";
	this->ctr_b = "b2";
	this->ctr_x = "b3";
	this->ctr_y = "b4";
	//
	this->ctr_lst = "b1";
	this->ctr_rst = "down";
	//
	this->ctr_lsr = "b1";
	this->ctr_rsr = "b2";
	//
	this->ctr_ltg = "bt";
	this->ctr_rtg = "b2";
	//
	this->ctr_f5 = SDL_CONTROLLER_BUTTON_LEFTSTICK;
	this->ctr_f9 = SDL_CONTROLLER_BUTTON_RIGHTSTICK;
}

//////////
////////// CREATE DEFAULT "config.txt" FILE
//////////

bool Settings::CreateConfigFile(std::string file_path)
{
    return true;
}

//////////
////////// READ CONFIG FILE
//////////

bool Settings::ReadConfigFile(std::string folder)
{
    return true;
}

bool Settings::ControllerButton(std::string c, std::string b)
{
	std::string control = c;

	if(c=="special")			control = "b1";
	else if(c=="fire")			control = "b2";
	else if(c=="weapon_prev")	control = "b3";
	else if(c=="weapon_next")	control = "b4";
	else if(c=="special2")		control = "bt";

	if(b=="ctr_a") {this->ctr_a = control;return true;};
	if(b=="ctr_b") {this->ctr_b = control;return true;};
	if(b=="ctr_x") {this->ctr_x = control;return true;};
	if(b=="ctr_y") {this->ctr_y = control;return true;};
	//
	if(b=="ctr_left_stick")		{this->ctr_lst = control;return true;};
	if(b=="ctr_right_stick")	{this->ctr_rst = control;return true;};
	//
	if(b=="ctr_left_shoulder")	{this->ctr_lsr = control;return true;};
	if(b=="ctr_right_shoulder")	{this->ctr_rsr = control;return true;};
	//
	if(b=="ctr_left_trigger")	{this->ctr_ltg = control;return true;};
	if(b=="ctr_right_trigger")	{this->ctr_rtg = control;return true;};

	return false;
}

//
// Display help
//
void showHelp(const char* executableName)
{
    printf( "\n" );
    printf( "Usage: %s [options]\n", executableName );
    printf( "Options:\n\n" );
    printf( "** Abuse Options **\n" );
    printf( "  -size <arg>       Set the size of the screen\n" );
    printf( "  -edit             Startup in editor mode\n" );
    printf( "  -a <arg>          Use addon named <arg>\n" );
    printf( "  -f <arg>          Load map file named <arg>\n" );
    printf( "  -lisp             Startup in lisp interpreter mode\n" );
    printf( "  -nodelay          Run at maximum speed\n" );
    printf( "\n" );
    printf( "** Abuse-SDL Options **\n" );
    printf( "  -datadir <arg>    Set the location of the game data to <arg>\n" );
    printf( "  -fullscreen       Enable fullscreen mode\n" );
    printf( "  -antialias        Enable anti-aliasing\n" );
    printf( "  -h, --help        Display this text\n" );
    printf( "  -mono             Disable stereo sound\n" );
    printf( "  -nosound          Disable sound\n" );
    printf( "  -scale <arg>      Scale to <arg>\n" );
//    printf( "  -x <arg>          Set the width to <arg>\n" );
//    printf( "  -y <arg>          Set the height to <arg>\n" );
    printf( "\n" );
    printf( "Anthony Kruize <trandor@labyrinth.net.au>\n" );
    printf( "\n" );
}

//
// Parse the command-line parameters
//
void parseCommandLine(int argc, char **argv)
{
	//AR this is called before settings.ReadConfigFile(), so I can override stuff via console

	for(int i=1;i<argc;i++)
	{
		if(!strcasecmp(argv[i],"-remote_save"))
		{
			settings.local_save = false;
		}
	}
}

//
// Setup SDL and configuration
//
void setup( int argc, char **argv )
{
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    // Initialize SDL with video and audio support
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER ) < 0 )
    {
        show_startup_error( "Unable to initialize SDL : %s\n", SDL_GetError() );
        exit( 1 );
    }
    atexit( SDL_Quit );

    char* save_path = SDL_GetPrefPath(NULL,"abuse");
    set_save_filename_prefix( save_path );

    set_filename_prefix( "app0:/data/" );

	//handle command-line parameters
    parseCommandLine(argc,argv);

	printf("Setting save dir to %s\n", get_save_filename_prefix());


	// Initialize default settings
	scale = settings.scale;
	xres = settings.xres;
	yres = settings.yres;
	sfx_volume = settings.volume_sound;
	music_volume = settings.volume_music;
}

//
// Get the key binding for the requested function
//
int get_key_binding(char const *dir, int i)
{
	if(strcasecmp(dir,"left")==0)			return settings.left;
	else if(strcasecmp(dir,"right")==0)		return settings.right;
	else if(strcasecmp(dir,"up")==0)		return settings.up;
	else if(strcasecmp(dir,"down")==0)		return settings.down;
	else if(strcasecmp(dir,"left2")==0)		return settings.left_2;
	else if(strcasecmp(dir,"right2")==0)	return settings.right_2;
	else if(strcasecmp(dir,"up2")==0)		return settings.up_2;
	else if(strcasecmp(dir,"down2")==0)		return settings.down_2;
	else if(strcasecmp(dir,"b1")==0)		return settings.b1;
	else if(strcasecmp(dir,"b2")==0)		return settings.b2;
	else if(strcasecmp(dir,"b3")==0)		return settings.b3;
	else if(strcasecmp(dir,"b4")==0)		return settings.b4;
	else if(strcasecmp(dir,"bt")==0)		return settings.bt;

	return 0;
}

//AR controller
std::string get_ctr_binding(std::string c)
{
    if(c=="ctr_a")			return settings.ctr_a;
	else if(c=="ctr_b")		return settings.ctr_b;
	else if(c=="ctr_x")		return settings.ctr_x;
	else if(c=="ctr_y")		return settings.ctr_y;
	//
	else if(c=="ctr_lst")	return settings.ctr_lst;
	else if(c=="ctr_rst")	return settings.ctr_rst;
	//
	else if(c=="ctr_lsh")	return settings.ctr_lsr;
	else if(c=="ctr_rsh")	return settings.ctr_rsr;
	//
	else if(c=="ctr_ltg")	return settings.ctr_ltg;
	else if(c=="ctr_rtg")	return settings.ctr_rtg;
	
	return "";
}
