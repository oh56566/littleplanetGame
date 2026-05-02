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
class UInventoryComponent;
class UInteractPromptWidget;
class UInventoryWidget;
class UDialogueWidget;
class UDialogueDataAsset;

UCLASS()
class PRINCE1_API APrinceCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APrinceCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds) override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/**
	 * Switches the character's reference planet at runtime.
	 *
	 * Updates CurrentPlanet, propagates to UPlanetCharacterMovementComponent
	 * (changing gravity direction immediately), and re-initialises
	 * CameraForwardFlat so the camera doesn't snap on the next tick when the
	 * surface normal changes orientation.
	 *
	 * Intended caller: APortalActor on teleport.
	 */
	UFUNCTION(BlueprintCallable, Category = "Planet")
	void SetCurrentPlanet(APlanetActor* NewPlanet);

	/**
	 * Routes the player into a dialogue with the given NPC. Suppresses the
	 * interact prompt, gates further interactable scans, and tells the
	 * DialogueWidget to start playback. No-op if a dialogue is already active
	 * or if no widget class was assigned.
	 *
	 * Intended caller: ANPCActor::OnInteract.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void BeginDialogue(const FText& NPCName, UDialogueDataAsset* Dialogue);

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsInDialogue() const { return bIsInDialogue; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	// Optional: explicit planet reference. If null, the character locates the first APlanetActor in BeginPlay.
	UPROPERTY(EditAnywhere, Category = "Planet")
	TObjectPtr<APlanetActor> CurrentPlanet;

	// ── Interaction ────────────────────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UInventoryComponent> Inventory;

	/** Radius (cm) of the sphere overlap used to detect nearby IInteractable actors. */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "50.0"))
	float InteractRadius = 200.f;

	/** Widget class spawned and added to the viewport for the interact prompt. */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TSubclassOf<UInteractPromptWidget> PromptWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UInteractPromptWidget> PromptWidget;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Interaction")
	TWeakObjectPtr<AActor> FocusedInteractable;

	/** Widget class spawned and added to viewport for the inventory HUD. */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UInventoryWidget> InventoryWidget;

	/** Widget class spawned and added to viewport for NPC dialogue playback. */
	UPROPERTY(EditDefaultsOnly, Category = "Dialogue")
	TSubclassOf<UDialogueWidget> DialogueWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<UDialogueWidget> DialogueWidget;

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
	void OnInteractStarted(const FInputActionValue& Value);

	/** Sphere-overlap scan for the closest valid IInteractable actor. */
	void UpdateFocusedInteractable();

	/** Bound to UDialogueWidget::OnDialogueClosed; clears bIsInDialogue. */
	UFUNCTION()
	void HandleDialogueClosed();

	/** True between BeginDialogue and the widget's OnDialogueClosed broadcast. */
	bool bIsInDialogue = false;

	// World-space camera direction, always kept perpendicular to current planet Up.
	// Rotated by mouse X (yaw around Up). Used as the reference for both movement and camera.
	FVector CameraForwardFlat = FVector::ForwardVector;

	// Camera pitch in degrees. Positive = looking down toward planet. Clamped by LookPitchMin/Max.
	float CameraPitch = 20.f;
};
