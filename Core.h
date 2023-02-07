#pragma once
#include "Includes.h"
#include "Replication.h"
#include "Inventory.h"
#include "Abilities.h"
#include <ostream>
#include <fstream>
FVector GetSpawnLoc() {
	TArray<AActor*> SpawnLocs = {};
	GGameplayStatics->GetAllActorsOfClass(GEngine->GameViewport->World, AFortPlayerStartWarmup::StaticClass(), &SpawnLocs);
	return SpawnLocs[(rand() % SpawnLocs.Num() - 1)]->K2_GetActorLocation();
}

APlayerPawn_Athena_C* SpawnPawn(FVector SpawnLoc = GetSpawnLoc()) {
	APlayerPawn_Athena_C* Pawn = SpawnActor<APlayerPawn_Athena_C>(SpawnLoc, nullptr);
	if (reinterpret_cast<AFortGameStateAthena*>(GEngine->GameViewport->World->AuthorityGameMode->GameState)->GamePhase == EAthenaGamePhase::Warmup) {
		//Pawn->bCanBeDamaged = false;
	}
	Pawn->HealthRegenDelayGameplayEffect = nullptr;
	Pawn->ShieldRegenDelayGameplayEffect = nullptr;
	Pawn->ShieldRegenGameplayEffect = nullptr;
	Pawn->HealthRegenGameplayEffect = nullptr;
	Pawn->SetReplicates(true);
	Pawn->SetReplicateMovement(true);
	Pawn->OnRep_ReplicatedBasedMovement();
	Pawn->OnRep_ReplicatedMovement();
	return Pawn;
}

struct OverridenBackpackSizeOffsetFix {
	unsigned char PAD[0x2208];
	int32_t OverriddenBackpackSize;
};

struct IA_BitFieldOffsetFix {
	unsigned char PAD[0x2204];
	uint8 bEnableVoiceChatPTT : 1;
	uint8 bInfiniteAmmo : 1;
	uint8 bNoCoolDown : 1;
	uint8 bInfiniteDurability : 1;
	uint8 bUsePickers : 1;
	uint8 bPickerOpen : 1;
	uint8 BitPad_52 : 1;
	uint8 bCheatGhost : 1;
};

struct HasServerFinishedLoadingOffsetFix {
	unsigned char PAD[0x778];
	bool bHasClientFinishedLoading;
	bool bHasServerFinishedLoading;
};

void FixPickups(AFortPlayerController* PC) {
	reinterpret_cast<OverridenBackpackSizeOffsetFix*>(PC)->OverriddenBackpackSize = 5;
}

std::vector<UCustomCharacterPart*> GetPlayerParts(AFortPlayerControllerAthena* PC) {
	auto Loadout = PC->CustomizationLoadout;
	std::vector<UCustomCharacterPart*> Ret = { UObject::FindObject<UCustomCharacterPart>("CustomCharacterPart F_Med_Head1.F_Med_Head1") , UObject::FindObject<UCustomCharacterPart>("CustomCharacterPart F_Med_Soldier_01.F_Med_Soldier_01") };
	if (Loadout.Character) {
		auto CID = Loadout.Character;
		auto Pawn = (AFortPlayerPawnAthena*)PC->Pawn;
		if (!CID) {
			return Ret;
		}
		auto Hero = CID->HeroDefinition;
		if (!Hero) {
			return Ret;
		}
		auto Specializations = Hero->Specializations;
		if (!Specializations.Data) {
			return Ret;
		}
		for (int i = 0; i < Specializations.Num(); i++) {
			static auto CharPartClass = UObject::FindObject("Class FortniteGame.CustomCharacterPart");
			static auto SpecializationClass = UObject::FindObject("Class FortniteGame.FortHeroSpecialization");
			auto SpecializationName = Specializations[i].ObjectID.AssetPathName.ToString();
			auto Specialization = LoadObject<UFortHeroSpecialization>(UFortHeroSpecialization::StaticClass(), std::wstring(SpecializationName.begin(), SpecializationName.end()).c_str());
			if (!Specialization) {
				continue;
			}
			auto Parts = Specialization->CharacterParts;
			if (!Parts.Data) {
				continue;
			}

			for (int x = 0; x < Parts.Num(); x++) {
				auto PartName = Parts[x].ObjectID.AssetPathName.ToString();
				auto Part = LoadObject<UCustomCharacterPart>(UCustomCharacterPart::StaticClass(), std::wstring(PartName.begin(), PartName.end()).c_str());
				if (!Part) {
					continue;
				}
				Ret.push_back(Part);
			}
		}
	}

	return Ret;
}

void ApplyDefaultCosmetics(AFortPlayerPawnAthena* Pawn) {
	((AFortPlayerPawnAthena*)Pawn)->ServerChoosePart(EFortCustomPartType::Head, UObject::FindObject<UCustomCharacterPart>("CustomCharacterPart F_Med_Head1.F_Med_Head1"));
	((AFortPlayerPawnAthena*)Pawn)->ServerChoosePart(EFortCustomPartType::Body, UObject::FindObject<UCustomCharacterPart>("CustomCharacterPart F_Med_Soldier_01.F_Med_Soldier_01"));
}

void ApplyCosmetics(AFortPlayerControllerAthena* PC) {
	auto Pawn = (AFortPlayerPawnAthena*)PC->Pawn;
	if (!Pawn) {
		return;
	}
	//PC->ClientForceProfileQuery();
	auto McpLoadout = UObject::FindObjectFast<UFortMcpContext>("FortMcpContext_0");
	auto Loadout = PC->CustomizationLoadout;
	if (!Loadout.Character) {
		Loadout = Pawn->CustomizationLoadout;
	}
	if (!Loadout.Character) {
		Loadout = McpLoadout->GetLoadoutForPlayer(PC->NetConnection->PlayerID);
	}
	if (Loadout.Character) {
		auto CID = Loadout.Character;
		if (!CID) {
			return ApplyDefaultCosmetics(Pawn);
		}
		auto Hero = CID->HeroDefinition;
		if (!Hero) {
			return ApplyDefaultCosmetics(Pawn);
		}
		auto Specializations = Hero->Specializations;
		if (!Specializations.Data) {
			return ApplyDefaultCosmetics(Pawn);
		}
		for (int i = 0; i < Specializations.Num(); i++) {
			auto SpecializationName = Specializations[i].ObjectID.AssetPathName.ToString();
			auto Specialization = LoadObject<UFortHeroSpecialization>(UFortHeroSpecialization::StaticClass(), std::wstring(SpecializationName.begin(), SpecializationName.end()).c_str());
			if (!Specialization) {
				continue;
			}
			auto Parts = Specialization->CharacterParts;
			if (!Parts.Data) {
				continue;
			}

			for (int x = 0; x < Parts.Num(); x++) {
				auto PartName = Parts[x].ObjectID.AssetPathName.ToString();
				auto Part = LoadObject<UCustomCharacterPart>(UCustomCharacterPart::StaticClass(), std::wstring(PartName.begin(), PartName.end()).c_str());
				if (!Part) {
					continue;
				}
				Pawn->ServerChoosePart(Part->CharacterPartType, Part);
			}
		}
	}
	else {
		MessageBoxA(0, "INVALID", "Test", MB_OK);
		return ApplyDefaultCosmetics(Pawn);
	}
}

