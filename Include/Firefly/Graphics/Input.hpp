#pragma once

#include <cstdint>

namespace Firefly
{
	/**
	 * Key enum.
	 * This contains all the supported keyboard keys and can be used to request for the state.
	 */
	enum class Key : uint8_t
	{
		Unknown,
		Space,
		Apostrophe,
		Comma,
		Minus,
		Preiod,
		Slash,
		Zero,
		One,
		Two,
		Three,
		Four,
		Five,
		Six,
		Seven,
		Eight,
		Nine,
		Semicolon,
		Equal,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		LeftBracket,
		RightBracket,
		Backslash,
		GraveAccent,
		WordOne,
		WordTwo,
		Escape,
		Enter,
		Tab,
		Backspace,
		Insert,
		Delete,
		Right,
		Left,
		Down,
		Up,
		PageUp,
		PageDowm,
		Home,
		End,
		CapsLock,
		ScrollLock,
		NumLock,
		PrintScreen,
		Pause,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,
		F21,
		F22,
		F23,
		F24,
		F25,
		KeyPadZero,
		KeyPadOne,
		KeyPadTwo,
		KeyPadThree,
		KeyPadFour,
		KeyPadFive,
		KeyPadSix,
		KeyPadSeven,
		KeyPadEight,
		KeyPadNine,
		KeyPadDecimal,
		KeyPadDivide,
		KeyPadMultiply,
		KeyPadSubtract,
		KeyPadAdd,
		KeyPadEqual,
		KeyPadEnter,
		LeftShift,
		LeftControl,
		LeftAlt,
		LeftSuper,
		RightShift,
		RightControl,
		RightAlt,
		RightSuper,
		Menu,

		KeyMax
	};

	/**
	 * State enum.
	 * This enum specifies the input state.
	 */
	enum class InputState : uint8_t
	{
		Released,
		Pressed,
		OnRepeat
	};

	/**
	 * Special characters enum.
	 */
	enum class SpecialKey : uint8_t {
		None = 0,
		Shift = 1 << 0,
		Control = 1 << 1,
		Alt = 1 << 2,
		Super = 1 << 3,
		CapsLock = 1 << 4,
		NumLock = 1 << 5
	};

	/**
	 * Button enum.
	 */
	enum class Button : uint8_t
	{
		Last, Left,
		Right, Middle,

		One = Left, Two = Right, Three = Middle,
		Four, Five, Six, Seven, Eight,

		ButtonMax
	};

	struct BasicInput
	{
		/**
		 * Constructor.
		 *
		 * @param state The input state.
		 * @param special The special keys pressed with it.
		 */
		explicit BasicInput(const InputState state, const uint8_t special) : m_State(state), m_SpecialKey(special) {}

		/**
		 * Check if the key is released.
		 *
		 * @return The boolean value stating if its released.
		 */
		bool isReleased() const { return m_State == InputState::Released; }

		/**
		 * Check if the key is pressed.
		 *
		 * @return The boolean value stating if its pressed.
		 */
		bool isPressed() const { return m_State == InputState::Pressed; }

		/**
		 * Check if the key is on repeat.
		 *
		 * @return The boolean value stating if its on repeat.
		 */
		bool isOnRepeat() const { return m_State == InputState::OnRepeat; }

		/**
		 * Check if shift was pressed.
		 *
		 * @return Boolean value stating true or not.
		 */
		bool shiftPressed() const { return m_SpecialKey & static_cast<uint8_t>(SpecialKey::Shift); }

		/**
		 * Check if control was pressed.
		 *
		 * @return Boolean value stating true or not.
		 */
		bool controlPressed() const { return m_SpecialKey & static_cast<uint8_t>(SpecialKey::Control); }

		/**
		 * Check if alt was pressed.
		 *
		 * @return Boolean value stating true or not.
		 */
		bool altPressed() const { return m_SpecialKey & static_cast<uint8_t>(SpecialKey::Alt); }

		/**
		 * Check if super was pressed.
		 *
		 * @return Boolean value stating true or not.
		 */
		bool superPressed() const { return m_SpecialKey & static_cast<uint8_t>(SpecialKey::Super); }

		/**
		 * Check if caps lock was pressed.
		 *
		 * @return Boolean value stating true or not.
		 */
		bool capsLockPressed() const { return m_SpecialKey & static_cast<uint8_t>(SpecialKey::CapsLock); }

		/**
		 * Check if numlock was pressed.
		 *
		 * @return Boolean value stating true or not.
		 */
		bool numLockPressed() const { return m_SpecialKey & static_cast<uint8_t>(SpecialKey::NumLock); }

	private:
		const InputState m_State = InputState::Released;
		const uint8_t m_SpecialKey = 0;
	};

	/**
	 * Key input
	 * This struct specifies the key input state when an input is submitted.
	 */
	struct KeyInput final : public BasicInput
	{
		/**
		 * Constructor.
		 *
		 * @param key The key that got the event.
		 * @param state The input state.
		 * @param special The special keys pressed with it.
		 */
		explicit KeyInput(const Key key, const InputState state, const uint8_t special) : BasicInput(state, special), m_Key(key) {}

		/**
		 * Get the key.
		 *
		 * @return The key.
		 */
		Key getKey() const { return m_Key; }

	private:
		const Key m_Key = Key::Unknown;
	};

	/**
	 * Mouse input struct.
	 * This struct contains all the mouse input information.
	 */
	struct MouseInput final : public BasicInput
	{
		/**
		 * Constructor.
		 *
		 * @param button The button to which the input was given.
		 * @param state The input state.
		 * @param special The special keys pressed with it.
		 */
		explicit MouseInput(const Button button, const InputState state, const uint8_t special) : BasicInput(state, special), m_Button(button) {}

		/**
		 * Get the button.
		 *
		 * @return The button.
		 */
		Button getButton() const { return m_Button; }

	private:
		const Button m_Button = Button::One;
	};
}