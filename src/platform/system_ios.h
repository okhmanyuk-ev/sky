#pragma once

#include <platform/system.h>

#if defined(PLATFORM_IOS)
#include <platform/low_level_api.h>

namespace Platform
{
    class SystemIos : public System
    {
    public:
        SystemIos(const std::string& appname);
        ~SystemIos();
        
    public:
        void process() override;
        void quit() override { /* nothing */ }

        bool isFinished() const override { return false; }
        
        int getWidth() const override { return mWidth; }
        int getHeight() const override { return mHeight; }
        
        float getScale() const override { return mScale; }
        void setScale(float value) override { /* nothing */ }
        
        bool isKeyPressed(Keyboard::Key key) const override { return false; }
        bool isKeyPressed(Mouse::Button key) const override { return false; }
        
        void resize(int width, int height) override { /* nothing */ }
        void setTitle(const std::string& text) override { /* nothing */ }
        void hideCursor() override { /* nothing */ }
        void showCursor() override { /* nothing */ }
        void setCursorPos(int x, int y) override { /* nothing */ }
        
        std::string getAppName() const override { return mAppName; }
        std::string getAppFolder() const override { return ""; }
        
        void showVirtualKeyboard() override;
        void hideVirtualKeyboard() override;
        bool isVirtualKeyboardOpened() const override;
        
        void initializeBilling(const ProductsMap& products) override { /*nothing*/ }
        void purchase(const std::string& product) override { /*nothing*/ }

    public:
        void setSize(int w, int h) { mWidth = w; mHeight = h; }
        
    public:
        static inline UIWindow* Window = nullptr;
        static inline UITextField* TextField = nullptr;

    private:
        int mWidth = 0;
        int mHeight = 0;
        float mScale = 0.0f;
        std::string mAppName;
    };
}
#endif