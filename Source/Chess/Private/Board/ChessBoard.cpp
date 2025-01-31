// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#include "Board/ChessBoard.h"

#include "Board/ChessPiece.h"
#include "Board/ChessTile.h"
#include "Core/ChessGameMode.h"
#include "Core/ChessPlayerController.h"
#include "Data/ChessBoardData.h"

#include "Kismet/GameplayStatics.h"

#define PRINTSTRING(Colour, DebugMessage) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, Colour, DebugMessage);

AChessBoard::AChessBoard() :
	ChessTileClass(AChessTile::StaticClass()),
	ChessPieceClass(AChessPiece::StaticClass()),
	NumOfMovesSinceLastCapture(0),
	MaxSearchDepthForAIMove(3)
{
	PrimaryActorTick.bCanEverTick = true;

	// DefaultSceneRootComponent
	DefaultSceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRootComponent"));
	SetRootComponent(DefaultSceneRootComponent);

	static ConstructorHelpers::FObjectFinder<UChessBoardData> ChessBoardDataAsset(TEXT("/Script/Chess.ChessBoardData'/Game/+Chess/Data/DA_ChessBoardData.DA_ChessBoardData'"));
	if (ChessBoardDataAsset.Succeeded()) ChessBoardData = ChessBoardDataAsset.Object;
}

void AChessBoard::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AChessBoard::BeginPlay()
{
	Super::BeginPlay();

	InitializeBoard();
}

void AChessBoard::InitializeBoard()
{
	// Reset all values
	ChessBoardInfo = FChessBoardInfo();
	ChessTileLocations.Empty();

	for (AChessTile* Tile : ChessTiles) if (Tile) Tile->Destroy();
	ChessTiles.Empty();
	
	for (AChessPiece* Piece : WhiteChessPieces) if (Piece) Piece->Destroy();
	WhiteChessPieces.Empty();
	
	for (AChessPiece* Piece : BlackChessPieces) if (Piece) Piece->Destroy();
	BlackChessPieces.Empty();
	
	// Initialize Board
	CreateBoard();

	SetupBoard();
}

void AChessBoard::CreateBoard()
{
	TArray<uint8> TileColourAtIndex = { 0, 1, 0, 1, 0, 1, 0, 1,
										1, 0, 1, 0, 1, 0, 1, 0,
										0, 1, 0, 1, 0, 1, 0, 1,
										1, 0, 1, 0, 1, 0, 1, 0,
										0, 1, 0, 1, 0, 1, 0, 1,
										1, 0, 1, 0, 1, 0, 1, 0,
										0, 1, 0, 1, 0, 1, 0, 1,
										1, 0, 1, 0, 1, 0, 1, 0 };

	const float TileSize = 250.f;

	int32 Offset = (TileSize * (static_cast<float>(8) / 2)) - (TileSize / 2);

	for (int32 i = 0; i < 8; i++)
		for (int32 j = 0; j < 8; j++)
			ChessTileLocations.AddUnique(FVector((i * TileSize) - Offset, (j * TileSize) - Offset, 0.f));
	
	// Ensure the board has exactly 64 squares
	ChessBoardInfo.TilesInfo.SetNum(64);

	// Clear all squares to empty pieces
	for (FChessTileInfo& Square : ChessBoardInfo.TilesInfo)
		Square = FChessTileInfo(); // Initialize as an empty piece

	for (int32 i = 0; i < 64; i++)
	{
		if (AChessTile* Tile = Cast<AChessTile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ChessTileClass, FTransform(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn, this)))
		{
			Tile->ChessTileInfo.ChessTilePositionIndex = i;
			Tile->ChessTileInfo.bIsWhite = (TileColourAtIndex[i] == 1);

			UGameplayStatics::FinishSpawningActor(Tile, FTransform());

			Tile->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
			Tile->SetActorRelativeLocation(ChessTileLocations[i]);
			ChessTiles.AddUnique(Tile);
			ChessBoardInfo.TilesInfo[i] = Tile->ChessTileInfo;
		}
	}
}

void AChessBoard::SetupBoard()
{
	/*
	if (!ParseFEN(DefaultFEN, ChessBoardInfo)) return PRINTSTRING(FColor::Red, "Invalid FEN String");

	// Spawn Chess Pieces
	for (FChessTileInfo& TileInfo : ChessBoardInfo.TilesInfo)
	{
		if (TileInfo.ChessPieceOnTile.ChessPiecePositionIndex == -1) continue;

		if (TileInfo.ChessPieceOnTile.bIsWhite)
		{
			WhiteChessPieces.AddUnique(SpawnChessPiece(TileInfo.ChessPieceOnTile));
		}
		else
		{
			BlackChessPieces.AddUnique(SpawnChessPiece(TileInfo.ChessPieceOnTile));
		}
	}
	*/

	TArray<FChessPieceInfo> WhiteChessPiecesInfo;
	TArray<FChessPieceInfo> BlackChessPiecesInfo;
	
	// Pawns
	for (int32 i = 8; i <= 15; i++)	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Pawn, EChessPieceSide::None, i, TArray<int32>()));

	for (int32 i = 48; i <= 55; i++) BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Pawn, EChessPieceSide::None, i, TArray<int32>()));

	// Rooks
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Rook, EChessPieceSide::QueenSide, 0, TArray<int32>()));
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Rook, EChessPieceSide::KingSide, 7, TArray<int32>()));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Rook, EChessPieceSide::QueenSide, 56, TArray<int32>()));
	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Rook, EChessPieceSide::KingSide, 63, TArray<int32>()));

	// Knights
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Knight, EChessPieceSide::None, 1, TArray<int32>()));
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Knight, EChessPieceSide::None, 6, TArray<int32>()));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Knight, EChessPieceSide::None, 57, TArray<int32>()));
	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Knight, EChessPieceSide::None, 62, TArray<int32>()));

	// Bishops
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Bishop, EChessPieceSide::None, 2, TArray<int32>()));
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Bishop, EChessPieceSide::None, 5, TArray<int32>()));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Bishop, EChessPieceSide::None, 58, TArray<int32>()));
	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Bishop, EChessPieceSide::None, 61, TArray<int32>()));

	// Queens
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::Queen, EChessPieceSide::None, 3, TArray<int32>()));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::Queen, EChessPieceSide::None, 59, TArray<int32>()));

	// Kings
	WhiteChessPiecesInfo.AddUnique(FChessPieceInfo(true, EChessPieceType::King, EChessPieceSide::None, 4, TArray<int32>()));

	BlackChessPiecesInfo.AddUnique(FChessPieceInfo(false, EChessPieceType::King, EChessPieceSide::None, 60, TArray<int32>()));

	// Spawn White Chess Pieces
	for (int32 i = 0; i < WhiteChessPiecesInfo.Num(); i++)
	{
		WhiteChessPieces.AddUnique(SpawnChessPiece(WhiteChessPiecesInfo[i]));
	}

	// Spawn Black Chess Pieces
	for (int32 i = 0; i < BlackChessPiecesInfo.Num(); i++)
	{
		BlackChessPieces.AddUnique(SpawnChessPiece(BlackChessPiecesInfo[i]));
	}
}

