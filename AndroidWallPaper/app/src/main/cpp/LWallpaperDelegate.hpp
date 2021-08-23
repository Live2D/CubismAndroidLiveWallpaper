/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "LWallpaperAllocator.hpp"
#include "Math/CubismVector2.hpp"

class LWallpaperView;
class LWallpaperTextureManager;

/**
* @brief   アプリケーションクラス。
*   Cubism SDK の管理を行う。
*/
class LWallpaperDelegate
{
public:
    /**
    * @brief   クラスのインスタンス（シングルトン）を返す。<br>
    *           インスタンスが生成されていない場合は内部でインスタンを生成する。
    *
    * @return  クラスのインスタンス
    */
    static LWallpaperDelegate* GetInstance();

    /**
    * @brief   クラスのインスタンス（シングルトン）を解放する。
    *
    */
    static void ReleaseInstance();

    /**
    * @brief JavaのActivityのOnStart()のコールバック関数。
    */
    void OnStart();

    /**
    * @brief JavaのActivityのOnPause()のコールバック関数。
    */
    void OnPause();

    /**
    * @brief JavaのActivityのOnStop()のコールバック関数。
    */
    void OnStop();

    /**
    * @brief JavaのActivityのOnDestroy()のコールバック関数。
    */
    void OnDestroy();

    /**
    * @brief   JavaのGLSurfaceviewのOnSurfaceCreate()のコールバック関数。
    */
    void OnSurfaceCreate();

    /**
     * @brief JavaのGLSurfaceviewのOnSurfaceChanged()のコールバック関数。
     * @param width
     * @param height
     */
    void OnSurfaceChanged(float width, float height);

    /**
    * @brief   実行処理。
    */
    void Run();

    /**
    * @brief Touch開始。
    *
    * @param[in] x x座標
    * @param[in] y x座標
    */
    void OnTouchBegan(double x, double y);

    /**
    * @brief Touch終了。
    *
    * @param[in] x x座標
    * @param[in] y x座標
    */
    void OnTouchEnded(double x, double y);

    /**
    * @brief Touch移動。
    *
    * @param[in] x x座標
    * @param[in] y x座標
    */
    void OnTouchMoved(double x, double y);

    /**
    * @brief ランダムなモーションの開始
    */
    void StartRandomMotion();

    /**
    * @brief モーションの開始
    */
    void StartMotion(Csm::csmInt32 index);

    /**
    * @brief シェーダーを登録する。
    */
    GLuint CreateShader();

    /**
    * @brief パラメータを初期状態に戻すまでの時間をカウントする。
    */
    void ParameterResetCount();

    /**
     * @brief 背景クリア色設定
     * @param[in]   r   赤(0.0~1.0)
     * @param[in]   g   緑(0.0~1.0)
     * @param[in]   b   青(0.0~1.0)
     */
    void SetClearColor(float r, float g, float b);

    /**
     * @brief 背景画像アルファ値
     * @param[in]   r   赤(0.0~1.0)
     * @param[in]   g   緑(0.0~1.0)
     * @param[in]   b   青(0.0~1.0)
     */
    void SetBackGroundSpriteAlpha(float a);

    void SetGravitationalAccelerationX(float gravity);

    /**
    * @brief テクスチャマネージャーの取得
    */
    LWallpaperTextureManager* GetTextureManager() { return _textureManager; }

    /**
    * @brief ウインドウ幅の設定
    */
    int GetWindowWidth() { return _width; }

    /**
    * @brief ウインドウ高さの取得
    */
    int GetWindowHeight() { return _height; }

    /**
    * @brief   View情報を取得する。
    */
    LWallpaperView* GetView() { return _view; }

    /**
    * @brief   View座標を取得する。
    */
    Csm::CubismVector2 GetViewPoint() { return _viewPoint; }

    /**
    * @brief   タップ状態かを取得する。
    */
    bool GetIsTapped() { return _isTapped; }

    /**
    * @brief   タップ状態から解放されたかどうかを取得する。
    */
    bool GetIsSecondCount() { return _isSecondCount; }

private:
    /**
    * @brief   コンストラクタ
    */
    LWallpaperDelegate();

    /**
    * @brief   デストラクタ
    */
    ~LWallpaperDelegate();

    LWallpaperAllocator _cubismAllocator;              ///< Cubism SDK Allocator
    Csm::CubismFramework::Option _cubismOption;  ///< Cubism SDK Option
    LWallpaperTextureManager* _textureManager;         ///< テクスチャマネージャー
    LWallpaperView* _view;                      ///< View情報
    int _width;                                  ///< Windowの幅
    int _height;                                 ///< windowの高さ
    bool _isCaptured;                            ///< クリックしているか
    bool _isTapped;                              ///< タップ状態か
    bool _isSecondCount;                         ///< 時間をカウントしているか
    bool _isActive;                              ///< アプリがアクティブ状態なのか
    float _mouseY;                               ///< マウスY座標
    float _mouseX;                               ///< マウスX座標

    Csm::CubismVector2 _viewPoint;               ///< ビュー座標
    Csm::csmFloat32 _deltaTimeCount;             ///< 経過したデルタ時間

    float _r,_g,_b;                              ///< カラー情報
};
