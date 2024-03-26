void AFortPlayerPawn::BeginZiplining(class AFortAthenaZipline* Zipline, struct USceneComponent* SocketComponent, class FName SocketName, const struct FVector* SocketOffset) {
    __m128 v12; // xmm1
    __m128 v13; // xmm2
    float v14; // xmm1_4
    float v15; // xmm0_4
    void *v21; // [rsp+30h] [rbp-28h] BYREF
    float v22; // [rsp+38h] [rbp-20h]
    float v24; // [rsp+48h] [rbp-10h]

    if (this->Role == ROLE_AutonomousProxy && Zipline && Zipline->bInitialized) {
        this->ZiplineState.AuthoritativeValue++;

        this->ZiplineState.Zipline = Zipline;
        this->ZiplineState.bIsZiplining = true;
        this->ZiplineState.TimeZipliningBegan = this->GetWorld()->GetDeltaTimeSeconds();
        this->ZiplineState.SocketOffset = *SocketOffset;

        if (SocketComponent && (SocketName.ComparisonIndex || SocketName.Number)) {
            FVector SocketLocation = SocketComponent->GetSocketLocation(SocketName);

            // not too sure on all this below so ill leave it as is
            if (this->RootComponent) {
                v12 = *((__m128 *)this->RootComponent + 26);
                LODWORD(v21) = v12.m128_i32[0];
                LODWORD(v22) = _mm_shuffle_ps(v12, v12, 170).m128_u32[0];
                HIDWORD(v21) = _mm_shuffle_ps(v12, v12, 85).m128_u32[0];
            }
            else
            {
                v21 = *(void **)&byte_5A5B808;
                v22 = *((float *)&byte_5A5B808 + 2);
            }

            v13 = (__m128)(unsigned __int64)v21;
            this->ZiplineState.SocketOffset.X = (float)(SocketLocation.X - *(float *)&v21) + this->ZiplineState.SocketOffset.X;
            v15 = (float)(SocketLocation.Z - v22) + this->ZiplineState.SocketOffset.Z;
            this->ZiplineState.SocketOffset.Y = (float)(SocketLocation.Y - _mm_shuffle_ps(v13, v13, 85).m128_f32[0])
                                                + this->ZiplineState.SocketOffset.Y;
            this->ZiplineState.SocketOffset.Z = v15;
        }

        // no idea why its the wrong func name, its clearly for beginning ziplining. it must be a mistake
        UE_LOG(LogTemp, Log, TEXT(L"ZIPLINES!! Role(%s)  AFortPlayerPawn::ServerEndZiplining_Implementation bFromJump=%d"), this->Role, off_5A15E9C);

        AFortPlayerPawn::OnRep_ZiplineState(this);
        AFortPlayerPawn::ServerSendZiplineState(&this->ZiplineState);
    }
}

// also known as AFortPlayerPawn::ServerEndZiplining_Implementation
void AFortPlayerPawn::EndZiplining(bool bFromJump) {
    if (this->Role == ROLE_AutonomousProxy && this->ZiplineState.bIsZiplining) {
        if (bFromJump) {
            float ZiplineJumpActivateDelayValue = FScalableFloat::GetValueAtLevel(&this->ZiplineJumpActivateDelay, bJumped, 0);
            float TimeSinceZiplineBegan = this->GetWorld()->GetDeltaTimeSeconds() - this->ZiplineState.TimeZipliningBegan;

            if (TimeSinceZiplineBegan < ZiplineJumpActivateDelayValue) {
                return;
            }

            FString ContextString = L"AFortPlayerPawn::ForceLaunchPlayerZiplining";
            FVector IDK = FVector();

            float v18 = 0.0f;

            FCurveTableRowHandle::Eval(&this->ZiplineJumpDampening, 0.0f, &IDK.X, &ContextString);
            FCurveTableRowHandle::Eval(&this->ZiplineJumpStrength, 0.0f, &v18, &ContextString);

            // some VFT call in RootComponent
            (*((void (__fastcall **)(AFortPlayerPawn *, float *))this->Vft + 0x59))(this, &v13);

            if (IDK.X >= -750.0) {
                IDK.X = fminf(IDK.X, 750.0);
            }
            else {
                IDK.X = -750.0;
            }

            if (IDK.X >= -750.0) {
                IDK.Y = fminf(IDK.X, 750.0);
            }
            else {
                IDK.Y = -750.0;
            }

            // hilarious IDA moment. try to guess real type (level impossible)
            // maybe fvetor2d but still pseudocode looks wrong with that
            ContextString.Data = *(void **)&IDK.X;
            ContextString.NumElements = (int)v18;

            // some function related to AFortPlayerController
            sub_13A3F50((__int64 *)this, (__int64)&ContextString, 0, v15 < 0.0, 0, 1);
        }

        UE_LOG(LogTemp, Log, TEXT(L"ZIPLINES!! Role(%s)  AFortPlayerPawn::ServerEndZiplining_Implementation   ZiplineState.bIsZiplining = false"), this->Role);

        this->ZiplineState.Zipline = nullptr;
        this->ZiplineState.bIsZiplining = false;
        this->ZiplineState.bJumped = bFromJump;
        if (bFromJump) {
            this->ZiplineState.TimeZipliningEndedFromJump = this->GetWorld()->GetDeltaTimeSeconds();
        }
        else {
            this->ZiplineState.TimeZipliningEndedFromJump = -1.0f;
        }
        this->ZiplineState.AuthoritativeValue++;
        this->ZiplineState.TimeZipliningEndedFromJump = v11;

        this->OnRep_ZiplineState();
        this->ServerSendZiplineState(&this->ZiplineState);
    }
}