// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#include "Board/ChessPiece.h"

#include "Chess/Chess.h"

#include "Board/ChessBoard.h"
#include "Board/ChessTile.h"
#include "Core/ChessPlayerController.h"
#include "Data/ChessBoardData.h"

#include "Components/InterpToMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#define PRINTSTRING(Colour, DebugMessage) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, Colour, DebugMessage);

AChessPiece::AChessPiece()
{
	PredictParams.bTraceComplex = true;
	PredictParams.bTraceWithChannel = true;
	PredictParams.bTraceWithCollision = true;
	PredictParams.ProjectileRadius = 0.f;
	PredictParams.TraceChannel = ECollisionChannel::ECC_WorldStatic;
	PredictParams.ActorsToIgnore = { this };
	PredictParams.DrawDebugType = EDrawDebugTrace::None;
	PredictParams.DrawDebugTime = 5.f;
	PredictParams.SimFrequency = 30.f;
	PredictParams.MaxSimTime = 5.f;
	PredictParams.OverrideGravityZ = 0.f;

	PrimaryActorTick.bCanEverTick = true;

	// DefaultSceneRootComponent
	DefaultSceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRootComponent"));
	SetRootComponent(DefaultSceneRootComponent);

	// ChessPieceMesh
	ChessPieceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChessPieceMesh"));
	ChessPieceMesh->SetupAttachment(RootComponent);
	ChessPieceMesh->SetCollisionProfileName("ChessPiece");

	// InterpToMovementcomponent
	InterpToMovementComponent = CreateDefaultSubobject<UInterpToMovementComponent>(TEXT("InterpToMovementComponent"));

	static ConstructorHelpers::FObjectFinder<UChessBoardData> ChessBoardDataAsset(TEXT("/Script/Chess.ChessBoardData'/Game/+Chess/Data/DA_ChessBoardData.DA_ChessBoardData'"));
	if (ChessBoardDataAsset.Succeeded()) ChessBoardData = ChessBoardDataAsset.Object;
}

void AChessPiece::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UpdateChessPieceStaticMesh();
}

void AChessPiece::BeginPlay()
{
	Super::BeginPlay();

	ChessBoard = Cast<AChessBoard>(UGameplayStatics::GetActorOfClass(GetWorld(), AChessBoard::StaticClass()));
	if (!ChessBoard) return PRINTSTRING(FColor::Red, "ChessBoard is INVALID in ChessPiece");
}

void AChessPiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChessPiece::CapturePiece()
{
	if (ChessBoard)
	{
		// Disable Castling if its a Rook
		if (ChessPieceInfo.ChessPieceType == EChessPieceType::Rook)
		{
			switch (ChessPieceInfo.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (ChessPieceInfo.bIsWhite)
				{
					ChessBoard->bIsWhiteKingSideRookAlive = false;
				}
				else
				{
					ChessBoard->bIsBlackKingSideRookAlive = false;
				}
				break;
			case EChessPieceSide::QueenSide:
				if (ChessPieceInfo.bIsWhite)
				{
					ChessBoard->bIsWhiteQueenSideRookAlive = false;
				}
				else
				{
					ChessBoard->bIsBlackQueenSideRookAlive = false;
				}
				break;
			default:
				break;
			}
		}

		// Remove Piece from array
		if (ChessPieceInfo.bIsWhite)
		{
			ChessBoard->WhiteChessPieces.Remove(this);
		}
		else
		{
			ChessBoard->BlackChessPieces.Remove(this);
		}
	}

	OnPieceCaptured.Broadcast();

	Destroy(); // Temporarily Destroy Piece
}

void AChessPiece::CalculateValidMoves()
{
	if (!ChessBoard)
	{
		PRINTSTRING(FColor::Red, "ChessBoard invalid in ChessPiece : " + GetName());
		return;
	}

	TArray<FChessTileInfo> ValidMovesBeforeFilteringForCheck;

	switch (ChessPieceInfo.ChessPieceType)
	{
		case EChessPieceType::King:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForKing(ChessBoard->ChessBoardLayout);
			break;
		case EChessPieceType::Queen:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForQueen(ChessBoard->ChessBoardLayout);
			break;
		case EChessPieceType::Bishop:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForBishop(ChessBoard->ChessBoardLayout);
			break;
		case EChessPieceType::Knight:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForKnight(ChessBoard->ChessBoardLayout);
			break;
		case EChessPieceType::Rook:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForRook(ChessBoard->ChessBoardLayout);
			break;
		case EChessPieceType::Pawn:
			ValidMovesBeforeFilteringForCheck = CalculateValidMoveTilesForPawn(ChessBoard->ChessBoardLayout, ChessBoard->EnpassantTileIndex);
			break;
		default:
			break;
	}

	ValidMoves = FilterMovesForCheck(ValidMovesBeforeFilteringForCheck);
}

TArray<FChessTileInfo> AChessPiece::FilterMovesForCheck(TArray<FChessTileInfo>& ValidMovesBeforeFilteration)
{
	if (!ChessBoard)
	{
		PRINTSTRING(FColor::Red, "ChessBoard invalid in ChessPiece : " + GetName());
		return TArray<FChessTileInfo>();
	}

	TArray<FChessTileInfo> ValidMovesAfterFilteration;

	for (FChessTileInfo ValidMove : ValidMovesBeforeFilteration)
	{
		TArray<FChessTileInfo> SimulatedBoardLayout = ChessBoard->ChessBoardLayout;
		int32 OutEnpassantTarget = ChessBoard->EnpassantTileIndex;

		SimulateMove(
			ValidMove.ChessTilePositionIndex,
			SimulatedBoardLayout,
			OutEnpassantTarget,
			ChessPieceInfo.bIsWhite
		);
		
		if (!ChessBoard->IsKingInCheck(ChessPieceInfo.bIsWhite, SimulatedBoardLayout, OutEnpassantTarget)) ValidMovesAfterFilteration.AddUnique(ValidMove);
	}

	return ValidMovesAfterFilteration;
}