UFortWeaponMeleeItemDefinition* GetPlayerPickaxe(AFortPlayerControllerAthena* PC) {
	auto Pickaxe = UObject::FindObject<UFortWeaponMeleeItemDefinition>("FortWeaponMeleeItemDefinition WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");
	auto Loadout = PC->CustomizationLoadout;
	if (Loadout.Pickaxe) {
		Pickaxe = Loadout.Pickaxe->WeaponDefinition;
	}
	return Pickaxe;
}

namespace Hooks {
	void HandleReloadCost(AFortWeapon* Weapon, int AmmoToRemove) {
		if (!Weapon) {
			return;
		}
		auto Pawn = (AFortPlayerPawnAthena*)Weapon->GetOwner();
		if (!Pawn)
		{
			return;
		}
		auto PC = (AFortPlayerControllerAthena*)Pawn->Controller;
		if (!PC) {
			return;
		}
		FFortItemEntry Entry = Inventory::GetEntryInInv(PC, Weapon->ItemEntryGuid);
		if (!Entry.ItemDefinition) {
			return;
		}
		Entry.LoadedAmmo = Weapon->AmmoCount;
		auto Item = (UFortWorldItemDefinition*)Weapon->WeaponData;
		if (!Item) {
			return;
		}
		auto AmmoDef = Item->GetAmmoWorldItemDefinition_BP();
		if (!AmmoDef) {
			return;
		}
		Inventory::AddItem(PC, AmmoDef, AmmoToRemove, 0, EFortQuickBars::Secondary);
	}

	UNetConnection* GetNetConnection_Hk(APlayerController* PC)
	{
		return (UNetConnection*)PC->Player;
	}

