#include "system_ios.h"

#if defined(PLATFORM_IOS)

#include <common/event_system.h>

using namespace Platform;

static UIWindow* gWindow = nullptr;

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

- (void)applicationWillResignActive:(UIApplication *)application
{
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
}

- (void)applicationWillTerminate:(UIApplication *)application
{
}

- (void)emitTouchEvent:(NSSet*)touches withType:(Input::Touch::Event::Type)type
{
	for (UITouch* touch in touches)
	{
		auto location = [touch locationInView:gWindow];
		auto x = location.x * PLATFORM->getScale();
		auto y = location.y * PLATFORM->getScale();
		EVENT->emit(Input::Touch::Event{
			.type = type,
			.pos = { x, y }
		});
		break;
	}
}

-(void)touchesBegan:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
	[self emitTouchEvent:touches withType:Input::Touch::Event::Type::Begin];
}

- (void)touchesMoved:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
	[self emitTouchEvent:touches withType:Input::Touch::Event::Type::Continue];
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
	[self emitTouchEvent:touches withType:Input::Touch::Event::Type::End];
}

- (void)touchesEnded:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
	[self emitTouchEvent:touches withType:Input::Touch::Event::Type::End];
}
@end

@interface ViewController : UIViewController<UITextFieldDelegate, SKProductsRequestDelegate>

@end

@implementation ViewController

-(bool)prefersHomeIndicatorAutoHidden
{
	return NO;
}

-(UIRectEdge)preferredScreenEdgesDeferringSystemGestures
{
	return UIRectEdgeBottom;
}

// UITextFieldDelegate

-(void)textFieldDidChange:(UITextField*)textField
{
	EVENT->emit(System::VirtualKeyboardTextChanged({ std::string(textField.text.UTF8String) }));
}

-(BOOL)textFieldShouldReturn:(UITextField *)textField
{
	EVENT->emit(System::VirtualKeyboardEnterPressed());
	return NO;
}

// SKProductsRequestDelegate

- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
	//assert(false);
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

	gWindow = [[UIWindow alloc]initWithFrame:bounds];
	[gWindow makeKeyAndVisible];

	for (UIGestureRecognizer* recognizer in gWindow.gestureRecognizers)
	{
		recognizer.delaysTouchesBegan = false;
		recognizer.delaysTouchesEnded = false;
		recognizer.cancelsTouchesInView = false;
		recognizer.enabled = false;
	}
	
	auto rootViewController = [[ViewController alloc] init];
	[gWindow setRootViewController: rootViewController];
	
	auto rootView = [[UIView alloc] initWithFrame:gWindow.frame];
	[rootView setAutoresizingMask:UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight];
	[rootViewController setView:rootView];
	
	mTextField = [[UITextField alloc] init];
	[mTextField setDelegate:rootViewController];
	[mTextField setKeyboardType:UIKeyboardTypeDefault];
	[mTextField setReturnKeyType:UIReturnKeyDone];
	[mTextField setSmartInsertDeleteType:UITextSmartInsertDeleteTypeYes];
	[mTextField addTarget:rootViewController action:@selector(textFieldDidChange:) forControlEvents:UIControlEventEditingChanged];
	[rootView addSubview:mTextField];
		
	refreshDimensions(); // we need mWidth & mHeight to be initialized here, but ResizeEvent may be a mistake here
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

std::string SystemIos::getUUID() const
{
	return UIDevice.currentDevice.identifierForVendor.UUIDString.UTF8String;
}

void* SystemIos::getWindow() const
{
	return gWindow;
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

	auto safeArea = [gWindow safeAreaInsets];
	
	mSafeAreaTopMargin = safeArea.top * mScale;
	mSafeAreaBottomMargin = safeArea.bottom * mScale;
	mSafeAreaLeftMargin = safeArea.left * mScale;
	mSafeAreaRightMargin = safeArea.right * mScale;
	
	if (prev_width != mWidth || prev_height != mHeight)
	{
		EVENT->emit(ResizeEvent({ mWidth, mHeight }));
	}
}

void SystemIos::initializeBilling(const ProductsMap& _products)
{
	products = _products;
	
	auto viewController = (ViewController*)[gWindow rootViewController];
	auto names = [[NSMutableSet alloc] init];
	
	for (auto [_name, callback] : products)
	{
		auto name = [NSString stringWithUTF8String:_name.c_str()];
		[names addObject:name];
	}
	
	auto productsRequest = [[SKProductsRequest alloc] initWithProductIdentifiers:names];
	productsRequest.delegate = viewController;
	[productsRequest start];
}

void SystemIos::purchase(const std::string& product)
{
}

void SystemIos::haptic(HapticType hapticType)
{
	if (hapticType == HapticType::Low)
	{
		auto generator = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];
		[generator impactOccurred];
	}
	else if (hapticType == HapticType::Medium)
	{
		auto generator = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleMedium];
		[generator impactOccurred];
	}
	else if (hapticType == HapticType::High)
	{
		auto generator = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleHeavy];
		[generator impactOccurred];
	}
}

#endif