bool AChessBoard::ParseFEN(const FString& FEN, FChessBoardInfo& BoardInfo)
{
	TMap<TCHAR, EChessPieceType> PieceTypeMap = {
		{ 'K', EChessPieceType::King }, { 'Q', EChessPieceType::Queen },
		{ 'R', EChessPieceType::Rook }, { 'B', EChessPieceType::Bishop },
		{ 'N', EChessPieceType::Knight }, { 'P', EChessPieceType::Pawn }
	};

	TArray<FString> FENSections;
	FEN.ParseIntoArray(FENSections, TEXT(" "), true);
	if (FENSections.Num() == 0) return false; // Invalid FEN

	// FEN section 1 represents board
	int32 BoardIndex = 0;

	TArray<FString> BoardRanks;
	FENSections[0].ParseIntoArray(BoardRanks, TEXT("/"), true);

	for (int32 Rank = BoardRanks.Num() - 1; Rank >= 0; Rank--)
	{
		FString BoardRank = BoardRanks[Rank];
		
		for (int32 File = 0; File < BoardRank.Len(); File++)
		{
			TCHAR Symbol = BoardRank[File];

			if (FChar::IsDigit(Symbol)) // Empty squares
			{
				BoardIndex += Symbol - '0';
			}
			else // Chess piece
			{
				bool bIsWhite = FChar::IsUpper(Symbol);
				EChessPieceType PieceType = PieceTypeMap[FChar::ToUpper(Symbol)];

				FChessPieceInfo NewPiece(
					bIsWhite,
					PieceType,
					EChessPieceSide::None, // Set side later if needed
					BoardIndex,
					TArray<int32>() // Empty valid moves initially
				);

				BoardInfo.TilesInfo[BoardIndex].ChessPieceOnTile = NewPiece;
				BoardIndex++;
			}
		}
	}


	/*
	FString BoardState = FENSections[0];
	int32 BoardIndex = 0;

	for (int32 i = 0; i < BoardState.Len(); i++)
	{
		TCHAR Symbol = BoardState[i];

		if (Symbol == '/') continue; // Skip row separators

		if (FChar::IsDigit(Symbol)) // Empty squares
		{
			BoardIndex += Symbol - '0';
		}
		else // Chess piece
		{
			bool bIsWhite = FChar::IsUpper(Symbol);
			EChessPieceType PieceType = PieceTypeMap[FChar::ToUpper(Symbol)];

			FChessPieceInfo NewPiece(
				bIsWhite,
				PieceType,
				EChessPieceSide::None, // Set side later if needed
				BoardIndex,
				TArray<int32>() // Empty valid moves initially
			);

			BoardInfo.TilesInfo[BoardIndex].ChessPieceOnTile = NewPiece;
			BoardIndex++;
		}
	}
	*/

	// FEN section 2 represents whose move it is 'w' for white 'b' for black
	BoardInfo.bIsWhiteTurn = (FENSections[1] == "w");

	// FEN section 3 represents Castiling Rights
	FString CastlingRights = FENSections[2];
	bool bCanWhiteCastleKingSide = CastlingRights.Contains("K");
	bool bCanWhiteCastleQueenSide = CastlingRights.Contains("Q");
	bool bCanBlackCastleKingSide = CastlingRights.Contains("k");
	bool bCanBlackCastleQueenSide = CastlingRights.Contains("q");

	// TODO : UPDATE CASTLING CODE AND REDUCE NUM OF VARIABLES TO TRACK CASTLING

	// FEN section 1 represents Enpassant Target Square
	FString EnpassantTarget = FENSections[3];
	int32 EnpassantIndex = -1;
	FChessPieceInfo EnpassantPiece = FChessPieceInfo();
	
	if (EnpassantTarget != "-")
	{
		int File = EnpassantTarget[0] - 'a';
		int Rank = EnpassantTarget[1] - '1';
		EnpassantIndex = Rank * 8 + File;
	}
	
	if (EnpassantIndex > -1)
	{
		EnpassantPiece = BoardInfo.TilesInfo[BoardInfo.bIsWhiteTurn ? EnpassantIndex - 8 : EnpassantIndex + 8].ChessPieceOnTile;
		if (EnpassantPiece.ChessPiecePositionIndex == -1) EnpassantPiece = FChessPieceInfo();
	}

	BoardInfo.EnpassantPawn = EnpassantPiece;
	BoardInfo.EnpassantTileIndex = EnpassantIndex;

	return true;
}

AChessPiece* AChessBoard::SpawnChessPiece(FChessPieceInfo ChessPieceInfo)
{
	AChessPiece* ChessPiece = Cast<AChessPiece>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ChessPieceClass, FTransform(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn, this));
	if (ChessPiece)
	{
		ChessPiece->ChessPieceInfo = ChessPieceInfo;

		UGameplayStatics::FinishSpawningActor(ChessPiece, FTransform());

		ChessPiece->SetActorLocation(ChessTiles[ChessPieceInfo.ChessPiecePositionIndex]->GetActorLocation());

		ChessTiles[ChessPieceInfo.ChessPiecePositionIndex]->ChessTileInfo.ChessPieceOnTile = ChessPieceInfo;
		ChessTiles[ChessPieceInfo.ChessPiecePositionIndex]->ChessPieceOnTile = ChessPiece;

		ChessBoardInfo.TilesInfo[ChessPieceInfo.ChessPiecePositionIndex].ChessPieceOnTile = ChessPieceInfo;
	}

	return ChessPiece;
}

bool AChessBoard::HightlightValidMovesOnTile(bool bHighlight, int32 TileIndex)
{
	if (ChessBoardInfo.TilesInfo[TileIndex].ChessPieceOnTile.ChessPiecePositionIndex == -1)
	{
		PRINTSTRING(FColor::Red, "ChessPieceOnTile is Invalid : ChessBoard.cpp > HighlightValidMovesOnTile()");
		return false;
	}

	if (bHighlight)
	{
		for (int32 ValidMove : ChessBoardInfo.TilesInfo[TileIndex].ChessPieceOnTile.ValidMoves) HighlightedTiles.AddUnique(ChessTiles[ValidMove]->ChessTileInfo);

		if (HighlightedTiles.Num() == 0) return false;

		for (FChessTileInfo& Tile : HighlightedTiles) ChessTiles[Tile.ChessTilePositionIndex]->HighlightTile(true);

	}
	else
	{
		for (FChessTileInfo& Tile : HighlightedTiles) ChessTiles[Tile.ChessTilePositionIndex]->HighlightTile(false);

		HighlightedTiles.Empty();
	}

	return true;
}

void AChessBoard::GenerateAllValidMoves(FChessBoardInfo& BoardInfo, bool bIsWhiteTurn, bool bGenerateOnlyCaptures)
{
	ClearAllValidMoves(BoardInfo);

	UpdateAttackStatusOfTiles(BoardInfo);

	for (FChessTileInfo& Tile : BoardInfo.TilesInfo)
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1) // if theres a piece on the tile
			if (Tile.ChessPieceOnTile.bIsWhite == bIsWhiteTurn) // if piece colour is same as current player turn
				CalculateValidMovesForPiece(BoardInfo, Tile.ChessPieceOnTile, bGenerateOnlyCaptures);
}

void AChessBoard::ClearAllValidMoves(FChessBoardInfo& BoardInfo)
{
	for (FChessTileInfo& Tile : BoardInfo.TilesInfo)
	{
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1) // if theres a piece on the tile
		{
			Tile.ChessPieceOnTile.ValidMoves.Empty();
		}
	}
}

void AChessBoard::UpdateAttackStatusOfTiles(FChessBoardInfo& BoardInfo)
{
	// Reset values
	for (FChessTileInfo& Tile : BoardInfo.TilesInfo)
	{
		Tile.bIsTileUnderAttackByWhitePiece = false;
		Tile.bIsTileUnderAttackByBlackPiece = false;
	}

	BoardInfo.bIsWhiteKingUnderCheck = false;
	BoardInfo.bIsBlackKingUnderCheck = false;



	for (FChessTileInfo& Tile : BoardInfo.TilesInfo)
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1) // if theres a piece on the tile
			UpdateTilesUnderAttackByPiece(BoardInfo, Tile.ChessPieceOnTile);

	for (FChessTileInfo& Tile : BoardInfo.TilesInfo)
	{
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex == -1) continue; // if theres no piece on the tile

		if (Tile.ChessPieceOnTile.ChessPieceType != EChessPieceType::King) continue; // if th piece on tile is not a king

		if (Tile.ChessPieceOnTile.bIsWhite)
		{
			// if White kings tile is under attack by black piece
			BoardInfo.bIsWhiteKingUnderCheck = Tile.bIsTileUnderAttackByBlackPiece;
		}
		else
		{
			// if Black kings tile is under attack by white piece
			BoardInfo.bIsBlackKingUnderCheck = Tile.bIsTileUnderAttackByWhitePiece;
		}
	}
}

