/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "LWallpaperView.hpp"
#include <cmath>
#include <string>
#include "LWallpaperPal.hpp"
#include "LWallpaperDelegate.hpp"
#include "LWallpaperDefine.hpp"
#include "LWallpaperTextureManager.hpp"
#include "LWallpaperLive2DManager.hpp"
#include "TouchManager.hpp"
#include "LWallpaperSprite.hpp"
#include "LWallpaperModel.hpp"

#include <Rendering/OpenGL/CubismOffscreenSurface_OpenGLES2.hpp>
#include <Rendering/OpenGL/CubismRenderer_OpenGLES2.hpp>

#include "JniBridgeC.hpp"

using namespace std;
using namespace LAppDefine;
using namespace Csm;

LWallpaperView::LWallpaperView():
        _programId(0),
        _renderSprite(nullptr),
        _renderTarget(SelectTarget_None),
        _backgroundImage(nullptr)
{
    _clearColor[0] = 1.0f;
    _clearColor[1] = 1.0f;
    _clearColor[2] = 1.0f;
    _clearColor[3] = 0.0f;

    // タッチ関係のイベント管理
    _touchManager = new TouchManager();

    // デバイス座標からスクリーン座標に変換するための
    _deviceToScreen = new CubismMatrix44();

    // 画面の表示の拡大縮小や移動の変換を行う行列
    _viewMatrix = new CubismViewMatrix();
}

LWallpaperView::~LWallpaperView()
{
    _renderBuffer.DestroyOffscreenFrame();
    delete _renderSprite;

    delete _viewMatrix;
    delete _deviceToScreen;
    delete _touchManager;
    delete _backgroundImage;
}

void LWallpaperView::Initialize()
{
    int width = LWallpaperDelegate::GetInstance()->GetWindowWidth();
    int height = LWallpaperDelegate::GetInstance()->GetWindowHeight();

    // 縦サイズを基準とする
    float ratio = static_cast<float>(width) / static_cast<float>(height);
    float left = -ratio;
    float right = ratio;
    float bottom = ViewLogicalLeft;
    float top = ViewLogicalRight;

    _viewMatrix->SetScreenRect(left, right, bottom, top); // デバイスに対応する画面の範囲。 Xの左端, Xの右端, Yの下端, Yの上端
    _viewMatrix->Scale(ViewScale, ViewScale);

    _deviceToScreen->LoadIdentity();
    if (width > height)
    {
        float screenW = fabsf(right - left);
        _deviceToScreen->ScaleRelative(screenW / width, -screenW / width);
    }
    else
    {
        float screenH = fabsf(top - bottom);
        _deviceToScreen->ScaleRelative(screenH / height, -screenH / height);
    }
    _deviceToScreen->TranslateRelative(-width * 0.5f, -height * 0.5f);

    // 表示範囲の設定
    _viewMatrix->SetMaxScale(ViewMaxScale); // 限界拡大率
    _viewMatrix->SetMinScale(ViewMinScale); // 限界縮小率

    // 表示できる最大範囲
    _viewMatrix->SetMaxScreenRect(
            ViewLogicalMaxLeft,
            ViewLogicalMaxRight,
            ViewLogicalMaxBottom,
            ViewLogicalMaxTop
    );
}

void LWallpaperView::InitializeShader()
{
    _programId = LWallpaperDelegate::GetInstance()->CreateShader();
}

void LWallpaperView::InitializeSprite()
{
    int width = LWallpaperDelegate::GetInstance()->GetWindowWidth();
    int height = LWallpaperDelegate::GetInstance()->GetWindowHeight();

    LWallpaperTextureManager* textureManager = LWallpaperDelegate::GetInstance()->GetTextureManager();
    const string resourcesPath = ResourcesPath;

    string imageName = BackImageName;


    float x = width * 0.5f;
    float y = height * 0.5f;
    float fWidth = width;
    float fHeight = height;



    // 画面全体を覆うサイズ
    x = width * 0.5f;
    y = height * 0.5f;

    if (!_renderSprite)
    {
        _renderSprite = new LWallpaperSprite(x, y, width, height, 0, _programId);
    }
    else
    {
        _renderSprite->ReSize(x, y, width, height);
    }
}

void LWallpaperView::Render()
{
    LWallpaperLive2DManager* Live2DManager = LWallpaperLive2DManager::GetInstance();



    // Cubism更新・描画
    Live2DManager->OnUpdate();

    // 各モデルが持つ描画ターゲットをテクスチャとする場合
    if (_renderTarget == SelectTarget_ModelFrameBuffer && _renderSprite)
    {
        const GLfloat uvVertex[] =
                {
                        1.0f, 1.0f,
                        0.0f, 1.0f,
                        0.0f, 0.0f,
                        1.0f, 0.0f,
                };

        float alpha = GetSpriteAlpha(2); // サンプルとしてαに適当な差をつける
        _renderSprite->SetColor(1.0f, 1.0f, 1.0f, alpha);

        LWallpaperModel *model = Live2DManager->GetModel();
        if (model)
        {
            _renderSprite->RenderImmidiate(model->GetRenderBuffer().GetColorBuffer(), uvVertex);
        }
    }
}

