// Copyright Kunal Patil (kroxyserver). All Rights Reserved.

#include "Chess.h"

#define LOCTEXT_NAMESPACE "FChessModule"

void FChessModule::StartupModule()
{
#if WITH_EDITOR
	static const FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);

	TSharedRef<FPropertySection> Section = PropertyModule.FindOrCreateSection("Object", "+Chess", LOCTEXT("+Chess", "+Chess"));
	Section->AddCategory("+Chess");
#endif // WITH_EDITOR
}

void FChessModule::ShutdownModule()
{
}

IMPLEMENT_PRIMARY_GAME_MODULE(FChessModule, Chess, "Chess");