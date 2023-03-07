# パッケージ化
リリースページへ追加するパッケージの作成方法を記載する。

1. UE5のメニューから「編集」→「Plugins」を開く。
2. PLATEAU SDK for Unrealを選択してパッケージ化を行う。
3. 出力されたフォルダの`PLATEAU-SDK-for-Unreal/HostProject/Plugins/PLATEAU-SDK-for-Unreal/PLATEAU-SDK-for-Unreal.uplugin`の内、Modulesを以下のように変更する。
```
	"Modules": [
		{
			"Name": "PLATEAURuntime",
			"Type": "Runtime",
			"LoadingPhase": "Default",
            "WhitelistPlatforms" : ["Win64", "Win32"]
		},
		{
			"Name": "PLATEAUEditor",
			"Type": "Editor",
			"LoadingPhase": "Default",
			"WhitelistPlatforms" : ["Win64", "Win32"]
		}
	]
```

4. `PLATEAU-SDK-for-Unreal/HostProject/Plugins/PLATEAU-SDK-for-Unreal`をzipファイルに圧縮する。