void AChessBoard::UpdateTilesUnderAttackByPiece(FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece)
{
	TArray<int32> ValidMovePositions;

	switch (Piece.ChessPieceType)
	{
	case EChessPieceType::King:
		for (FVector2D& Position : KingMovePositionTileOffsets)
		{
			FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + Position;
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

				// ignore tile if friendly piece is on that tile
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
						continue;

				ValidMovePositions.Add(RelativePositionIndex);
			}
		}
		break;
	case EChessPieceType::Queen:
	{
		for (FVector2D& Position : QueenMovePositionDirections)
		{
			for (int32 i = 1; i < 8; i++)
			{
				FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + (Position * i);
				if (IsPositionWithinBounds(RelativePosition))
				{
					int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

					// if a piece is on that tile...
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
					{
						// and if its a friendly piece
						if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
							break;

						ValidMovePositions.Add(RelativePositionIndex);

						// if it not opponents king
						if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPieceType != EChessPieceType::King)
							break;
					}

					ValidMovePositions.Add(RelativePositionIndex);
				}
			}
		}
		break;
	}
	case EChessPieceType::Bishop:
	{
		for (FVector2D& Position : BishopMovePositionDirections)
		{
			for (int32 i = 1; i < 8; i++)
			{
				FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + (Position * i);
				if (IsPositionWithinBounds(RelativePosition))
				{
					int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

					// if a piece is on that tile...
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
					{
						// and if its a friendly piece
						if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
							break;

						ValidMovePositions.Add(RelativePositionIndex);

						// if it not opponents king
						if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPieceType != EChessPieceType::King)
							break;
					}

					ValidMovePositions.Add(RelativePositionIndex);
				}
			}
		}
		break;
	}
	case EChessPieceType::Knight:
	{
		for (FVector2D& Position : KnightMovePositionTileOffsets)
		{
			FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + Position;
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

				// ignore tile if friendly piece is on that tile
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
						continue;

				ValidMovePositions.Add(RelativePositionIndex);
			}
		}
		break;
	}
	case EChessPieceType::Rook:
	{
		for (FVector2D& Position : RookMovePositionDirections)
		{
			for (int32 i = 1; i < 8; i++)
			{
				FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + (Position * i);
				if (IsPositionWithinBounds(RelativePosition))
				{
					int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

					// if a piece is on that tile...
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
					{
						// and if its a friendly piece
						if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
							break;

						ValidMovePositions.Add(RelativePositionIndex);

						// if it not opponents king
						if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPieceType != EChessPieceType::King)
							break;
					}

					ValidMovePositions.Add(RelativePositionIndex);
				}
			}
		}
		break;
	}
	case EChessPieceType::Pawn:
	{
		if (Piece.bIsWhite)
		{
			// Diagonal tiles under attack
			if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, -1)))
			{
				// if a piece is present on diagonal tile
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// if the piece is not friendly
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 - 1);
				}
				else // if the tile is empty
				{
					ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 - 1);
				}
			}

			if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, 1)))
			{
				// if a piece is present on diagonal tile
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// if the piece is not friendly
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 + 1);
				}
				else // if the tile is empty
				{
					ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 + 1);
				}
			}

			// En Passant White -> Black
			if (Piece.GetChessPiecePositionFromIndex().X == 4 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, -1)))
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 - 1);

			if (Piece.GetChessPiecePositionFromIndex().X == 4 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, 1)))
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 + 1);
		}
		else
		{
			// Diagonal tiles under attack
			if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, -1)))
			{
				// if a piece is present on diagonal tile
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// if the piece is not friendly
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex - 8 - 1);
				}
				else // if the tile is empty
				{
					ValidMovePositions.Add(Piece.ChessPiecePositionIndex - 8 - 1);
				}
			}

			if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, 1)))
			{
				// if a piece is present on diagonal tile
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// if the piece is not friendly
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex + 8 + 1);
				}
				else // if the tile is empty
				{
					ValidMovePositions.Add(Piece.ChessPiecePositionIndex - 8 + 1);
				}
			}

			// En Passant Black -> White
			if (Piece.GetChessPiecePositionFromIndex().X == 3 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, -1)))
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex - 8 - 1);

			if (Piece.GetChessPiecePositionFromIndex().X == 3 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, 1)))
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
						ValidMovePositions.Add(Piece.ChessPiecePositionIndex - 8 + 1);
		}
		break;
	}
	default:
		break;
	}

	for (int32 Position : ValidMovePositions)
	{
		if (Piece.bIsWhite)
			BoardInfo.TilesInfo[Position].bIsTileUnderAttackByWhitePiece = true;
		else
			BoardInfo.TilesInfo[Position].bIsTileUnderAttackByBlackPiece = true;
	}
}

void AChessBoard::CalculateValidMovesForPiece(FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures)
{
	TArray<FChessTileInfo> ValidMovesBeforeFilteringForCheck;

	switch (Piece.ChessPieceType)
	{
		case EChessPieceType::King:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForKing(BoardInfo, Piece, bGenerateOnlyCaptures);
			break;
		case EChessPieceType::Queen:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForQueen(BoardInfo, Piece, bGenerateOnlyCaptures);
			break;
		case EChessPieceType::Bishop:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForBishop(BoardInfo, Piece, bGenerateOnlyCaptures);
			break;
		case EChessPieceType::Knight:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForKnight(BoardInfo, Piece, bGenerateOnlyCaptures);
			break;
		case EChessPieceType::Rook:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForRook(BoardInfo, Piece, bGenerateOnlyCaptures);
			break;
		case EChessPieceType::Pawn:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForPawn(BoardInfo, Piece, bGenerateOnlyCaptures);
			break;
		default:
			break;
	}

	for (FChessTileInfo& ValidTile : FilterMovesForCheck(BoardInfo, Piece, ValidMovesBeforeFilteringForCheck))
		Piece.ValidMoves.Add(ValidTile.ChessTilePositionIndex);
		
	if (Piece.ChessPiecePositionIndex > -1) BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex].ChessPieceOnTile = Piece;
	else PRINTSTRING(FColor::Red, "ChessPiecePositionIndex is -1 in CalculateValidMovesForPiece(), ChessBoard.cpp");
}

