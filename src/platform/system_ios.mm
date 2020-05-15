#include "system_ios.h"

#if defined(PLATFORM_IOS)

#include <common/event_system.h>

using namespace Platform;

// UIApplicationDelegate

@interface AppDelegate : UIResponder<UIApplicationDelegate>

@end

@implementation AppDelegate

- (void)runMain {
    sky_main();
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    [self performSelectorOnMainThread:@selector(runMain) withObject:nil waitUntilDone:NO];
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
}

- (void)applicationWillTerminate:(UIApplication *)application {
}

- (void)emitTouchEvent:(NSSet*)touches withType:(Platform::Touch::Event::Type)type {
    for (UITouch* touch in touches)
    {
        auto location = [touch locationInView:SystemIos::Window];
        auto e = Platform::Touch::Event();
        e.type = type;
        e.x = location.x * PLATFORM->getScale();
        e.y = location.y * PLATFORM->getScale();
        EVENT->emit(e);
        break;
    }
}

-(void)touchesBegan:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
    [self emitTouchEvent:touches withType:Platform::Touch::Event::Type::Begin];
}

- (void)touchesMoved:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
    [self emitTouchEvent:touches withType:Platform::Touch::Event::Type::Continue];
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
    [self emitTouchEvent:touches withType:Platform::Touch::Event::Type::End];
}

- (void)touchesEnded:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
    [self emitTouchEvent:touches withType:Platform::Touch::Event::Type::End];
}
@end

// entry point

int main(int argc, char * argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}

// ios system

std::shared_ptr<System> System::create(const std::string& appname)
{
    return std::make_shared<SystemIos>(appname);
}

SystemIos::SystemIos(const std::string& appname) : mAppName(appname)
{
    auto screen = [UIScreen mainScreen];
    auto bounds = [screen bounds];

    Window = [[UIWindow alloc]initWithFrame:bounds];
    [Window makeKeyAndVisible];
    
    [Window setRootViewController: [[UIViewController alloc] init]];
    
    mTextField = [[UITextField alloc] init];
    [Window addSubview:mTextField];
    
    /*-(void)textFieldDidChange:(UITextField *) textField
    {
        auto c = textField.text;
        textField.text = @"";
        auto e = Platform::Keyboard::Event();
        
        e.asciiChar = std::string([c UTF8String])[0];
        e.key = Platform::Keyboard::Key::None;

        e.type = Platform::Keyboard::Event::Type::Pressed;
        EVENT->emit(e);
        
        e.type = Platform::Keyboard::Event::Type::Released;
        EVENT->emit(e);
    }*/
}

SystemIos::~SystemIos()
{
    //
}

void SystemIos::process()
{
    SInt32 result;
    do {
        result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, TRUE);
    } while (result == kCFRunLoopRunHandledSource);
    
    refreshDimensions();
}

void SystemIos::showVirtualKeyboard()
{
    [mTextField becomeFirstResponder];
}

void SystemIos::hideVirtualKeyboard()
{
    [mTextField resignFirstResponder];
}

bool SystemIos::isVirtualKeyboardOpened() const
{
    return [mTextField isFirstResponder];
}

void SystemIos::refreshDimensions()
{
    auto screen = [UIScreen mainScreen];
    auto bounds = [screen bounds];
    
    auto prev_width = mWidth;
    auto prev_height = mHeight;
    
    mWidth = static_cast<int>(bounds.size.width);
    mHeight = static_cast<int>(bounds.size.height);
    mScale = screen.scale;
    mWidth *= mScale;
    mHeight *= mScale;
    
    if (prev_width != mWidth || prev_height != mHeight)
    {
        EVENT->emit(ResizeEvent({ mWidth, mHeight }));
    }
}

#endif
