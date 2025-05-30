// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CGUICustomTabControl.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include <IGUIButton.h>
#include <IGUISkin.h>
#include <IGUIEnvironment.h>
#include <IGUIFont.h>
#include <IVideoDriver.h>
#include <rect.h>

namespace irr {
namespace gui {

//! constructor
CGUICustomTabControl::CGUICustomTabControl(IGUIEnvironment* environment,
										   IGUIElement* parent, const core::rect<s32>& rectangle,
										   bool fillbackground, bool border, s32 id)
	: IGUITabControl(environment, parent, id, rectangle), ActiveTab(-1),
	Border(border), FillBackground(fillbackground), ScrollControl(false), TabHeight(0), VerticalAlignment(EGUIA_UPPERLEFT),
	UpButton(0), DownButton(0), TabMaxWidth(0), CurrentScrollTabIndex(0), TabExtraWidth(20) {
#ifdef _DEBUG
	setDebugName("CGUICustomTabControl");
#endif

	IGUISkin* skin = Environment->getSkin();
	IGUISpriteBank* sprites = 0;

	TabHeight = 32;

	if(skin) {
		sprites = skin->getSpriteBank();
		TabHeight = skin->getSize(gui::EGDS_BUTTON_HEIGHT) + 2;
	}

	UpButton = Environment->addButton(core::rect<s32>(0, 0, 10, 10), this);

	if(UpButton) {
		UpButton->setSpriteBank(sprites);
		UpButton->setVisible(false);
		UpButton->setSubElement(true);
		UpButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
		UpButton->setOverrideFont(Environment->getBuiltInFont());
		UpButton->grab();
	}

	DownButton = Environment->addButton(core::rect<s32>(0, 0, 10, 10), this);

	if(DownButton) {
		DownButton->setSpriteBank(sprites);
		DownButton->setVisible(false);
		DownButton->setSubElement(true);
		DownButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
		DownButton->setOverrideFont(Environment->getBuiltInFont());
		DownButton->grab();
	}

	setTabVerticalAlignment(EGUIA_UPPERLEFT);
	refreshSprites();
}

IGUITabControl* CGUICustomTabControl::addCustomTabControl(IGUIEnvironment * environment, const core::rect<s32>& rectangle, IGUIElement * parent, bool fillbackground, bool border, s32 id) {
	if(!parent)
		parent = environment->getRootGUIElement();
	IGUITabControl* obj = new CGUICustomTabControl(environment, parent, rectangle, fillbackground, border, id);
	obj->drop();
	return obj;
}

//! destructor
CGUICustomTabControl::~CGUICustomTabControl() {
	for(u32 i = 0; i < Tabs.size(); ++i) {
		if(Tabs[i])
			Tabs[i]->drop();
	}

	if(UpButton)
		UpButton->drop();

	if(DownButton)
		DownButton->drop();
}

void CGUICustomTabControl::refreshSprites() {
	video::SColor color(255, 255, 255, 255);
	IGUISkin* skin = Environment->getSkin();
	if(skin) {
		color = skin->getColor(isEnabled() ? EGDC_WINDOW_SYMBOL : EGDC_GRAY_WINDOW_SYMBOL);
	}

	if(UpButton) {
		UpButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_LEFT), color);
		UpButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_LEFT), color);
	}

	if(DownButton) {
		DownButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_RIGHT), color);
		DownButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_RIGHT), color);
	}
}