TArray<FChessTileInfo> AChessBoard::CalculateValidMoveTilesForKing(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures)
{
	TArray<FChessTileInfo> ValidTiles;
		
	// One Tile Omnidirectional moves
	for (FVector2D& Position : KingMovePositionTileOffsets)
	{
		FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + Position;
		
		if (IsPositionWithinBounds(RelativePosition))
		{
			int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

			// if theres a piece on that tile...
			if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
			{
				// ignore if friendly piece
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
					continue;
			}
			else // if no piece is on that tile...
			{
				// and generate only captures is true, then ignore tile
				if (bGenerateOnlyCaptures)
					continue;
			}
		
			// ignore tile if its is under attack by enemy piece
			if ((Piece.bIsWhite)
				? BoardInfo.TilesInfo[RelativePositionIndex].bIsTileUnderAttackByBlackPiece
				: BoardInfo.TilesInfo[RelativePositionIndex].bIsTileUnderAttackByWhitePiece)
				continue;
		
			ValidTiles.Add(BoardInfo.TilesInfo[RelativePositionIndex]);
		}
	}

	if (bGenerateOnlyCaptures) return ValidTiles;

	// Castling Moves if available
	if (Piece.bIsWhite)
	{
		// if White king is not under check
		if (!BoardInfo.bIsWhiteKingUnderCheck)
		{
			// if White king side rook alive
			if (BoardInfo.bIsWhiteKingSideRookAlive)
				// if white king and king side rook have not moved
				if (!BoardInfo.HasWhiteKingOrKingSideRookMoved())
					// if no piece is present one tile to the right
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 1].ChessPieceOnTile.ChessPiecePositionIndex == -1)
						// if one tile to the right isnt under attack
						if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 1].bIsTileUnderAttackByBlackPiece)
							// if no piece is present two tiles to the right
							if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 2].ChessPieceOnTile.ChessPiecePositionIndex == -1)
								// if two tiles to the right isnt under attack
								if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 2].bIsTileUnderAttackByBlackPiece)
									ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 2]);
		
			// if White queen side rook alive
			if (BoardInfo.bIsWhiteQueenSideRookAlive)
				// if white king and queen side rook have not moved
				if (!BoardInfo.HasWhiteKingOrQueenSideRookMoved())
					// if no piece is present one tile to the left
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 1].ChessPieceOnTile.ChessPiecePositionIndex == -1)
						// if one tile to the left isnt under attack
						if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 1].bIsTileUnderAttackByBlackPiece)
							// if no piece is present two tiles to the left
							if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 2].ChessPieceOnTile.ChessPiecePositionIndex == -1)
								// if two tiles to the left isnt under attack
								if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 2].bIsTileUnderAttackByBlackPiece)
									// if no piece is present three tiles to the left
									if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 3].ChessPieceOnTile.ChessPiecePositionIndex == -1)
										ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 2]);
		}
	}
	else
	{
		// if Black king is not under check
		if (!BoardInfo.bIsBlackKingUnderCheck)
		{
			// if black king side rook alive
			if (BoardInfo.bIsBlackKingSideRookAlive)
				// if black king and king side rook have not moved
				if (!BoardInfo.HasBlackKingOrKingSideRookMoved())
					// if no piece is present one tile to the right
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 1].ChessPieceOnTile.ChessPiecePositionIndex == -1)
						// if one tile to the right isnt under attack
						if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 1].bIsTileUnderAttackByWhitePiece)
							// if no piece is present two tiles to the right
							if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 2].ChessPieceOnTile.ChessPiecePositionIndex == -1)
								// if two tiles to the right isnt under attack
								if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 2].bIsTileUnderAttackByWhitePiece)
									ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 2]);
		
			// if black queen side rook alive
			if (BoardInfo.bIsBlackQueenSideRookAlive)
				// if black king and queen side rook have not moved
				if (!BoardInfo.HasBlackKingOrQueenSideRookMoved())
					// if no piece is present one tile to the left
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 1].ChessPieceOnTile.ChessPiecePositionIndex == -1)
						// if one tile to the left isnt under attack
						if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 1].bIsTileUnderAttackByWhitePiece)
							// if no piece is present two tiles to the left
							if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 2].ChessPieceOnTile.ChessPiecePositionIndex == -1)
								// if two tiles to the left isnt under attack
								if (!BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 2].bIsTileUnderAttackByWhitePiece)
									// if no piece is present three tiles to the left
									if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 3].ChessPieceOnTile.ChessPiecePositionIndex == -1)
										ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 2]);
		}
	}
		
	return ValidTiles;
}

TArray<FChessTileInfo> AChessBoard::CalculateValidMoveTilesForQueen(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures)
{
	TArray<FChessTileInfo> ValidTiles;
		
	for (FVector2D& Position : QueenMovePositionDirections)
	{
		for (int32 i = 1; i < 8; i++)
		{
			FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + (Position * i);
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;
		
				// if a piece is on that tile...
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// and if its an opponent piece
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);
		
					break;
				}

				if (!bGenerateOnlyCaptures)
					ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);
			}
		}
	}
			
	return ValidTiles;
}

TArray<FChessTileInfo> AChessBoard::CalculateValidMoveTilesForBishop(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures)
{
	TArray<FChessTileInfo> ValidTiles;

	for (FVector2D& Position : BishopMovePositionDirections)
	{
		for (int32 i = 1; i < 8; i++)
		{
			FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + (Position * i);
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

				// if a piece is on that tile...
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// and if its an opponent piece
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);

					break;
				}

				if (!bGenerateOnlyCaptures)
					ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);
			}
		}
	}

	return ValidTiles;
}

TArray<FChessTileInfo> AChessBoard::CalculateValidMoveTilesForKnight(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures)
{
	TArray<FChessTileInfo> ValidTiles;
		
	for (FVector2D& Position : KnightMovePositionTileOffsets)
	{
		FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + Position;
		if (IsPositionWithinBounds(RelativePosition))
		{
			int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

			if (bGenerateOnlyCaptures)
			{
				// if a piece is on that tile...
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
					// and if its an opponent piece
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);
			}
			else
			{
				// ignore tile if friendly piece is on that tile
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite == Piece.bIsWhite)
						continue;

				ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);
			}
		}
	}
		
	return ValidTiles;
}

TArray<FChessTileInfo> AChessBoard::CalculateValidMoveTilesForRook(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures)
{
	TArray<FChessTileInfo> ValidTiles;

	for (FVector2D& Position : RookMovePositionDirections)
	{
		for (int32 i = 1; i < 8; i++)
		{
			FVector2D RelativePosition = Piece.GetChessPiecePositionFromIndex() + (Position * i);
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

				// if a piece is on that tile...
				if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.ChessPiecePositionIndex > -1)
				{
					// and if its an opponent piece
					if (BoardInfo.TilesInfo[RelativePositionIndex].ChessPieceOnTile.bIsWhite != Piece.bIsWhite)
						ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);

					break;
				}

				if (!bGenerateOnlyCaptures)
					ValidTiles.AddUnique(BoardInfo.TilesInfo[RelativePositionIndex]);
			}
		}
	}

	return ValidTiles;
}

TArray<FChessTileInfo> AChessBoard::CalculateValidMoveTilesForPawn(const FChessBoardInfo& BoardInfo, FChessPieceInfo& Piece, bool bGenerateOnlyCaptures)
{
	TArray<FChessTileInfo> ValidTiles;
		
	if (Piece.bIsWhite)
	{
		if (!bGenerateOnlyCaptures)
		{
			if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, 0)))
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile front
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8]);
	
			if (Piece.GetChessPiecePositionFromIndex().X == 1 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(2, 0)))
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile front
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 16].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in two tiles front
						ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 16]);
		}
	
		// Diagonal Captures
		if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, -1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex > -1) // if a piece on diagonal tile present then make it a valid move position
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite) // and if the piece on diagonal tile is not that same colour as to current piece
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1]);

		if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, 1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex > -1) // if a piece on diagonal tile present then make it a valid move position
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite) // and if the piece on diagonal tile is not that same colour as to current piece
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1]);
	
		// En Passant White -> Black
		if (Piece.GetChessPiecePositionFromIndex().X == 4 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, -1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 - 1]);
	
		if (Piece.GetChessPiecePositionFromIndex().X == 4 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(1, 1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex + 8 + 1]);
	}
	else
	{
		if (!bGenerateOnlyCaptures)
		{
			if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, 0)))
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile front
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8]);
	
			if (Piece.GetChessPiecePositionFromIndex().X == 6 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-2, 0)))
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile front
					if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 16].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in two tiles front
						ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 16]);
		}

		// Diagonal Captures
		if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, 1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex > -1) // if a piece on diagonal tile present then make it a valid move position
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite) // and if the piece on diagonal tile is not that same colour as to current piece
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1]);
	
		if (IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, -1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex > -1) // if a piece on diagonal tile present then make it a valid move position
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile.bIsWhite != Piece.bIsWhite) // and if the piece on diagonal tile is not that same colour as to current piece
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1]);
	
		// En Passant Black -> White
		if (Piece.GetChessPiecePositionFromIndex().X == 3 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, 1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 + 1]);
	
		if (Piece.GetChessPiecePositionFromIndex().X == 3 && IsPositionWithinBounds(Piece.GetChessPiecePositionFromIndex() + FVector2D(-1, -1)))
			if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile.ChessPiecePositionIndex == -1) // if no piece is present in one tile diagonal
				if (BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1].ChessTilePositionIndex == BoardInfo.EnpassantTileIndex) // and if diagonal tile is available for enpassant
					ValidTiles.AddUnique(BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex - 8 - 1]);
	}
	
	return ValidTiles;
}