	APlayerController* (*SpawnPlayActor)(UWorld* World, UNetConnection* Connection, ENetRole NetRole, FURL InUrl, void* UniqueId, FString& Error, uint8_t InNetPlayerIndex);
	AFortPlayerControllerAthena* SpawnPlayActor_Hk(UWorld* World, UNetConnection* Connection, ENetRole NetRole, FURL InUrl, void* UniqueId, FString& Error, uint8_t InNetPlayerIndex) {
		Replication::ServerReplicateActors_ProcessActors(Connection, Replication::ServerReplicateActors_BuildConsiderList(Connection->Driver));
		AFortPlayerControllerAthena* PlayerController = (AFortPlayerControllerAthena*)SpawnPlayActor(GEngine->GameViewport->World, Connection, NetRole, InUrl, UniqueId, Error, InNetPlayerIndex);
		//MessageBoxA(0, PlayerController->GetName().c_str(), "SPA", MB_OK);
		Connection->PlayerController = PlayerController;
		PlayerController->NetConnection = Connection;
		PlayerController->Player = Connection;
		Connection->OwningActor = PlayerController;
		//AFortPlayerControllerAthena* PlayerController = SpawnActor<AFortPlayerControllerAthena>();
		/*PlayerController->NetPlayerIndex = InNetPlayerIndex;
		PlayerController->RemoteRole = ENetRole::ROLE_Authority;
		Connection->PlayerID = *UniqueId.Get();
		PlayerController->SetReplicates(true);*/
		//reinterpret_cast<void(*)(AGameModeBase*, APlayerController*)>(GEngine->GameViewport->World->AuthorityGameMode->Vft[0xE2])(GEngine->GameViewport->World->AuthorityGameMode, PlayerController);
		//if (PlayerController != GEngine->GameInstance->LocalPlayers[0]->PlayerController) {
		//PlayerController->bAlwaysRelevant = true;
		Replication::ReplicateToClient(PlayerController, Connection);
		Replication::ReplicateToClient(PlayerController->PlayerState, Connection);
		Replication::ReplicateToClient(World->GameState, Connection);
		auto PlayerState = (AFortPlayerStateAthena*)PlayerController->PlayerState;
		PlayerState->SetReplicates(true);
		APlayerPawn_Athena_C* Pawn = SpawnPawn();
		Pawn->SetOwner(PlayerController);
		Pawn->SetReplicates(true);
		PlayerController->Possess(Pawn);
		Pawn->SetMaxHealth(100);
		Pawn->SetHealth(100);
		ApplyDefaultCosmetics(Pawn);
		PlayerState->SetOwner(PlayerController);
		Pawn->PlayerState = PlayerState;
		Replication::ReplicateToClient(Pawn, Connection);
		PlayerController->bClientPawnIsLoaded = true;
		PlayerController->bReadyToStartMatch = true;
		PlayerController->bAssignedStartSpawn = true;
		PlayerController->bHasInitiallySpawned = true;
		reinterpret_cast<HasServerFinishedLoadingOffsetFix*>(PlayerController)->bHasClientFinishedLoading = true;
		reinterpret_cast<HasServerFinishedLoadingOffsetFix*>(PlayerController)->bHasServerFinishedLoading = true;
		PlayerController->OnRep_bHasServerFinishedLoading();
		PlayerController->ServerReadyToStartMatch();
		PlayerState->OnRep_CharacterParts();

		PlayerState->bHasStartedPlaying = true;
		PlayerState->bHasFinishedLoading = true;
		PlayerState->bIsReadyToContinue = true;
		PlayerState->OnRep_bHasStartedPlaying();
		PlayerState->bAlwaysRelevant = true;

		PlayerState->OnRep_PlayerTeam();
		auto WorldInventory = SpawnActor<AFortInventory>({ 0,0,0 }, PlayerController);
		WorldInventory->bAlwaysRelevant = true;
		WorldInventory->InventoryType = EFortInventoryType::World;
		WorldInventory->Inventory = FFortItemList();
		WorldInventory->Inventory.ReplicatedEntries = TArray<struct FFortItemEntry>(6);
		WorldInventory->Inventory.ItemInstances = TArray<class UFortWorldItem*>(6);
		WorldInventory->SetOwner(PlayerController);
		(PlayerController)->WorldInventory = WorldInventory;
		reinterpret_cast<Inventory::WorldInventoryOffsetFix*>(PlayerController)->WorldInventory = WorldInventory;
		AFortQuickBars* QuickBars = SpawnActor<AFortQuickBars>({ 0,0,0 }, PlayerController);
		QuickBars->bAlwaysRelevant = true;
		QuickBars->SetOwner(PlayerController);
		reinterpret_cast<Inventory::QuickbarOffsetFix*>(PlayerController)->QuickBars = QuickBars;
		(PlayerController)->QuickBars = QuickBars;
		PlayerController->OnRep_QuickBar();
		QuickBars->EnableSlot(EFortQuickBars::Primary, 0);
		QuickBars->EnableSlot(EFortQuickBars::Primary, 1);
		QuickBars->EnableSlot(EFortQuickBars::Primary, 2);
		QuickBars->EnableSlot(EFortQuickBars::Primary, 3);
		QuickBars->EnableSlot(EFortQuickBars::Primary, 4);
		QuickBars->EnableSlot(EFortQuickBars::Secondary, 0);
		QuickBars->EnableSlot(EFortQuickBars::Secondary, 1);
		QuickBars->EnableSlot(EFortQuickBars::Secondary, 2);
		QuickBars->EnableSlot(EFortQuickBars::Secondary, 3);
		QuickBars->EnableSlot(EFortQuickBars::Secondary, 4);

		Abilities::GiveBaseAbilities(Pawn);

		FixPickups(PlayerController);
		Inventory::SetupInventory(PlayerController);

		Inventory::AddItem(PlayerController, GetPlayerPickaxe(PlayerController), 1, 0);
		ApplyCosmetics(PlayerController);
		PlayerController->ServerExecuteInventoryItem(reinterpret_cast<Inventory::QuickbarOffsetFix*>(PlayerController)->QuickBars->PrimaryQuickBar.Slots[0].Items[0]);

		FixPickups(PlayerController);
		PlayerController->bHasInitializedWorldInventory = true;
		Inventory::Update(PlayerController);

		auto HealthSet = reinterpret_cast<AFortPlayerPawnAthena*>(PlayerController->Pawn)->HealthSet;
		HealthSet->CurrentShield.Minimum = 0;
		HealthSet->CurrentShield.Maximum = 100;
		HealthSet->CurrentShield.BaseValue = 0;
		HealthSet->CurrentShield.CurrentValue = 0;
		HealthSet->Shield.Minimum = 0;
		HealthSet->Shield.Maximum = 100;
		HealthSet->Shield.BaseValue = 100;
		HealthSet->Shield.CurrentValue = 100;
		HealthSet->OnRep_Shield();
		HealthSet->OnRep_CurrentShield();

		reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker = SpawnActor<ABuildingPlayerPrimitivePreview>({ 0, 0, 5000 }, PlayerController);
		Replication::ReplicateToClient(reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker, Connection);
		PlayerController->CheatManager = (UCheatManager*)GGameplayStatics->SpawnObject(UFortCheatManager::StaticClass(), PlayerController);
		//reinterpret_cast<IA_BitFieldOffsetFix*>(PlayerController)->bInfiniteAmmo = true;
		//}
		return PlayerController;
	}

	void (*NCM)(UWorld* InWorld, UNetConnection* Client, uint8_t MessageType, void* Bunch);

	void NCM_Hk(UWorld* InWorld, UNetConnection* Client, uint8_t MessageType, void* Bunch) {
		if (MessageType == 15) {
			return;
		}

		return NCM(InWorld, Client, MessageType, Bunch);
	}
}

namespace Core {
	void StartServer() {
		UNetDriver* NetDriver = reinterpret_cast<UNetDriver * (*)(UEngine * Engine, UWorld * InWorld, FName NetDriverDefinition)>(Base + Offsets::CreateNetDriver)(GEngine, GEngine->GameViewport->World, FName(282));
		NetDriver->NetDriverName = FName(282);
		NetDriver->World = GEngine->GameViewport->World;
		FURL ListenURL = {};
		ListenURL.Port = 7777;
		if (reinterpret_cast<bool(*)(UNetDriver*, void*, FURL*, bool, FString*)>(NetDriver->Vft[Offsets::InitListen])(NetDriver, GEngine->GameViewport->World, &ListenURL, false, {})) {
			LOG("Server Listening on Port 7777!", false);
		}
		reinterpret_cast<void(*)(UNetDriver*, UWorld*)>(Base + Offsets::SetWorld)(NetDriver, GEngine->GameViewport->World);
		GEngine->GameViewport->World->NetDriver = NetDriver;
		GEngine->GameViewport->World->LevelCollections[0].NetDriver = NetDriver;
		GEngine->GameViewport->World->LevelCollections[1].NetDriver = NetDriver;
		uintptr_t TickFlushAddr = (Base + Offsets::TickFlush);
		CreateHook(TickFlushAddr, Hooks::TickFlush_Hk, (void**)&Hooks::TickFlushO);
		uintptr_t SpawnPlayActorAddr = (Base + Offsets::SpawnPlayActor);
		CreateHook(SpawnPlayActorAddr, Hooks::SpawnPlayActor_Hk, (void**)&Hooks::SpawnPlayActor);
		/*uintptr_t GetNetConnectionAddr = __int64(GEngine->GameInstance->LocalPlayers[0]->PlayerController->Vft[Offsets::GetNetConnection]);
		CreateHook(GetNetConnectionAddr, Hooks::GetNetConnection_Hk, nullptr);*/
		uintptr_t NCMAddr = (Base + Offsets::NCM);
		CreateHook(NCMAddr, Hooks::NCM_Hk, (void**)&Hooks::NCM);
	}