void AChessPiece::SimulateMove(int32 ToPosition, TArray<FChessTileInfo>& BoardLayout, int32& OutEnPassantTarget, bool bIsWhiteTurn)
{
	FChessTileInfo& FromSquare = BoardLayout[ChessPieceInfo.ChessPiecePositionIndex];
	FChessTileInfo& ToSquare = BoardLayout[ToPosition];

	// Move the piece to the target square
	ToSquare = FromSquare;
	FromSquare = FChessTileInfo(); // Clear the original square

	// Handle en passant captures
	if (ChessPieceInfo.ChessPieceType == EChessPieceType::Pawn && ToPosition == OutEnPassantTarget)
	{
		// Determine the captured pawn's position
		int32 CapturedPawnPosition = bIsWhiteTurn ? ToPosition - 8 : ToPosition + 8;
		BoardLayout[CapturedPawnPosition] = FChessTileInfo(); // Remove the captured pawn
	}

	// Update en passant target
	if (ChessPieceInfo.ChessPieceType == EChessPieceType::Pawn && FMath::Abs(ToPosition - ChessPieceInfo.ChessPiecePositionIndex) == 16)
	{
		// Set the en passant target to the square behind the pawn
		OutEnPassantTarget = bIsWhiteTurn ? ChessPieceInfo.ChessPiecePositionIndex + 8 : ChessPieceInfo.ChessPiecePositionIndex - 8;
	}
	else
	{
		OutEnPassantTarget = -1; // Clear the en passant target
	}

	// Handle castling (if the king moves two squares)
	if (ChessPieceInfo.ChessPieceType == EChessPieceType::King && FMath::Abs(ToPosition - ChessPieceInfo.ChessPiecePositionIndex) == 2)
	{
		if (ToPosition > ChessPieceInfo.ChessPiecePositionIndex)
		{
			// King-side castling
			BoardLayout[ToPosition - 1] = BoardLayout[ToPosition + 1]; // Move the rook
			BoardLayout[ToPosition + 1] = FChessTileInfo(); // Clear the original rook square
		}
		else
		{
			// Queen-side castling
			BoardLayout[ToPosition + 1] = BoardLayout[ToPosition - 2]; // Move the rook
			BoardLayout[ToPosition - 2] = FChessTileInfo(); // Clear the original rook square
		}
	}

	// Handle pawn promotion (if a pawn reaches the opposite end of the board)
	if (ChessPieceInfo.ChessPieceType == EChessPieceType::Pawn && (ToPosition < 8 || ToPosition >= 56))
	{
		// Promote to a queen (default behavior)
		ChessPieceInfo.ChessPieceType = EChessPieceType::Queen;
	}
}