TArray<FChessTileInfo> AChessBoard::FilterMovesForCheck(FChessBoardInfo Board, FChessPieceInfo& Piece, TArray<FChessTileInfo> ValidMovesBeforeFilteration)
{
	TArray<FChessTileInfo> ValidMovesAfterFilteration;

	for (FChessTileInfo ValidMove : ValidMovesBeforeFilteration)
	{
		FChessBoardInfo SimulatedBoard = Board;

		SimulateMove(SimulatedBoard, Piece, ValidMove.ChessTilePositionIndex);
		
		if (!IsKingInCheck(SimulatedBoard, Piece.bIsWhite)) ValidMovesAfterFilteration.AddUnique(ValidMove);
	}

	return ValidMovesAfterFilteration;
}

void AChessBoard::SimulateMove(FChessBoardInfo& BoardInfo, FChessPieceInfo Piece, int32 ToPosition)
{
	FChessTileInfo& FromTile = BoardInfo.TilesInfo[Piece.ChessPiecePositionIndex];
	FChessTileInfo& ToTile = BoardInfo.TilesInfo[ToPosition];

	// Move the piece to the target tile
	ToTile.ChessPieceOnTile = FromTile.ChessPieceOnTile;
	FromTile.ChessPieceOnTile = FChessPieceInfo(); // Clear the original tile

	// Update Piece Position
	ToTile.ChessPieceOnTile.ChessPiecePositionIndex = ToTile.ChessTilePositionIndex;

	switch (Piece.ChessPieceType)
	{
	case EChessPieceType::King:
		// Handle castling (if the king moves two tiles)
		if (FMath::Abs(ToPosition - Piece.ChessPiecePositionIndex) == 2)
		{
			if (ToPosition > Piece.ChessPiecePositionIndex)
			{
				// King-side castling
				SimulateMove(BoardInfo, BoardInfo.TilesInfo[ToPosition + 1].ChessPieceOnTile, ToPosition - 1);
			}
			else
			{
				// Queen-side castling
				SimulateMove(BoardInfo, BoardInfo.TilesInfo[ToPosition - 2].ChessPieceOnTile, ToPosition + 1);
			}
		}

		if (Piece.bIsWhite)
		{
			if (!BoardInfo.bHasWhiteKingMoved) BoardInfo.bHasWhiteKingMoved = true;
		}
		else
		{
			if (!BoardInfo.bHasBlackKingMoved) BoardInfo.bHasBlackKingMoved = true;
		}
		break;
	case EChessPieceType::Queen:
		break;
	case EChessPieceType::Bishop:
		break;
	case EChessPieceType::Knight:
		break;
	case EChessPieceType::Rook:
		if (Piece.bIsWhite)
		{
			switch (Piece.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (!BoardInfo.bHasWhiteKingSideRookMoved) BoardInfo.bHasWhiteKingSideRookMoved = true;
				break;
			case EChessPieceSide::QueenSide:
				if (!BoardInfo.bHasWhiteQueenSideRookMoved) BoardInfo.bHasWhiteQueenSideRookMoved = true;
				break;
			default:
				break;
			}
		}
		else
		{
			switch (Piece.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (!BoardInfo.bHasBlackKingSideRookMoved) BoardInfo.bHasBlackKingSideRookMoved = true;
				break;
			case EChessPieceSide::QueenSide:
				if (!BoardInfo.bHasBlackQueenSideRookMoved) BoardInfo.bHasBlackQueenSideRookMoved = true;
				break;
			default:
				break;
			}
		}
		break;
	case EChessPieceType::Pawn:
		// Handle enpassant captures
		if (ToPosition == BoardInfo.EnpassantTileIndex)
		{
			BoardInfo.TilesInfo[BoardInfo.EnpassantPawn.ChessPiecePositionIndex].ChessPieceOnTile = FChessPieceInfo(); // Remove the captured pawn
		}

		// Update enpassant target
		if (FMath::Abs(ToPosition - Piece.ChessPiecePositionIndex) == 16)
		{
			// Set the enpassant target to the tile behind the pawn
			BoardInfo.EnpassantTileIndex = ToTile.ChessPieceOnTile.bIsWhite ? ToTile.ChessPieceOnTile.ChessPiecePositionIndex - 8 : ToTile.ChessPieceOnTile.ChessPiecePositionIndex + 8;
			BoardInfo.EnpassantPawn = ToTile.ChessPieceOnTile;
		}
		else
		{
			// Clear the enpassant target
			BoardInfo.EnpassantTileIndex = -1;
			BoardInfo.EnpassantPawn = FChessPieceInfo();
		}

		// Handle pawn promotion (if a pawn reaches the opposite end of the board)
		if ((ToPosition < 8 || ToPosition >= 56))
		{
			// Promote to a queen (default behavior)
			ToTile.ChessPieceOnTile.ChessPieceType = EChessPieceType::Queen;
		}
		break;
	default:
		break;
	}
}