//! Adds a tab
IGUITab* CGUICustomTabControl::addTab(const wchar_t* caption, s32 id) {
#if IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9
	CGUITab* tab = new CGUITab(Environment, this, calcTabPos(), id);
#else
	CGUITab* tab = new CGUITab(Tabs.size(), Environment, this, calcTabPos(), id);
#endif

	tab->setText(caption);
	tab->setAlignment(EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
	tab->setVisible(false);
	Tabs.push_back(tab);

	if(ActiveTab == -1) {
		ActiveTab = 0;
		tab->setVisible(true);
	}

	recalculateScrollBar();

	return tab;
}


#if IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9
#else
//! adds a tab which has been created elsewhere
void CGUICustomTabControl::addTab(CGUITab* tab) {
	if(!tab)
		return;

	// check if its already added
	for(u32 i = 0; i < Tabs.size(); ++i) {
		if(Tabs[i] == tab)
			return;
	}

	tab->grab();

	if(tab->getNumber() == -1)
		tab->setNumber((s32)Tabs.size());

	while(tab->getNumber() >= (s32)Tabs.size())
		Tabs.push_back(0);

	if(Tabs[tab->getNumber()]) {
		Tabs.push_back(Tabs[tab->getNumber()]);
		Tabs[Tabs.size() - 1]->setNumber(Tabs.size());
	}
	Tabs[tab->getNumber()] = tab;

	if(ActiveTab == -1)
		ActiveTab = tab->getNumber();


	if(tab->getNumber() == ActiveTab) {
		setActiveTab(ActiveTab);
	}
}
#endif

//! Insert the tab at the given index
IGUITab* CGUICustomTabControl::insertTab(s32 idx, const wchar_t* caption, s32 id) {
	if(idx < 0 || idx >(s32)Tabs.size())	// idx == Tabs.size() is indeed ok here as core::array can handle that
		return NULL;

#if IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9
	CGUITab* tab = new CGUITab(Environment, this, calcTabPos(), id);
#else
	CGUITab* tab = new CGUITab(idx, Environment, this, calcTabPos(), id);
#endif

	tab->setText(caption);
	tab->setAlignment(EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
	tab->setVisible(false);
	Tabs.insert(tab, (u32)idx);

	if(ActiveTab == -1) {
		ActiveTab = 0;
		tab->setVisible(true);
	}

#if !(IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9)
	for(u32 i = (u32)idx + 1; i < Tabs.size(); ++i) {
		Tabs[i]->setNumber(i);
	}
#endif

	recalculateScrollBar();

	return tab;
}

//! Removes a tab from the tabcontrol
void CGUICustomTabControl::removeTab(s32 idx) {
	if(idx < 0 || idx >= (s32)Tabs.size())
		return;

	Tabs[(u32)idx]->drop();
	Tabs.erase((u32)idx);
#if !(IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9)
	for(u32 i = (u32)idx; i < Tabs.size(); ++i) {
		Tabs[i]->setNumber(i);
	}
#endif
}

//! Clears the tabcontrol removing all tabs
void CGUICustomTabControl::clear() {
	for(u32 i = 0; i < Tabs.size(); ++i) {
		if(Tabs[i])
			Tabs[i]->drop();
	}
	Tabs.clear();
}

//! Returns amount of tabs in the tabcontrol
s32 CGUICustomTabControl::getTabCount() const {
	return Tabs.size();
}


//! Returns a tab based on zero based index
IGUITab* CGUICustomTabControl::getTab(s32 idx) const {
	if((u32)idx >= Tabs.size())
		return 0;

	return Tabs[idx];
}


//! called if an event happened.
bool CGUICustomTabControl::OnEvent(const SEvent& event) {
	if(isEnabled()) {

		switch(event.EventType) {
			case EET_GUI_EVENT:
				switch(event.GUIEvent.EventType) {
					case EGET_BUTTON_CLICKED:
						if(event.GUIEvent.Caller == UpButton) {
							scrollLeft();
							return true;
						} else if(event.GUIEvent.Caller == DownButton) {
							scrollRight();
							return true;
						}

						break;
					default:
						break;
				}
				break;
			case EET_MOUSE_INPUT_EVENT:
				switch(event.MouseInput.Event) {
					case EMIE_LMOUSE_PRESSED_DOWN:
					{
						s32 idx = getTabAt(event.MouseInput.X, event.MouseInput.Y);
						if(idx >= 0)
							return true;
						// todo: dragging tabs around
						break;
					}
					case EMIE_LMOUSE_LEFT_UP:
					{
						s32 idx = getTabAt(event.MouseInput.X, event.MouseInput.Y);
						if(idx >= 0) {
							setActiveTab(idx);
							return true;
						}
						break;
					}
					default:
						break;
				}
				break;
			default:
				break;
		}
	}

	return IGUIElement::OnEvent(event);
}


void CGUICustomTabControl::scrollLeft() {
	if(CurrentScrollTabIndex > 0)
		--CurrentScrollTabIndex;
	recalculateScrollBar();
}


void CGUICustomTabControl::scrollRight() {
	if(CurrentScrollTabIndex < (s32)(Tabs.size()) - 1) {
		if(needScrollControl(CurrentScrollTabIndex, true))
			++CurrentScrollTabIndex;
	}
	recalculateScrollBar();
}

s32 CGUICustomTabControl::calcTabWidth(s32 pos, IGUIFont* font, const wchar_t* text, bool withScrollControl) const {
	if(!font)
		return 0;

	s32 len = font->getDimension(text).Width + TabExtraWidth;
	if(TabMaxWidth > 0 && len > TabMaxWidth)
		len = TabMaxWidth;

	// check if we miss the place to draw the tab-button
	if(withScrollControl && ScrollControl && pos + len > UpButton->getAbsolutePosition().UpperLeftCorner.X - 2) {
		s32 tabMinWidth = font->getDimension(L"A").Width;
		if(TabExtraWidth > 0 && TabExtraWidth > tabMinWidth)
			tabMinWidth = TabExtraWidth;

		if(ScrollControl && pos + tabMinWidth <= UpButton->getAbsolutePosition().UpperLeftCorner.X - 2) {
			len = UpButton->getAbsolutePosition().UpperLeftCorner.X - 2 - pos;
		}
	}
	return len;
}

bool CGUICustomTabControl::needScrollControl(s32 startIndex, bool withScrollControl) {
	if(startIndex >= (s32)Tabs.size())
		startIndex -= 1;

	if(startIndex < 0)
		startIndex = 0;

	IGUISkin* skin = Environment->getSkin();
	if(!skin)
		return false;

	IGUIFont* font = skin->getFont();

	core::rect<s32> frameRect(AbsoluteRect);

	if(Tabs.empty())
		return false;

	if(!font)
		return false;

	s32 pos = frameRect.UpperLeftCorner.X + 2;

	for(s32 i = startIndex; i < (s32)Tabs.size(); ++i) {
		// get Text
		const wchar_t* text = 0;
		if(Tabs[i])
			text = Tabs[i]->getText();

		// get text length
		s32 len = calcTabWidth(pos, font, text, false);	// always without withScrollControl here or len would be shortened

		frameRect.LowerRightCorner.X += len;

		frameRect.UpperLeftCorner.X = pos;
		frameRect.LowerRightCorner.X = frameRect.UpperLeftCorner.X + len;
		pos += len;

		if(withScrollControl && pos > UpButton->getAbsolutePosition().UpperLeftCorner.X - 2)
			return true;

		if(!withScrollControl && pos > AbsoluteRect.LowerRightCorner.X)
			return true;
	}

	return false;
}


core::rect<s32> CGUICustomTabControl::calcTabPos() {
	core::rect<s32> r;
	r.UpperLeftCorner.X = 0;
	r.LowerRightCorner.X = AbsoluteRect.getWidth();
	if(Border) {
		++r.UpperLeftCorner.X;
		--r.LowerRightCorner.X;
	}

	if(VerticalAlignment == EGUIA_UPPERLEFT) {
		r.UpperLeftCorner.Y = TabHeight + 2;
		r.LowerRightCorner.Y = AbsoluteRect.getHeight() - 1;
		if(Border) {
			--r.LowerRightCorner.Y;
		}
	} else {
		r.UpperLeftCorner.Y = 0;
		r.LowerRightCorner.Y = AbsoluteRect.getHeight() - (TabHeight + 2);
		if(Border) {
			++r.UpperLeftCorner.Y;
		}
	}

	return r;
}


//! draws the element and its children
void CGUICustomTabControl::draw() {
	if(!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	if(!skin)
		return;

	IGUIFont* font = skin->getFont();
	video::IVideoDriver* driver = Environment->getVideoDriver();

	core::rect<s32> frameRect(AbsoluteRect);

	if(Tabs.empty())
		driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), frameRect, &AbsoluteClippingRect);

	if(!font)
		return;

	if(VerticalAlignment == EGUIA_UPPERLEFT) {
		frameRect.UpperLeftCorner.Y += 2;
		frameRect.LowerRightCorner.Y = frameRect.UpperLeftCorner.Y + TabHeight;
	} else {
		frameRect.UpperLeftCorner.Y = frameRect.LowerRightCorner.Y - TabHeight - 1;
		frameRect.LowerRightCorner.Y -= 2;
	}

	core::rect<s32> tr;
	s32 pos = frameRect.UpperLeftCorner.X + 2;

	bool needLeftScroll = CurrentScrollTabIndex > 0;
	bool needRightScroll = false;

	// left and right pos of the active tab
	s32 left = 0;
	s32 right = 0;

	//const wchar_t* activetext = 0;
	CGUITab *activeTab = 0;

	for(u32 i = CurrentScrollTabIndex; i < Tabs.size(); ++i) {
		// get Text
		const wchar_t* text = 0;
		if(Tabs[i])
			text = Tabs[i]->getText();

		// get text length
		s32 len = calcTabWidth(pos, font, text, false);
		if(ScrollControl && pos + len > UpButton->getAbsolutePosition().UpperLeftCorner.X - 2)
			needRightScroll = true;

		len = calcTabWidth(pos, font, text, true);

		frameRect.LowerRightCorner.X += len;
		frameRect.UpperLeftCorner.X = pos;
		frameRect.LowerRightCorner.X = frameRect.UpperLeftCorner.X + len;

		pos += len;

#if !(IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9)
		if(text)
			Tabs[i]->refreshSkinColors();
#endif

		if((s32)i == ActiveTab) {
			left = frameRect.UpperLeftCorner.X;
			right = frameRect.LowerRightCorner.X;
			//activetext = text;
			activeTab = Tabs[i];
		} else {
			skin->draw3DTabButton(this, false, frameRect, &AbsoluteClippingRect, VerticalAlignment);

			// draw text
			core::rect<s32> textClipRect(frameRect);	// TODO: exact size depends on borders in draw3DTabButton which we don't get with current interface
			textClipRect.clipAgainst(AbsoluteClippingRect);
			font->draw(text, frameRect, Tabs[i]->getTextColor(),
					   true, true, &textClipRect);
		}

		if(needRightScroll)
			break;
	}

	// draw active tab
	if(left != 0 && right != 0 && activeTab != 0) {
		// draw upper highlight frame
		if(VerticalAlignment == EGUIA_UPPERLEFT) {
			frameRect.UpperLeftCorner.X = left - 2;
			frameRect.LowerRightCorner.X = right + 2;
			frameRect.UpperLeftCorner.Y -= 2;

			skin->draw3DTabButton(this, true, frameRect, &AbsoluteClippingRect, VerticalAlignment);

			// draw text
			core::rect<s32> textClipRect(frameRect);	// TODO: exact size depends on borders in draw3DTabButton which we don't get with current interface
			textClipRect.clipAgainst(AbsoluteClippingRect);
			font->draw(activeTab->getText(), frameRect, activeTab->getTextColor(),
					   true, true, &textClipRect);

			tr.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X;
			tr.LowerRightCorner.X = left - 1;
			tr.UpperLeftCorner.Y = frameRect.LowerRightCorner.Y - 1;
			tr.LowerRightCorner.Y = frameRect.LowerRightCorner.Y;
			driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), tr, &AbsoluteClippingRect);

			tr.UpperLeftCorner.X = right;
			tr.LowerRightCorner.X = AbsoluteRect.LowerRightCorner.X;
			driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), tr, &AbsoluteClippingRect);
		} else {

			frameRect.UpperLeftCorner.X = left - 2;
			frameRect.LowerRightCorner.X = right + 2;
			frameRect.LowerRightCorner.Y += 2;

			skin->draw3DTabButton(this, true, frameRect, &AbsoluteClippingRect, VerticalAlignment);

			// draw text
			font->draw(activeTab->getText(), frameRect, activeTab->getTextColor(),
					   true, true, &frameRect);

			tr.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X;
			tr.LowerRightCorner.X = left - 1;
			tr.UpperLeftCorner.Y = frameRect.UpperLeftCorner.Y - 1;
			tr.LowerRightCorner.Y = frameRect.UpperLeftCorner.Y;
			driver->draw2DRectangle(skin->getColor(EGDC_3D_DARK_SHADOW), tr, &AbsoluteClippingRect);

			tr.UpperLeftCorner.X = right;
			tr.LowerRightCorner.X = AbsoluteRect.LowerRightCorner.X;
			driver->draw2DRectangle(skin->getColor(EGDC_3D_DARK_SHADOW), tr, &AbsoluteClippingRect);
		}
	} else {
		if(VerticalAlignment == EGUIA_UPPERLEFT) {
			tr.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X;
			tr.LowerRightCorner.X = AbsoluteRect.LowerRightCorner.X;
			tr.UpperLeftCorner.Y = frameRect.LowerRightCorner.Y - 1;
			tr.LowerRightCorner.Y = frameRect.LowerRightCorner.Y;
			driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), tr, &AbsoluteClippingRect);
		} else {
			tr.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X;
			tr.LowerRightCorner.X = 1000;
			tr.UpperLeftCorner.Y = frameRect.UpperLeftCorner.Y - 1;
			tr.LowerRightCorner.Y = frameRect.UpperLeftCorner.Y;
			driver->draw2DRectangle(skin->getColor(EGDC_3D_DARK_SHADOW), tr, &AbsoluteClippingRect);
		}
	}

	skin->draw3DTabBody(this, Border, FillBackground, AbsoluteRect, &AbsoluteClippingRect, TabHeight, VerticalAlignment);

	// enable scrollcontrols on need
	if(UpButton)
		UpButton->setEnabled(needLeftScroll);
	if(DownButton)
		DownButton->setEnabled(needRightScroll);
	refreshSprites();

	IGUIElement::draw();
}


