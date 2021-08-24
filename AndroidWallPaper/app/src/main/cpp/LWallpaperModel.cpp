/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "LWallpaperModel.hpp"
#include <fstream>
#include <vector>
#include <Live2DCubismCore.hpp>
#include <CubismModelSettingJson.hpp>
#include <Motion/CubismMotion.hpp>
#include <Physics/CubismPhysics.hpp>
#include <CubismDefaultParameterId.hpp>
#include <Rendering/OpenGL/CubismRenderer_OpenGLES2.hpp>
#include <Utils/CubismString.hpp>
#include <Id/CubismIdManager.hpp>
#include <Motion/CubismMotionQueueEntry.hpp>
#include <Math/CubismMath.hpp>
#include "LWallpaperDefine.hpp"
#include "LWallpaperPal.hpp"
#include "LWallpaperTextureManager.hpp"
#include "LWallpaperDelegate.hpp"

using namespace Live2D::Cubism::Framework;
using namespace Live2D::Cubism::Framework::DefaultParameterId;
using namespace LWallpaperDefine;

namespace {
    const csmFloat32 gravityMaxValue = 9.81f;
    const csmFloat32 gravitationalAccelerationRange = 0.6f;
    const csmFloat32 conditionStandardValue = 0.001f;
    const csmFloat32 calculationReferenceNumber = 10.0f;

    csmByte* CreateBuffer(const csmChar* path, csmSizeInt* size)
    {
        if (DebugLogEnable)
        {
            LWallpaperPal::PrintLog("[APP]create buffer: %s ", path);
        }
        return LWallpaperPal::LoadFileAsBytes(path, size);
    }

    void DeleteBuffer(csmByte* buffer, const csmChar* path = "")
    {
        if (DebugLogEnable)
        {
            LWallpaperPal::PrintLog("[APP]delete buffer: %s", path);
        }
        LWallpaperPal::ReleaseBytes(buffer);
    }
}

LWallpaperModel::LWallpaperModel()
    : CubismUserModel()
    , _modelJson(nullptr)
    , _userTimeSeconds(0.0f)
{
    if (DebugLogEnable)
    {
        _debugMode = true;
    }

    _gravitationalAccelerationX = conditionStandardValue / calculationReferenceNumber;

    _idParamAngleX = CubismFramework::GetIdManager()->GetId(ParamAngleX);
    _idParamAngleY = CubismFramework::GetIdManager()->GetId(ParamAngleY);
    _idParamAngleZ = CubismFramework::GetIdManager()->GetId(ParamAngleZ);
    _idParamBodyAngleX = CubismFramework::GetIdManager()->GetId(ParamBodyAngleX);
    _idParamEyeBallX = CubismFramework::GetIdManager()->GetId(ParamEyeBallX);
    _idParamEyeBallY = CubismFramework::GetIdManager()->GetId(ParamEyeBallY);
}

LWallpaperModel::LWallpaperModel(const std::string modelDirectoryName, const std::string currentModelDirectory)
        : CubismUserModel(),_modelDirName(modelDirectoryName), _currentModelDirectory(currentModelDirectory), _modelJson(nullptr), _userTimeSeconds(0.0f)
{
    if (DebugLogEnable)
    {
        _debugMode = true;
    }

    _gravitationalAccelerationX = conditionStandardValue / calculationReferenceNumber;

    _idParamAngleX = CubismFramework::GetIdManager()->GetId(ParamAngleX);
    _idParamAngleY = CubismFramework::GetIdManager()->GetId(ParamAngleY);
    _idParamAngleZ = CubismFramework::GetIdManager()->GetId(ParamAngleZ);
    _idParamBodyAngleX = CubismFramework::GetIdManager()->GetId(ParamBodyAngleX);
    _idParamEyeBallX = CubismFramework::GetIdManager()->GetId(ParamEyeBallX);
    _idParamEyeBallY = CubismFramework::GetIdManager()->GetId(ParamEyeBallY);
}

LWallpaperModel::~LWallpaperModel()
{
    ReleaseMotions();
    ReleaseExpressions();

    for (csmInt32 i = 0; i < _modelJson->GetMotionGroupCount(); i++)
    {
        const csmChar* group = _modelJson->GetMotionGroupName(i);
        ReleaseMotionGroup(group);
    }
    delete _modelJson;
}

