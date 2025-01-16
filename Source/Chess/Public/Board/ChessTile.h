// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"

#include "ChessTile.generated.h"

class AChessBoard;
class AChessPiece;

USTRUCT(BlueprintType)
struct FChessTileInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AChessPiece* ChessPieceOnTile;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ChessTilePositionIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsWhite;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsHighlighted;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsTileUnderAttackByWhitePiece;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsTileUnderAttackByBlackPiece;

	FChessTileInfo() :
		ChessPieceOnTile(nullptr),
		ChessTilePositionIndex(-1),
		bIsWhite(true),
		bIsHighlighted(false),
		bIsTileUnderAttackByWhitePiece(false),
		bIsTileUnderAttackByBlackPiece(false) {}

	bool operator==(const FChessTileInfo& Other) const
	{
		return ChessPieceOnTile == Other.ChessPieceOnTile && ChessTilePositionIndex == Other.ChessTilePositionIndex && bIsWhite == Other.bIsWhite && bIsHighlighted == Other.bIsHighlighted && bIsTileUnderAttackByWhitePiece == Other.bIsTileUnderAttackByWhitePiece && bIsTileUnderAttackByBlackPiece == Other.bIsTileUnderAttackByBlackPiece;
	}

	FVector2D GetChessTilePositionFromIndex()
	{
		return FVector2D(ChessTilePositionIndex / 8, ChessTilePositionIndex % 8);
	}
};

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

#pragma endregion
};