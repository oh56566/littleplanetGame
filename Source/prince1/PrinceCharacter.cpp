#include "PrinceCharacter.h"

#include "PlanetActor.h"
#include "PlanetCharacterMovementComponent.h"
#include "InventoryComponent.h"
#include "InteractPromptWidget.h"
#include "InventoryWidget.h"
#include "DialogueWidget.h"
#include "DialogueDataAsset.h"
#include "Interactable.h"

#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "TimerManager.h"

APrinceCharacter::APrinceCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPlanetCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// Capsule is rotated exclusively by UPlanetCharacterMovementComponent.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		// Character facing is handled manually in CMC::TickComponent (velocity-based yaw).
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->JumpZVelocity             = 600.f;
		MoveComp->AirControl                = 0.35f; // default is 0.35f
		MoveComp->MaxWalkSpeed              = 500.f;
		MoveComp->MinAnalogWalkSpeed        = 20.f;
		MoveComp->BrakingDecelerationWalking = 2000.f;
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength        = 400.f;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bDoCollisionTest       = true;
	SpringArm->bEnableCameraLag       = true;
	SpringArm->CameraLagSpeed         = 12.f;
	SpringArm->SocketOffset           = FVector(0.f, 40.f, 60.f);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}

void APrinceCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Auto-find planet if not set in editor.
	if (!CurrentPlanet)
	{
		for (TActorIterator<APlanetActor> It(GetWorld()); It; ++It)
		{
			CurrentPlanet = *It;
			break;
		}
	}

	if (auto* PlanetCMC = Cast<UPlanetCharacterMovementComponent>(GetCharacterMovement()))
	{
		PlanetCMC->SetPlanet(CurrentPlanet);
	}

	// Initialise camera forward in the plane perpendicular to planet Up.
	if (CurrentPlanet)
	{
		const FVector Up = (GetActorLocation() - CurrentPlanet->GetCenter()).GetSafeNormal();
		CameraForwardFlat = FVector::VectorPlaneProject(GetActorForwardVector(), Up).GetSafeNormal();
		if (CameraForwardFlat.IsNearlyZero())
		{
			const FVector AnyPerp = FMath::Abs(Up.X) < 0.9f ? FVector(1, 0, 0) : FVector(0, 1, 0);
			CameraForwardFlat = FVector::VectorPlaneProject(AnyPerp, Up).GetSafeNormal();
		}
	}
	else
	{
		CameraForwardFlat = GetActorForwardVector();
	}

	// Spawn HUD widgets for the local player (skip on dedicated server).
	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (PromptWidgetClass)
			{
				PromptWidget = CreateWidget<UInteractPromptWidget>(PC, PromptWidgetClass);
				if (PromptWidget)
				{
					PromptWidget->AddToViewport(/*ZOrder=*/10);
					PromptWidget->SetPrompt(FText::GetEmpty());
				}
			}

			if (InventoryWidgetClass)
			{
				InventoryWidget = CreateWidget<UInventoryWidget>(PC, InventoryWidgetClass);
				if (InventoryWidget)
				{
					InventoryWidget->AddToViewport(/*ZOrder=*/5);
					InventoryWidget->BindToInventory(Inventory);
				}
			}

			if (DialogueWidgetClass)
			{
				DialogueWidget = CreateWidget<UDialogueWidget>(PC, DialogueWidgetClass);
				if (DialogueWidget)
				{
					// Above prompt(10) and inventory(5); dialogue is the foreground modal.
					DialogueWidget->AddToViewport(/*ZOrder=*/20);
					DialogueWidget->OnDialogueClosed.RemoveAll(this);
					DialogueWidget->OnDialogueClosed.AddUObject(this, &APrinceCharacter::HandleDialogueClosed);
				}
			}
		}
	}
}

void APrinceCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DialogueWidget)
	{
		DialogueWidget->OnDialogueClosed.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void APrinceCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void APrinceCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (auto* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APrinceCharacter::OnMove);
		}
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &APrinceCharacter::OnLook);
		}
		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Started,    this, &APrinceCharacter::OnJumpStarted);
			EIC->BindAction(JumpAction, ETriggerEvent::Completed,  this, &APrinceCharacter::OnJumpReleased);
		}
		if (InteractAction)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &APrinceCharacter::OnInteractStarted);
		}
	}
}