void AChessPiece::MovePiece(AChessTile* MoveToTile)
{
	if (!ChessBoard) return PRINTSTRING(FColor::Red, "ChessBoard Invalid in ChessPiece : " + GetName());

	switch (ChessPieceInfo.ChessPieceType)
	{
	case EChessPieceType::King:
		// if King hasn't moved
		if (!ChessPieceInfo.bHasMoved)
		{
			// if first move is two tiles away
			if (FMath::Abs(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().Y - ChessPieceInfo.GetChessPiecePositionFromIndex().Y) == 2)
			{		
				// King Side Castling
				if (MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().Y > ChessPieceInfo.GetChessPiecePositionFromIndex().Y)
				{
					// if king side rook piece
					if (AChessPiece* KingSideRook = ChessBoard->GetChessTileAtPosition(FVector2D(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X, 7))->ChessTileInfo.ChessPieceOnTile)
					{
						// and if the rook is same colour as king
						if (KingSideRook->ChessPieceInfo.bIsWhite == ChessPieceInfo.bIsWhite)
						{
							// and if it is actually a rook and it is in fact on the king side
							if (KingSideRook->ChessPieceInfo.ChessPieceType == EChessPieceType::Rook && KingSideRook->ChessPieceInfo.ChessPieceSide == EChessPieceSide::KingSide)
							{
								// then get the king side rook castling tile
								if (AChessTile* KingSideRookCastlingTile = ChessBoard->GetChessTileAtPosition(FVector2D(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X, 5)))
								{
									// and if no piece is on that tile
									if (KingSideRookCastlingTile->ChessTileInfo.ChessPieceOnTile == nullptr)
									{
										// move the rook to king side rook castling tile
										KingSideRook->MovePiece(KingSideRookCastlingTile);

										MoveToTile->ChessTileInfo.ChessPieceOnTile = KingSideRook;
										KingSideRook->ChessPieceInfo.ChessPiecePositionIndex = MoveToTile->ChessTileInfo.ChessTilePositionIndex;

										ChessBoard->GetChessTileAtPosition(FVector2D(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X, 7))->ChessTileInfo.ChessPieceOnTile = nullptr;

										PRINTSTRING(FColor::Green, "King Side Castling");
									}
									else
									{
										PRINTSTRING(FColor::Red, "KingSideRookCastlingTile is not empty");
									}
								}
								else
								{
									PRINTSTRING(FColor::Red, "KingSideRookCastlingTile is invalid");
								}
							}
							else
							{
								PRINTSTRING(FColor::Red, "Not a rook or not a king side rook");
							}
						}
						else
						{
							PRINTSTRING(FColor::Red, "Rook is not of the same colour as King");
						}
					}
					else
					{
						PRINTSTRING(FColor::Red, "KingSideRook Invalid");
					}
				}
				else // Queen Side Castling
				{
					// if queen side rook piece
					if (AChessPiece* QueenSideRook = ChessBoard->GetChessTileAtPosition(FVector2D(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X, 0))->ChessTileInfo.ChessPieceOnTile)
					{
						// and if the rook is same colour as king
						if (QueenSideRook->ChessPieceInfo.bIsWhite == ChessPieceInfo.bIsWhite)
						{
							// and if it is actually a rook and it is in fact on the queen side
							if (QueenSideRook->ChessPieceInfo.ChessPieceType == EChessPieceType::Rook && QueenSideRook->ChessPieceInfo.ChessPieceSide == EChessPieceSide::QueenSide)
							{
								// then get the queen side rook castling tile
								if (AChessTile* QueenSideRookCastlingTile = ChessBoard->GetChessTileAtPosition(FVector2D(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X, 3)))
								{
									// and if no piece is on that tile
									if (QueenSideRookCastlingTile->ChessTileInfo.ChessPieceOnTile == nullptr)
									{
										// move the rook to queen side rook castling tile
										QueenSideRook->MovePiece(QueenSideRookCastlingTile);

										MoveToTile->ChessTileInfo.ChessPieceOnTile = QueenSideRook;
										QueenSideRook->ChessPieceInfo.ChessPiecePositionIndex = MoveToTile->ChessTileInfo.ChessTilePositionIndex;

										ChessBoard->GetChessTileAtPosition(FVector2D(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X, 0))->ChessTileInfo.ChessPieceOnTile = nullptr;

										PRINTSTRING(FColor::Green, "Queen Side Castling");
									}
									else
									{
										PRINTSTRING(FColor::Red, "QueenSideRookCastlingTile is not empty");
									}
								}
								else
								{
									PRINTSTRING(FColor::Red, "QueenSideRookCastlingTile is invalid");
								}
							}
							else
							{
								PRINTSTRING(FColor::Red, "Not a rook or not a queen side rook");
							}
						}
						else
						{
							PRINTSTRING(FColor::Red, "Rook is not of the same colour as King");
						}
					}
					else
					{
						PRINTSTRING(FColor::Red, "QueenSideRook Invalid");
					}
				}
			}
		}
		else
		{
			PRINTSTRING(FColor::Red, "King has Moved");
		}

		if (ChessPieceInfo.bIsWhite)
		{
			if (!ChessBoard->bHasWhiteKingMoved) ChessBoard->bHasWhiteKingMoved = true;
		}
		else
		{
			if (!ChessBoard->bHasBlackKingMoved) ChessBoard->bHasBlackKingMoved = true;
		}
		break;
	case EChessPieceType::Queen:
		break;
	case EChessPieceType::Bishop:
		break;
	case EChessPieceType::Knight:
		break;
	case EChessPieceType::Rook:
		if (ChessPieceInfo.bIsWhite)
		{
			switch (ChessPieceInfo.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (!ChessBoard->bHasWhiteKingSideRookMoved) ChessBoard->bHasWhiteKingSideRookMoved = true;
				break;
			case EChessPieceSide::QueenSide:
				if (!ChessBoard->bHasWhiteQueenSideRookMoved) ChessBoard->bHasWhiteQueenSideRookMoved = true;
				break;
			default:
				break;
			}
		}
		else
		{
			switch (ChessPieceInfo.ChessPieceSide)
			{
			case EChessPieceSide::None:
				break;
			case EChessPieceSide::KingSide:
				if (!ChessBoard->bHasBlackKingSideRookMoved) ChessBoard->bHasBlackKingSideRookMoved = true;
				break;
			case EChessPieceSide::QueenSide:
				if (!ChessBoard->bHasBlackQueenSideRookMoved) ChessBoard->bHasBlackQueenSideRookMoved = true;
				break;
			default:
				break;
			}
		}
		break;
	case EChessPieceType::Pawn:
		if (!ChessPieceInfo.bHasMoved) // is first move of pawn
		{
			// if first move is two tiles away
			if (FMath::Abs(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X - ChessPieceInfo.GetChessPiecePositionFromIndex().X) == 2)
			{
				bool bEnableEnpassant = false;

				for (int32 i = -1; i <= 1; i += 2) // two sides of piece
				{
					if (AChessTile* ChessTileOnSide = ChessBoard->GetChessTileAtPosition(MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex() + FVector2D(0, i)))
					{
						if (ChessTileOnSide->ChessTileInfo.ChessPieceOnTile) // if theres a piece on the side...
						{
							// and if the side piece is an opponent pawn
							if (ChessTileOnSide->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite && ChessTileOnSide->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.ChessPieceType == EChessPieceType::Pawn)
							{
								bEnableEnpassant = true;
							}
						}
					}
				}

				if (bEnableEnpassant) ChessBoard->EnableEnpassant(this);
			}
		}
		else if (MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X == 0 || MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().X == 7) // Pawn has reached the end of the line
		{
			AChessPlayerController* ChessPlayerController = Cast<AChessPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
			if (ChessPlayerController)
			{
				ChessPlayerController->SpawnPawnPromotionUI(this);
			}
			else
			{
				PRINTSTRING(FColor::Red, "ChessPlayerController is INVALID in ChessPiece");
				PromotePawn(EChessPieceType::Queen); // fallback promotion if UI doesn't spawn
			}
		}
		else if (MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().Y == (ChessPieceInfo.GetChessPiecePositionFromIndex().Y + 1) || MoveToTile->ChessTileInfo.GetChessTilePositionFromIndex().Y == (ChessPieceInfo.GetChessPiecePositionFromIndex().Y - 1)) // is a diagonal move
		{
			if (MoveToTile->ChessTileInfo.ChessTilePositionIndex == ChessBoard->EnpassantTileIndex && ChessBoard->EnpassantPawn) // Enpassant Capture
			{
				if (AChessTile* EnpassantPawnOnTile = ChessBoard->ChessTiles[ChessBoard->EnpassantPawn->ChessPieceInfo.ChessPiecePositionIndex])
				{
					EnpassantPawnOnTile->ChessTileInfo.ChessPieceOnTile = nullptr;
					ChessBoard->EnpassantPawn->CapturePiece();
					ChessBoard->DisableEnpassant(ChessPieceInfo.bIsWhite);
				}
			}
		}
		break;
	default:
		break;
	}

	if (!ChessPieceInfo.bHasMoved) ChessPieceInfo.bHasMoved = true;

	// Movement code
	FVector OutLaunchVelocity;
	UGameplayStatics::SuggestProjectileVelocity_CustomArc(
		GetWorld(),
		OutLaunchVelocity,
		GetActorLocation(),
		MoveToTile->GetActorLocation(),
		0.f,
		UKismetMathLibrary::MapRangeClamped(
			UKismetMathLibrary::Vector_Distance(
				GetActorLocation(),
				MoveToTile->GetActorLocation()
			),
			0.f,
			2500.f,
			.1f,
			.4f
		)
	);

	PredictParams.StartLocation = GetActorLocation();
	PredictParams.LaunchVelocity = OutLaunchVelocity;

	FPredictProjectilePathResult PredictResult;
	UGameplayStatics::PredictProjectilePath(GetWorld(), PredictParams, PredictResult);

	InterpToMovementComponent->ResetControlPoints();
	InterpToMovementComponent->RestartMovement(1.f);

	TotalTravelDistance = 0.1f;

	for (int32 i = 0; i < PredictResult.PathData.Num(); i++)
	{
		InterpToMovementComponent->AddControlPointPosition(PredictResult.PathData[i].Location, false);

		if (i > 0 && i < PredictResult.PathData.Num() - 1)
		{
			TotalTravelDistance += UKismetMathLibrary::Vector_Distance(PredictResult.PathData[i].Location, PredictResult.PathData[i - 1].Location);
		}
	}

	InterpToMovementComponent->Duration = TotalTravelDistance / 2000.f;

	InterpToMovementComponent->FinaliseControlPoints();
}

