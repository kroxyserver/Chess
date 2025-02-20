// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Miscellaneous/StructuresAndEnumerations.h"

#include "Engine/DataAsset.h"

#include "ChessBoardData.generated.h"

struct FChessPieceInfo;

class UNiagaraSystem;

UCLASS()
class CHESS_API UChessBoardData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece|WhitePieces")
	TSoftObjectPtr<UStaticMesh> WhiteKing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece|WhitePieces")
	TSoftObjectPtr<UStaticMesh> WhiteQueen;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece|WhitePieces")
	TSoftObjectPtr<UStaticMesh> WhiteBishop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece|WhitePieces")
	TSoftObjectPtr<UStaticMesh> WhiteKnight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece|WhitePieces")
	TSoftObjectPtr<UStaticMesh> WhiteRook;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece|WhitePieces")
	TSoftObjectPtr<UStaticMesh> WhitePawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece|BlackPieces")
	TSoftObjectPtr<UStaticMesh> BlackKing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece|BlackPieces")
	TSoftObjectPtr<UStaticMesh> BlackQueen;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece|BlackPieces")
	TSoftObjectPtr<UStaticMesh> BlackBishop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece|BlackPieces")
	TSoftObjectPtr<UStaticMesh> BlackKnight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece|BlackPieces")
	TSoftObjectPtr<UStaticMesh> BlackRook;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Piece|BlackPieces")
	TSoftObjectPtr<UStaticMesh> BlackPawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Tile")
	TSoftObjectPtr<UMaterialInterface> WhiteTileMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Tile")
	TSoftObjectPtr<UMaterialInterface> BlackTileMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Tile")
	UNiagaraSystem* ValidMovesHighlightFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Tile")
	UNiagaraSystem* PreviousMoveHighlightFX;
};