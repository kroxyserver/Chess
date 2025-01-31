// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Miscellaneous/StructuresAndEnumerations.h"

#include "GameFramework/Actor.h"
#include "Kismet/GameplayStaticsTypes.h"

#include "ChessPiece.generated.h"

class AChessBoard;
class AChessTile;
class UChessBoardData;

class UInterpToMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPieceCaptured);

UCLASS()
class CHESS_API AChessPiece : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece", meta = (AllowPrivateAccess = "true"))
	USceneComponent* DefaultSceneRootComponent = nullptr;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ChessPieceMesh = nullptr;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece", meta = (AllowPrivateAccess = "true"))
	UInterpToMovementComponent* InterpToMovementComponent = nullptr;

public:
	AChessPiece();

	virtual void OnConstruction(const FTransform& Transform) override;

	FORCEINLINE UStaticMeshComponent* GetChessPieceMesh() const { return ChessPieceMesh; }
	FORCEINLINE UInterpToMovementComponent* GetInterpToMovementComponent() const { return InterpToMovementComponent; }

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

#pragma region FUNCTIONS

public:
	void UpdateChessPieceStaticMesh();

	void CapturePiece();

	float MovePiece(AChessTile* MoveToTile);

	UFUNCTION(BlueprintCallable, Category = "+Chess|Piece")
	void PromotePawn(EChessPieceType PromotionType);

#pragma endregion

#pragma region VARIABLES

public:
	float TotalTravelDistance = 0.f;

	UPROPERTY(BlueprintAssignable, Category = "+Chess|Piece")
	FOnPieceCaptured OnPieceCaptured;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece")
	UChessBoardData* ChessBoardData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "+Chess|Piece")
	AChessBoard* ChessBoard = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "+Chess|Piece")
	FChessPieceInfo ChessPieceInfo;

private:
	FPredictProjectilePathParams PredictParams;

#pragma endregion
};