void AChessPiece::UpdateTilesUnderAttack(TArray<FChessTileInfo>& TilesUnderAttack)
{
	if (!ChessBoard) return PRINTSTRING(FColor::Red, "ChessBoard is INVALID in ChessPiece");

	TArray<FVector2D> ValidMovePositions;

	switch (ChessPieceInfo.ChessPieceType)
	{
	case EChessPieceType::King:
	{
		for (FVector2D& Position : KingMovePositionTileOffsets)
		{
			FVector2D RelativePosition = ChessPieceInfo.GetChessPiecePositionFromIndex() + Position;
			if (IsPositionWithinBounds(RelativePosition))
			{
				// ignore tile if friendly piece is on that tile
				if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile)
					if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite == ChessPieceInfo.bIsWhite)
						continue;

				ValidMovePositions.Add(RelativePosition);
			} 
		}
		break;
	}
	case EChessPieceType::Queen:
	{
		for (FVector2D& Position : QueenMovePositionDirections)
		{
			for (int32 i = 1; i < 8; i++)
			{
				FVector2D RelativePosition = ChessPieceInfo.GetChessPiecePositionFromIndex() + (Position * i);
				if (IsPositionWithinBounds(RelativePosition))
				{
					// if a piece is on that tile...
					if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile)
					{
						// and if its a friendly piece
						if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite == ChessPieceInfo.bIsWhite)
							break;

						ValidMovePositions.Add(RelativePosition);

						// if it not opponents king
						if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.ChessPieceType != EChessPieceType::King)
							break;
					}

					ValidMovePositions.Add(RelativePosition);
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
				FVector2D RelativePosition = ChessPieceInfo.GetChessPiecePositionFromIndex() + (Position * i);
				if (IsPositionWithinBounds(RelativePosition))
				{
					// if a piece is on that tile...
					if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile)
					{
						// and if its a friendly piece
						if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite == ChessPieceInfo.bIsWhite)
							break;

						ValidMovePositions.Add(RelativePosition);

						// if it not opponents king
						if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.ChessPieceType != EChessPieceType::King)
							break;
					}

					ValidMovePositions.Add(RelativePosition);
				}
			}
		}
		break;
	}
	case EChessPieceType::Knight:
	{
		for (FVector2D& Position : KnightMovePositionTileOffsets)
		{
			FVector2D RelativePosition = ChessPieceInfo.GetChessPiecePositionFromIndex() + Position;
			if (IsPositionWithinBounds(RelativePosition))
			{
				// ignore tile if friendly piece is on that tile
				if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile)
					if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite == ChessPieceInfo.bIsWhite)
						continue;

				ValidMovePositions.Add(RelativePosition);
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
				FVector2D RelativePosition = ChessPieceInfo.GetChessPiecePositionFromIndex() + (Position * i);
				if (IsPositionWithinBounds(RelativePosition))
				{
					// if a piece is on that tile...
					if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile)
					{
						// and if its a friendly piece
						if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite == ChessPieceInfo.bIsWhite)
							break;

						ValidMovePositions.Add(RelativePosition);

						// if it not opponents king
						if (ChessBoard->GetChessTileAtPosition(RelativePosition)->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.ChessPieceType != EChessPieceType::King)
							break;
					}

					ValidMovePositions.Add(RelativePosition);
				}
			}
		}
		break;
	}
	case EChessPieceType::Pawn:
	{
		if (ChessPieceInfo.bIsWhite)
		{
			// Diagonal Captures
			if (IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, -1)))
			{
				// if a piece is present on diagonal tile
				if (ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, -1))->ChessTileInfo.ChessPieceOnTile)
				{
					// if the piece is not friendly
					if (ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, -1))->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite)
						ValidMovePositions.Add(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, -1));
				}
				else // if the tile is empty
				{
					ValidMovePositions.Add(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, -1));
				}
			}

			if (IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, 1)))
			{
				// if a piece is present on diagonal tile
				if (ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, 1))->ChessTileInfo.ChessPieceOnTile)
				{
					// if the piece is not friendly
					if (ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, 1))->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite)
						ValidMovePositions.Add(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, 1));
				}
				else // if the tile is empty
				{
					ValidMovePositions.Add(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, 1));
				}
			}

			// En Passant White -> Black
			if (ChessPieceInfo.GetChessPiecePositionFromIndex().X == 4 && IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, -1)))
				if (!ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, -1))->ChessTileInfo.ChessPieceOnTile) // if no piece is present in one tile diagonal
					if (ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, -1))->ChessTileInfo.ChessTilePositionIndex == ChessBoard->EnpassantTileIndex) // and if diagonal tile is available for enpassant
						ValidMovePositions.Add(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, -1));

			if (ChessPieceInfo.GetChessPiecePositionFromIndex().X == 4 && IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, 1)))
				if (!ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, 1))->ChessTileInfo.ChessPieceOnTile) // if no piece is present in one tile diagonal
					if (ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, 1))->ChessTileInfo.ChessTilePositionIndex == ChessBoard->EnpassantTileIndex) // and if diagonal tile is available for enpassant
						ValidMovePositions.Add(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, 1));
		}
		else
		{
			// Diagonal Captures
			if (IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, 1)))
			{
				// if a piece is present on diagonal tile
				if (ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, 1))->ChessTileInfo.ChessPieceOnTile)
				{
					// if the piece is not friendly
					if (ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, 1))->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite)
						ValidMovePositions.Add(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, 1));
				}
				else // if the tile is empty
				{
					ValidMovePositions.Add(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, 1));
				}
			}

			if (IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, -1)))
			{
				// if a piece is present on diagonal tile
				if (ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, -1))->ChessTileInfo.ChessPieceOnTile)
				{
					// if the piece is not friendly
					if (ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, -1))->ChessTileInfo.ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite)
						ValidMovePositions.Add(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, -1));
				}
				else // if the tile is empty
				{
					ValidMovePositions.Add(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, -1));
				}
			}

			// En Passant Black -> White
			if (ChessPieceInfo.GetChessPiecePositionFromIndex().X == 3 && IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, 1)))
				if (!ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, 1))->ChessTileInfo.ChessPieceOnTile) // if no piece is present in one tile diagonal
					if (ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, 1))->ChessTileInfo.ChessTilePositionIndex == ChessBoard->EnpassantTileIndex) // and if diagonal tile is available for enpassant
						ValidMovePositions.Add(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, 1));

			if (ChessPieceInfo.GetChessPiecePositionFromIndex().X == 3 && IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, -1)))
				if (!ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, -1))->ChessTileInfo.ChessPieceOnTile) // if no piece is present in one tile diagonal
					if (ChessBoard->GetChessTileAtPosition(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, -1))->ChessTileInfo.ChessTilePositionIndex == ChessBoard->EnpassantTileIndex) // and if diagonal tile is available for enpassant
						ValidMovePositions.Add(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, -1));
		}
		break;
	}
	default:
		break;
	}

	for (FVector2D& Position : ValidMovePositions)
	{
		if (ChessPieceInfo.bIsWhite)
			ChessBoard->GetChessTileAtPosition(Position)->ChessTileInfo.bIsTileUnderAttackByWhitePiece = true;
		else
			ChessBoard->GetChessTileAtPosition(Position)->ChessTileInfo.bIsTileUnderAttackByBlackPiece = true;

		TilesUnderAttack.AddUnique(ChessBoard->GetChessTileAtPosition(Position)->ChessTileInfo);
	}
}

