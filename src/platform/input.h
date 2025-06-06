#pragma once

#include <glm/glm.hpp>

namespace Platform::Input
{
	namespace Keyboard
	{
		enum class Key
		{
			None = 0,

			Backspace = 0x8,
			Tab = 0x9,

			Enter = 0xd,

		//	Shift = 16,
		//	Ctrl = 17,
		//	Alt = 18,

			Pause = 0x13,
			CapsLock = 0x14,
			Kana = 0x15,

			Kanji = 0x19,

			Escape = 0x1b,
			ImeConvert = 0x1c,
			ImeNoConvert = 0x1d,

			Space = 0x20,
			PageUp = 0x21,
			PageDown = 0x22,
			End = 0x23,
			Home = 0x24,
			Left = 0x25,
			Up = 0x26,
			Right = 0x27,
			Down = 0x28,
			Select = 0x29,
			Print = 0x2a,
			Execute = 0x2b,
			PrintScreen = 0x2c,
			Insert = 0x2d,
			Delete = 0x2e,
			Help = 0x2f,
			D0 = 0x30,
			D1 = 0x31,
			D2 = 0x32,
			D3 = 0x33,
			D4 = 0x34,
			D5 = 0x35,
			D6 = 0x36,
			D7 = 0x37,
			D8 = 0x38,
			D9 = 0x39,

			A = 0x41,
			B = 0x42,
			C = 0x43,
			D = 0x44,
			E = 0x45,
			F = 0x46,
			G = 0x47,
			H = 0x48,
			I = 0x49,
			J = 0x4a,
			K = 0x4b,
			L = 0x4c,
			M = 0x4d,
			N = 0x4e,
			O = 0x4f,
			P = 0x50,
			Q = 0x51,
			R = 0x52,
			S = 0x53,
			T = 0x54,
			U = 0x55,
			V = 0x56,
			W = 0x57,
			X = 0x58,
			Y = 0x59,
			Z = 0x5a,
			LeftWindows = 0x5b,
			RightWindows = 0x5c,
			Apps = 0x5d,

			Sleep = 0x5f,
			NumPad0 = 0x60,
			NumPad1 = 0x61,
			NumPad2 = 0x62,
			NumPad3 = 0x63,
			NumPad4 = 0x64,
			NumPad5 = 0x65,
			NumPad6 = 0x66,
			NumPad7 = 0x67,
			NumPad8 = 0x68,
			NumPad9 = 0x69,
			Multiply = 0x6a,
			Add = 0x6b,
			Separator = 0x6c,
			Subtract = 0x6d,

			Decimal = 0x6e,
			Divide = 0x6f,
			F1 = 0x70,
			F2 = 0x71,
			F3 = 0x72,
			F4 = 0x73,
			F5 = 0x74,
			F6 = 0x75,
			F7 = 0x76,
			F8 = 0x77,
			F9 = 0x78,
			F10 = 0x79,
			F11 = 0x7a,
			F12 = 0x7b,
		/*	F13 = 0x7c,
			F14 = 0x7d,
			F15 = 0x7e,
			F16 = 0x7f,
			F17 = 0x80,
			F18 = 0x81,
			F19 = 0x82,
			F20 = 0x83,
			F21 = 0x84,
			F22 = 0x85,
			F23 = 0x86,
			F24 = 0x87,

			NumLock = 0x90,
			Scroll = 0x91,*/

			LeftShift = 0xa0,
			RightShift = 0xa1,
			LeftCtrl = 0xa2,
			RightCtrl = 0xa3,
			LeftAlt = 0xa4,
			RightAlt = 0xa5,
		/*	BrowserBack = 0xa6,
			BrowserForward = 0xa7,
			BrowserRefresh = 0xa8,
			BrowserStop = 0xa9,
			BrowserSearch = 0xaa,
			BrowserFavorites = 0xab,
			BrowserHome = 0xac,
			VolumeMute = 0xad,
			VolumeDown = 0xae,
			VolumeUp = 0xaf,
			MediaNextTrack = 0xb0,
			MediaPreviousTrack = 0xb1,
			MediaStop = 0xb2,
			MediaPlayPause = 0xb3,
			LaunchMail = 0xb4,
			SelectMedia = 0xb5,
			LaunchApplication1 = 0xb6,
			LaunchApplication2 = 0xb7,

			Semicolon = 0xba,
			Plus = 0xbb,
			Comma = 0xbc,
			Minus = 0xbd,
			Period = 0xbe,
			Question = 0xbf, */
			Tilde = 0xc0,

		/*	OpenBrackets = 0xdb,
			Pipe = 0xdc,
			CloseBrackets = 0xdd,
			Quotes = 0xde,
			Oem8 = 0xdf,

			Backslash = 0xe2,

			ProcessKey = 0xe5,

			Copy = 0xf2,
			Auto = 0xf3,
			EnlW = 0xf4,

			Attn = 0xf6,
			Crsel = 0xf7,
			Exsel = 0xf8,
			EraseEof = 0xf9,
			Play = 0xfa,
			Zoom = 0xfb,

			Pa1 = 0xfd,
			Clear = 0xfe,*/
		};

		struct Event
		{
			enum class Type
			{
				Pressed,
				Repeat,
				Released
			};

			Type type;              ///< The event type
			Key  key;               ///< The last key that was pressed/released
		};

		struct CharEvent
		{
			char32_t codepoint;
		};
	}

	namespace Mouse
	{
		enum class Button
		{
			Left,
			Middle,
			Right
		};

		struct ButtonEvent
		{
			enum class Type
			{
				Pressed,
				Released
			};

			Type type;
			Button button;
			glm::ivec2 pos;
		};

		struct MoveEvent
		{
			glm::ivec2 pos;
		};

		struct ScrollEvent
		{
			glm::ivec2 pos;
			glm::vec2 scroll;
		};
	}

	namespace Touch
	{
		struct Event
		{
			enum class Type
			{
				Begin,
				Continue,
				End
			};

			Type type;
			glm::ivec2 pos;
		};
	}
}
