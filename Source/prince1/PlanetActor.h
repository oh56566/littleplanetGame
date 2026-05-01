#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlanetActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class PRINCE1_API APlanetActor : public AActor
{
	GENERATED_BODY()

public:
	APlanetActor();

	UFUNCTION(BlueprintPure, Category = "Planet")
	FVector GetCenter() const { return GetActorLocation(); }

	UFUNCTION(BlueprintPure, Category = "Planet")
	float GetRadius() const { return Radius; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Planet", meta = (ClampMin = "100.0"))
	float Radius = 5000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Planet")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Planet")
	TObjectPtr<UStaticMeshComponent> VisualMesh;
};