std::string LWallpaperModel::MakeAssetPath(const std::string &assetFileName)
{
    return _currentModelDirectory + assetFileName;
}

void LWallpaperModel::LoadAssets(const std::string & fiileName, const std::function<void(Csm::csmByte*, Csm::csmSizeInt)>& afterLoadCallback)
{
    Csm::csmSizeInt bufferSize = 0;
    Csm::csmByte* buffer = nullptr;

    if (fiileName.empty())
    {
        return;
    }

    // バッファの設定
    buffer = LWallpaperPal::LoadFileAsBytes(MakeAssetPath(fiileName).c_str(), &bufferSize);

    // コールバック関数の呼び出し
    afterLoadCallback(buffer, bufferSize);

    // バッファの解放
    LWallpaperPal::ReleaseBytes(buffer);
}


void LWallpaperModel::SetupModel()
{
    _updating = true;
    _initialized = false;

    // モデルの設定データをJsonファイルから読み込み
    LoadAssets(_modelDirName + ".model3.json", [=](Csm::csmByte* buffer, Csm::csmSizeInt bufferSize) { _modelJson = new Csm::CubismModelSettingJson(buffer, bufferSize); });
    // モデルの設定データからモデルデータを読み込み
    LoadAssets(_modelJson->GetModelFileName(), [=](Csm::csmByte* buffer, Csm::csmSizeInt bufferSize) { LoadModel(buffer, bufferSize); });

    // 表情データの読み込み
    for (auto expressionIndex = 0; expressionIndex < _modelJson->GetExpressionCount(); ++expressionIndex)
    {
        LoadAssets(_modelJson->GetExpressionFileName(expressionIndex), [=](Csm::csmByte* buffer, Csm::csmSizeInt bufferSize) {
            auto expressionName = _modelJson->GetExpressionName(expressionIndex);
            ACubismMotion* motion = LoadExpression(buffer, bufferSize, expressionName);
            _expressions[expressionName] = motion;
        });
    }

    //ポーズデータの読み込み
    LoadAssets(_modelJson->GetPoseFileName(), [=](Csm::csmByte* buffer, Csm::csmSizeInt bufferSize) {
        LoadPose(buffer, bufferSize);
    });

    // 物理演算データの読み込み
    LoadAssets(_modelJson->GetPhysicsFileName(), [=](Csm::csmByte* buffer, Csm::csmSizeInt bufferSize) {
        LoadPhysics(buffer, bufferSize);
    });

    // モデルに付属するユーザーデータの読み込み
    LoadAssets(_modelJson->GetUserDataFile(), [=](Csm::csmByte* buffer, Csm::csmSizeInt bufferSize) {
        LoadUserData(buffer, bufferSize);
    });

    // Breathの作成
    {
        _breath = CubismBreath::Create();

        csmVector<CubismBreath::BreathParameterData> breathParameters;

        breathParameters.PushBack(CubismBreath::BreathParameterData(_idParamAngleX, 0.0f, 15.0f, 6.5345f, 0.5f));
        breathParameters.PushBack(CubismBreath::BreathParameterData(_idParamAngleY, 0.0f, 8.0f, 3.5345f, 0.5f));
        breathParameters.PushBack(CubismBreath::BreathParameterData(_idParamAngleZ, 0.0f, 10.0f, 5.5345f, 0.5f));
        breathParameters.PushBack(CubismBreath::BreathParameterData(_idParamBodyAngleX, 0.0f, 4.0f, 15.5345f, 0.5f));
        breathParameters.PushBack(CubismBreath::BreathParameterData(CubismFramework::GetIdManager()->GetId(ParamBreath), 0.5f, 0.5f, 3.2345f, 0.5f));

        _breath->SetParameters(breathParameters);
    }

    // Layout
    csmMap<csmString, csmFloat32> layout;
    _modelJson->GetLayoutMap(layout);
    // レイアウト情報から位置を設定
    _modelMatrix->SetupFromLayout(layout);

    // パラメータを保存
    _model->SaveParameters();

    // モデル読み込み時のパラメータを保存
    _initParameterValues = new csmFloat32[_model->GetParameterCount()];
    for (csmInt32 i = 0; i < _model->GetParameterCount(); ++i) 
    {
        _initParameterValues[i] = _model->GetParameterValue(i);
    }

    // モーションデータの読み込み
    for (csmInt32 i = 0; i < _modelJson->GetMotionGroupCount(); i++)
    {
        const csmChar* group = _modelJson->GetMotionGroupName(i);
        // モーションデータをグループ名から一括でロードする
        PreloadMotionGroup(group);
    }

    _motionManager->StopAllMotions();

    // レンダラの作成
    CreateRenderer();

    // テクスチャのセットアップ
    SetupTextures();

    _updating = false;
    _initialized = true;
}