//! Set the height of the tabs
void CGUICustomTabControl::setTabHeight(s32 height) {
	if(height < 0)
		height = 0;

	TabHeight = height;

	recalculateScrollButtonPlacement();
	recalculateScrollBar();
}


//! Get the height of the tabs
s32 CGUICustomTabControl::getTabHeight() const {
	return TabHeight;
}

//! set the maximal width of a tab. Per default width is 0 which means "no width restriction".
void CGUICustomTabControl::setTabMaxWidth(s32 width) {
	TabMaxWidth = width;
}

//! get the maximal width of a tab
s32 CGUICustomTabControl::getTabMaxWidth() const {
	return TabMaxWidth;
}


//! Set the extra width added to tabs on each side of the text
void CGUICustomTabControl::setTabExtraWidth(s32 extraWidth) {
	if(extraWidth < 0)
		extraWidth = 0;

	TabExtraWidth = extraWidth;

	recalculateScrollBar();
}


//! Get the extra width added to tabs on each side of the text
s32 CGUICustomTabControl::getTabExtraWidth() const {
	return TabExtraWidth;
}


void CGUICustomTabControl::recalculateScrollBar() {
	if(!UpButton || !DownButton)
		return;

	ScrollControl = needScrollControl();

	if(ScrollControl) {
		UpButton->setVisible(true);
		DownButton->setVisible(true);
	} else {
		CurrentScrollTabIndex = 0;
		UpButton->setVisible(false);
		DownButton->setVisible(false);
	}

	bringToFront(UpButton);
	bringToFront(DownButton);
}

