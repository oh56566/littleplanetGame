#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PrinceCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
class APlanetActor;

UCLASS()
class PRINCE1_API APrinceCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APrinceCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds) override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	// Assigned in editor (BP_PrinceCharacter) or left null for keyboard-only fallback to Character::Jump.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	// Optional: explicit planet reference. If null, the character locates the first APlanetActor in BeginPlay.
	UPROPERTY(EditAnywhere, Category = "Planet")
	TObjectPtr<APlanetActor> CurrentPlanet;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float LookPitchMin = -75.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float LookPitchMax = 75.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float LookSensitivity = 1.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	bool bDrawDebug = false;

private:
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnJumpStarted(const FInputActionValue& Value);
	void OnJumpReleased(const FInputActionValue& Value);

	// World-space camera direction, always kept perpendicular to current planet Up.
	// Rotated by mouse X (yaw around Up). Used as the reference for both movement and camera.
	FVector CameraForwardFlat = FVector::ForwardVector;

	// Camera pitch in degrees. Positive = looking down toward planet. Clamped by LookPitchMin/Max.
	float CameraPitch = 20.f;
};
