// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DataAsset.h"

#include "ChessBoardData.generated.h"

struct FChessPieceInfo;

UCLASS()
class CHESS_API UChessBoardData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Mesh|WhitePieces")
	UStaticMesh* WhiteKing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Mesh|WhitePieces")
	UStaticMesh* WhiteQueen;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Mesh|WhitePieces")
	UStaticMesh* WhiteBishop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Mesh|WhitePieces")
	UStaticMesh* WhiteKnight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Mesh|WhitePieces")
	UStaticMesh* WhiteRook;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Mesh|WhitePieces")
	UStaticMesh* WhitePawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Mesh|BlackPieces")
	UStaticMesh* BlackKing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Mesh|BlackPieces")
	UStaticMesh* BlackQueen;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Mesh|BlackPieces")
	UStaticMesh* BlackBishop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Mesh|BlackPieces")
	UStaticMesh* BlackKnight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Mesh|BlackPieces")
	UStaticMesh* BlackRook;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|Mesh|BlackPieces")
	UStaticMesh* BlackPawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|WhitePieces")
	TArray<FChessPieceInfo> WhiteChessPiecesInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "+Chess|BlackPieces")
	TArray<FChessPieceInfo> BlackChessPiecesInfo;
};