//! Set the alignment of the tabs
void CGUICustomTabControl::setTabVerticalAlignment(EGUI_ALIGNMENT alignment) {
	VerticalAlignment = alignment;

	recalculateScrollButtonPlacement();
	recalculateScrollBar();

	core::rect<s32> r(calcTabPos());
	for(u32 i = 0; i < Tabs.size(); ++i) {
		Tabs[i]->setRelativePosition(r);
	}
}

void CGUICustomTabControl::recalculateScrollButtonPlacement() {
	IGUISkin* skin = Environment->getSkin();
	s32 ButtonSize = 16;
	s32 ButtonHeight = TabHeight - 2;
	if(ButtonHeight < 0)
		ButtonHeight = TabHeight;
	if(skin) {
		ButtonSize = skin->getSize(EGDS_WINDOW_BUTTON_WIDTH);
		if(ButtonSize > TabHeight)
			ButtonSize = TabHeight;
	}

	s32 ButtonX = RelativeRect.getWidth() - (s32)(2.5f*(f32)ButtonSize) - 1;
	s32 ButtonY = 0;

	if(VerticalAlignment == EGUIA_UPPERLEFT) {
		ButtonY = 2 + (TabHeight / 2) - (ButtonHeight / 2);
		UpButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
		DownButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
	} else {
		ButtonY = RelativeRect.getHeight() - (TabHeight / 2) - (ButtonHeight / 2) - 2;
		UpButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT);
		DownButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT);
	}

	UpButton->setRelativePosition(core::rect<s32>(ButtonX, ButtonY, ButtonX + ButtonSize, ButtonY + ButtonHeight));
	ButtonX += ButtonSize + 1;
	DownButton->setRelativePosition(core::rect<s32>(ButtonX, ButtonY, ButtonX + ButtonSize, ButtonY + ButtonHeight));
}