void AChessPiece::PromotePawn(EChessPieceType PromotionType)
{
	if (ChessPieceInfo.ChessPieceType != EChessPieceType::Pawn) return;

	if (PromotionType == EChessPieceType::King || PromotionType == EChessPieceType::Pawn) return;

	ChessPieceInfo.ChessPieceType = PromotionType; // Set ChessPieceType to PromotionType

	UpdateChessPieceStaticMesh(); // Update Static Mesh to new PieceType
}

void AChessPiece::UpdateChessPieceStaticMesh()
{
	if (ChessBoardData)
	{
		if (ChessPieceInfo.bIsWhite)
		{
			switch (ChessPieceInfo.ChessPieceType)
			{
			case EChessPieceType::King:
				if (ChessBoardData->WhiteKing) ChessPieceMesh->SetStaticMesh(ChessBoardData->WhiteKing);
				break;
			case EChessPieceType::Queen:
				if (ChessBoardData->WhiteQueen) ChessPieceMesh->SetStaticMesh(ChessBoardData->WhiteQueen);
				break;
			case EChessPieceType::Bishop:
				if (ChessBoardData->WhiteBishop) ChessPieceMesh->SetStaticMesh(ChessBoardData->WhiteBishop);
				break;
			case EChessPieceType::Knight:
				if (ChessBoardData->WhiteKnight) ChessPieceMesh->SetStaticMesh(ChessBoardData->WhiteKnight);
				break;
			case EChessPieceType::Rook:
				if (ChessBoardData->WhiteRook) ChessPieceMesh->SetStaticMesh(ChessBoardData->WhiteRook);
				break;
			case EChessPieceType::Pawn:
				if (ChessBoardData->WhitePawn) ChessPieceMesh->SetStaticMesh(ChessBoardData->WhitePawn);
				break;
			default:
				break;
			}
		}
		else
		{
			switch (ChessPieceInfo.ChessPieceType)
			{
			case EChessPieceType::King:
				if (ChessBoardData->BlackKing) ChessPieceMesh->SetStaticMesh(ChessBoardData->BlackKing);
				break;
			case EChessPieceType::Queen:
				if (ChessBoardData->BlackQueen) ChessPieceMesh->SetStaticMesh(ChessBoardData->BlackQueen);
				break;
			case EChessPieceType::Bishop:
				if (ChessBoardData->BlackBishop) ChessPieceMesh->SetStaticMesh(ChessBoardData->BlackBishop);
				break;
			case EChessPieceType::Knight:
				if (ChessBoardData->BlackKnight) ChessPieceMesh->SetStaticMesh(ChessBoardData->BlackKnight);
				break;
			case EChessPieceType::Rook:
				if (ChessBoardData->BlackRook) ChessPieceMesh->SetStaticMesh(ChessBoardData->BlackRook);
				break;
			case EChessPieceType::Pawn:
				if (ChessBoardData->BlackPawn) ChessPieceMesh->SetStaticMesh(ChessBoardData->BlackPawn);
				break;
			default:
				break;
			}
		}
	}
}