void LWallpaperModel::PreloadMotionGroup(const csmChar* group)
{
    const csmInt32 count = _modelJson->GetMotionCount(group);

    for (csmInt32 i = 0; i < count; i++)
    {
        //ex) idle_0
        csmString name = Utils::CubismString::GetFormatedString("%s_%d", group, i);
        csmString path = _modelJson->GetMotionFileName(group, i);
        path = csmString(_currentModelDirectory.c_str()) + path;

        if (_debugMode)
        {
            LWallpaperPal::PrintLog("[APP]load motion: %s => [%s_%d] ", path.GetRawString(), group, i);
        }

        csmByte* buffer;
        csmSizeInt size;
        buffer = CreateBuffer(path.GetRawString(), &size);
        CubismMotion* tmpMotion = static_cast<CubismMotion*>(LoadMotion(buffer, size, name.GetRawString()));

        csmFloat32 fadeTime = _modelJson->GetMotionFadeInTimeValue(group, i);
        if (fadeTime >= 0.0f)
        {
            tmpMotion->SetFadeInTime(fadeTime);
        }

        fadeTime = _modelJson->GetMotionFadeOutTimeValue(group, i);
        if (fadeTime >= 0.0f)
        {
            tmpMotion->SetFadeOutTime(fadeTime);
        }

        if (_motions[name] != NULL)
        {
            ACubismMotion::Delete(_motions[name]);
        }
        _motions[name] = tmpMotion;

        DeleteBuffer(buffer, path.GetRawString());
    }

    CreateRenderer();

    SetupTextures();
}

void LWallpaperModel::ReleaseMotionGroup(const csmChar* group) const
{
    const csmInt32 count = _modelJson->GetMotionCount(group);
    for (csmInt32 i = 0; i < count; i++)
    {
        csmString voice = _modelJson->GetMotionSoundFileName(group, i);
        if (strcmp(voice.GetRawString(), "") != 0)
        {
            csmString path = voice;
            path = csmString(_currentModelDirectory.c_str()) + path;
        }
    }
}

/**
* @brief すべてのモーションデータの解放
*
* すべてのモーションデータを解放する。
*/
void LWallpaperModel::ReleaseMotions()
{
    for (csmMap<csmString, ACubismMotion*>::const_iterator iter = _motions.Begin(); iter != _motions.End(); ++iter)
    {
        ACubismMotion::Delete(iter->Second);
    }

    _motions.Clear();
}

/**
* @brief すべての表情データの解放
*
* すべての表情データを解放する。
*/
void LWallpaperModel::ReleaseExpressions()
{
    for (csmMap<csmString, ACubismMotion*>::const_iterator iter = _expressions.Begin(); iter != _expressions.End(); ++iter)
    {
        ACubismMotion::Delete(iter->Second);
    }

    _expressions.Clear();
}

