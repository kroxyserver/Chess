// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#include "Core/ChessGameInstance.h"

UChessGameInstance::UChessGameInstance() :
	ChessGameModeType(EChessGameModeType::Player_VS_Player),
	ChessMapType(EChessMapType::Beach)
{
	static ConstructorHelpers::FObjectFinder<UDataTable> ChessMapDataTableAsset(TEXT("/Script/Engine.DataTable'/Game/+Chess/Data/DT_ChessMapData.DT_ChessMapData'"));
	if (ChessMapDataTableAsset.Succeeded()) ChessMapDataTable = ChessMapDataTableAsset.Object;
}

void UChessGameInstance::UpdateChessGameModeType(EChessGameModeType NewChessGameModeType)
{
	ChessGameModeType = NewChessGameModeType;
}

void UChessGameInstance::UpdateChessMapType(EChessMapType NewChessMapType)
{
	ChessMapType = NewChessMapType;
}