/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "LAppMinimumDelegate.hpp"
#include <iostream>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include "LAppMinimumView.hpp"
#include "LAppPal.hpp"
#include "LAppDefine.hpp"
#include "LAppMinimumLive2DManager.hpp"
#include "LAppTextureManager.hpp"
#include "JniBridgeC.hpp"

#include "Utils/CubismDebug.hpp"
#include "LAppMinimumModel.hpp"

using namespace Csm;
using namespace std;
using namespace LAppDefine;

namespace {
    LAppMinimumDelegate* s_instance = nullptr;
}

LAppMinimumDelegate* LAppMinimumDelegate::GetInstance()
{
    if (!s_instance)
    {
        s_instance = new LAppMinimumDelegate();
    }

    return s_instance;
}

void LAppMinimumDelegate::ReleaseInstance()
{
    if (s_instance)
    {
        delete s_instance;
    }

    s_instance = nullptr;
}


void LAppMinimumDelegate::OnStart()
{
    LAppPal::UpdateTime();
}

void LAppMinimumDelegate::OnPause()
{

}

void LAppMinimumDelegate::OnStop()
{
    if (_view)
    {
        delete _view;
        _view = nullptr;
    }
    if (_textureManager)
    {
        delete _textureManager;
        _textureManager = nullptr;
    }

    // リソースを解放
    LAppMinimumLive2DManager::ReleaseInstance();

    CubismFramework::Dispose();
}

void LAppMinimumDelegate::OnDestroy()
{
    ReleaseInstance();
}

void LAppMinimumDelegate::Run()
{
    // 時間更新
    LAppPal::UpdateTime();

    // 画面の初期化
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepthf(1.0f);

    //描画更新
    if (_view)
    {
        _view->Render();
    }
}

void LAppMinimumDelegate::OnSurfaceCreate()
{
    _view = new LAppMinimumView();
    _textureManager = new LAppTextureManager();

    // setup view
    int width,height;
    glViewport(0, 0, width, height);
    _width = width;
    _height = height;

    //テクスチャサンプリング設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //透過設定
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Initialize cubism
    CubismFramework::Initialize();

    _view->InitializeShader();
}

void LAppMinimumDelegate::OnSurfaceChanged(float width, float height)
{
    glViewport(0, 0, width, height);
    _width = width;
    _height = height;

    //AppViewの初期化
    _view->Initialize();
    _view->InitializeSprite();

    _isActive = true;
}

LAppMinimumDelegate::LAppMinimumDelegate():
    _cubismOption(),
    _captured(false),
    _mouseX(0.0f),
    _mouseY(0.0f),
    _isActive(true),
    _width(0),
    _height(0),
    _viewPoint(0,0),
    _tapped(false),
    _isSecondCount(false),
    _deltaTimeCount(0.0f)
{
    // Setup Cubism
    _cubismOption.LogFunction = LAppPal::PrintMessage;
    _cubismOption.LoggingLevel = LAppDefine::CubismLoggingLevel;
    CubismFramework::CleanUp();
    CubismFramework::StartUp(&_cubismAllocator, &_cubismOption);
}

LAppMinimumDelegate::~LAppMinimumDelegate()
{
}

void LAppMinimumDelegate::OnTouchBegan(double x, double y)
{
    _mouseX = static_cast<float>(x);
    _mouseY = static_cast<float>(y);

    if (_view)
    {
        _isTapped = true;
        _isSecondCount = false;
        _isCaptured = true;
        _view->OnTouchesBegan(_mouseX, _mouseY);
    }
}

void LAppMinimumDelegate::OnTouchEnded(double x, double y)
{
    _mouseX = static_cast<float>(x);
    _mouseY = static_cast<float>(y);

    if (_view)
    {
        _isTapped = false;
        _isSecondCount = true;
        _deltaTimeCount = 0.0f;
        _isCaptured = false;
        _viewPoint = _view->OnTouchesEnded(_mouseX, _mouseY);
    }
}

void LAppMinimumDelegate::OnTouchMoved(double x, double y)
{
    _mouseX = static_cast<float>(x);
    _mouseY = static_cast<float>(y);

    if (_isCaptured && _view)
    {
        _isTapped = false;
        _isSecondCount = false;
        _view->OnTouchesMoved(_mouseX, _mouseY);
    }
}

GLuint LAppMinimumDelegate::CreateShader()
{
    //バーテックスシェーダのコンパイル
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShader =
        "#version 100\n"
        "attribute vec3 position;"
        "attribute vec2 uv;"
        "varying vec2 vuv;"
        "void main(void){"
        "    gl_Position = vec4(position, 1.0);"
        "    vuv = uv;"
        "}";
    glShaderSource(vertexShaderId, 1, &vertexShader, nullptr);
    glCompileShader(vertexShaderId);

    //フラグメントシェーダのコンパイル
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShader =
        "#version 100\n"
        "precision mediump float;"
        "varying vec2 vuv;"
        "uniform sampler2D texture;"
        "uniform vec4 baseColor;"
        "void main(void){"
        "    gl_FragColor = texture2D(texture, vuv) * baseColor;"
        "}";
    glShaderSource(fragmentShaderId, 1, &fragmentShader, nullptr);
    glCompileShader(fragmentShaderId);

    //プログラムオブジェクトの作成
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    // リンク
    glLinkProgram(programId);

    glUseProgram(programId);

    return programId;
}

void LAppMinimumDelegate::StartRandomMotion()
{
    LAppMinimumLive2DManager::GetInstance()->GetModel()->StartRandomMotion();
}

void LAppMinimumDelegate::StartMotion(Csm::csmInt32 index)
{
    LAppMinimumLive2DManager::GetInstance()->GetModel()->StartOrderMotion(MotionGroupIdle,index,PriorityIdle);
}

void LAppMinimumDelegate::ParameterResetCount()
{
    if (_isSecondCount)
    {
        if (_deltaTimeCount > 1.0f)
        {
            _isSecondCount = false;
            _deltaTimeCount = 0.0f;
        }

        _deltaTimeCount += LAppPal::GetDeltaTime();
    }
}