void LWallpaperModel::Update()
{
    const csmFloat32 deltaTimeSeconds = LWallpaperPal::GetDeltaTime();
    _userTimeSeconds += deltaTimeSeconds;

    _dragManager->Update(deltaTimeSeconds);
    _dragX = _dragManager->GetX();
    _dragY = _dragManager->GetY();

    LWallpaperDelegate* delegateInstance = LWallpaperDelegate::GetInstance();
    delegateInstance->ParameterResetCount();

    // 重力加速度を-1~1の範囲になるよう正規化
    _gravitationalAccelerationX = _gravitationalAccelerationX / gravityMaxValue;

    // モーションによるパラメータ更新の有無
    csmBool motionUpdated = false;

    //-----------------------------------------------------------------
    _model->LoadParameters(); // 前回セーブされた状態をロード
    if (!_motionManager->IsFinished())
    {
        motionUpdated = _motionManager->UpdateMotion(_model, deltaTimeSeconds); // モーションを更新
    }
    _model->SaveParameters(); // 状態を保存
    //-----------------------------------------------------------------

    bool canResetParameter = (!delegateInstance->GetIsTapped() && !delegateInstance->GetIsSecondCount());

    // メインモーションの更新がないとき
    if (!motionUpdated)
    {
        if (_eyeBlink)
        {
            _eyeBlink->UpdateParameters(_model, deltaTimeSeconds); // まばたき
        }

        if (canResetParameter && (CubismMath::AbsF(_gravitationalAccelerationX) < conditionStandardValue))
        {
            // モデル読み込み時のパラメータとの差分を出し、元に戻す
            for (csmInt32 i = 0; i < _model->GetParameterCount(); ++i)
            {
                csmFloat32 diff = _initParameterValues[i] - _model->GetParameterValue(i);
                if (CubismMath::AbsF(diff) > conditionStandardValue)
                {
                    _model->AddParameterValue(i,diff * deltaTimeSeconds * calculationReferenceNumber);
                } else{
                    _model->SetParameterValue(i,_initParameterValues[i]);
                }
            }
        }
    }

    if (_expressionManager)
    {
        _expressionManager->UpdateMotion(_model, deltaTimeSeconds); // 表情でパラメータ更新（相対変化）
    }


    if (canResetParameter)
    {
        //ドラッグによる変化
        //ドラッグによる顔の向きの調整
        _model->AddParameterValue(_idParamAngleX, _dragX * 30); // -30から30の値を加える
        _model->AddParameterValue(_idParamAngleY, _dragY * 30);
        _model->AddParameterValue(_idParamAngleZ, _dragX * _dragY * -30);

        //ドラッグによる体の向きの調整
        _model->AddParameterValue(_idParamBodyAngleX, _dragX * calculationReferenceNumber); // -10から10の値を加える

        //ドラッグによる目の向きの調整
        _model->AddParameterValue(_idParamEyeBallX, _dragX); // -1から1の値を加える
        _model->AddParameterValue(_idParamEyeBallY, _dragY);
    }
    else
    {
        Csm::CubismVector2 vec = delegateInstance->GetViewPoint();

        //タップによる向きの調整
        //タップによる顔の向きの調整
        _model->AddParameterValue(_idParamAngleX, vec.X * 30); // -30から30の値を加える
        _model->AddParameterValue(_idParamAngleY, vec.Y * 30);
        _model->AddParameterValue(_idParamAngleZ, vec.X * vec.Y * -30);

        //タップによる体の向きの調整
        _model->AddParameterValue(_idParamBodyAngleX, vec.X * calculationReferenceNumber); // -10から10の値を加える

        //タップによる目の向きの調整
        _model->AddParameterValue(_idParamEyeBallX, vec.X); // -1から1の値を加える
        _model->AddParameterValue(_idParamEyeBallY, vec.Y);
    }

    // 重力加速度による向きと位置の調整
    {
        // 10倍した値を設定
        csmFloat32 accelValue = _gravitationalAccelerationX * calculationReferenceNumber;

        // 顔の向きの調整
        _model->AddParameterValue(_idParamAngleX, accelValue); // -10から10の値を加える

        // 体の向きの調整
        _model->AddParameterValue(_idParamBodyAngleX, accelValue); // -10から10の値を加える

        // 目の向きの調整
        _model->AddParameterValue(_idParamEyeBallX, -_gravitationalAccelerationX); // -1から1の値を加える

        // 範囲を超えないように設定
        _gravitationalAccelerationX = CubismMath::RangeF(_gravitationalAccelerationX,-gravitationalAccelerationRange,gravitationalAccelerationRange);
        //モデルの位置を設定
        _modelMatrix->SetX(-_gravitationalAccelerationX / 2.0f);
    }

    _model->SaveParameters(); // 状態を保存

    // 呼吸など
    if (_breath)
    {
        _breath->UpdateParameters(_model, deltaTimeSeconds);
    }

    // 物理演算の設定
    if (_physics)
    {
        _physics->Evaluate(_model, deltaTimeSeconds);
    }

    // ポーズの設定
    if (_pose)
    {
        _pose->UpdateParameters(_model, deltaTimeSeconds);
    }

    _model->Update();

}