float AChessBoard::MakeMove(AChessTile* StartTile, AChessTile* EndTile, bool bIsAIMove)
{
	if (!StartTile)
	{
		PRINTSTRING(FColor::Red, "StartTile is INVALID in ChessBoard");
		return 1.f;
	}
	if (!EndTile)
	{
		PRINTSTRING(FColor::Red, "EndTile is INVALID in ChessBoard");
		return 1.f;
	}

	AChessPiece* ChessPiece = Cast<AChessPiece>(StartTile->ChessPieceOnTile);
	if (!ChessPiece)
	{
		PRINTSTRING(FColor::Red, "ChessPiece is INVALID in ChessBoard");
		return 1.f;
	}

	FChessPieceInfo Piece = StartTile->ChessPieceOnTile->ChessPieceInfo;
	int32 ToPosition = EndTile->ChessTileInfo.ChessTilePositionIndex;

	FChessTileInfo& FromTile = ChessBoardInfo.TilesInfo[Piece.ChessPiecePositionIndex];
	FChessTileInfo& ToTile = ChessBoardInfo.TilesInfo[ToPosition];

	// capture enemy piece if present (there will never be a friendly piece on EndTile if the Valid move generation code works properly)
	if (EndTile->ChessPieceOnTile)
	{
		EndTile->ChessPieceOnTile->CapturePiece();
		NumOfMovesSinceLastCapture = 0;
	}
	else
	{
		NumOfMovesSinceLastCapture++;
	}

	// Move the piece to the target tile
	float TimeTakenToMovePiece = ChessPiece->MovePiece(EndTile);

	// Update ChessPieceOnTile
	ToTile.ChessPieceOnTile = FromTile.ChessPieceOnTile;
	FromTile.ChessPieceOnTile = FChessPieceInfo(); // Clear the original tile

	EndTile->ChessPieceOnTile = StartTile->ChessPieceOnTile;
	StartTile->ChessPieceOnTile = nullptr;

	// Update Piece
	ToTile.ChessPieceOnTile.ChessPiecePositionIndex = ToTile.ChessTilePositionIndex;

	ChessPiece->ChessPieceInfo = ToTile.ChessPieceOnTile;

	// Update Tile Infos
	StartTile->ChessTileInfo = FromTile;
	EndTile->ChessTileInfo = ToTile;

	switch (Piece.ChessPieceType)
	{
	case EChessPieceType::King:
		// Handle castling (if the king moves two tiles)
		if (FMath::Abs(ToPosition - Piece.ChessPiecePositionIndex) == 2)
		{
			float TimeTakenToMoveRook = 1.f;
			if (ToPosition > Piece.ChessPiecePositionIndex)
			{
				// King-side castling
				MakeMove(ChessTiles[ToPosition + 1], ChessTiles[ToPosition - 1], bIsAIMove);
			}
			else
			{
				// Queen-side castling
				MakeMove(ChessTiles[ToPosition - 2], ChessTiles[ToPosition + 1], bIsAIMove);
			}
		}

		if (Piece.bIsWhite)
		{
			if (!ChessBoardInfo.bHasWhiteKingMoved) ChessBoardInfo.bHasWhiteKingMoved = true;
		}
		else
		{
			if (!ChessBoardInfo.bHasBlackKingMoved) ChessBoardInfo.bHasBlackKingMoved = true;
		}
		break;
	case EChessPieceType::Queen:
		break;
	case EChessPieceType::Bishop:
		break;
	case EChessPieceType::Knight:
		break;
	case EChessPieceType::Rook:
		if (Piece.bIsWhite)
		{
			switch (Piece.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (!ChessBoardInfo.bHasWhiteKingSideRookMoved) ChessBoardInfo.bHasWhiteKingSideRookMoved = true;
				break;
			case EChessPieceSide::QueenSide:
				if (!ChessBoardInfo.bHasWhiteQueenSideRookMoved) ChessBoardInfo.bHasWhiteQueenSideRookMoved = true;
				break;
			default:
				break;
			}
		}
		else
		{
			switch (Piece.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (!ChessBoardInfo.bHasBlackKingSideRookMoved) ChessBoardInfo.bHasBlackKingSideRookMoved = true;
				break;
			case EChessPieceSide::QueenSide:
				if (!ChessBoardInfo.bHasBlackQueenSideRookMoved) ChessBoardInfo.bHasBlackQueenSideRookMoved = true;
				break;
			default:
				break;
			}
		}
		break;
	case EChessPieceType::Pawn:
		// Handle enpassant captures
		if (ToPosition == ChessBoardInfo.EnpassantTileIndex && EnpassantPawn)
		{
			ChessBoardInfo.TilesInfo[ChessBoardInfo.EnpassantPawn.ChessPiecePositionIndex].ChessPieceOnTile = FChessPieceInfo();

			if (AChessTile* EnpassantPawnTile = ChessTiles[EnpassantPawn->ChessPieceInfo.ChessPiecePositionIndex]) EnpassantPawnTile->ChessPieceOnTile = nullptr;

			EnpassantPawn->CapturePiece();
		}

		// Update enpassant target
		if (FMath::Abs(ToPosition - Piece.ChessPiecePositionIndex) == 16)
		{
			EnableEnpassant(ChessPiece);
		}
		else
		{
			DisableEnpassant(ChessPiece->ChessPieceInfo.bIsWhite);
		}

		// Handle pawn promotion (if a pawn reaches the opposite end of the board)
		if ((ToPosition < 8 || ToPosition >= 56))
		{
			if (bIsAIMove)
			{
				ChessPiece->PromotePawn(FindBestPawnPromotionType(ChessBoardInfo, ChessPiece->ChessPieceInfo.ChessPiecePositionIndex, ChessPiece->ChessPieceInfo.bIsWhite));
			}
			else
			{
				AChessPlayerController* ChessPlayerController = Cast<AChessPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
				if (ChessPlayerController)
				{
					ChessPlayerController->SpawnPawnPromotionUI(ChessPiece);
				}
				else
				{
					PRINTSTRING(FColor::Red, "ChessPlayerController is INVALID in ChessBoard");

					// Promote to a queen (default behavior)
					ChessPiece->PromotePawn(EChessPieceType::Queen);
				}
			}
		}
		break;
	default:
		break;
	}

	return TimeTakenToMovePiece;
}

bool AChessBoard::IsKingInCheck(const FChessBoardInfo& BoardInfo, bool bIsWhiteKing)
{
	// Locate the king
	int32 KingPosition = -1;
	for (int32 i = 0; i < BoardInfo.TilesInfo.Num(); i++)
	{
		if (BoardInfo.TilesInfo[i].ChessPieceOnTile.ChessPiecePositionIndex > -1)
		{
			if (BoardInfo.TilesInfo[i].ChessPieceOnTile.ChessPieceType == EChessPieceType::King && BoardInfo.TilesInfo[i].ChessPieceOnTile.bIsWhite == bIsWhiteKing)
			{
				KingPosition = i;
				break;
			}
		}
	}

	if (KingPosition == -1) {
		// King not found on the board (shouldn't happen in a valid game state)
		return false;
	}

	 //Check for threats from opponent pieces
	for (int32 i = 0; i < BoardInfo.TilesInfo.Num(); i++)
	{
		FChessTileInfo OpponentPieceTile = BoardInfo.TilesInfo[i];

		if (OpponentPieceTile.ChessPieceOnTile.ChessPiecePositionIndex == -1) continue;

		if (OpponentPieceTile.ChessPieceOnTile.bIsWhite != bIsWhiteKing)
		{
			// Get valid moves for the opponent piece
			TArray<FChessTileInfo> OpponentMoves;

			switch (OpponentPieceTile.ChessPieceOnTile.ChessPieceType)
			{
			case EChessPieceType::King:
				OpponentMoves = CalculateValidMoveTilesForKing(BoardInfo, OpponentPieceTile.ChessPieceOnTile);
				break;
			case EChessPieceType::Queen:
				OpponentMoves = CalculateValidMoveTilesForQueen(BoardInfo, OpponentPieceTile.ChessPieceOnTile);
				break;
			case EChessPieceType::Bishop:
				OpponentMoves = CalculateValidMoveTilesForBishop(BoardInfo, OpponentPieceTile.ChessPieceOnTile);
				break;
			case EChessPieceType::Knight:
				OpponentMoves = CalculateValidMoveTilesForKnight(BoardInfo, OpponentPieceTile.ChessPieceOnTile);
				break;
			case EChessPieceType::Rook:
				OpponentMoves = CalculateValidMoveTilesForRook(BoardInfo, OpponentPieceTile.ChessPieceOnTile);
				break;
			case EChessPieceType::Pawn:
				OpponentMoves = CalculateValidMoveTilesForPawn(BoardInfo, OpponentPieceTile.ChessPieceOnTile);
				break;
			default:
				break;
			}

			// Check if the king's position is in the opponent's moves
			if (OpponentMoves.Contains(BoardInfo.TilesInfo[KingPosition]))
				return true; // The king is in check
		}
	}

	return false; // The king is not in check
}