void APrinceCharacter::SetCurrentPlanet(APlanetActor* NewPlanet)
{
	CurrentPlanet = NewPlanet;

	// Propagate to the movement component so gravity direction tracks the new planet
	// from the very next physics step.
	if (auto* PlanetCMC = Cast<UPlanetCharacterMovementComponent>(GetCharacterMovement()))
	{
		PlanetCMC->SetPlanet(NewPlanet);
	}

	// Re-derive CameraForwardFlat in the new planet's tangent plane so the camera
	// doesn't snap when Tick re-orthogonalises against a surface normal that has
	// just rotated relative to world space.
	if (NewPlanet)
	{
		const FVector Up = (GetActorLocation() - NewPlanet->GetCenter()).GetSafeNormal();
		CameraForwardFlat = FVector::VectorPlaneProject(GetActorForwardVector(), Up).GetSafeNormal();
		if (CameraForwardFlat.IsNearlyZero())
		{
			const FVector AnyPerp = FMath::Abs(Up.X) < 0.9f ? FVector(1, 0, 0) : FVector(0, 1, 0);
			CameraForwardFlat = FVector::VectorPlaneProject(AnyPerp, Up).GetSafeNormal();
		}
	}

	// Spring arm caches its previous-frame world position for the lag lerp. After
	// a long-distance teleport that cache points back at the *old planet*, so the
	// next ticks would interpolate the camera from there into the new desired
	// position — visible as the camera "flying in" once the post-teleport fade-in
	// clears.
	//
	// Suppress lag for ~6 frames so the spring arm snaps cleanly to its new
	// target. The window is well inside the typical fade-in (0.5 s) so the
	// snap is invisible to the player. Lag is restored to its prior value
	// afterward so we don't permanently disable the polish for normal play.
	if (SpringArm && SpringArm->bEnableCameraLag)
	{
		SpringArm->bEnableCameraLag = false;

		FTimerHandle CamLagRestoreHandle;
		GetWorldTimerManager().SetTimer(
			CamLagRestoreHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (SpringArm)
				{
					SpringArm->bEnableCameraLag = true;
				}
			}),
			0.1f, /*bLoop=*/false);
	}
}

void APrinceCharacter::BeginDialogue(const FText& NPCName, UDialogueDataAsset* Dialogue)
{
	// Reject if no widget was assigned in BP, no data to play, or we're already
	// in a conversation (NPC OnInteract also guards, but defend in depth).
	if (!DialogueWidget || !Dialogue || bIsInDialogue)
	{
		return;
	}

	bIsInDialogue = true;

	// Hide the interact prompt so the player isn't told to press [E] for
	// "Talk" while the dialogue panel is already up.
	if (PromptWidget)
	{
		PromptWidget->SetPrompt(FText::GetEmpty());
	}

	// Drop any cached focused target so the next post-dialogue scan starts
	// clean rather than instantly re-triggering the same NPC.
	FocusedInteractable = nullptr;

	DialogueWidget->StartDialogue(NPCName, Dialogue);
}

void APrinceCharacter::HandleDialogueClosed()
{
	bIsInDialogue = false;
	// Next Tick will resume UpdateFocusedInteractable and refresh the prompt.
}

void APrinceCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// ─── Interaction scan ──────────────────────────────────────────────────────
	// Pause scanning while a dialogue is open: the prompt is hidden and [E]
	// routes to Advance() instead of (re)triggering the focused interactable.
	if (IsLocallyControlled() && !bIsInDialogue)
	{
		UpdateFocusedInteractable();
	}

	// ─── Camera ────────────────────────────────────────────────────────────────
	// SpringArm rotation is set in world space so it's completely independent of
	// the capsule's yaw. This prevents the double-rotation when both character
	// body and camera change direction simultaneously.
	if (SpringArm && CurrentPlanet)
	{
		const FVector Up = (GetActorLocation() - CurrentPlanet->GetCenter()).GetSafeNormal();

		// Re-orthogonalise CameraForwardFlat to current Up (planet Up shifts as
		// the character walks around the sphere).
		CameraForwardFlat = FVector::VectorPlaneProject(CameraForwardFlat, Up).GetSafeNormal();
		if (CameraForwardFlat.IsNearlyZero())
		{
			const FVector AnyPerp = FMath::Abs(Up.X) < 0.9f ? FVector(1, 0, 0) : FVector(0, 1, 0);
			CameraForwardFlat = FVector::VectorPlaneProject(AnyPerp, Up).GetSafeNormal();
		}

		// Camera right axis (in the Up-plane, perpendicular to forward).
		const FVector CamRight = FVector::CrossProduct(Up, CameraForwardFlat).GetSafeNormal();

		// Pitch: rotate view direction around CamRight.
		// Positive CameraPitch = camera tilts downward toward planet.
		const FQuat PitchQuat(CamRight, FMath::DegreesToRadians(CameraPitch));
		const FVector ViewDir = PitchQuat.RotateVector(CameraForwardFlat);

		// SpringArm X-axis = ViewDir.  The arm extends in -X, placing the camera
		// behind the character.  MakeFromXZ keeps the arm's Z close to planet Up.
		SpringArm->SetWorldRotation(FRotationMatrix::MakeFromXZ(ViewDir, Up).Rotator());
	}

#if !UE_BUILD_SHIPPING
	if (bDrawDebug && CurrentPlanet)
	{
		const FVector Loc    = GetActorLocation();
		const FVector Center = CurrentPlanet->GetCenter();
		DrawDebugLine(GetWorld(), Loc, Center, FColor::Red, false, 0.f, 0, 2.f);
		DrawDebugDirectionalArrow(GetWorld(), Loc, Loc + GetActorUpVector()      * 200.f, 40.f, FColor::Green, false, 0.f, 0, 3.f);
		DrawDebugDirectionalArrow(GetWorld(), Loc, Loc + GetActorForwardVector() * 200.f, 40.f, FColor::Blue,  false, 0.f, 0, 3.f);
		// Show camera forward in world space.
		DrawDebugDirectionalArrow(GetWorld(), Loc, Loc + CameraForwardFlat * 250.f, 40.f, FColor::Yellow, false, 0.f, 0, 2.f);
	}