CubismMotionQueueEntryHandle LWallpaperModel::StartRandomMotion(const csmChar* group, csmInt32 priority, ACubismMotion::FinishedMotionCallback onFinishedMotionHandler)
{
    if (!_modelJson->GetMotionCount(group))
    {
        return InvalidMotionQueueEntryHandleValue;
    }

    csmInt32 no = rand() % _modelJson->GetMotionCount(group);

    return StartMotion(group, no, priority, onFinishedMotionHandler);
}

CubismMotionQueueEntryHandle LWallpaperModel::StartMotion(const csmChar* group, csmInt32 no, csmInt32 priority, ACubismMotion::FinishedMotionCallback onFinishedMotionHandler)
{
    if (priority == PriorityForce)
    {
        _motionManager->SetReservePriority(priority);
    }
    else if (!_motionManager->ReserveMotion(priority))
    {
        if (_debugMode)
        {
            LWallpaperPal::PrintLog("[APP]can't start motion.");
        }
        return InvalidMotionQueueEntryHandleValue;
    }

    const csmString fileName = _modelJson->GetMotionFileName(group, no);

    //ex) idle_0
    csmString name = Utils::CubismString::GetFormatedString("%s_%d", group, no);
    CubismMotion* motion = static_cast<CubismMotion*>(_motions[name.GetRawString()]);
    csmBool autoDelete = false;

    if (!motion)
    {
        csmString path = fileName;
        path = csmString(_currentModelDirectory.c_str()) + path;

        csmByte* buffer;
        csmSizeInt size;
        buffer = CreateBuffer(path.GetRawString(), &size);
        motion = static_cast<CubismMotion*>(LoadMotion(buffer, size, nullptr));
        csmFloat32 fadeTime = _modelJson->GetMotionFadeInTimeValue(group, no);
        if (fadeTime >= 0.0f)
        {
            motion->SetFadeInTime(fadeTime);
        }

        fadeTime = _modelJson->GetMotionFadeOutTimeValue(group, no);
        if (fadeTime >= 0.0f)
        {
            motion->SetFadeOutTime(fadeTime);
        }
        autoDelete = true; // 終了時にメモリから削除

        DeleteBuffer(buffer, path.GetRawString());
    }

    motion->SetFinishedMotionHandler(onFinishedMotionHandler);

    //voice
    csmString voice = _modelJson->GetMotionSoundFileName(group, no);
    if (strcmp(voice.GetRawString(), "") != 0)
    {
        csmString path = voice;
        path = csmString(_currentModelDirectory.c_str()) + path;
    }

    if (_debugMode)
    {
        LWallpaperPal::PrintLog("[APP]start motion: [%s_%d]", group, no);
    }
    return  _motionManager->StartMotionPriority(motion, autoDelete, priority);
}

csmBool LWallpaperModel::HitTest(const csmChar* hitAreaName, csmFloat32 x, csmFloat32 y)
{
    // 透明時は当たり判定なし。
    if (_opacity < 1)
    {
        return false;
    }
    const csmInt32 count = _modelJson->GetHitAreasCount();
    for (csmInt32 i = 0; i < count; i++)
    {
        if (!strcmp(_modelJson->GetHitAreaName(i), hitAreaName))
        {
            const CubismIdHandle drawID = _modelJson->GetHitAreaId(i);
            return IsHit(drawID, x, y);
        }
    }
    return false; // 存在しない場合はfalse
}

void LWallpaperModel::Draw(CubismMatrix44& matrix)
{
    if (!_model)
    {
        return;
    }

    matrix.MultiplyByMatrix(_modelMatrix);

    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->SetMvpMatrix(&matrix);
    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->DrawModel();
}