TArray<FChessTileInfo> AChessPiece::CalculateValidMoveTilesForKing(TArray<FChessTileInfo> Board)
{
	if (!ChessBoard)
	{
		PRINTSTRING(FColor::Red, "ChessBoard is INVALID in ChessPiece");
		return TArray<FChessTileInfo>();
	}

	TArray<FChessTileInfo> ValidTiles;

	// One Tile Omnidirectional moves
	for (FVector2D& Position : KingMovePositionTileOffsets)
	{
		FVector2D RelativePosition = ChessPieceInfo.GetChessPiecePositionFromIndex() + Position;

		if (IsPositionWithinBounds(RelativePosition))
		{
			int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

			if (Board[RelativePositionIndex].ChessPieceOnTile) // if theres a piece on the tile
			{
				// ignore tile if friendly piece is on that tile
				if (Board[RelativePositionIndex].ChessPieceOnTile->ChessPieceInfo.bIsWhite == ChessPieceInfo.bIsWhite)
					continue;
			}

			// ignore tile if its is under attack by enemy piece
			if ((ChessPieceInfo.bIsWhite)
				? Board[RelativePositionIndex].bIsTileUnderAttackByBlackPiece
				: Board[RelativePositionIndex].bIsTileUnderAttackByWhitePiece)
			{
				//if (!bIsSimulatedBoard) PRINTSTRING(FColor::Green, FString::SanitizeFloat(RelativePositionIndex));
				continue;
			}

			ValidTiles.Add(Board[RelativePositionIndex]);
		}
	}

	if (ChessPieceInfo.bHasMoved) return ValidTiles;

	// Castling Moves if available
	if (ChessPieceInfo.bIsWhite)
	{
		// if White king is not under check
		if (!ChessBoard->bIsWhiteKingUnderCheck)
		{
			// if White king side rook alive
			if (ChessBoard->bIsWhiteKingSideRookAlive)
				// if white king and king side rook have not moved
				if (!ChessBoard->HasWhiteKingOrKingSideRookMoved())
					// if no piece is present one tile to the right
					if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 1].ChessPieceOnTile)
						// if one tile to the right isnt under attack
						if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 1].bIsTileUnderAttackByBlackPiece)
							// if no piece is present two tiles to the right
							if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 2].ChessPieceOnTile)
								// if two tiles to the right isnt under attack
								if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 2].ChessPieceOnTile)
									ValidTiles.AddUnique(Board[ChessPieceInfo.ChessPiecePositionIndex + 2]);

			// if White queen side rook alive
			if (ChessBoard->bIsWhiteQueenSideRookAlive)
				// if white king and queen side rook have not moved
				if (!ChessBoard->HasWhiteKingOrQueenSideRookMoved())
					// if no piece is present one tile to the left
					if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 1].ChessPieceOnTile)
						// if one tile to the left isnt under attack
						if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 1].ChessPieceOnTile)
							// if no piece is present two tiles to the left
							if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 2].ChessPieceOnTile)
								// if two tiles to the left isnt under attack
								if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 2].ChessPieceOnTile)
									// if no piece is present three tiles to the left
									if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 3].ChessPieceOnTile)
										ValidTiles.AddUnique(Board[ChessPieceInfo.ChessPiecePositionIndex - 2]);
		}
	}
	else
	{
		// if Black king is not under check
		if (!ChessBoard->bIsBlackKingUnderCheck)
		{
			// if black king side rook alive
			if (ChessBoard->bIsBlackKingSideRookAlive)
				// if black king and king side rook have not moved
				if (!ChessBoard->HasBlackKingOrKingSideRookMoved())
					// if no piece is present one tile to the right
					if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 1].ChessPieceOnTile)
						// if one tile to the right isnt under attack
						if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 1].bIsTileUnderAttackByWhitePiece)
							// if no piece is present two tiles to the right
							if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 2].ChessPieceOnTile)
								// if two tiles to the right isnt under attack
								if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 2].bIsTileUnderAttackByWhitePiece)
									ValidTiles.AddUnique(Board[ChessPieceInfo.ChessPiecePositionIndex + 2]);

			// if black queen side rook alive
			if (ChessBoard->bIsBlackQueenSideRookAlive)
				// if black king and queen side rook have not moved
				if (!ChessBoard->HasBlackKingOrQueenSideRookMoved())
					// if no piece is present one tile to the left
					if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 1].ChessPieceOnTile)
						// if one tile to the left isnt under attack
						if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 1].bIsTileUnderAttackByWhitePiece)
							// if no piece is present two tiles to the left
							if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 2].ChessPieceOnTile)
								// if two tiles to the left isnt under attack
								if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 2].bIsTileUnderAttackByWhitePiece)
									// if no piece is present three tiles to the left
									if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 3].ChessPieceOnTile)
										ValidTiles.AddUnique(Board[ChessPieceInfo.ChessPiecePositionIndex - 2]);
		}
	}

	return ValidTiles;
}

