#pragma once

class PLATEAUCityObject : public UObject {
    TSharedPtr<FJsonObject>& GetRootObject();
    void SetRootObject(const TSharedPtr<FJsonObject>& JsonObject);
private:
    TSharedPtr<FJsonObject> JsonObj;
};
