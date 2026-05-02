#include "PrinceCharacter.h"

#include "PlanetActor.h"
#include "PlanetCharacterMovementComponent.h"
#include "InventoryComponent.h"
#include "InteractPromptWidget.h"
#include "InventoryWidget.h"
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
		}
	}
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

void APrinceCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// ─── Interaction scan ──────────────────────────────────────────────────────
	if (IsLocallyControlled())
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
