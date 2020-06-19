#include "system_ios.h"

#if defined(PLATFORM_IOS)

#include <common/event_system.h>

using namespace Platform;

// SkyDelegate

@implementation SkyDelegate

- (void)runMain
{
    sky_main();
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    [self performSelectorOnMainThread:@selector(runMain) withObject:nil waitUntilDone:NO];
    return YES;
}

- (void)emitTouchEvent:(NSSet*)touches withType:(Platform::Touch::Event::Type)type
{
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

@interface ViewController : UIViewController<UITextFieldDelegate>

@end

@implementation ViewController

-(void)textFieldDidChange:(UITextField*)textField
{
    EVENT->emit(System::VirtualKeyboardTextChanged({ std::string(textField.text.UTF8String) }));
}

-(BOOL)textFieldShouldReturn:(UITextField *)textField
{
    EVENT->emit(System::VirtualKeyboardEnterPressed());
    return NO;
}

@end

// entry point

int main(int argc, char * argv[])
{
    @autoreleasepool {
        // AppDelegate must be implemented in the project
        return UIApplicationMain(argc, argv, nil, @"AppDelegate");
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
    
    auto rootViewController = [[ViewController alloc] init];
    [Window setRootViewController: rootViewController];
    
    auto rootView = [[UIView alloc] initWithFrame:Window.frame];
    [rootView setAutoresizingMask:UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight];
    [rootViewController setView:rootView];
    
    mTextField = [[UITextField alloc] init];
    [mTextField setDelegate:rootViewController];
    [mTextField setKeyboardType:UIKeyboardTypeDefault];
    [mTextField setReturnKeyType:UIReturnKeyDone];
    [mTextField setSmartInsertDeleteType:UITextSmartInsertDeleteTypeYes];
    [mTextField addTarget:rootViewController action:@selector(textFieldDidChange:) forControlEvents:UIControlEventEditingChanged];
    [rootView addSubview:mTextField];
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

std::string SystemIos::getVirtualKeyboardText() const
{
    return [[mTextField text] UTF8String];
}

void SystemIos::setVirtualKeyboardText(const std::string& text)
{
    [mTextField setText:[NSString stringWithUTF8String:text.c_str()]];
    EVENT->emit(System::VirtualKeyboardTextChanged({ text }));
}

void SystemIos::refreshDimensions()
{
    auto screen = [UIScreen mainScreen];

    auto prev_width = mWidth;
    auto prev_height = mHeight;
    
    mWidth = static_cast<int>(screen.bounds.size.width);
    mHeight = static_cast<int>(screen.bounds.size.height);
    
    mScale = screen.scale;
    
    mWidth *= mScale;
    mHeight *= mScale;
    
    if (prev_width != mWidth || prev_height != mHeight)
    {
        EVENT->emit(ResizeEvent({ mWidth, mHeight }));
    }
}

std::string SystemIos::getAppFolder() const
{
    return std::string([NSHomeDirectory() UTF8String]) + "/Documents/";
}

#endif
