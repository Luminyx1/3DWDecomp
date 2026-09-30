#pragma once

#include "Library/Execute/ExecuteOrder.hpp"

namespace al {
namespace {
const ExecuteOrder sUpdateTable[] = {
    {"OceanWaterRequest", "Execute", 1, "システム", "Ocean Water Update Request"},
    {"TimerManager", "Execute", 1, "システム", "Timer Management"},
    {"ライト管理", "Execute", 2, "システム", "LightDirector"},
    {"Clipping", "Execute", 1, "システム", "ClippingDirector"},
    {"ModelUpdate", "ActorModelUpdate", 1024, "システム", "ModelUpdate"},
    {"センサー", "Execute", 1, "システム", "Hit Sensor Director"},
    {"ポインティング", "Execute", 8, "システム", "pointer"},
    {"DRCアシスト", "ActorMovementCalcAnim", 2, "システム", "DRCassist"},
    {"DRCアシスト同期グループ", "ActorMovement", 1, "システム", "DRCassist"},
    {"空", "ActorMovementCalcAnim", 8, "地形", "DRCassistSyncGroup"},
    {"遠景", "ActorMovementCalcAnim", 8, "地形", "DistantView"},
    {"コリジョン地形", "ActorMovementCalcAnim", 256, "地形", "Terrain"},
    {"コリジョン地形[Movement]", "ActorMovement", 128, "地形", "Terrain[Movement]"},
    {"コリジョン地形装飾", "ActorMovementCalcAnim", 32, "地形", "TerrainDecoration"},
    {"コリジョン地形装飾[Movement]", "ActorMovement", 32, "地形", "TerrainDecoration[Movement]"},
    {"コリジョン地形オブジェ", "ActorMovementCalcAnim", 64, "地形オブジェ", "TerrainObj"},
    {"コリジョン地形オブジェ[Movement]", "ActorMovement", 32, "地形オブジェ", "TerrainObj[Movement]"},
    {"コリジョンアイテム", "ActorMovementCalcAnim", 32, "アイテム", "CollisionItem"},
    {"コリジョンディレクター", "Execute", 1, "システム", "Collision Director"},
    {"地形オブジェ", "ActorMovementCalcAnim", 128, "地形オブジェ", nullptr},
    {"地形オブジェ[Movement]", "ActorMovement", 32, "地形オブジェ", nullptr},
    {"EnemyMapObj[Movement]", "ActorMovement", 32, "地形オブジェ", nullptr},
    {"エフェクトオブジェ", "ActorMovement", 32, "エフェクト", nullptr},
    {"地形オブジェ装飾", "ActorMovementCalcAnim", 32, "地形オブジェ", nullptr},
    {"乗り物", "ActorMovementCalcAnim", 32, "乗り物", nullptr},
    {"乗り物[Movement]", "ActorMovement", 32, "乗り物", nullptr},
    {"乗り物装飾", "ActorMovementCalcAnim", 32, "乗り物", nullptr},
    {"デモプレイヤーロケーター", "ActorMovementCalcAnim", 4, "プレイヤー", nullptr},
    {"デモプレイヤー前処理", "ActorMovement", 16, "プレイヤー", nullptr},
    {"プレイヤー前処理", "Functor", 1, "プレイヤー", nullptr},
    {"プレイヤー[Movement]", "ActorMovement", 1, "プレイヤー", nullptr},
    {"プレイヤー", "ActorMovementCalcAnim", 64, "プレイヤー", nullptr},
    {"プレイヤー後処理", "Functor", 1, "プレイヤー", nullptr},
    {"プレイヤー装飾", "ActorMovementCalcAnim", 64, "プレイヤー", nullptr},
    {"プレイヤー装飾２", "ActorMovementCalcAnim", 64, "プレイヤー", nullptr},
    {"ゴーストプレイヤー記録", "Functor", 1, "敵", nullptr},
    {"敵", "ActorMovementCalcAnim", 128, "敵", nullptr},
    {"敵[Movement]", "ActorMovement", 32, "敵", nullptr},
    {"敵装飾", "ActorMovementCalcAnim", 32, "敵", nullptr},
    {"敵装飾[Movement]", "ActorMovement", 32, "敵", nullptr},
    {"デモ管理者", "Execute", 64, "地形", nullptr},
    {"デモ", "ActorMovementCalcAnim", 64, "地形", nullptr},
    {"デモオブジェクト", "ActorMovement", 32, "地形", nullptr},
    {"ＮＰＣ", "ActorMovementCalcAnim", 32, "ＮＰＣ", nullptr},
    {"ＮＰＣ装飾", "ActorMovementCalcAnim", 32, "ＮＰＣ", nullptr},
    {"コインローテータ", "Execute", 1, "アイテム", nullptr},
    {"ゴーストディレクター", "Execute", 1, "プレイヤー", nullptr},
    {"エコーエミッター管理", "Execute", 1, "地形オブジェ", nullptr},
    {"アイテム", "ActorMovementCalcAnim", 64, "アイテム", nullptr},
    {"アイテム装飾", "ActorMovementCalcAnim", 32, "アイテム", nullptr},
    {"シャドウマスク", "ActorMovement", 64, "影", nullptr},
    {"グラフィックス要求者", "ActorMovement", 64, "地形オブジェ", nullptr},
    {"監視オブジェ", "ActorMovement", 32, "地形オブジェ", nullptr},
    {"フォグディレクター", "Execute", 1, "システム", "Fog Director"},
    {"ステージスイッチディレクター", "Execute", 1, "システム", "Stage Switch Director"},
    {"カメラ振動", "Functor", 1, "システム", nullptr},
    {"２Ｄ", "LayoutUpdate", 256, "レイアウト", "2D"},
    {"2D StopScene", "LayoutUpdate", 8, "レイアウト", "2D"},
    {"2D StallScene", "LayoutUpdate", 4, "レイアウト", "2DAboveBlur2"},
    {"２Ｄ（ポーズ無視）", "LayoutUpdate", 96, "レイアウト", "2D(IgnorePose)"},
    {"エフェクト（前処理）", "Functor", 1, "エフェクト", nullptr},
    {"エフェクト（３Ｄ）", "Execute", 1, "エフェクト", nullptr},
    {"エフェクト（プレイヤー）", "Execute", 1, "エフェクト", nullptr},
    {"Effect (HitStop)", "Execute", 1, "エフェクト", nullptr},
    {"エフェクト（カメラデモ）", "Execute", 1, "エフェクト", nullptr},
    {"エフェクト（ベース２Ｄ）", "Execute", 1, "エフェクト", nullptr},
    {"エフェクト（２Ｄ）", "Execute", 1, "エフェクト", nullptr},
    {"エフェクト（下画面ベース２Ｄ）", "Execute", 1, "エフェクト", nullptr},
    {"エフェクト（下画面２Ｄ）", "Execute", 1, "エフェクト", nullptr},
    {"Effect (2DAboveBlur)", "Execute", 1, "エフェクト", nullptr},
    {"エフェクト（後処理）", "Functor", 1, "エフェクト", nullptr},
    {"Stamp", "ActorMovementCalcAnim", 2, "システム", nullptr},
    {"Water", "ActorMovement", 1, "地形オブジェ", "Water Movement"},
    {"HitSensorThreadUpdate", "Execute", 1, "システム", "Hit Sensor Director MultiThreaded"},
    {"ClippingDirectorRequest", "Execute", 1, "システム", "Clipping Director Request"},
};

const ExecuteOrder sDrawTable0[] = {
    {"デプスシャドウ[キャラクター]", "ActorModelDrawDepthShadow", 256, "影", nullptr},
    {"デプスシャドウ[地形]", "ActorModelDrawDepthShadow", 256, "影", nullptr},
};

const ExecuteOrder sDrawTable1[] = {
    {"プレイヤー", "ActorModelDrawDepthShadow", 256, "影", nullptr},
    {"Mii[顔モデル](デプスシャドウ)", "Draw", 8, "影", nullptr},
};

const ExecuteOrder sDrawTable2[] = {
    {"デプスシャドウ[影のみ]", "ActorModelDrawDepthShadow", 32, "影", nullptr},
    {"ＮＰＣ", "ActorModelDrawDepthShadow", 128, "影", nullptr},
    {"アイテム", "ActorModelDrawDepthShadow", 128, "影", nullptr},
    {"敵", "ActorModelDrawDepthShadow", 128, "影", nullptr},
    {"地形オブジェ[地形前]", "ActorModelDrawDepthShadow", 64, "影", nullptr},
    {"地形オブジェ", "ActorModelDrawDepthShadow", 128, "影", nullptr},
    {"地形オブジェ目立たせ", "ActorModelDrawDepthShadow", 128, "影", nullptr},
    {"地形オブジェ目立たせ[キャラ後]", "ActorModelDrawDepthShadow", 32, "影", nullptr},
    {"アイテム[フォワード]", "ActorModelDrawDepthShadow", 32, "影", nullptr},
};

const ExecuteOrder sDrawTable3[] = {
    {"空", "ActorModelDraw", 8, "地形", nullptr},
};

const ExecuteOrder sDrawTable4[] = {
    {"撮影用[地形]", "ActorModelDrawCubeMap", 128, "地形", nullptr},
    {"撮影用[空]", "ActorModelDraw", 128, "地形オブジェ", nullptr},
};

const ExecuteOrder sDrawTable5[] = {
    {"Ｚプリパス[キャラクター]", "ActorModelDrawDepthOnly", 128, "敵", nullptr},
    {"Ｚプリパス[地形]", "ActorModelDrawDepthOnly", 128, "地形", nullptr},
    {"ZPrePass[FarLOD]", "ActorModelDrawDepthOnly", 128, "地形", nullptr},
    {"敵目立たせ[地形前]", "ActorModelDrawDeferredCharacter", 16, "敵", nullptr},
    {"地形オブジェ目立たせ[地形前]", "ActorModelDrawDeferredCharacter", 128, "地形オブジェ", nullptr},
    {"地形オブジェ目立たせ[地形前][不透明]", "ActorModelDrawDeferredCharacterOpa", 32, "地形オブジェ", nullptr},
    {"地形オブジェ[地形前]", "ActorModelDrawDeferred", 64, "地形オブジェ", nullptr},
    {"地形[浮遊]", "ActorModelDrawDeferred", 32, "地形", nullptr},
    {"地形", "ActorModelDrawDeferred", 512, "地形", nullptr},
    {"地形[LOD]", "ActorModelDrawDeferred", 128, "LOD", nullptr},
    {"地形[不透明]", "ActorModelDrawDeferredOpa", 32, "地形", nullptr},
    {"地形[エコー]", "ActorModelDrawDeferredEcho", 128, "地形", nullptr},
    {"シルエット[乗り物]", "ActorModelDrawDeferredSilhouetteRide", 16, "プレイヤー", nullptr},
    {"地形[埋没]", "ActorModelDrawDeferred", 32, "地形", nullptr},
    {"RaidonSurfDeferred", "ActorModelDrawDeferredSilhouetteRide", 16, "プレイヤー", nullptr},
};

const ExecuteOrder sDrawTable6[] = {
    {"Stamp", "ActorModelDrawDeferredSSD", 8, "アイテム", nullptr},
    {"シャドウマスク[地形オブジェ]", "Draw", 3, "地形オブジェ", nullptr},
    {"シャドウマスク[地形と水オブジェ]", "Draw", 3, "地形と水オブジェ", nullptr},
    {"エコーブロック", "ActorModelDrawDeferredEcho", 32, "地形", nullptr},
    {"足跡", "ActorModelDrawDeferredFootPrint", 4, "地形オブジェ", nullptr},
    {"地形オブジェ目立たせ", "ActorModelDrawDeferredCharacter", 128, "地形オブジェ", nullptr},
    {"地形オブジェ", "ActorModelDrawDeferred", 128, "地形オブジェ", nullptr},
    {"地形キャラクター", "ActorModelDrawDeferredCharacter", 32, "敵", nullptr},
    {"シルエット[プレイヤー]", "ActorModelDrawDeferredSilhouette", 256, "プレイヤー", nullptr},
    {"シルエット[お化け]", "ActorModelDrawDeferredCharacter", 64, "敵", nullptr},
    {"シャドウマスク[敵]", "Draw", 3, "敵", nullptr},
    {"敵", "ActorModelDrawDeferredCharacter", 128, "敵", nullptr},
    {"ＮＰＣ", "ActorModelDrawDeferredCharacter", 32, "ＮＰＣ", nullptr},
    {"Mii[顔モデル]", "Draw", 4, "システム", nullptr},
    {"シャドウマスク[アイテム]", "Draw", 3, "地形オブジェ", nullptr},
    {"アイテム", "ActorModelDrawDeferredCharacter", 64, "アイテム", nullptr},
    {"シャドウマスク[プレイヤー]", "Draw", 4, "プレイヤー", nullptr},
    {"プレイヤー", "ActorModelDrawDeferredPlayer", 256, "プレイヤー", nullptr},
    {"無敵[プレイヤー]", "ActorModelDrawInvincible", 128, "プレイヤー", nullptr},
    {"地形オブジェ目立たせ[キャラ後]", "ActorModelDrawDeferredCharacter", 32, "地形オブジェ", nullptr},
    {"地形オブジェ目立たせ[キャラ後][不透明]", "ActorModelDrawDeferredCharacterOpa", 32, "地形オブジェ", nullptr},
    {"地形オブジェ[キャラ後]", "ActorModelDrawDeferred", 32, "地形オブジェ", nullptr},
    {"シャドウマスク[ブロック]", "Draw", 3, "地形オブジェ", nullptr},
    {"中景", "ActorModelDrawDeferred", 8, "地形", nullptr},
    {"中景[不透明]", "ActorModelDrawDeferredOpa", 8, "地形", nullptr},
    {"ディファード空", "ActorModelDrawDeferredSky", 8, "地形", nullptr},
    {"ディファード空[デモ]", "ActorModelDrawDeferredSky", 8, "地形", nullptr},
    {"ディファード半透明目立たせ", "ActorModelDrawDeferredCharacter", 16, "地形オブジェ", nullptr},
    {"シャドウマスク[モデルライト後]", "Draw", 4, "プレイヤー", nullptr},
    {"アクター描画", "ActorDraw", 8, "敵", nullptr},
};

const ExecuteOrder sDrawTable7[] = {
    {"遠景[ライトバッファ]", "ActorModelDrawXlu", 8, "地形", nullptr},
    {"大気散乱雲[ライトバッファ]", "ActorModelDrawXlu", 8, "地形", nullptr},
};

const ExecuteOrder sDrawTable8[] = {
    {"WaterDeferred2", "ActorDraw", 1, "地形", nullptr},
};

const ExecuteOrder sDrawTable9[] = {
    {"プレイヤー", "ActorModelDrawDepthForce", 256, "プレイヤー", nullptr},
    {"GoalItem[ForceForward]", "ActorModelDrawDepthForceOpt", 32, "アイテム", nullptr},
    {"プレイヤー", "ActorModelDrawCharacter", 256, "プレイヤー", nullptr},
    {"GoalItem[ForceForward]", "ActorModelDrawCharacterOpt", 32, "アイテム", nullptr},
};

const ExecuteOrder sDrawTable10[] = {
    {"Zマスク", "ActorModelDrawDepthOnly", 8, "地形", nullptr},
    {"地形[半透明]", "ActorModelDrawXlu", 32, "地形", nullptr},
    {"地形オブジェ[フォワード]", "ActorModelDraw", 32, "地形オブジェ", nullptr},
    {"地形オブジェ目立たせ[フォワード]", "ActorModelDrawCharacter", 32, "地形オブジェ", nullptr},
    {"敵[フォワード]", "ActorModelDrawCharacter", 32, "敵", nullptr},
    {"アイテム[フォワード]", "ActorModelDrawCharacter", 32, "アイテム", nullptr},
    {"中景[フォワード]", "ActorModelDraw", 8, "地形オブジェ", nullptr},
    {"中景[半透明]", "ActorModelDrawXlu", 8, "地形", nullptr},
    {"アクター描画[フォワード]", "ActorDraw", 32, "敵", nullptr},
};

const ExecuteOrder sDrawTable11[] = {
    {"WaterIndirect", "ActorDraw", 1, "敵", nullptr},
    {"WaterDeferred", "ActorDraw", 1, "敵", nullptr},
};

const ExecuteOrder sDrawTable12[] = {
    {"シャドウマスク[敵]", "Draw", 3, "敵", nullptr},
    {"シャドウマスク[プレイヤー]", "Draw", 4, "プレイヤー", nullptr},
    {"シャドウマスク[アイテム]", "Draw", 3, "地形オブジェ", nullptr},
    {"シャドウマスク[ブロック]", "Draw", 3, "地形オブジェ", nullptr},
    {"シャドウマスク[地形と水オブジェ]", "Draw", 3, "地形と水オブジェ", nullptr},
};

const ExecuteOrder sDrawTable13[] = {
    {"ルート土管内側[インダイレクト]", "ActorModelDrawCharacter", 32, "地形オブジェ", nullptr},
    {"ルート土管外側[インダイレクト]", "ActorModelDrawCharacter", 32, "地形オブジェ", nullptr},
    {"PostIndirect", "ActorModelDrawCharacter", 32, "地形オブジェ", nullptr},
    {"PostIndirectLandformXlu", "ActorModelDrawXlu", 32, "地形オブジェ", nullptr},
    {"地形オブジェ[インダイレクト]", "ActorModelDraw", 32, "地形オブジェ", nullptr},
    {"地形オブジェ[インダイレクト]", "ActorModelDraw", 32, "地形オブジェ", nullptr},
    {"地形オブジェ目立たせ[インダイレクト]", "ActorModelDrawCharacter", 32, "地形オブジェ", nullptr},
    {"地形オブジェ目立たせ[地形前][インダイレクト]", "ActorModelDrawCharacterXlu", 32, "地形オブジェ", nullptr},
    {"敵[インダイレクト]", "ActorModelDrawCharacter", 32, "敵", nullptr},
    {"アイテム[インダイレクト]", "ActorModelDrawCharacter", 32, "アイテム", nullptr},
    {"GoalItem[Forward]", "ActorModelDrawCharacter", 32, "アイテム", nullptr},
};

const ExecuteOrder sDrawTable14[] = {
    {"ActorModel[PreIndirect]", "ActorModelDraw", 32, "アイテム", nullptr},
};

const ExecuteOrder sDrawTable15[] = {
    {"２Ｄベース", "LayoutDraw", 64, "レイアウト", nullptr},
    {"２Ｄ", "LayoutDraw", 320, "レイアウト", nullptr},
    {"２Ｄベースエフェクト", "Draw", 1, "エフェクト", nullptr},
    {"２Ｄガイド", "LayoutDraw", 16, "レイアウト", nullptr},
    {"２Ｄヘッド", "LayoutDraw", 32, "レイアウト", nullptr},
    {"２Ｄワイプ", "LayoutDraw", 32, "レイアウト", nullptr},
    {"２Ｄポーズ", "LayoutDraw", 8, "レイアウト", nullptr},
    {"２Ｄランキング", "LayoutDraw", 4, "レイアウト", nullptr},
    {"２Ｄリザルト", "LayoutDraw", 4, "レイアウト", nullptr},
    {"２Ｄゲームオーバー", "LayoutDraw", 16, "レイアウト", nullptr},
    {"２Ｄエフェクト", "Draw", 1, "エフェクト", nullptr},
    {"２Ｄウィンドウ", "LayoutDraw", 12, "レイアウト", nullptr},
};

const ExecuteOrder sDrawTable16[] = {
    {"２Ｄベース", "LayoutDraw", 64, "レイアウト", nullptr},
    {"２Ｄ", "LayoutDraw", 320, "レイアウト", nullptr},
    {"２Ｄベースエフェクト", "Draw", 1, "エフェクト", nullptr},
    {"２Ｄ（サブ画面のみ）", "LayoutDraw", 256, "レイアウト", nullptr},
    {"２Ｄヘッド", "LayoutDraw", 32, "レイアウト", nullptr},
    {"２Ｄガイド", "LayoutDraw", 16, "レイアウト", nullptr},
    {"２Ｄワイプ", "LayoutDraw", 32, "レイアウト", nullptr},
    {"２Ｄポーズ", "LayoutDraw", 8, "レイアウト", nullptr},
    {"２Ｄランキング", "LayoutDraw", 4, "レイアウト", nullptr},
    {"２Ｄリザルト", "LayoutDraw", 4, "レイアウト", nullptr},
    {"２Ｄゲームオーバー", "LayoutDraw", 16, "レイアウト", nullptr},
    {"２Ｄエフェクト", "Draw", 1, "エフェクト", nullptr},
    {"２Ｄウィンドウ", "LayoutDraw", 12, "レイアウト", nullptr},
};

const ExecuteOrder sDrawTable17[] = {
    {"２Ｄヘッド", "LayoutDraw", 32, "レイアウト", nullptr},
    {"２Ｄワイプ", "LayoutDraw", 32, "レイアウト", nullptr},
    {"２Ｄポーズ", "LayoutDraw", 8, "レイアウト", nullptr},
    {"２Ｄランキング", "LayoutDraw", 4, "レイアウト", nullptr},
    {"２Ｄリザルト", "LayoutDraw", 4, "レイアウト", nullptr},
    {"２Ｄゲームオーバー", "LayoutDraw", 16, "レイアウト", nullptr},
};

const ExecuteOrder sDrawTable18[] = {
    {"２Ｄヘッド", "LayoutDraw", 32, "レイアウト", nullptr},
    {"２Ｄワイプ", "LayoutDraw", 32, "レイアウト", nullptr},
    {"２Ｄポーズ", "LayoutDraw", 8, "レイアウト", nullptr},
    {"２Ｄランキング", "LayoutDraw", 4, "レイアウト", nullptr},
    {"２Ｄリザルト", "LayoutDraw", 4, "レイアウト", nullptr},
    {"２Ｄゲームオーバー", "LayoutDraw", 16, "レイアウト", nullptr},
};

const ExecuteOrder sDrawTable19[] = {
    {"2DAboveBlur1", "LayoutDraw", 64, "レイアウト", nullptr},
    {"２Ｄエフェクト", "Draw", 1, "エフェクト", nullptr},
    {"2DEffectAboveBlur", "Draw", 1, "エフェクト", nullptr},
};

const ExecuteOrder sDrawTable20[] = {
    {"2DAboveBlur1", "LayoutDraw", 64, "レイアウト", nullptr},
    {"２Ｄエフェクト", "Draw", 1, "エフェクト", nullptr},
};

const ExecuteOrder sDrawTable21[] = {
    {"2DAboveBlur2", "LayoutDraw", 64, "レイアウト", nullptr},
};

const ExecuteOrder sDrawTable22[] = {
    {"ポストエフェクトマスク", "ActorModelDrawPostEffectMask", 32, "地形", nullptr},
};

const ExecuteOrder sDrawTable23[] = {
    {"ShadowMask[add]", "ActorDraw", 32, "敵", nullptr},
};

const ExecuteOrder sDrawTable24[] = {
    {"2DSequence", "LayoutDraw", 64, "レイアウト", nullptr},
};
}  // namespace
}  // namespace al
