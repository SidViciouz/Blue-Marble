#include "GameState.h"

GameState::GameState()
{
	mGamePhase = GamePhase::ReadyToRollDice;
}

void GameState::Next()
{
	if (mGamePhase == GamePhase::ReadyToRollDice)
	{
		mGamePhase = GamePhase::DiceRolling;
	}
	else if (mGamePhase == GamePhase::DiceRolling)
	{
		mGamePhase = GamePhase::DiceStop;
	}
	else if (mGamePhase == GamePhase::DiceStop)
	{
		mGamePhase = GamePhase::PickPlace;
	}
	else if (mGamePhase == GamePhase::PickPlace)
	{
		mGamePhase = GamePhase::CharacterMoving;
	}
	else if (mGamePhase == GamePhase::CharacterMoving)
	{
		mGamePhase = GamePhase::ReadyToRollDice;
	}
}

GamePhase GameState::GetPhase()
{
	return mGamePhase;
}