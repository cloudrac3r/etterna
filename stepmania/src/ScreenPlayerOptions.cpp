#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenPlayerOptions

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenPlayerOptions.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "AnnouncerManager.h"
#include "RageSounds.h"
#include "ScreenSongOptions.h"
#include "PrefsManager.h"


#define PREV_SCREEN( play_mode )	THEME->GetMetric ("ScreenPlayerOptions","PrevScreen"+Capitalize(PlayModeToString(play_mode)))
#define NEXT_SCREEN( play_mode )	THEME->GetMetric ("ScreenPlayerOptions","NextScreen"+Capitalize(PlayModeToString(play_mode)))

ScreenPlayerOptions::ScreenPlayerOptions( CString sClassName ) :
	ScreenOptionsMaster( sClassName )
{
	LOG->Trace( "ScreenPlayerOptions::ScreenPlayerOptions()" );
	
	/* If we're going to "press start for more options" or skipping options
	 * entirely, we need a different fade out. XXX: this is a hack */
	if(PREFSMAN->m_ShowSongOptions == PrefsManager::NO)
		m_Menu.m_Out.Load( THEME->GetPathToB("ScreenPlayerOptions direct out") ); /* direct to stage */
	else if(PREFSMAN->m_ShowSongOptions == PrefsManager::ASK)
		m_Menu.m_Out.Load( THEME->GetPathToB("ScreenPlayerOptions option out") ); /* optional song options */

	m_sprOptionsMessage.Load( THEME->GetPathToG("ScreenPlayerOptions options") );
	m_sprOptionsMessage.StopAnimating();
	m_sprOptionsMessage.SetXY( CENTER_X, CENTER_Y );
	m_sprOptionsMessage.SetZoom( 1 );
	m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
	//this->AddChild( &m_sprOptionsMessage );       // we have to draw this manually over the top of transitions

	m_bAcceptedChoices = false;
	m_bGoToOptions = ( PREFSMAN->m_ShowSongOptions == PrefsManager::YES );

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("player options intro") );
}


void ScreenPlayerOptions::GoToPrevState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else
		SCREENMAN->SetNewScreen( PREV_SCREEN(GAMESTATE->m_PlayMode) );
}

void ScreenPlayerOptions::GoToNextState()
{
	if( GAMESTATE->m_bEditing )
		SCREENMAN->PopTopScreen();
	else
	{
		GAMESTATE->AdjustFailType();

		if( m_bGoToOptions )
			SCREENMAN->SetNewScreen( NEXT_SCREEN(GAMESTATE->m_PlayMode) );
		else
			SCREENMAN->SetNewScreen( ScreenSongOptions::GetNextScreen() );
	}
}


void ScreenPlayerOptions::Update( float fDelta )
{
	ScreenOptionsMaster::Update( fDelta );
	m_sprOptionsMessage.Update( fDelta );
}

void ScreenPlayerOptions::DrawPrimitives()
{
	ScreenOptionsMaster::DrawPrimitives();
	m_sprOptionsMessage.Draw();
}


void ScreenPlayerOptions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( !GAMESTATE->m_bEditing &&
		type == IET_FIRST_PRESS  &&
		!m_Menu.m_In.IsTransitioning()  &&
		MenuI.IsValid()  &&
		MenuI.button == MENU_BUTTON_START  &&
		PREFSMAN->m_ShowSongOptions == PrefsManager::ASK )
	{
		if( m_bAcceptedChoices  &&  !m_bGoToOptions )
		{
			m_bGoToOptions = true;
			m_sprOptionsMessage.SetState( 1 );
			SOUND->PlayOnce( THEME->GetPathToS("Common start") );
		}
	}

	ScreenOptionsMaster::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenPlayerOptions::HandleScreenMessage( const ScreenMessage SM )
{
	if( PREFSMAN->m_ShowSongOptions == PrefsManager::ASK )
	{
		switch( SM )
		{
		case SM_BeginFadingOut: // when the user accepts the page of options
			{
				m_bAcceptedChoices = true;

				float fShowSeconds = m_Menu.m_Out.GetLengthSeconds();

				// show "hold START for options"
				m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
				m_sprOptionsMessage.BeginTweening( 0.15f );     // fade in
				m_sprOptionsMessage.SetZoomY( 1 );
				m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,1) );
				m_sprOptionsMessage.BeginTweening( fShowSeconds-0.3f ); // sleep
				m_sprOptionsMessage.BeginTweening( 0.15f );     // fade out
				m_sprOptionsMessage.SetDiffuse( RageColor(1,1,1,0) );
				m_sprOptionsMessage.SetZoomY( 0 );
			}
			break;
		}
	}

	ScreenOptionsMaster::HandleScreenMessage( SM );
}
