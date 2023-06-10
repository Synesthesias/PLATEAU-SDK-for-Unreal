// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUEditorUtilTest.h"
#include "PLATEAUEditorUtil.h"
#include "HAL/FileManagerGeneric.h"


TArray<int64> UPLATEAUEditorUtilTest::GetAllPackages() {
    return UPLATEAUEditorUtil::GetAllPackages();
}

bool UPLATEAUEditorUtilTest::MakeDirectory(const FString& Path, const bool CreateTree) {
    if (!FPaths::DirectoryExists(Path)) {
        FFileManagerGeneric::Get().MakeDirectory(*Path, CreateTree);
        return true;
    }

    return false;
}

bool UPLATEAUEditorUtilTest::DeleteDirectory(const FString& Path) {
    if (FPaths::DirectoryExists(Path)) {
        FFileManagerGeneric::Get().DeleteDirectory(*Path, true, true);
        return true;
    }

    return false;    
}

TArray<FString> UPLATEAUEditorUtilTest::FindFiles(const FString& Path) {
    TArray<FString> OutputArray;
    OutputArray.Empty();
    
    if (FPaths::DirectoryExists(Path)) {
        UE_LOG(LogTemp, Log, TEXT("Path:%s"), *(Path + "/*"));

        FFileManagerGeneric::Get().FindFiles(OutputArray, *(Path + "/*"), true, false);
    }
    
    return OutputArray;
}