TArray<FChessTileInfo> AChessPiece::CalculateValidMoveTilesForQueen(TArray<FChessTileInfo> Board)
{
	if (!ChessBoard)
	{
		PRINTSTRING(FColor::Red, "ChessBoard is INVALID in ChessPiece");
		return TArray<FChessTileInfo>();
	}

	TArray<int32> ValidMovePositions;

	for (FVector2D& Position : QueenMovePositionDirections)
	{
		for (int32 i = 1; i < 8; i++)
		{
			FVector2D RelativePosition = ChessPieceInfo.GetChessPiecePositionFromIndex() + (Position * i);
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

				// if a piece is on that tile...
				if (Board[RelativePositionIndex].ChessPieceOnTile)
				{
					// and if its an opponent piece
					if (Board[RelativePositionIndex].ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite)
						ValidMovePositions.Add(RelativePositionIndex);

					break;
				}

				ValidMovePositions.Add(RelativePositionIndex);
			}
		}
	}

	TArray<FChessTileInfo> ValidTiles;

	for (int32& Position : ValidMovePositions)
		ValidTiles.AddUnique(Board[Position]);

	return ValidTiles;
}

TArray<FChessTileInfo> AChessPiece::CalculateValidMoveTilesForBishop(TArray<FChessTileInfo> Board)
{
	if (!ChessBoard)
	{
		PRINTSTRING(FColor::Red, "ChessBoard is INVALID in ChessPiece");
		return TArray<FChessTileInfo>();
	}

	TArray<int32> ValidMovePositions;

	for (FVector2D& Position : BishopMovePositionDirections)
	{
		for (int32 i = 1; i < 8; i++)
		{
			FVector2D RelativePosition = ChessPieceInfo.GetChessPiecePositionFromIndex() + (Position * i);
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

				// if a piece is on that tile...
				if (Board[RelativePositionIndex].ChessPieceOnTile)
				{
					// and if its an opponent piece
					if (Board[RelativePositionIndex].ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite)
						ValidMovePositions.Add(RelativePositionIndex);

					break;
				}

				ValidMovePositions.Add(RelativePositionIndex);
			}
		}
	}

	TArray<FChessTileInfo> ValidTiles;

	for (int32& Position : ValidMovePositions)
		ValidTiles.AddUnique(Board[Position]);

	return ValidTiles;
}

TArray<FChessTileInfo> AChessPiece::CalculateValidMoveTilesForKnight(TArray<FChessTileInfo> Board)
{
	if (!ChessBoard)
	{
		PRINTSTRING(FColor::Red, "ChessBoard is INVALID in ChessPiece");
		return TArray<FChessTileInfo>();
	}

	TArray<int32> ValidMovePositions;

	for (FVector2D& Position : KnightMovePositionTileOffsets)
	{
		FVector2D RelativePosition = ChessPieceInfo.GetChessPiecePositionFromIndex() + Position;
		if (IsPositionWithinBounds(RelativePosition))
		{
			int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

			// ignore tile if friendly piece is on that tile
			if (Board[RelativePositionIndex].ChessPieceOnTile)
				if (Board[RelativePositionIndex].ChessPieceOnTile->ChessPieceInfo.bIsWhite == ChessPieceInfo.bIsWhite)
					continue;

			ValidMovePositions.Add(RelativePositionIndex);
		}
	}

	TArray<FChessTileInfo> ValidTiles;

	for (int32& Position : ValidMovePositions)
		ValidTiles.AddUnique(Board[Position]);

	return ValidTiles;
}

TArray<FChessTileInfo> AChessPiece::CalculateValidMoveTilesForRook(TArray<FChessTileInfo> Board)
{
	if (!ChessBoard)
	{
		PRINTSTRING(FColor::Red, "ChessBoard is INVALID in ChessPiece");
		return TArray<FChessTileInfo>();
	}

	TArray<int32> ValidMovePositions;

	for (FVector2D& Position : RookMovePositionDirections)
	{
		for (int32 i = 1; i < 8; i++)
		{
			FVector2D RelativePosition = ChessPieceInfo.GetChessPiecePositionFromIndex() + (Position * i);
			if (IsPositionWithinBounds(RelativePosition))
			{
				int32 RelativePositionIndex = RelativePosition.X * 8 + RelativePosition.Y;

				// if a piece is on that tile...
				if (Board[RelativePositionIndex].ChessPieceOnTile)
				{
					// and if its an opponent piece
					if (Board[RelativePositionIndex].ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite)
						ValidMovePositions.Add(RelativePositionIndex);

					break;
				}

				ValidMovePositions.Add(RelativePositionIndex);
			}
		}
	}

	TArray<FChessTileInfo> ValidTiles;

	for (int32& Position : ValidMovePositions)
		ValidTiles.AddUnique(Board[Position]);

	return ValidTiles;
}

