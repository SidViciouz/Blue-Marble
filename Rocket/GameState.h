#pragma once

enum class GamePhase
{
	ReadyToRollDice,
	DiceRolling,
	DiceStop,
	PickPlace,
	CharacterMoving
};

class GameState
{
public:
												GameState();
												GameState(const GameState& gameState) = delete;
												GameState(GameState&& gameState) = delete;
	GameState&									operator=(const GameState& gameState) = delete;
	GameState&									operator=(GameState&& gameState) = delete;

	void										Next();
	GamePhase									GetPhase();

protected:
	GamePhase									mGamePhase;
};
