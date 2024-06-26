// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUEditorUtil.h"
#include "Interfaces/IMainFrameModule.h"
#include "DesktopPlatformModule.h"
#include "PLATEAUImportSettings.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidget.h"
#include "Modules/ModuleManager.h"
#include "ISettingsModule.h"
#include "ISettingsContainer.h"
#include "ISettingsCategory.h"
#include "ISettingsSection.h"

#define LOCTEXT_NAMESPACE "PLATEAUEditorUtil"

static constexpr TCHAR OpenDirectoryDialogTitle[] = TEXT("フォルダ選択");

/**
 * @brief OSのウィンドウハンドルを他のAPIのためにvoidポインタの形で取得する
 * @return OSのウィンドウハンドル
 */
void* UPLATEAUEditorUtil::GetWindowHandle() {
    const IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
    const TSharedPtr<SWindow> MainWindow = MainFrameModule.GetParentWindow();

    if (MainWindow.IsValid() && MainWindow->GetNativeWindow().IsValid()) {
        return MainWindow->GetNativeWindow()->GetOSWindowHandle();
    }

    return nullptr;
}

/**
 * @brief フォルダ選択ダイアログ表示
 * @param SourcePath 選択された入力フォルダパス
 * @return 入力フォルダが選択されたか？
 */
bool UPLATEAUEditorUtil::OpenDirectoryDialog(FString& SourcePath) {
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (FString OutFolderName; DesktopPlatform->OpenDirectoryDialog(GetWindowHandle(), OpenDirectoryDialogTitle, "", OutFolderName)) {
        try {
            SourcePath = OutFolderName;
            return true;
        } catch (...) {
            UE_LOG(LogTemp, Error, TEXT("Invalid source path : %s"), *OutFolderName);
            return false;
        }
    }
    return false;
}

TMap<UEditorUtilityWidget*, FName> UPLATEAUEditorUtil::RunEditorUtilityWidget(UEditorUtilityWidgetBlueprint* EditorWidget) {
    auto IDMap = TMap<UEditorUtilityWidget*, FName>();
    if (EditorWidget) {
        UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
        FName TabID;
        UEditorUtilityWidget* Widget = EditorUtilitySubsystem->SpawnAndRegisterTabAndGetID(EditorWidget, TabID);
        IDMap.Emplace(Widget, TabID);
    }
    return IDMap;
}

bool UPLATEAUEditorUtil::CloseEditorUtilityWidgetTab(UEditorUtilityWidget* Widget) {
    //EditorUtilitySubsystemのTabID生成と同様
    FName TabID = FName(*(Widget->GetClass()->GetClassPathName().ToString().LeftChop(2) + LOCTEXT("ActiveTabSuffix", "_ActiveTab").ToString()));
    return CloseEditorUtilityWidgetTabByID(TabID);
}

bool UPLATEAUEditorUtil::CloseEditorUtilityWidgetTabByID(FName TabID){
    UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
    return EditorUtilitySubsystem->CloseTabByID(TabID);
}

void UPLATEAUEditorUtil::SelectComponent(UActorComponent* Component) {
    GEditor->SelectComponent(Component, true, true);
}

//Editr Preferences内のObjectMixerのSyncSelectionフラグをON/OFFします
void UPLATEAUEditorUtil::SetObjectMixerSyncSelection(bool bSync) {

    FName CategoryName(TEXT("Plugins"));
    FName SectionName(TEXT("Object Mixer"));
    FName PropertyName(TEXT("bSyncSelection"));
    if (auto SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
        if (auto SettingsContainer = SettingsModule->GetContainer("Editor")) {
            TArray<ISettingsCategoryPtr> Categories;
            int32 CatNum = SettingsContainer->GetCategories(Categories);
            if (CatNum > 0) {
                for(auto Category : Categories){
                    if (Category->GetName() == CategoryName) {
                        auto Section = Category->GetSection(SectionName);
                        if (Section) {
                            auto Setting = Section->GetSettingsObject();
                            if (Setting.IsValid()) {
                                FProperty* Property = Setting->GetClass()->FindPropertyByName(PropertyName);
                                if (Property) {
                                    if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property)) {
                                        BoolProperty->SetPropertyValue_InContainer(Setting.Get(), bSync);
                                    }
                                }

                                if (Section.IsValid()) {
                                    Section->Save();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE