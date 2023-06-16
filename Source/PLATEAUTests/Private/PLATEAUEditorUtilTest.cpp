// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUEditorUtilTest.h"
#include "PLATEAUEditorUtil.h"
#include "HAL/FileManagerGeneric.h"


TArray<int64> UPLATEAUEditorUtilTest::GetAllPackages() {
    return UPLATEAUEditorUtil::GetAllPackages();
}

bool UPLATEAUEditorUtilTest::MakeDirectory(const FString& Path, const bool CreateTree) {
    if (!FPaths::DirectoryExists(Path)) {
        return FFileManagerGeneric::Get().MakeDirectory(*Path, CreateTree);
    }

    return true;
}

bool UPLATEAUEditorUtilTest::DeleteFile(const FString& Path) {
    if (FPaths::FileExists(Path)) {
        return FFileManagerGeneric::Get().Delete(*Path, true, true);
    }

    return true;
}

bool UPLATEAUEditorUtilTest::DeleteDirectory(const FString& Path) {
    if (FPaths::DirectoryExists(Path)) {
        return FFileManagerGeneric::Get().DeleteDirectory(*Path, true, true);
    }

    return true;
}

TArray<FString> UPLATEAUEditorUtilTest::FindFiles(const FString& Path, const FString& Filter) {
    TArray<FString> FoundFileArray;
    FoundFileArray.Empty();

    if (FPaths::DirectoryExists(Path)) {
        if (0 < Filter.Len()) {
            if (Path.Right(1) == "/") {
                FFileManagerGeneric::Get().FindFiles(FoundFileArray, *(Path + Filter), true, false);
            } else {
                FFileManagerGeneric::Get().FindFiles(FoundFileArray, *(Path + "/" + Filter), true, false);
            }
        }
        FFileManagerGeneric::Get().FindFiles(FoundFileArray, *Path, true, false);
    }

    return FoundFileArray;
}

bool UPLATEAUEditorUtilTest::WriteToFile(const FString& Path, const FString& Text) {
    const FString& DirectoryPath = FPaths::GetPath(Path);
    if (!FPaths::DirectoryExists(DirectoryPath)) {
        FFileManagerGeneric::Get().MakeDirectory(*DirectoryPath, true);
    }

    return FFileHelper::SaveStringToFile(Text, *(DirectoryPath + "/" + FPaths::GetBaseFilename(Path) + ".txt"));
}