void LWallpaperView::OnTouchesBegan(float pointX, float pointY) const
{

}

void LWallpaperView::OnTouchesMoved(float pointX, float pointY) const
{

}

Csm::CubismVector2 LWallpaperView::OnTouchesEnded(float pointX, float pointY)
{
    // タッチ終了
    LWallpaperLive2DManager* live2DManager = LWallpaperLive2DManager::GetInstance();
    live2DManager->OnDrag(0.0f, 0.0f);

    // シングルタップ
    float x=0.0f; // 論理座標変換した座標を取得。
    float y=0.0f; // 論理座標変換した座標を取得。



    return {x,y};
}

float LWallpaperView::TransformViewX(float deviceX) const
{
    float screenX = _deviceToScreen->TransformX(deviceX); // 論理座標変換した座標を取得。
    return _viewMatrix->InvertTransformX(screenX); // 拡大、縮小、移動後の値。
}

float LWallpaperView::TransformViewY(float deviceY) const
{
    float screenY = _deviceToScreen->TransformY(deviceY); // 論理座標変換した座標を取得。
    return _viewMatrix->InvertTransformY(screenY); // 拡大、縮小、移動後の値。
}

float LWallpaperView::TransformScreenX(float deviceX) const
{
    return _deviceToScreen->TransformX(deviceX);
}

float LWallpaperView::TransformScreenY(float deviceY) const
{
    return _deviceToScreen->TransformY(deviceY);
}

void LWallpaperView::PreModelDraw(LWallpaperModel &refModel)
{
    // 別のレンダリングターゲットへ向けて描画する場合の使用するフレームバッファ
    Csm::Rendering::CubismOffscreenFrame_OpenGLES2* useTarget = nullptr;

    if (_renderTarget != SelectTarget_None)
    {// 別のレンダリングターゲットへ向けて描画する場合

        // 使用するターゲット
        useTarget = (_renderTarget == SelectTarget_ViewFrameBuffer) ? &_renderBuffer : &refModel.GetRenderBuffer();

        if (!useTarget->IsValid())
        {// 描画ターゲット内部未作成の場合はここで作成
            int width = LWallpaperDelegate::GetInstance()->GetWindowWidth();
            int height = LWallpaperDelegate::GetInstance()->GetWindowHeight();

            // モデル描画キャンバス
            useTarget->CreateOffscreenFrame(static_cast<csmUint32>(width), static_cast<csmUint32>(height));
        }

        // レンダリング開始
        useTarget->BeginDraw();
        useTarget->Clear(_clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]); // 背景クリアカラー
    }
}

void LWallpaperView::PostModelDraw(LWallpaperModel &refModel)
{
    // 別のレンダリングターゲットへ向けて描画する場合の使用するフレームバッファ
    Csm::Rendering::CubismOffscreenFrame_OpenGLES2* useTarget = nullptr;

    if (_renderTarget != SelectTarget_None)
    {// 別のレンダリングターゲットへ向けて描画する場合

        // 使用するターゲット
        useTarget = (_renderTarget == SelectTarget_ViewFrameBuffer) ? &_renderBuffer : &refModel.GetRenderBuffer();

        // レンダリング終了
        useTarget->EndDraw();

        // LAppMinimumViewの持つフレームバッファを使うなら、スプライトへの描画はここ
        if (_renderTarget == SelectTarget_ViewFrameBuffer && _renderSprite)
        {
            const GLfloat uvVertex[] =
                    {
                            1.0f, 1.0f,
                            0.0f, 1.0f,
                            0.0f, 0.0f,
                            1.0f, 0.0f,
                    };

            _renderSprite->SetColor(1.0f, 1.0f, 1.0f, GetSpriteAlpha(0));
            _renderSprite->RenderImmidiate(useTarget->GetColorBuffer(), uvVertex);
        }
    }
}

void LWallpaperView::SwitchRenderingTarget(SelectTarget targetType)
{
    _renderTarget = targetType;
}

void LWallpaperView::SetRenderTargetClearColor(float r, float g, float b)
{
    _clearColor[0] = r;
    _clearColor[1] = g;
    _clearColor[2] = b;
}

float LWallpaperView::GetSpriteAlpha(int assign) const
{
    // assignの数値に応じて適当に決定
    float alpha = 0.25f + static_cast<float>(assign) * 0.5f; // サンプルとしてαに適当な差をつける
    if (alpha > 1.0f)
    {
        alpha = 1.0f;
    }
    if (alpha < 0.1f)
    {
        alpha = 0.1f;
    }

    return alpha;
}

void LWallpaperView::SetBackGroundSpriteColor(float r, float g, float b, float a)
{

}