	void (*ProcessEventO)(UObject* Obj, UFunction* Func, void* Params);
	void ProcessEvent_Hk(UObject* Obj, UFunction* Func, void* Params) {
		std::string FuncName = Func->GetName();
		static bool bRTSM = false;
		if (FuncName == "ReadyToStartMatch" && !bRTSM) {
			bRTSM = true;
			StartServer();
			AFortGameModeAthena* GM = reinterpret_cast<AFortGameModeAthena*>(Obj);
			AFortGameStateAthena* GS = reinterpret_cast<AFortGameStateAthena*>(GM->GameState);
			GM->GameSession = SpawnActor<AFortGameSession>({}, nullptr);
			GM->GameSession->MaxPlayers = 100;
			GS->GameSessionID = L"GSID";
			GS->OnRep_GameSessionID();
			GM->StartMatch();
			GM->StartPlay();
			EAthenaGamePhase OldGP = GS->GamePhase;
			/*GS->GamePhase = EAthenaGamePhase::Warmup;
			GS->OnRep_GamePhase(OldGP);*/
		}

		//Battle Bus
		if (FuncName == "ServerAttemptAircraftJump") {
			AFortPlayerControllerAthena* PlayerController = (AFortPlayerControllerAthena*)Obj;
			APlayerPawn_Athena_C* Pawn = SpawnPawn(((AFortGameStateAthena*)GEngine->GameViewport->World->GameState)->GetAircraft()->K2_GetActorLocation());
			Pawn->SetOwner(PlayerController);
			PlayerController->Possess(Pawn);
			if (!reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker) {
				reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker = SpawnActor<ABuildingPlayerPrimitivePreview>({ 0, 0, 5000 }, PlayerController);
				Replication::ReplicateToClient(reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker, PlayerController->NetConnection);
			}
			auto HealthSet = reinterpret_cast<AFortPlayerPawnAthena*>(PlayerController->Pawn)->HealthSet;
			HealthSet->CurrentShield.Minimum = 0;
			HealthSet->CurrentShield.Maximum = 100;
			HealthSet->CurrentShield.BaseValue = 0;
			HealthSet->CurrentShield.CurrentValue = 0;
			HealthSet->Shield.Minimum = 0;
			HealthSet->Shield.Maximum = 100;
			HealthSet->Shield.BaseValue = 100;
			HealthSet->Shield.CurrentValue = 100;
			HealthSet->OnRep_Shield();
			HealthSet->OnRep_CurrentShield();
			FixPickups(PlayerController);
			((AFortPlayerPawnAthena*)Pawn)->ServerChoosePart(EFortCustomPartType::Head, UObject::FindObject<UCustomCharacterPart>("CustomCharacterPart F_Med_Head1.F_Med_Head1"));
			((AFortPlayerPawnAthena*)Pawn)->ServerChoosePart(EFortCustomPartType::Body, UObject::FindObject<UCustomCharacterPart>("CustomCharacterPart F_Med_Soldier_01.F_Med_Soldier_01"));
			Abilities::GiveBaseAbilities(Pawn);
			Inventory::Update(PlayerController);
		}

		if (FuncName == "OnAircraftExitedDropZone") {
			auto Clients = GEngine->GameViewport->World->NetDriver->ClientConnections;
			for (int i = 0; i < Clients.Num(); i++) {
				auto Client = Clients[i];
				if (Client && Client->PlayerController && Client->PlayerController && ((AFortPlayerControllerAthena*)(Client->PlayerController))->IsInAircraft()) {
					((AFortPlayerControllerAthena*)(Client->PlayerController))->ServerAttemptAircraftJump({});
				}
			}
		}

		//Abilities
		if (FuncName == "ServerTryActivateAbility") {
			UAbilitySystemComponent* ASC = (UAbilitySystemComponent*)Obj;
			auto InParams = (Params::UAbilitySystemComponent_ServerTryActivateAbility_Params*)Params;

			FGameplayAbilitySpecHandle AbilityToActivate = InParams->AbilityToActivate;
			FPredictionKey PredictionKey = InParams->PredictionKey;

			static auto InternalTryActivateAbility = reinterpret_cast<bool(*)(UAbilitySystemComponent * ASC, FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey InPredictionKey, UGameplayAbility * *OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, const FGameplayEventData * TriggerEventData)>(Base + Offsets::InternalTryActivateAbility);

			auto Spec = Abilities::FindAbilitySpecFromHandle(ASC, AbilityToActivate);
			Spec->InputPressed = true;

			UGameplayAbility* InstancedAbility = nullptr;
			FGameplayEventData* TriggerEventData = nullptr;

			if (!InternalTryActivateAbility(ASC, AbilityToActivate, PredictionKey, &InstancedAbility, nullptr, TriggerEventData)) {
				ASC->ClientActivateAbilityFailed(AbilityToActivate, PredictionKey.Current);
				return;
			}

			reinterpret_cast<bool(*)(UAbilitySystemComponent * ASC, FGameplayAbilitySpec & Spec)>(Base + Offsets::MarkAbilitySpecDirty)(ASC, *Spec);
		}

		if (FuncName == "ServerTryActivateAbilityWithEventData") {
			UAbilitySystemComponent* ASC = (UAbilitySystemComponent*)Obj;
			auto InParams = (Params::UAbilitySystemComponent_ServerTryActivateAbilityWithEventData_Params*)Params;

			FGameplayAbilitySpecHandle AbilityToActivate = InParams->AbilityToActivate;
			FPredictionKey PredictionKey = InParams->PredictionKey;

			static auto InternalTryActivateAbility = reinterpret_cast<bool(*)(UAbilitySystemComponent * ASC, FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey InPredictionKey, UGameplayAbility * *OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, const FGameplayEventData * TriggerEventData)>(Base + Offsets::InternalTryActivateAbility);

			auto Spec = Abilities::FindAbilitySpecFromHandle(ASC, AbilityToActivate);
			Spec->InputPressed = true;

			UGameplayAbility* InstancedAbility = nullptr;
			FGameplayEventData* TriggerEventData = nullptr;

			if (!InternalTryActivateAbility(ASC, AbilityToActivate, PredictionKey, &InstancedAbility, nullptr, &InParams->TriggerEventData)) {
				ASC->ClientActivateAbilityFailed(AbilityToActivate, PredictionKey.Current);
				return;
			}

			reinterpret_cast<bool(*)(UAbilitySystemComponent * ASC, FGameplayAbilitySpec & Spec)>(Base + Offsets::MarkAbilitySpecDirty)(ASC, *Spec);
		}

		if (FuncName == "ServerAbilityRPCBatch") {
			UAbilitySystemComponent* ASC = (UAbilitySystemComponent*)Obj;
			auto InParams = (Params::UAbilitySystemComponent_ServerAbilityRPCBatch_Params*)Params;

			static auto InternalTryActivateAbility = reinterpret_cast<bool(*)(UAbilitySystemComponent * ASC, FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey InPredictionKey, UGameplayAbility * *OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, const FGameplayEventData * TriggerEventData)>(Base + Offsets::InternalTryActivateAbility);

			auto Spec = Abilities::FindAbilitySpecFromHandle(ASC, InParams->BatchInfo.AbilitySpecHandle);
			Spec->InputPressed = true;

			UGameplayAbility* InstancedAbility = nullptr;
			FGameplayEventData* TriggerEventData = nullptr;


			if (!InternalTryActivateAbility(ASC, InParams->BatchInfo.AbilitySpecHandle, InParams->BatchInfo.PredictionKey, &InstancedAbility, nullptr, TriggerEventData)) {
				ASC->ClientActivateAbilityFailed(InParams->BatchInfo.AbilitySpecHandle, InParams->BatchInfo.PredictionKey.Current);
				return;
			}

			reinterpret_cast<bool(*)(UAbilitySystemComponent * ASC, FGameplayAbilitySpec & Spec)>(Base + Offsets::MarkAbilitySpecDirty)(ASC, *Spec);
		}

		//Inventory
		if (FuncName == "ServerExecuteInventoryItem") {
			AFortPlayerControllerAthena* PlayerController = (AFortPlayerControllerAthena*)Obj;
			auto InParams = (Params::AFortPlayerController_ServerExecuteInventoryItem_Params*)Params;
			if (!PlayerController || !InParams || PlayerController->IsInAircraft()) {
				return ProcessEventO(Obj, Func, Params);
			}

			static UFortBuildingItemDefinition* Wall = UObject::FindObject<UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_Wall.BuildingItemData_Wall");
			static UFortBuildingItemDefinition* Floor = UObject::FindObject<UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_Floor.BuildingItemData_Floor");
			static UFortBuildingItemDefinition* Stair = UObject::FindObject<UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_Stair_W.BuildingItemData_Stair_W");
			static UFortBuildingItemDefinition* Roof = UObject::FindObject<UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_RoofS.BuildingItemData_RoofS");

			auto Weapon = Inventory::EquipInventoryItem(PlayerController, InParams->ItemGuid);

			if (false) {
				auto Def = Weapon;
				if (reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker) {
					reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker->SetActorHiddenInGame(true);
				}
				if (Def && Def->IsA(UFortBuildingItemDefinition::StaticClass())) {
					auto BuildPreviewMarker = reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker;
					//PlayerController->CheatManager = (UCheatManager*)GGameplayStatics->SpawnObject(UFortCheatManager::StaticClass(), PlayerController);
					//auto CheatManager = (UFortCheatManager*)PlayerController->CheatManager;
					/*CheatManager->BuildWith(TEXT("Wood"));
					CheatManager->BuildWith(TEXT("Stone"));
					CheatManager->BuildWith(TEXT("Wood"));*/
					if (BuildPreviewMarker) {
						//auto OldResourceType = BuildPreviewMarker->ResourceType;
						//reinterpret_cast<Inventory::CurrentResourceTypeOffsetFix*>(PlayerController)->CurrentResourceType = EFortResourceType::Wood;
						if (Def == Wall) {
							//if (reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker) {
							//	reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker->K2_DestroyActor();
							//}
							reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker->SetActorHiddenInGame(false);
							static UClass* BuildClass = UObject::FindClass("BlueprintGeneratedClass PBWA_W1_Solid.PBWA_W1_Solid_C");
							//static UStaticMesh* Mesh = UObject::FindObject<UStaticMesh>("StaticMesh PBW_W1_Solid.PBW_W1_Solid");
							static UBuildingEditModeMetadata* Metadata = UObject::FindObject<UBuildingEditModeMetadata>("BuildingEditModeMetadata_Wall EMP_Wall_Solid.EMP_Wall_Solid");
							reinterpret_cast<Inventory::CurrentBuildableClassOffsetFix*>(PlayerController)->CurrentBuildableClass = BuildClass;
							//auto BuildPreviewMarker = SpawnActor<ABuildingPlayerPrimitivePreview>({ 0,0,5000 }, PlayerController);
							BuildPreviewMarker->EditModeSupport = (UBuildingEditModeSupport*)GGameplayStatics->SpawnObject(UBuildingEditModeSupport_Wall::StaticClass(), BuildPreviewMarker);
							if (BuildPreviewMarker) {
								//BuildPreviewMarker->GetBuildingMeshComponent()->SetStaticMesh(Mesh);
								//BuildPreviewMarker->GetBuildingMeshComponent()->SetMaterial(0, reinterpret_cast<Inventory::BuildPreviewMarkerMIDOffsetFix*>(PlayerController)->BuildPreviewMarkerMID);
								BuildPreviewMarker->BuildingType = EFortBuildingType::Wall;
								BuildPreviewMarker->EditModePatternData = Metadata;
								BuildPreviewMarker->EditModeSupportClass = UBuildingEditModeSupport_Wall::StaticClass();
								BuildPreviewMarker->OnBuildingActorInitialized(EFortBuildingInitializationReason::Replaced, EFortBuildingPersistentState::Default);
								//Replication::ReplicateToClient(reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker, PlayerController->NetConnection);
							}
						}

						if (Def == Floor) {
							//CheatManager->BuildWith(TEXT("Wood"));
							/*if (reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker) {
								reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker->K2_DestroyActor();
							}*/
							reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker->SetActorHiddenInGame(false);
							static UClass* BuildClass = UObject::FindClass("BlueprintGeneratedClass PBWA_W1_Floor_C.PBWA_W1_Floor_C");
							//static UStaticMesh* Mesh = UObject::FindObject<UStaticMesh>("StaticMesh PBW_W1_Floor.PBW_W1_Floor");
							static UBuildingEditModeMetadata* Metadata = UObject::FindObject<UBuildingEditModeMetadata>("BuildingEditModeMetadata_Floor EMP_Floor_Floor.EMP_Floor_Floor");
							reinterpret_cast<Inventory::CurrentBuildableClassOffsetFix*>(PlayerController)->CurrentBuildableClass = BuildClass;
							//auto BuildPreviewMarker = SpawnActor<ABuildingPlayerPrimitivePreview>({ 0,0,5000 }, PlayerController);
							BuildPreviewMarker->EditModeSupport = (UBuildingEditModeSupport*)GGameplayStatics->SpawnObject(UBuildingEditModeSupport_Floor::StaticClass(), BuildPreviewMarker);
							if (BuildPreviewMarker) {
								//BuildPreviewMarker->GetBuildingMeshComponent()->SetStaticMesh(Mesh);
								//BuildPreviewMarker->GetBuildingMeshComponent()->SetMaterial(0, reinterpret_cast<Inventory::BuildPreviewMarkerMIDOffsetFix*>(PlayerController)->BuildPreviewMarkerMID);
								BuildPreviewMarker->BuildingType = EFortBuildingType::Floor;
								BuildPreviewMarker->EditModePatternData = Metadata;
								BuildPreviewMarker->EditModeSupportClass = UBuildingEditModeSupport_Floor::StaticClass();
								//BuildPreviewMarker->OnBuildingActorInitialized(EFortBuildingInitializationReason::PlacementTool, EFortBuildingPersistentState::New);
								//reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker = BuildPreviewMarker;
								//Replication::ReplicateToClient(reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker, PlayerController->NetConnection);
							}
						}

						if (Def == Stair) {
							//CheatManager->BuildWith(TEXT("Wood"));
							/*if (reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker) {
								reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker->K2_DestroyActor();
							}*/
							reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker->SetActorHiddenInGame(false);
							static UClass* BuildClass = UObject::FindClass("BlueprintGeneratedClass PBWA_M1_StairW_C.PBWA_M1_StairW_C");
							//static UStaticMesh* Mesh = UObject::FindObject<UStaticMesh>("StaticMesh PBW_W1_StairW.PBW_W1_StairW");
							static UBuildingEditModeMetadata* Metadata = UObject::FindObject<UBuildingEditModeMetadata>("BuildingEditModeMetadata_Stair EMP_Stair_StairW.EMP_Stair_StairW");
							reinterpret_cast<Inventory::CurrentBuildableClassOffsetFix*>(PlayerController)->CurrentBuildableClass = BuildClass;
							//auto BuildPreviewMarker = SpawnActor<ABuildingPlayerPrimitivePreview>({ 0,0,5000 }, PlayerController);
							BuildPreviewMarker->EditModeSupport = (UBuildingEditModeSupport*)GGameplayStatics->SpawnObject(UBuildingEditModeSupport_Stair::StaticClass(), BuildPreviewMarker);
							if (BuildPreviewMarker) {
								//BuildPreviewMarker->GetBuildingMeshComponent()->SetStaticMesh(Mesh);
								//BuildPreviewMarker->GetBuildingMeshComponent()->SetMaterial(0, reinterpret_cast<Inventory::BuildPreviewMarkerMIDOffsetFix*>(PlayerController)->BuildPreviewMarkerMID);
								BuildPreviewMarker->BuildingType = EFortBuildingType::Stairs;
								BuildPreviewMarker->EditModePatternData = Metadata;
								BuildPreviewMarker->EditModeSupportClass = UBuildingEditModeSupport_Stair::StaticClass();
								//BuildPreviewMarker->OnBuildingActorInitialized(EFortBuildingInitializationReason::PlacementTool, EFortBuildingPersistentState::New);
								//reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker = BuildPreviewMarker;
								//Replication::ReplicateToClient(reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker, PlayerController->NetConnection);
							}
						}

						if (Def == Roof) {
							//CheatManager->BuildWith(TEXT("Wood"));
							/*if (reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker) {
								reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker->K2_DestroyActor();
							}*/
							reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker->SetActorHiddenInGame(false);
							static UClass* BuildClass = UObject::FindClass("BlueprintGeneratedClass PBWA_W1_RoofC_C.PBWA_W1_RoofC_C");
							//static UStaticMesh* Mesh = UObject::FindObject<UStaticMesh>("StaticMesh PBW_W1_RoofC.PBW_W1_RoofC");
							static UBuildingEditModeMetadata* Metadata = UObject::FindObject<UBuildingEditModeMetadata>("BuildingEditModeMetadata_Roof EMP_Roof_RoofC.EMP_Roof_RoofC");
							reinterpret_cast<Inventory::CurrentBuildableClassOffsetFix*>(PlayerController)->CurrentBuildableClass = BuildClass;
							//auto BuildPreviewMarker = SpawnActor<ABuildingPlayerPrimitivePreview>({ 0,0,5000 }, PlayerController);
							//BuildPreviewMarker->EditModeSupport = (UBuildingEditModeSupport*)GGameplayStatics->SpawnObject(UBuildingEditModeSupport_Roof::StaticClass(), BuildPreviewMarker);
							if (BuildPreviewMarker) {
								//BuildPreviewMarker->GetBuildingMeshComponent()->SetStaticMesh(Mesh);
								//BuildPreviewMarker->GetBuildingMeshComponent()->SetMaterial(0, reinterpret_cast<Inventory::BuildPreviewMarkerMIDOffsetFix*>(PlayerController)->BuildPreviewMarkerMID);
								BuildPreviewMarker->BuildingType = EFortBuildingType::Roof;
								//BuildPreviewMarker->EditModePatternData = Metadata;
								BuildPreviewMarker->EditModeSupportClass = UBuildingEditModeSupport_Roof::StaticClass();
								//BuildPreviewMarker->OnBuildingActorInitialized(EFortBuildingInitializationReason::PlacementTool, EFortBuildingPersistentState::New);
								//reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker = BuildPreviewMarker;
							}
						}
						/*BuildPreviewMarker->ResourceType = reinterpret_cast<Inventory::CurrentResourceTypeOffsetFix*>(PlayerController)->CurrentResourceType;
						BuildPreviewMarker->OnRep_ResourceType(OldResourceType);
						BuildPreviewMarker->OnRep_MetaData();
						BuildPreviewMarker->OnBuildingActorInitialized(EFortBuildingInitializationReason::PlacementTool, EFortBuildingPersistentState::New);*/
						//Replication::ReplicateToClient(reinterpret_cast<Inventory::BuildPreviewMarkerOffsetFix*>(PlayerController)->BuildPreviewMarker, PlayerController->NetConnection);
					}
				}
			}
		}

		if (FuncName == "ServerHandlePickup") {
			auto Pawn = (AFortPlayerPawn*)Obj;
			auto InParams = (Params::AFortPlayerPawn_ServerHandlePickup_Params*)Params;
			AFortPlayerControllerAthena* PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;

			if (!InParams->Pickup || !InParams->Pickup->PrimaryPickupItemEntry.ItemDefinition) {
				return ProcessEventO(Obj, Func, Params);
			}

			auto ItemEntry = InParams->Pickup->PrimaryPickupItemEntry;

			InParams->Pickup->K2_DestroyActor();

			EFortQuickBars QB = ((ItemEntry.ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) || ItemEntry.ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass())) ? EFortQuickBars::Secondary : EFortQuickBars::Primary);

			Inventory::AddItem(PlayerController, ItemEntry.ItemDefinition, ItemEntry.Count, -1, QB);
		}

		if (FuncName == "ServerSpawnInventoryDrop") {
			AFortPlayerControllerAthena* PlayerController = (AFortPlayerControllerAthena*)Obj;
			auto InParams = (Params::AFortPlayerController_ServerSpawnInventoryDrop_Params*)Params;

			Inventory::DropItem(PlayerController, InParams->ItemGuid);
		}

		//Interacting
		if (FuncName == "ServerAttemptInteract") {
			AFortPlayerControllerAthena* PlayerController = (AFortPlayerControllerAthena*)Obj;
			auto InParams = (Params::AFortPlayerController_ServerAttemptInteract_Params*)Params;

			auto Actor = InParams->ReceivingActor;
			if (Actor) {
				if (Actor->Class->GetName().contains("Tiered_Short_Ammo")) {
					FVector Location = Actor->K2_GetActorLocation();
					//TODO
					Actor->K2_DestroyActor();
					return;
				}

				if (Actor->Class->GetName().contains("Tiered_Chest")) {
					FVector Location = Actor->K2_GetActorLocation();
					Actor->K2_DestroyActor();
					auto ItemDef = UObject::FindObject<UFortWeaponRangedItemDefinition>(Inventory::LootPool[rand() % (Inventory::LootPool.size())]);
					while (!ItemDef) {
						ItemDef = UObject::FindObject<UFortWeaponRangedItemDefinition>(Inventory::LootPool[rand() % (Inventory::LootPool.size())]);
					}
					UFortWorldItem* Item = (UFortWorldItem*)ItemDef->CreateTemporaryItemInstanceBP(1, 1);
					Item->ItemEntry.Count = 1;
					//Item->ItemEntry.LoadedAmmo = Inventory::GetMaxAmmo(ItemDef);
					Inventory::SpawnPickup(Item, Location);

					UFortWeaponRangedItemDefinition* ItemDef2 = UObject::FindObject<UFortWeaponRangedItemDefinition>(Inventory::Consumables[rand() % (Inventory::Consumables.size())]);
					int Count = (ItemDef2->MaxStackSize / 2);

					UFortWorldItem* Item2 = (UFortWorldItem*)ItemDef2->CreateTemporaryItemInstanceBP(Count, 1);
					Item2->ItemEntry.Count = Count;
					Inventory::SpawnPickup(Item2, Location);
					return;
				}
			}
		}

		//Harvesting (No Work)
		/*if (FuncName == "ClientReportDamagedResourceBuilding") {
			AFortPlayerControllerAthena* PlayerController = (AFortPlayerControllerAthena*)Obj;
			auto InParams = (Params::AFortPlayerController_ClientReportDamagedResourceBuilding_Params*)Params;
			static UFortResourceItemDefinition* Wood = UObject::FindObject<UFortResourceItemDefinition>("FortResourceItemDefinition WoodItemData.WoodItemData");
			static UFortResourceItemDefinition* Stone = UObject::FindObject<UFortResourceItemDefinition>("FortResourceItemDefinition StoneItemData.StoneItemData");
			static UFortResourceItemDefinition* Metal = UObject::FindObject<UFortResourceItemDefinition>("FortResourceItemDefinition MetalItemData.MetalItemData");
			UFortResourceItemDefinition* ItemDef = nullptr;
			if (InParams->PotentialResourceType == EFortResourceType::Wood)
				ItemDef = Wood;

			if (InParams->PotentialResourceType == EFortResourceType::Stone)
				ItemDef = Stone;

			if (InParams->PotentialResourceType == EFortResourceType::Metal)
				ItemDef = Metal;

			if (ItemDef != nullptr) {
				Inventory::AddItem(PlayerController, ItemDef, InParams->PotentialResourceCount, 0, EFortQuickBars::Secondary);
			}
		}

		if (FuncName == "OnDamageServer") {
			if (Obj->IsA(ABuildingSMActor::StaticClass())) {
				auto BuildingActor = (ABuildingSMActor*)Obj;
				auto InParams = (Params::ABuildingActor_OnDamageServer_Params*)Params;
				static UFortWeaponMeleeItemDefinition* PicaxeDef = UObject::FindObject<UFortWeaponMeleeItemDefinition>("FortWeaponMeleeItemDefinition WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");
				if (InParams->InstigatedBy && InParams->InstigatedBy->IsA(AFortPlayerController::StaticClass()) && !BuildingActor->bPlayerPlaced) {
					AFortPlayerController* FortController = (AFortPlayerController*)InParams->InstigatedBy;
					if (FortController->MyFortPawn->CurrentWeapon && FortController->MyFortPawn->CurrentWeapon->WeaponData == PicaxeDef)
						FortController->ClientReportDamagedResourceBuilding(BuildingActor, BuildingActor->ResourceType, 6, false, (InParams->Damage == 100.f));
				}
			}
		}*/

		//Misc
		if (FuncName == "ServerCheat" || FuncName == "ServerCheatAll" || FuncName == "CheatAll") {
			return;
		}

		if (FuncName == "ServerLoadingScreenDropped") {
			auto PC = (AFortPlayerControllerAthena*)Obj;
		}

		if (FuncName == "ServerChoosePart") {
			auto InParams = (Params::AFortPlayerPawn_ServerChoosePart_Params*)(Params);
			if (!InParams->ChosenCharacterPart) {
				return;
			}
			else {
				auto Pawn = reinterpret_cast<AFortPlayerPawnAthena*>(Obj);
				bool Allowed = false;
				for (auto Part : GetPlayerParts(reinterpret_cast<AFortPlayerControllerAthena*>(Pawn->Controller))) {
					if (InParams->ChosenCharacterPart == Part) {
						Allowed = true;
						break;
					}
				}
				if (!Allowed) {
					return;
				}
			}
		}

		//Building
		if (FuncName == "ServerCreateBuildingActor") {
			auto InParams = (Params::AFortPlayerController_ServerCreateBuildingActor_Params*)(Params);
			auto Class = InParams->BuildingClassData.BuildingClass;
			auto Loc = InParams->BuildLoc;
			auto Rot = InParams->BuildRot;

			ABuildingSMActor* Build = (ABuildingSMActor*)SpawnActor2(Class, Rot, Loc);

			if (Build) {
				auto PC = (AFortPlayerController*)(Obj);
				Build->bPlayerPlaced = true;
				Build->ForceNetUpdate();
				Build->InitializeKismetSpawnedBuildingActor(Build, PC);
			}
		}

		if (FuncName == "ServerEditBuildingActor") {
			AFortPlayerController* PC = (AFortPlayerController*)(Obj);
			auto InParams = (Params::AFortPlayerController_ServerEditBuildingActor_Params*)(Params);
			if (InParams->BuildingActorToEdit) {
				auto Loc = InParams->BuildingActorToEdit->K2_GetActorLocation();
				auto Rot = InParams->BuildingActorToEdit->K2_GetActorRotation();
				InParams->BuildingActorToEdit->K2_DestroyActor();
				FBuildingClassData Data = {};
				Data.PreviousBuildingLevel = 0;
				Data.UpgradeLevel = 1;
				Data.BuildingClass = InParams->NewBuildingClass;
				PC->ServerCreateBuildingActor(Data, Loc, Rot, InParams->bMirrored);
			}
		}

		if (FuncName == "ServerEndEditingBuildingActor") {
			auto InParams = (Params::AFortPlayerController_ServerEndEditingBuildingActor_Params*)(Params);
			if (InParams->BuildingActorToStopEditing) {
				InParams->BuildingActorToStopEditing->EditingPlayer = nullptr;
				InParams->BuildingActorToStopEditing->OnRep_EditingPlayer();
			}
		}

		if (FuncName == "ServerBeginEditingBuildingActor") {
			AFortPlayerControllerAthena* PC = (AFortPlayerControllerAthena*)(Obj);
			auto InParams = (Params::AFortPlayerController_ServerBeginEditingBuildingActor_Params*)(Params);
			static UFortEditToolItemDefinition* EditToolDef = UObject::FindObject<UFortEditToolItemDefinition>("FortEditToolItemDefinition EditTool.EditTool");
			if (PC && InParams->BuildingActorToEdit) {
				auto EditTool = Inventory::GetItemInInv(PC, EditToolDef);
				auto EditToolData = (AFortWeap_EditingTool*)Inventory::EquipItem(PC, EditTool);
				InParams->BuildingActorToEdit->EditingPlayer = (AFortPlayerStateZone*)PC->PlayerState;
				InParams->BuildingActorToEdit->OnRep_EditingPlayer();
				if (EditToolData) {
					EditToolData->EditActor = InParams->BuildingActorToEdit;
				}
			}
		}

		return ProcessEventO(Obj, Func, Params);
	}

	void Init() {
		AllocConsole();
		FILE* pFile;
		freopen_s(&pFile, ("CONOUT$"), "w", stdout);
		MH_Initialize();
		InitGObjects();
		using namespace Replication;
		FnCreateChannel = decltype(FnCreateChannel)(Base + Offsets::CreateChannel);
		FnSetChannelActor = decltype(FnSetChannelActor)(Base + Offsets::SetChannelActor);
		FnReplicateActor = decltype(FnReplicateActor)(Base + Offsets::ReplicateActor);
		FnCloseChannel = decltype(FnCloseChannel)(Base + Offsets::CreateChannel);
		FnClientSendAdjustment = decltype(FnClientSendAdjustment)(Base + Offsets::SendClientAdjustment);
		FnCallPreReplication = decltype(FnCallPreReplication)(Base + Offsets::CallPreReplication);
		/*std::ofstream log("Objects.txt");
		for (int i = 0; i < UObject::GObjects->Num(); i++) {
			UObject* Object = UObject::GObjects->GetByIndex(i);
			std::string ObjName = Object->GetFullName();
			std::string item = "\nName: " + ObjName;
			log << item;
		}*/

		GGameplayStatics = UObject::FindObjectFast<UGameplayStatics>("Default__GameplayStatics");
		GEngine = UObject::FindObjectFast<UFortEngine>("FortEngine_0");

		GEngine->GameViewport->ViewportConsole = (UConsole*)GGameplayStatics->SpawnObject(UConsole::StaticClass(), GEngine->GameViewport);

		GEngine->GameInstance->LocalPlayers[0]->PlayerController->SwitchLevel(L"Athena_Terrain?game=/game/athena/athena_gamemode.athena_gamemode_C");

		uintptr_t ProcessEventAddr = (Base + Offsets::ProcessEvent);
		CreateHook(ProcessEventAddr, ProcessEvent_Hk, (void**)&ProcessEventO);
		CreateHook(Base + Offsets::NoReservation, Patch, nullptr);
		CreateHook(Base + Offsets::KickPlayer, Patch, nullptr);
		CreateHook(Base + Offsets::ValidationFailure, Patch, nullptr);
		CreateHook(Base + Offsets::CollectGarbage, Patch, nullptr);
		/*CreateHook(Base + Offsets::NoMcp, Patch, nullptr);*/
		//CreateHook(Base + Offsets::GetNetMode, Patch, nullptr);
		CreateHook(Base + Offsets::HandleReloadCost, Hooks::HandleReloadCost, nullptr);
	}
}