void AChessBoard::EnableEnpassant(AChessPiece* EnpassantPiece)
{
	if (!EnpassantPiece) return PRINTSTRING(FColor::Red, "EnpassantPiece Invalid in ChessBoard");

	AChessPlayerController* ChessPlayerController = Cast<AChessPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (!ChessPlayerController) return PRINTSTRING(FColor::Red, "PlayerController Invalid in ChessBoard");

	// Set the enpassant target to the tile behind the pawn
	ChessBoardInfo.EnpassantPawn = EnpassantPiece->ChessPieceInfo;
	ChessBoardInfo.EnpassantTileIndex = ChessBoardInfo.EnpassantPawn.ChessPiecePositionIndex + ((ChessBoardInfo.EnpassantPawn.bIsWhite) ? -8 : 8);

	EnpassantPawn = EnpassantPiece;

	ChessPlayerController->OnPieceMoved.AddDynamic(this, &AChessBoard::DisableEnpassant);
}

void AChessBoard::DisableEnpassant(bool bIsWhite)
{
	if (bIsWhite != ChessBoardInfo.EnpassantPawn.bIsWhite)
	{
		ChessBoardInfo.EnpassantPawn = FChessPieceInfo();
		ChessBoardInfo.EnpassantTileIndex = -1;

		EnpassantPawn = nullptr;
		
		AChessPlayerController* ChessPlayerController = Cast<AChessPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		if (!ChessPlayerController) return PRINTSTRING(FColor::Red, "PlayerController Invalid in ChessBoard");

		ChessPlayerController->OnPieceMoved.RemoveDynamic(this, &AChessBoard::DisableEnpassant);
	}
}

int32 AChessBoard::MoveGenerationTest(FChessBoardInfo BoardInfo, bool bIsWhiteTurn, int32 Depth)
{
	if (Depth == 0) return 1;

	int32 NumberOfPositions = 0;

	GenerateAllValidMoves(BoardInfo, bIsWhiteTurn);

	for (FChessTileInfo Tile : BoardInfo.TilesInfo)
	{
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1)
		{
			if (Tile.ChessPieceOnTile.bIsWhite == bIsWhiteTurn)
			{
				for (int32 Move : Tile.ChessPieceOnTile.ValidMoves)
				{
					FChessBoardInfo SimulatedBoardInfo = BoardInfo;
					SimulateMove(SimulatedBoardInfo, Tile.ChessPieceOnTile, Move);
					NumberOfPositions += MoveGenerationTest(SimulatedBoardInfo, !bIsWhiteTurn, Depth - 1);
				}
			}
		}
	}

	return NumberOfPositions;
}

FChessMove AChessBoard::CalculateBestAIMove(FChessBoardInfo BoardInfo, bool bIsWhiteTurn, int32 MaxDepth)
{
	int32 BestEval = bIsWhiteTurn ? INT_MIN : INT_MAX;
	FChessMove BestMove(-1, -1);

	for (FChessTileInfo Tile : BoardInfo.TilesInfo)
	{
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1)
		{
			if (Tile.ChessPieceOnTile.bIsWhite == bIsWhiteTurn)
			{
				for (int32 Move : Tile.ChessPieceOnTile.ValidMoves)
				{
					FChessBoardInfo SimulatedBoardInfo = BoardInfo;
					SimulateMove(SimulatedBoardInfo, Tile.ChessPieceOnTile, Move);

					int32 Eval = AlphaBetaSearch(SimulatedBoardInfo, MaxDepth - 1, INT_MIN, INT_MAX, !bIsWhiteTurn);
					
					if ((bIsWhiteTurn && Eval > BestEval) || (!bIsWhiteTurn && Eval < BestEval))
					{
						BestEval = Eval;
						BestMove = FChessMove(Tile.ChessTilePositionIndex, Move);
					}
				}
			}
		}
	}

	return BestMove;
}

EChessPieceType AChessBoard::FindBestPawnPromotionType(FChessBoardInfo BoardInfo, int32 PiecePositionIndex, bool bIsPieceWhite)
{
	EChessPieceType BestPromotionType = EChessPieceType::Queen;
	int32 BestEval = INT_MIN;

	for (EChessPieceType Type : { EChessPieceType::King, EChessPieceType::Queen, EChessPieceType::Bishop, EChessPieceType::Knight, EChessPieceType::Rook, EChessPieceType::Pawn })
	{
		BoardInfo.TilesInfo[PiecePositionIndex].ChessPieceOnTile.ChessPieceType = Type;

		int32 Eval = EvaluateBoard(BoardInfo);
	
		if ((bIsPieceWhite && Eval > BestEval) || (!bIsPieceWhite && Eval < BestEval))
		{
			BestEval = Eval;
			BestPromotionType = Type;
		}
	}

	return BestPromotionType;
}

int32 AChessBoard::AlphaBetaSearch(FChessBoardInfo BoardInfo, int32 Depth, int32 Alpha, int32 Beta, bool bIsWhiteTurn)
{
	GenerateAllValidMoves(BoardInfo, bIsWhiteTurn);

	if (Depth == 0 || IsGameOver(BoardInfo, bIsWhiteTurn) != EChessGameState::Ongoing) return /*EvaluateBoard(BoardInfo);*/ AlphaBetaSearchAllCaptures(BoardInfo, 1, Alpha, Beta, bIsWhiteTurn);

	if (bIsWhiteTurn)
	{
		int32 MaxEval = INT_MIN;

		for (FChessTileInfo Tile : BoardInfo.TilesInfo)
		{
			if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1)
			{
				if (Tile.ChessPieceOnTile.bIsWhite == bIsWhiteTurn)
				{
					for (int32 Move : Tile.ChessPieceOnTile.ValidMoves)
					{
						FChessBoardInfo SimulatedBoardInfo = BoardInfo;
						SimulateMove(SimulatedBoardInfo, Tile.ChessPieceOnTile, Move);

						int32 Eval = AlphaBetaSearch(SimulatedBoardInfo, Depth - 1, Alpha, Beta, !bIsWhiteTurn);
						
						MaxEval = FMath::Max(MaxEval, Eval);
						Alpha = FMath::Max(Alpha, Eval);

						if (Beta <= Alpha) break; // Beta cutoff
					}
				}
			}
		}

		return MaxEval;
	}
	else
	{
		int32 MinEval = INT_MAX;

		for (FChessTileInfo Tile : BoardInfo.TilesInfo)
		{
			if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1)
			{
				if (Tile.ChessPieceOnTile.bIsWhite == bIsWhiteTurn)
				{
					for (int32 Move : Tile.ChessPieceOnTile.ValidMoves)
					{
						FChessBoardInfo SimulatedBoardInfo = BoardInfo;
						SimulateMove(SimulatedBoardInfo, Tile.ChessPieceOnTile, Move);

						int32 Eval = AlphaBetaSearch(SimulatedBoardInfo, Depth - 1, Alpha, Beta, !bIsWhiteTurn);

						MinEval = FMath::Min(MinEval, Eval);
						Beta = FMath::Min(Beta, Eval);

						if (Beta <= Alpha) break; // Alpha cutoff
					}
				}
			}
		}

		return MinEval;
	}
}