//! Get the alignment of the tabs
EGUI_ALIGNMENT CGUICustomTabControl::getTabVerticalAlignment() const {
	return VerticalAlignment;
}


s32 CGUICustomTabControl::getTabAt(s32 xpos, s32 ypos) const {
	core::vector2di p(xpos, ypos);
	IGUISkin* skin = Environment->getSkin();
	IGUIFont* font = skin->getFont();

	core::rect<s32> frameRect(AbsoluteRect);

	if(VerticalAlignment == EGUIA_UPPERLEFT) {
		frameRect.UpperLeftCorner.Y += 2;
		frameRect.LowerRightCorner.Y = frameRect.UpperLeftCorner.Y + TabHeight;
	} else {
		frameRect.UpperLeftCorner.Y = frameRect.LowerRightCorner.Y - TabHeight;
	}

	s32 pos = frameRect.UpperLeftCorner.X + 2;

	if(!frameRect.isPointInside(p))
		return -1;

	for(s32 i = CurrentScrollTabIndex; i < (s32)Tabs.size(); ++i) {
		// get Text
		const wchar_t* text = 0;
		if(Tabs[i])
			text = Tabs[i]->getText();

		// get text length
		s32 len = calcTabWidth(pos, font, text, true);
		if(ScrollControl && pos + len > UpButton->getAbsolutePosition().UpperLeftCorner.X - 2)
			return -1;

		frameRect.UpperLeftCorner.X = pos;
		frameRect.LowerRightCorner.X = frameRect.UpperLeftCorner.X + len;

		pos += len;

		if(frameRect.isPointInside(p)) {
			return i;
		}
	}
	return -1;
}