void LWallpaperModel::SetExpression(const csmChar* expressionID)
{
    ACubismMotion* motion = _expressions[expressionID];
    if (_debugMode)
    {
        LWallpaperPal::PrintLog("[APP]expression: [%s]", expressionID);
    }

    if (motion)
    {
        _expressionManager->StartMotionPriority(motion, false, PriorityForce);
    }
    else
    {
        if (_debugMode) LWallpaperPal::PrintLog("[APP]expression[%s] is null ", expressionID);
    }
}

void LWallpaperModel::SetRandomExpression()
{
    if (!_expressions.GetSize())
    {
        return;
    }

    csmInt32 no = rand() % _expressions.GetSize();
    csmMap<csmString, ACubismMotion*>::const_iterator map_ite(&_expressions,no);
    csmString name = (*map_ite).First;
    SetExpression(name.GetRawString());
}

void LWallpaperModel::ReloadRenderer()
{
    DeleteRenderer();

    CreateRenderer();

    SetupTextures();
}

void LWallpaperModel::SetupTextures()
{
    for (csmInt32 modelTextureNumber = 0; modelTextureNumber < _modelJson->GetTextureCount(); modelTextureNumber++)
    {
        // テクスチャ名が空文字だった場合はロード・バインド処理をスキップ
        if (strcmp(_modelJson->GetTextureFileName(modelTextureNumber), "") == 0)
        {
            continue;
        }

        //OpenGLのテクスチャユニットにテクスチャをロードする
        csmString texturePath = _modelJson->GetTextureFileName(modelTextureNumber);
        texturePath = csmString(_currentModelDirectory.c_str()) + texturePath;

        LWallpaperTextureManager::TextureInfo* texture = LWallpaperDelegate::GetInstance()->GetTextureManager()->CreateTextureFromPngFile(texturePath.GetRawString());
        const csmInt32 glTextueNumber = texture->id;

        //OpenGL
        GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->BindTexture(modelTextureNumber, glTextueNumber);
    }

#ifdef PREMULTIPLIED_ALPHA_ENABLE
    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->IsPremultipliedAlpha(true);
#else
    GetRenderer<Rendering::CubismRenderer_OpenGLES2>()->IsPremultipliedAlpha(false);
#endif
}

void LWallpaperModel::MotionEventFired(const csmString& eventValue)
{
    CubismLogInfo("%s is fired on LWallpaperModel!!", eventValue.GetRawString());
}

Csm::Rendering::CubismOffscreenFrame_OpenGLES2 &LWallpaperModel::GetRenderBuffer()
{
    return _renderBuffer;
}

void LWallpaperModel::StartRandomMotion()
{
    //-----------------------------------------------------------------
    _model->LoadParameters(); // 前回セーブされた状態をロード
    if (_motionManager->IsFinished())
    {
        // モーションの再生がない場合、MotionGroupIdleに設定されているモーションをランダムに再生する
        StartRandomMotion(LWallpaperDefine::MotionGroupIdle, LWallpaperDefine::PriorityIdle);
    }
    _model->SaveParameters(); // 状態を保存
    //-----------------------------------------------------------------
}

void LWallpaperModel::StartRandomMotionWithOption(const Csm::csmChar* group, Csm::csmInt32 priority, Csm::ACubismMotion::FinishedMotionCallback onFinishedMotionHandler)
{
    //-----------------------------------------------------------------
    _model->LoadParameters(); // 前回セーブされた状態をロード
    if (_motionManager->IsFinished())
    {
        // モーションの再生がない場合、groupに設定されているモーションをランダムに再生する
        StartRandomMotion(group, priority,onFinishedMotionHandler);
    }
    _model->SaveParameters(); // 状態を保存
    //-----------------------------------------------------------------
}

void LWallpaperModel::StartOrderMotion(const Csm::csmChar* group, Csm::csmInt32 index, Csm::csmInt32 priority)
{
    //-----------------------------------------------------------------
    _model->LoadParameters(); // 前回セーブされた状態をロード
    if (_motionManager->IsFinished())
    {
        // モーションの再生がない場合、始めに登録されているモーションを再生する
        StartMotion(group,index, priority);
    }
    _model->SaveParameters(); // 状態を保存
    //-----------------------------------------------------------------
}

void LWallpaperModel::SetGravitationalAccelerationX(Csm::csmFloat32 gravity)
{
    _gravitationalAccelerationX = gravity;
}