#endif
}

void APrinceCharacter::OnMove(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	if (Input.IsNearlyZero() || !Controller)
	{
		return;
	}

	// Movement directions come from CameraForwardFlat (world-space, already updated
	// by OnLook this frame or the previous frame).  This makes WASD always move
	// relative to the camera without rotating the capsule here.
	const FVector Up = CurrentPlanet
		? (GetActorLocation() - CurrentPlanet->GetCenter()).GetSafeNormal()
		: GetActorUpVector();

	const FVector FlatForward = FVector::VectorPlaneProject(CameraForwardFlat, Up).GetSafeNormal();
	const FVector FlatRight   = FVector::CrossProduct(Up, FlatForward).GetSafeNormal();

	if (!FlatForward.IsNearlyZero())
	{
		AddMovementInput(FlatForward, Input.Y);
	}
	if (!FlatRight.IsNearlyZero())
	{
		AddMovementInput(FlatRight, Input.X);
	}
	// NOTE: capsule yaw is handled in UPlanetCharacterMovementComponent::TickComponent
	// (character faces its velocity direction, same as bOrientRotationToMovement but
	// constrained to the planet-Up plane).
}

void APrinceCharacter::OnLook(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>() * LookSensitivity;

	const FVector Up = CurrentPlanet
		? (GetActorLocation() - CurrentPlanet->GetCenter()).GetSafeNormal()
		: GetActorUpVector();

	// Yaw: rotate camera forward AROUND planet Up.
	// FQuat(axis, angle) uses the right-hand rule; negate Input.X so that
	// moving the mouse right rotates the camera clockwise (looking right).
	if (FMath::Abs(Input.X) > KINDA_SMALL_NUMBER && !Up.IsNearlyZero())
	{
		const FQuat YawQuat(Up, FMath::DegreesToRadians(-Input.X));
		CameraForwardFlat = YawQuat.RotateVector(CameraForwardFlat);
	}

	// Pitch: positive pitch = camera looks more downward (toward planet surface).
	// If pitch feels inverted in-game, add a "Negate" modifier to Mouse Y in IMC_PrinceDefault.
	CameraPitch = FMath::Clamp(CameraPitch + Input.Y, LookPitchMin, LookPitchMax);
}

void APrinceCharacter::OnJumpStarted(const FInputActionValue& /*Value*/)
{
	Jump();
}

void APrinceCharacter::OnJumpReleased(const FInputActionValue& /*Value*/)
{
	StopJumping();
}

void APrinceCharacter::OnInteractStarted(const FInputActionValue& /*Value*/)
{
	// During dialogue, the interact key advances the conversation rather than
	// triggering whatever happens to be in the overlap radius (which could be
	// the same NPC and instantly re-open a fresh dialogue).
	if (bIsInDialogue)
	{
		if (DialogueWidget && DialogueWidget->IsDialogueActive())
		{
			DialogueWidget->Advance();
		}
		else
		{
			bIsInDialogue = false;
		}
		return;
	}

	AActor* Target = FocusedInteractable.Get();
	if (!Target || !Target->Implements<UInteractable>())
	{
		return;
	}
	if (!IInteractable::Execute_CanInteract(Target, this))
	{
		return;
	}
	IInteractable::Execute_OnInteract(Target, this);

	// Refresh prompt immediately — the interaction may have changed CanInteract.
	UpdateFocusedInteractable();
}

void APrinceCharacter::UpdateFocusedInteractable()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Sphere overlap centred on the character. Cheap on a small planet and works
	// regardless of facing direction (closest-wins picks the natural target).
	const FVector Origin = GetActorLocation();
	const FCollisionShape Shape = FCollisionShape::MakeSphere(InteractRadius);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractScan), false, this);
	Params.AddIgnoredActor(this);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, ECC_WorldDynamic, Shape, Params);
	// Also check static channel so static-mesh-only pickups are caught.
	TArray<FOverlapResult> StaticOverlaps;
	World->OverlapMultiByChannel(StaticOverlaps, Origin, FQuat::Identity, ECC_WorldStatic, Shape, Params);
	Overlaps.Append(StaticOverlaps);

	AActor* Best = nullptr;
	float   BestDistSq = TNumericLimits<float>::Max();

	for (const FOverlapResult& O : Overlaps)
	{
		AActor* A = O.GetActor();
		if (!A || !A->Implements<UInteractable>())
		{
			continue;
		}
		if (!IInteractable::Execute_CanInteract(A, this))
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(Origin, A->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best       = A;
		}
	}

	FocusedInteractable = Best;

	if (PromptWidget)
	{
		const FText Prompt = Best ? IInteractable::Execute_GetInteractPrompt(Best) : FText::GetEmpty();
		PromptWidget->SetPrompt(Prompt);
	}
}
