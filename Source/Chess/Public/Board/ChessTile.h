// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Miscellaneous/StructuresAndEnumerations.h"

#include "GameFramework/Actor.h"

#include "ChessTile.generated.h"

class AChessPiece;

UCLASS()
class CHESS_API AChessTile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Tile", meta = (AllowPrivateAccess = "true"))
	USceneComponent* DefaultSceneRootComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Tile", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ChessTileMesh = nullptr;

public:
	AChessTile();

	virtual void OnConstruction(const FTransform& Transform) override;

	FORCEINLINE UStaticMeshComponent* GetChessTileMesh() const { return ChessTileMesh; }

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

#pragma region FUNCTIONS

	void HighlightTile(bool bHighlight);

#pragma endregion

#pragma region VARIABLES

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Tile")
	UMaterialInterface* TileMaterial = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Tile")
	UMaterialInstanceDynamic* TileMaterialInstanceDynamic = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Tile")
	FChessTileInfo ChessTileInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Tile")
	AChessPiece* ChessPieceOnTile = nullptr;

#pragma endregion
};