TArray<FChessTileInfo> AChessPiece::CalculateValidMoveTilesForPawn(TArray<FChessTileInfo> Board, int32 OutEnpassantTarget)
{
	if (!ChessBoard)
	{
		PRINTSTRING(FColor::Red, "ChessBoard is INVALID in ChessPiece");
		return TArray<FChessTileInfo>();
	}

	TArray<int32> ValidMovePositions;

	if (ChessPieceInfo.bIsWhite)
	{
		if (IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, 0)))
			if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 8].ChessPieceOnTile) // if no piece is present in one tile front
				ValidMovePositions.Add(ChessPieceInfo.ChessPiecePositionIndex + 8);

		if (ChessPieceInfo.GetChessPiecePositionFromIndex().X == 1 && IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(2, 0)))
			if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 8].ChessPieceOnTile) // if no piece is present in one tile front
				if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 16].ChessPieceOnTile) // if no piece is present in two tiles front
					ValidMovePositions.Add(ChessPieceInfo.ChessPiecePositionIndex + 16);

		// Diagonal Captures
		if (IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, -1)))
			if (Board[ChessPieceInfo.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile) // if a piece on diagonal tile present then make it a valid move position
				if (Board[ChessPieceInfo.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite) // and if the piece on diagonal tile is not that same colour as to current piece
					ValidMovePositions.Add(ChessPieceInfo.ChessPiecePositionIndex + 8 - 1);

		if (IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, 1)))
			if (Board[ChessPieceInfo.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile) // if a piece on diagonal tile present then make it a valid move position
				if (Board[ChessPieceInfo.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite) // and if the piece on diagonal tile is not that same colour as to current piece
					ValidMovePositions.Add(ChessPieceInfo.ChessPiecePositionIndex + 8 + 1);

		// En Passant White -> Black
		if (ChessPieceInfo.GetChessPiecePositionFromIndex().X == 4 && IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, -1)))
			if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 8 - 1].ChessPieceOnTile) // if no piece is present in one tile diagonal
				if (Board[ChessPieceInfo.ChessPiecePositionIndex + 8 - 1].ChessTilePositionIndex == OutEnpassantTarget) // and if diagonal tile is available for enpassant
					ValidMovePositions.Add(ChessPieceInfo.ChessPiecePositionIndex + 8 - 1);

		if (ChessPieceInfo.GetChessPiecePositionFromIndex().X == 4 && IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(1, 1)))
			if (!Board[ChessPieceInfo.ChessPiecePositionIndex + 8 + 1].ChessPieceOnTile) // if no piece is present in one tile diagonal
				if (Board[ChessPieceInfo.ChessPiecePositionIndex + 8 + 1].ChessTilePositionIndex == OutEnpassantTarget) // and if diagonal tile is available for enpassant
					ValidMovePositions.Add(ChessPieceInfo.ChessPiecePositionIndex + 8 + 1);
	}
	else
	{
		if (IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, 0)))
			if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 8].ChessPieceOnTile) // if no piece is present in one tile front
				ValidMovePositions.Add(ChessPieceInfo.ChessPiecePositionIndex - 8);

		if (ChessPieceInfo.GetChessPiecePositionFromIndex().X == 6 && IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-2, 0)))
			if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 8].ChessPieceOnTile) // if no piece is present in one tile front
				if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 16].ChessPieceOnTile) // if no piece is present in two tiles front
					ValidMovePositions.Add(ChessPieceInfo.ChessPiecePositionIndex - 16);

		// Diagonal Captures
		if (IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, 1)))
			if (Board[ChessPieceInfo.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile) // if a piece on diagonal tile present then make it a valid move position
				if (Board[ChessPieceInfo.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite) // and if the piece on diagonal tile is not that same colour as to current piece
					ValidMovePositions.Add(ChessPieceInfo.ChessPiecePositionIndex - 8 + 1);

		if (IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, -1)))
			if (Board[ChessPieceInfo.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile) // if a piece on diagonal tile present then make it a valid move position
				if (Board[ChessPieceInfo.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile->ChessPieceInfo.bIsWhite != ChessPieceInfo.bIsWhite) // and if the piece on diagonal tile is not that same colour as to current piece
					ValidMovePositions.Add(ChessPieceInfo.ChessPiecePositionIndex - 8 - 1);

		// En Passant Black -> White
		if (ChessPieceInfo.GetChessPiecePositionFromIndex().X == 3 && IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, 1)))
			if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 8 + 1].ChessPieceOnTile) // if no piece is present in one tile diagonal
				if (Board[ChessPieceInfo.ChessPiecePositionIndex - 8 + 1].ChessTilePositionIndex == OutEnpassantTarget) // and if diagonal tile is available for enpassant
					ValidMovePositions.Add(ChessPieceInfo.ChessPiecePositionIndex - 8 + 1);

		if (ChessPieceInfo.GetChessPiecePositionFromIndex().X == 3 && IsPositionWithinBounds(ChessPieceInfo.GetChessPiecePositionFromIndex() + FVector2D(-1, -1)))
			if (!Board[ChessPieceInfo.ChessPiecePositionIndex - 8 - 1].ChessPieceOnTile) // if no piece is present in one tile diagonal
				if (Board[ChessPieceInfo.ChessPiecePositionIndex - 8 - 1].ChessTilePositionIndex == OutEnpassantTarget) // and if diagonal tile is available for enpassant
					ValidMovePositions.Add(ChessPieceInfo.ChessPiecePositionIndex - 8 - 1);
	}

	TArray<FChessTileInfo> ValidTiles;

	for (int32& Position : ValidMovePositions)
		ValidTiles.AddUnique(Board[Position]);

	return ValidTiles;
}