int32 AChessBoard::AlphaBetaSearchAllCaptures(FChessBoardInfo BoardInfo, int32 Depth, int32 Alpha, int32 Beta, bool bIsWhiteTurn)
{
	int32 Evaluation = EvaluateBoard(BoardInfo);

	if (Depth == 0) return Evaluation;

	GenerateAllValidMoves(BoardInfo, bIsWhiteTurn, true);

	if (bIsWhiteTurn)
	{
		int32 MaxEval = Evaluation;

		for (FChessTileInfo Tile : BoardInfo.TilesInfo)
		{
			if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1)
			{
				if (Tile.ChessPieceOnTile.bIsWhite == bIsWhiteTurn)
				{
					for (int32 Move : Tile.ChessPieceOnTile.ValidMoves)
					{
						FChessBoardInfo SimulatedBoardInfo = BoardInfo;
						SimulateMove(SimulatedBoardInfo, Tile.ChessPieceOnTile, Move);

						int32 Eval = AlphaBetaSearchAllCaptures(SimulatedBoardInfo, Depth - 1, Alpha, Beta, !bIsWhiteTurn);

						MaxEval = FMath::Max(MaxEval, Eval);
						Alpha = FMath::Max(Alpha, Eval);

						if (Beta <= Alpha) break; // Beta cutoff
					}
				}
			}
		}

		return MaxEval;
	}
	else
	{
		int32 MinEval = Evaluation;

		for (FChessTileInfo Tile : BoardInfo.TilesInfo)
		{
			if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1)
			{
				if (Tile.ChessPieceOnTile.bIsWhite == bIsWhiteTurn)
				{
					for (int32 Move : Tile.ChessPieceOnTile.ValidMoves)
					{
						FChessBoardInfo SimulatedBoardInfo = BoardInfo;
						SimulateMove(SimulatedBoardInfo, Tile.ChessPieceOnTile, Move);

						int32 Eval = AlphaBetaSearchAllCaptures(SimulatedBoardInfo, Depth - 1, Alpha, Beta, !bIsWhiteTurn);

						MinEval = FMath::Min(MinEval, Eval);
						Beta = FMath::Min(Beta, Eval);

						if (Beta <= Alpha) break; // Alpha cutoff
					}
				}
			}
		}

		return MinEval;
	}
}

int32 AChessBoard::EvaluateBoard(const FChessBoardInfo& BoardInfo)
{
	int32 Score = 0;
	int32 WhiteKingPosition = -1;
	int32 BlackKingPosition = -1;

	for (const FChessTileInfo& Tile : BoardInfo.TilesInfo)
	{
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1)
		{
			int32 PieceValue = 0;

			switch (Tile.ChessPieceOnTile.ChessPieceType)
			{
				case EChessPieceType::King:		/*PieceValue = 100000;*/	break;
				case EChessPieceType::Queen:	PieceValue = 900;			break;
				case EChessPieceType::Bishop:	PieceValue = 300;			break;
				case EChessPieceType::Knight:	PieceValue = 300;			break;
				case EChessPieceType::Rook:		PieceValue = 500;			break;
				case EChessPieceType::Pawn:		PieceValue = 100;			break;
				default: break;
			}

			if (Tile.ChessPieceOnTile.bIsWhite)
			{
				Score += PieceValue;

				if (Tile.ChessPieceOnTile.ChessPieceType == EChessPieceType::King)
					WhiteKingPosition = Tile.ChessTilePositionIndex;
			}
			else
			{
				Score -= PieceValue;

				if (Tile.ChessPieceOnTile.ChessPieceType == EChessPieceType::King)
					BlackKingPosition = Tile.ChessTilePositionIndex;
			}
		}
	}

	if (IsEndGame(BoardInfo))
	{
		// Add heuristic to drive the enemy king to corners
		if (WhiteKingPosition != -1) Score += KingPositionScore(BlackKingPosition); // Enemy king
		if (BlackKingPosition != -1) Score -= KingPositionScore(WhiteKingPosition); // Enemy king
	}

	return Score;
}

bool AChessBoard::IsEndGame(const FChessBoardInfo& BoardInfo)
{
	int32 BoardScore = 0;

	for (const FChessTileInfo& Tile : BoardInfo.TilesInfo)
	{
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1)
		{
			switch (Tile.ChessPieceOnTile.ChessPieceType)
			{
			case EChessPieceType::King:							break;
			case EChessPieceType::Queen:	BoardScore += 9;	break;
			case EChessPieceType::Bishop:	BoardScore += 3;	break;
			case EChessPieceType::Knight:	BoardScore += 3;	break;
			case EChessPieceType::Rook:		BoardScore += 5;	break;
			case EChessPieceType::Pawn:		BoardScore += 1;	break;
			default: break;
			}
		}
	}

	return BoardScore <= 12; // Arbitrary threshold for "endgame"
}

int32 AChessBoard::KingPositionScore(int32 KingPosition)
{
	int32 Rank = KingPosition / 8;
	int32 File = KingPosition % 8;

	// calculate Manhattan Distance to corners
	int32 DistToA1 = Rank + File;
	int32 DistToH1 = Rank + (7 - File);
	int32 DistToA8 = (7 - Rank) + File;
	int32 DistToH8 = (7 - Rank) + (7 - File);

	// Find Shortest Distance to any corner
	int32 MinDist = FMath::Min(
		FMath::Min(DistToA1, DistToH1),
		FMath::Min(DistToA8, DistToH8)
	);

	// assign higher score for closer distances
	return 14 - MinDist; // Closer to 14 (corner), best score
}

EChessGameState AChessBoard::IsGameOver(const FChessBoardInfo& BoardInfo, bool bIsWhiteTurn)
{
	int32 NumOfValidMoves = 0;

	for (FChessTileInfo Tile : BoardInfo.TilesInfo)
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex > -1)
			if (Tile.ChessPieceOnTile.bIsWhite == bIsWhiteTurn)
				NumOfValidMoves += Tile.ChessPieceOnTile.ValidMoves.Num();

	// if No Valid moves available, Game Over
	if (NumOfValidMoves == 0)
	{
		if ((bIsWhiteTurn) ? BoardInfo.bIsWhiteKingUnderCheck : BoardInfo.bIsBlackKingUnderCheck)
		{
			//PRINTSTRING(FColor::Red, "CHECKMATE");
			return EChessGameState::Checkmate;
		} 
		else
		{
			//PRINTSTRING(FColor::Red, "STALEMATE");
			return EChessGameState::Stalemate;
		}
	}

	if (!CanCheckMateOccur(BoardInfo)) return EChessGameState::Draw; // if checkmate cannot occur, Game Over

	if (NumOfMovesSinceLastCapture >= 100) return EChessGameState::Draw; // if 100 half-moves since last capture, Game Over

	// TODO : Implement three time repetition rule

	return EChessGameState::Ongoing;
}

bool AChessBoard::CanCheckMateOccur(FChessBoardInfo BoardInfo)
{
	int32 NumWhitePieces = 0;
	int32 NumBlackPieces = 0;
	int32 NumWhiteBishops = 0;
	int32 NumBlackBishops = 0;
	int32 NumWhiteKnights = 0;
	int32 NumBlackKnights = 0;

	for (FChessTileInfo Tile : BoardInfo.TilesInfo)
	{
		// if tile is empty
		if (Tile.ChessPieceOnTile.ChessPiecePositionIndex == -1) continue;

		if (Tile.ChessPieceOnTile.bIsWhite)
		{
			NumWhitePieces++;

			if (Tile.ChessPieceOnTile.ChessPieceType == EChessPieceType::Bishop) NumWhiteBishops++;
			if (Tile.ChessPieceOnTile.ChessPieceType == EChessPieceType::Knight) NumWhiteKnights++;
		}
		else
		{
			NumBlackPieces++;

			if (Tile.ChessPieceOnTile.ChessPieceType == EChessPieceType::Bishop) NumBlackBishops++;
			if (Tile.ChessPieceOnTile.ChessPieceType == EChessPieceType::Knight) NumBlackKnights++;
		}
	}

	// Check for insufficient material conditions
	if ((NumWhitePieces == 1 && NumBlackPieces == 1) ||								// King vs. King
		(NumWhitePieces == 2 && NumBlackPieces == 1 && NumWhiteKnights == 1) ||		// King & Knight vs. King
		(NumWhitePieces == 2 && NumBlackPieces == 1 && NumWhiteBishops == 1) ||		// King & Bishop vs. King
		(NumWhitePieces == 1 && NumBlackPieces == 2 && NumBlackKnights == 1) ||		// King vs. King & Knight
		(NumWhitePieces == 1 && NumBlackPieces == 2 && NumBlackBishops == 1))		// King vs. King & Bishop
		return false;

	return true;
}