//! Returns which tab is currently active
s32 CGUICustomTabControl::getActiveTab() const {
	return ActiveTab;
}


//! Brings a tab to front.
bool CGUICustomTabControl::setActiveTab(s32 idx) {
	if((u32)idx >= Tabs.size())
		return false;

	bool changed = (ActiveTab != idx);

	ActiveTab = idx;

	for(s32 i = 0; i < (s32)Tabs.size(); ++i)
		if(Tabs[i])
			Tabs[i]->setVisible(i == ActiveTab);

	if(changed) {
		if(needScrollControl()) {
			core::rect<s32> frameRect(AbsoluteRect);
			s32 pos = frameRect.UpperLeftCorner.X + 2;
			IGUIFont* font = Environment->getSkin()->getFont();
			for(s32 i = CurrentScrollTabIndex; i < idx; ++i) {
				// get Text
				const wchar_t* text = 0;
				if(Tabs[i])
					text = Tabs[i]->getText();
				// get text length
				s32 len = calcTabWidth(pos, font, text, true);
				pos += len;
			}

			s32 activepos = calcTabWidth(pos, font, Tabs[idx]->getText(), false);
			s32 buttonpos = UpButton->getAbsolutePosition().UpperLeftCorner.X - 2;

			if((pos + activepos) > buttonpos) {
				s32 i = 0;
				for(i = CurrentScrollTabIndex; i < idx; ++i) {
					// get Text
					const wchar_t* text = 0;
					if(Tabs[i])
						text = Tabs[i]->getText();
					// get text length
					s32 len = calcTabWidth(pos, font, text, true);
					pos -= len;
					if((pos + activepos) <= buttonpos)
						break;
				}
				if((pos + activepos) > buttonpos)
					CurrentScrollTabIndex = idx;
				else
					CurrentScrollTabIndex = i;
			}
		}
		SEvent event;
		event.EventType = EET_GUI_EVENT;
		event.GUIEvent.Caller = this;
		event.GUIEvent.Element = 0;
		event.GUIEvent.EventType = EGET_TAB_CHANGED;
		Parent->OnEvent(event);
	}

	return true;
}


