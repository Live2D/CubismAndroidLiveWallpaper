/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "LWallpaperLive2DManager.hpp"
#include <string>
#include <GLES2/gl2.h>
#include <Rendering/CubismRenderer.hpp>
#include "LWallpaperPal.hpp"
#include "LWallpaperDefine.hpp"
#include "LWallpaperDelegate.hpp"
#include "LWallpaperModel.hpp"
#include "LWallpaperView.hpp"

using namespace Csm;
using namespace LAppDefine;
using namespace std;

namespace {
    LWallpaperLive2DManager* s_instance = nullptr;
}

LWallpaperLive2DManager* LWallpaperLive2DManager::GetInstance()
{
    if (!s_instance)
    {
        s_instance = new LWallpaperLive2DManager();
    }

    return s_instance;
}

void LWallpaperLive2DManager::ReleaseInstance()
{
    if (s_instance)
    {
        delete s_instance;
    }

    s_instance = nullptr;
}

LWallpaperLive2DManager::LWallpaperLive2DManager()
{
    _viewMatrix = new CubismMatrix44();
    LoadModel(_modelDirectoryName);
}

LWallpaperLive2DManager::~LWallpaperLive2DManager()
{
    ReleaseModel();
}

void LWallpaperLive2DManager::ReleaseModel()
{
    delete _model;
}

LWallpaperModel* LWallpaperLive2DManager::GetModel() const
{
    return _model;
}

void LWallpaperLive2DManager::OnDrag(csmFloat32 x, csmFloat32 y) const
{
    LWallpaperModel* model = GetModel();
    model->SetDragging(x, y);
}

void LWallpaperLive2DManager::OnTap(csmFloat32 x, csmFloat32 y)
{
    if (_model->HitTest(HitAreaNameHead, x, y))
    {
        _model->SetRandomExpression();
    }
    else if (_model->HitTest(HitAreaNameBody, x, y))
    {
        _model->StartRandomMotionWithOption(MotionGroupTapBody, PriorityNormal);
    }
}

void LWallpaperLive2DManager::OnUpdate() const
{
    int width = LWallpaperDelegate::GetInstance()->GetWindowWidth();
    int height = LWallpaperDelegate::GetInstance()->GetWindowHeight();

    CubismMatrix44 projection;

    LWallpaperModel* model = GetModel();

    if (model->GetModel()->GetCanvasWidth() > 1.0f && width < height)
    {
        // 横に長いモデルを縦長ウィンドウに表示する際モデルの横サイズでscaleを算出する
        model->GetModelMatrix()->SetWidth(2.0f);
        projection.Scale(1.0f, static_cast<float>(width) / static_cast<float>(height));
    }
    else
    {
        projection.Scale(static_cast<float>(height) / static_cast<float>(width), 1.0f);
    }

    // 必要があればここで乗算
    if (_viewMatrix)
    {
        projection.MultiplyByMatrix(_viewMatrix);
    }

    // モデル1体描画前コール
    LWallpaperDelegate::GetInstance()->GetView()->PreModelDraw(*model);

    model->Update();
    model->Draw(projection);///< 参照渡しなのでprojectionは変質する

    // モデル1体描画前コール
    LWallpaperDelegate::GetInstance()->GetView()->PostModelDraw(*model);
}

void LWallpaperLive2DManager::SetAssetDirectory(const std::string &path)
{
    _currentModelDirectory = path;
}

void LWallpaperLive2DManager::LoadModel(const std::string modelDirectoryName)
{
    // モデルのディレクトリを指定
    SetAssetDirectory(LAppDefine::ResourcesPath + modelDirectoryName + "/");

    // モデルデータの新規生成
    _model = new LWallpaperModel(modelDirectoryName, _currentModelDirectory);

    // モデルデータの読み込み及び生成とセットアップを行う
    static_cast<LWallpaperModel*>(_model)->SetupModel();
}

void LWallpaperLive2DManager::SetGravitationalAccelerationX(float gravity)
{
    _model->SetGravitationalAccelerationX(gravity);
}


