#include "PrinceGameMode.h"

#include "PrinceCharacter.h"

APrinceGameMode::APrinceGameMode()
{
	DefaultPawnClass = APrinceCharacter::StaticClass();
}