bool CGUICustomTabControl::setActiveTab(IGUITab *tab) {
	for(s32 i = 0; i < (s32)Tabs.size(); ++i)
		if(Tabs[i] == tab)
			return setActiveTab(i);
	return false;
}

#if IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9
s32 CGUICustomTabControl::getTabIndex(const IGUIElement *tab) const
{
	for (u32 i=0; i<Tabs.size(); ++i)
		if (Tabs[i] == tab)
			return (s32)i;

	return -1;
}
#endif


//! Removes a child.
void CGUICustomTabControl::removeChild(IGUIElement* child) {
	[[maybe_unused]] bool isTab = false;

	u32 i = 0;
	// check if it is a tab
	while(i < Tabs.size()) {
		if(Tabs[i] == child) {
			Tabs[i]->drop();
			Tabs.erase(i);
			isTab = true;
		} else
			++i;
	}

#if !(IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9)
	// reassign numbers
	if(isTab) {
		for(i = 0; i < Tabs.size(); ++i)
			if(Tabs[i])
				Tabs[i]->setNumber(i);
	}
#endif

	// remove real element
	IGUIElement::removeChild(child);

	recalculateScrollBar();
}


//! Update the position of the element, decides scroll button status
void CGUICustomTabControl::updateAbsolutePosition() {
	IGUIElement::updateAbsolutePosition();
	recalculateScrollBar();
}


//! Writes attributes of the element.
void CGUICustomTabControl::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options = 0) const {
	IGUITabControl::serializeAttributes(out, options);

	out->addInt("ActiveTab", ActiveTab);
	out->addBool("Border", Border);
	out->addBool("FillBackground", FillBackground);
	out->addInt("TabHeight", TabHeight);
	out->addInt("TabMaxWidth", TabMaxWidth);
	out->addEnum("TabVerticalAlignment", s32(VerticalAlignment), GUIAlignmentNames);
}


//! Reads attributes of the element
void CGUICustomTabControl::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options = 0) {
	Border = in->getAttributeAsBool("Border");
	FillBackground = in->getAttributeAsBool("FillBackground");

	ActiveTab = -1;

	setTabHeight(in->getAttributeAsInt("TabHeight"));
	TabMaxWidth = in->getAttributeAsInt("TabMaxWidth");

	IGUITabControl::deserializeAttributes(in, options);

	setActiveTab(in->getAttributeAsInt("ActiveTab"));
	setTabVerticalAlignment(static_cast<EGUI_ALIGNMENT>(in->getAttributeAsEnumeration("TabVerticalAlignment", GUIAlignmentNames)));
}


} // end namespace irr
} // end namespace gui

#endif // _IRR_COMPILE_WITH_GUI_

