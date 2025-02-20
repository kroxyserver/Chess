// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#include "Board/ChessPiece.h"

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
					ChessBoard->ChessBoardInfo.bIsWhiteKingSideRookAlive = false;
				}
				else
				{
					ChessBoard->ChessBoardInfo.bIsBlackKingSideRookAlive = false;
				}
				break;
			case EChessPieceSide::QueenSide:
				if (ChessPieceInfo.bIsWhite)
				{
					ChessBoard->ChessBoardInfo.bIsWhiteQueenSideRookAlive = false;
				}
				else
				{
					ChessBoard->ChessBoardInfo.bIsBlackQueenSideRookAlive = false;
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

float AChessPiece::MovePiece(AChessTile* MoveToTile)
{
	// Movement code

	InterpToMovementComponent->ResetControlPoints();
	InterpToMovementComponent->RestartMovement(1.f);

	//if (ChessPieceInfo.ChessPieceType == EChessPieceType::Knight)
	//{
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
	//}
	//else
	//{
	//	InterpToMovementComponent->AddControlPointPosition(GetActorLocation(), false);
	//	InterpToMovementComponent->AddControlPointPosition(MoveToTile->GetActorLocation(), false);

	//	TotalTravelDistance = UKismetMathLibrary::Vector_Distance(GetActorLocation(), MoveToTile->GetActorLocation());

	//	InterpToMovementComponent->Duration = TotalTravelDistance / 1000.f;
	//}

	InterpToMovementComponent->FinaliseControlPoints();

	return InterpToMovementComponent->Duration;
}

//void AChessPiece::PromotePawn(EChessPieceType PromotionType)
//{
//	if (ChessPieceInfo.ChessPieceType != EChessPieceType::Pawn) return;
//
//	if (PromotionType == EChessPieceType::King || PromotionType == EChessPieceType::Pawn) return;
//
//	ChessPieceInfo.ChessPieceType = PromotionType; // Set ChessPieceType to PromotionType
//
//	UpdateChessPieceStaticMesh(); // Update Static Mesh to new PieceType
//}

void AChessPiece::UpdateChessPieceStaticMesh()
{
	if (ChessBoardData)
	{
		if (ChessPieceInfo.bIsWhite)
		{
			switch (ChessPieceInfo.ChessPieceType)
			{
			case EChessPieceType::King:
				if (ChessBoardData->WhiteKing.LoadSynchronous()) ChessPieceMesh->SetStaticMesh(ChessBoardData->WhiteKing.LoadSynchronous());
				break;
			case EChessPieceType::Queen:
				if (ChessBoardData->WhiteQueen.LoadSynchronous()) ChessPieceMesh->SetStaticMesh(ChessBoardData->WhiteQueen.LoadSynchronous());
				break;
			case EChessPieceType::Bishop:
				if (ChessBoardData->WhiteBishop.LoadSynchronous()) ChessPieceMesh->SetStaticMesh(ChessBoardData->WhiteBishop.LoadSynchronous());
				break;
			case EChessPieceType::Knight:
				if (ChessBoardData->WhiteKnight.LoadSynchronous()) ChessPieceMesh->SetStaticMesh(ChessBoardData->WhiteKnight.LoadSynchronous());
				break;
			case EChessPieceType::Rook:
				if (ChessBoardData->WhiteRook.LoadSynchronous()) ChessPieceMesh->SetStaticMesh(ChessBoardData->WhiteRook.LoadSynchronous());
				break;
			case EChessPieceType::Pawn:
				if (ChessBoardData->WhitePawn.LoadSynchronous()) ChessPieceMesh->SetStaticMesh(ChessBoardData->WhitePawn.LoadSynchronous());
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
				if (ChessBoardData->BlackKing.LoadSynchronous()) ChessPieceMesh->SetStaticMesh(ChessBoardData->BlackKing.LoadSynchronous());
				break;
			case EChessPieceType::Queen:
				if (ChessBoardData->BlackQueen.LoadSynchronous()) ChessPieceMesh->SetStaticMesh(ChessBoardData->BlackQueen.LoadSynchronous());
				break;
			case EChessPieceType::Bishop:
				if (ChessBoardData->BlackBishop.LoadSynchronous()) ChessPieceMesh->SetStaticMesh(ChessBoardData->BlackBishop.LoadSynchronous());
				break;
			case EChessPieceType::Knight:
				if (ChessBoardData->BlackKnight.LoadSynchronous()) ChessPieceMesh->SetStaticMesh(ChessBoardData->BlackKnight.LoadSynchronous());
				break;
			case EChessPieceType::Rook:
				if (ChessBoardData->BlackRook.LoadSynchronous()) ChessPieceMesh->SetStaticMesh(ChessBoardData->BlackRook.LoadSynchronous());
				break;
			case EChessPieceType::Pawn:
				if (ChessBoardData->BlackPawn.LoadSynchronous()) ChessPieceMesh->SetStaticMesh(ChessBoardData->BlackPawn.LoadSynchronous());
				break;
			default:
				break;
			}
		}
	}
}