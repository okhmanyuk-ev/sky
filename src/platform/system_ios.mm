#include "system_ios.h"

#if defined(PLATFORM_IOS)

#include <common/event_system.h>

using namespace Platform;

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

- (void)emitTouchEvent:(UITouch*)touch withType:(Platform::Touch::Event::Type)type {
    auto location = [touch locationInView:SystemIos::Window];
    auto e = Platform::Touch::Event();
    e.type = type;
    e.x = location.x * PLATFORM->getScale();
    e.y = location.y * PLATFORM->getScale();
    EVENT->emit(e);
}

-(void)touchesBegan:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
    for (auto touch in touches)
    {
        [self emitTouchEvent:touch withType:Platform::Touch::Event::Type::Begin];
        break;
    }
}

- (void)touchesMoved:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
    for (auto touch in touches)
    {
        [self emitTouchEvent:touch withType:Platform::Touch::Event::Type::Continue];
        break;
    }
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
    for (auto touch in touches)
    {
        [self emitTouchEvent:touch withType:Platform::Touch::Event::Type::End];
        break;
    }
}

- (void)touchesEnded:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
    for (auto touch in touches)
    {
        [self emitTouchEvent:touch withType:Platform::Touch::Event::Type::End];
        break;
    }
}
@end

int main(int argc, char * argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}

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
        
    TextField = [[UITextField alloc] init];
    TextField.hidden = YES;
    TextField.keyboardType = UIKeyboardTypeDefault;
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
    [TextField becomeFirstResponder];
}

void SystemIos::hideVirtualKeyboard()
{
    [TextField resignFirstResponder];
}

bool SystemIos::isVirtualKeyboardOpened() const
{
    return [TextField isFirstResponder];
}

void SystemIos::refreshDimensions()
{
    auto screen = [UIScreen mainScreen];
    auto bounds = [screen bounds];
    
    mWidth = static_cast<int>(bounds.size.width);
    mHeight = static_cast<int>(bounds.size.height);
    mScale = screen.scale;
    mWidth *= mScale;
    mHeight *= mScale;
}

#endif