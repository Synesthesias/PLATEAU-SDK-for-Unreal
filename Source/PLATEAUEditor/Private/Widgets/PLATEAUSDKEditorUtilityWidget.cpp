// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "PLATEAUEditor/Public/Widgets/PLATEAUSDKEditorUtilityWidget.h"


void UPLATEAUSDKEditorUtilityWidget::AreaSelectSuccessInvoke(const FVector3d& ReferencePoint, const int64& PackageMask) const {
    AreaSelectSuccessDelegate.Broadcast(ReferencePoint, PackageMask);
}
