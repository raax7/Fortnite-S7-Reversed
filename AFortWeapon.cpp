// AFortPawn::StaticClass(): 0x1791180
// AFortWeapon::GetWeaponOwner(): 0x15B2C00
// AFortPlayerController::GetWeaponOwner(): 0x17A2C60
// AFortPlayerController::StopSprinting(): 0x1434530
// this->PlayReloadFX(); // AFortWeapon::PlayReloadFX(): 0x40CEA48
// UAbilitySystemComponent::TryActivateAbility(): 0x6B3F80
// AFortWeapon::HasStatsAndRole(): 0x15B4DC0
// AFortWeapon::IsTriggerTypeAutomatic(): 0x15B4D40
// AFortWeapon::CancelWeaponCharge(): 0x15A9F10

bool FFortBaseWeaponStats::IsChargedWeapon()
{
    return this->MaxChargeTime > 0.0f;
}

void AFortPlayerController::StopSprinting()
{
    AFortPlayerPawn* MyFortPawn;

    if (!this->bSprintByDefault && (!this->bHoldingSprint || (MyFortPawn = this->MyFortPawn) != false && (MyFortPawn->UnkwonBitField1 & 1) != 0))
    {
        this->bWantsToSprint = false;
    }
}

UObject* AFortWeapon::GetWeaponOwner()
{
    AController* Controller = nullptr;

    APawn* Instigator = this->Instigator;
    if (Instigator)
    {
        Controller = Instigator->Controller;

        if (Controller)
        {
            return Controller;
        }

        if (Instigator->IsA(AFortPlayerPawn::StaticClass()))
        {
            AFortPlayerPawn* FortInstigator = static_cast<AFortPlayerPawn*>(Instigator);

            if (FortInstigator)
            {
                if (FortInstigator->IsA(AFortPlayerPawn::StaticClass()))
                {
                    APawn* ControllerAsPawn = FWeakObjectPtr::Get(FortInstigator->ControlledRCPawn)->OldPawn;

                    if (ControllerAsPawn)
                    {
                        return ControllerAsPawn;
                    }
                }
            }
        }
    }

    if (this->Owner)
    {
        Controller = this->Owner;

        if (Controller->IsA(AController::StaticClass()))
        {
            return Controller;
        }

        Controller = nullptr;
    }

    UWorld* World = this->GetWorld();
    if (!World)
    {
        return Controller;
    }

    AController* ControllerFromWorld = this->GetLocalControllerFromWorld(World);
    if (!ControllerFromWorld->IsA(FortLiveBroadcastController::StaticClass()))
    {
        return Controller;
    }

    if (World->IsA(FortLiveBroadcastController::StaticClass()))
    {
        return Controller;
    }

    bool DoingQuickbarAction = AFortPlayerController::GetLocalOrQuickbarActions(World, World->Class) == this->Instigator;
    if (!DoingQuickbarAction)
    {
        return Controller;
    }

    return ControllerFromWorld;
}
bool AFortWeapon::IsChargedWeapon()
{
    FFortBaseWeaponStats* WeaponStats = this->GetWeaponStats();

    return this->GetWorld() && WeaponStats && WeaponStats->IsChargedWeapon();
}
void AFortWeapon::Reload()
{
    APawn* Instigator = this->Instigator;
    if (!Instigator->IsA(AFortPawn::StaticClass()))
    {
        Instigator = nullptr;
    }

    if (this->ReloadAbilitySpecHandle.Handle != -1)
    {
        UObject* WeaponOwner = this->GetWeaponOwner();

        if (WeaponOwner)
        {
            if (WeaponOwner->IsA(AFortPlayerController::StaticClass()))
            {
                static_cast<AFortPlayerController*>(WeaponOwner)->StopSprinting();
            }
        }

        this->LastFireTime = this->GetWorld()->DeltaTimeSeconds;

        this->CancelActiveAbility();

        if (Instigator)
        {
            UAbilitySystemComponent* PrimaryAbilitySystemComponent = static_cast<AFortPawn*>(Instigator)->PrimaryAbility;

            if (PrimaryAbilitySystemComponent)
            {
                bool PrimaryAbilitySuccess = PrimaryAbilitySystemComponent->TryActivateAbility(this->ReloadAbilitySpecHandle.Handle, true);

                if (PrimaryAbilitySuccess && this->IsChargedWeapon() && this->IsTriggerTypeAutomatic())
                {
                    this->WeaponStateFlags |= 0x40u; // something to do with charging
                    this->CancelWeaponCharge();
                }
            }
        }
    }
}
