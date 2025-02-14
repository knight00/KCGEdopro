#include "game_config.h"
#include <irrlicht.h>
#include "game.h"
#include "materials.h"
#include "client_card.h"
#include "deck_manager.h"
#include "duelclient.h"
#include "CGUITTFont/CGUITTFont.h"
#include "CGUIImageButton/CGUIImageButton.h"
#include "custom_skin_enum.h"
#include "image_manager.h"
#include "fmt.h"
//////kdiy///////
#include "replay_mode.h"
#include "sound_manager.h"
//////kdiy///////

namespace ygo {
void Game::DrawSelectionLine(const Materials::QuadVertex vec, bool strip, int width, irr::video::SColor color) {
	driver->setMaterial(matManager.mOutLine);
	if(strip && !gGameConfig->dotted_lines) {
		int pattern = linePatternD3D - 14;
		bool swap = false;
		if(linePatternD3D < 15) {
			pattern += 15;
			swap = true;
		}
		auto drawLine = [&](const auto& pos1, const auto& pos2) -> void {
			if(swap)
				driver->draw3DLineW(pos1 + (pos2 - pos1) * (pattern) / 15.0, pos2, color, width);
			else
				driver->draw3DLineW(pos1, pos1 + (pos2 - pos1) * (pattern) / 15.0, color, width);
		};
		drawLine(vec[0].Pos, vec[1].Pos);
		drawLine(vec[1].Pos, vec[3].Pos);
		drawLine(vec[3].Pos, vec[2].Pos);
		drawLine(vec[2].Pos, vec[0].Pos);
	} else {
		const std::array<irr::core::vector3df, 4> pos{ vec[0].Pos, vec[1].Pos, vec[3].Pos, vec[2].Pos };
		driver->draw3DShapeW(pos.data(), static_cast<irr::u32>(pos.size()), color, width, strip ? linePatternGL : 0xffff);
	}
}
void Game::DrawBackGround() {
	static float selFieldAlpha = 255;
	static float selFieldDAlpha = -10;
	//draw field spell card
	driver->setTransform(irr::video::ETS_WORLD, irr::core::IdentityMatrix);
	auto tfield = [dfield = dInfo.duel_field] {
		switch(dfield) {
		case 1:
		case 2:
			return 2;
		case 3:
			return 0;
		case 4:
			return 1;
		default:
			return 3;
		}
	}();
	auto DrawTextureRect = [this](Materials::QuadVertex vertices, irr::video::ITexture* texture) {
		matManager.mTexture.setTexture(0, texture);
		driver->setMaterial(matManager.mTexture);
		driver->drawVertexPrimitiveList(vertices, 4, matManager.iRectangle, 2);
	};
	const int three_columns = dInfo.HasFieldFlag(DUEL_3_COLUMNS_FIELD);
	auto DrawFieldSpell = [&]() -> bool {
		if(!gGameConfig->draw_field_spell)
			return false;
		uint32_t fieldcode1 = 0;
		if(dField.szone[0][5] && dField.szone[0][5]->position & POS_FACEUP)
		    /////kdiy//////
			//fieldcode1 = dField.szone[0][5]->code;
			fieldcode1 = dField.szone[0][5]->piccode > 0 ? dField.szone[0][5]->piccode : dField.szone[0][5]->code;
		    /////kdiy//////
		uint32_t fieldcode2 = 0;
		if(dField.szone[1][5] && dField.szone[1][5]->position & POS_FACEUP)
		    /////kdiy//////
			//fieldcode2 = dField.szone[1][5]->code;
			fieldcode2 = dField.szone[1][5]->piccode > 0 ? dField.szone[1][5]->piccode : dField.szone[1][5]->code;
		    /////kdiy//////
		auto both = fieldcode1 | fieldcode2;
		if(both == 0)
			return false;
		if(fieldcode1 == 0 || fieldcode2 == 0 || fieldcode1 == fieldcode2) {
			auto* texture = imageManager.GetTextureField(both);
			if(texture)
				DrawTextureRect(matManager.vFieldSpell[three_columns], texture);
			return texture;
		}
		auto* texture1 = imageManager.GetTextureField(fieldcode1);
		if(texture1)
			DrawTextureRect(matManager.vFieldSpell1[three_columns], texture1);
		auto texture2 = imageManager.GetTextureField(fieldcode2);
		if(texture2)
			DrawTextureRect(matManager.vFieldSpell2[three_columns], texture2);
		return texture1 || texture2;
	};

	//draw field
    /////kdiy//////
	if(isAnime) {
		if(PlayVideo())
		    if(videotexture) {
				if(gGameConfig->animefull) driver->draw2DImage(videotexture, Resize(0, 0, 1024, 640), irr::core::recti(0, 0, videoFrame->width, videoFrame->height));
				else DrawTextureRect(matManager.vFieldSpell[three_columns], videotexture);
			}
    } else {
    if(!gGameConfig->chkField && DrawFieldSpell())
	    DrawTextureRect(matManager.vField, imageManager.tFieldTransparent[three_columns][tfield]);
	    //DrawFieldSpell();
    else if(gGameConfig->chkField || !gGameConfig->randombgdeck || (tfield != 3 && tfield != 1))
    /////kdiy//////
	DrawTextureRect(matManager.vField, DrawFieldSpell() ? imageManager.tFieldTransparent[three_columns][tfield] : imageManager.tField[three_columns][tfield]);
    /////kdiy//////
    }
    /////kdiy//////

	driver->setMaterial(matManager.mBackLine);
	//select field
	if((dInfo.curMsg == MSG_SELECT_PLACE || dInfo.curMsg == MSG_SELECT_DISFIELD || dInfo.curMsg == MSG_HINT) && dField.selectable_field) {
		irr::video::SColor outline_color = skin::DUELFIELD_SELECTABLE_FIELD_OUTLINE_VAL;
		uint32_t filter = 0x1;
		for (int i = 0; i < 7; ++i, filter <<= 1) {
			if (dField.selectable_field & filter)
				DrawSelectionLine(matManager.vFieldMzone[0][i], !(dField.selected_field & filter), 2, outline_color);
		}
		filter = 0x100;
		for (int i = 0; i < 8; ++i, filter <<= 1) {
			if (dField.selectable_field & filter)
				DrawSelectionLine(matManager.getSzone()[0][i], !(dField.selected_field & filter), 2, outline_color);
		}
		filter = 0x10000;
		for (int i = 0; i < 7; ++i, filter <<= 1) {
			if (dField.selectable_field & filter)
				DrawSelectionLine(matManager.vFieldMzone[1][i], !(dField.selected_field & filter), 2, outline_color);
		}
		filter = 0x1000000;
		for (int i = 0; i < 8; ++i, filter <<= 1) {
			if (dField.selectable_field & filter)
				DrawSelectionLine(matManager.getSzone()[1][i], !(dField.selected_field & filter), 2, outline_color);
		}
	}
	//disabled field
	{
		irr::video::SColor disabled_color = skin::DUELFIELD_DISABLED_FIELD_COLOR_VAL;
		uint32_t filter = 0x1;
		for (int i = 0; i < 7; ++i, filter <<= 1) {
			if (dField.disabled_field & filter) {
				driver->draw3DLine(matManager.vFieldMzone[0][i][0].Pos, matManager.vFieldMzone[0][i][3].Pos, disabled_color);
				driver->draw3DLine(matManager.vFieldMzone[0][i][1].Pos, matManager.vFieldMzone[0][i][2].Pos, disabled_color);
			}
		}
		filter = 0x100;
		for (int i = 0; i < 8; ++i, filter <<= 1) {
			if (dField.disabled_field & filter) {
				driver->draw3DLine(matManager.getSzone()[0][i][0].Pos, matManager.getSzone()[0][i][3].Pos, disabled_color);
				driver->draw3DLine(matManager.getSzone()[0][i][1].Pos, matManager.getSzone()[0][i][2].Pos, disabled_color);
			}
		}
		filter = 0x10000;
		for (int i = 0; i < 7; ++i, filter <<= 1) {
			if (dField.disabled_field & filter) {
				driver->draw3DLine(matManager.vFieldMzone[1][i][0].Pos, matManager.vFieldMzone[1][i][3].Pos, disabled_color);
				driver->draw3DLine(matManager.vFieldMzone[1][i][1].Pos, matManager.vFieldMzone[1][i][2].Pos, disabled_color);
			}
		}
		filter = 0x1000000;
		for (int i = 0; i < 8; ++i, filter <<= 1) {
			if (dField.disabled_field & filter) {
				driver->draw3DLine(matManager.getSzone()[1][i][0].Pos, matManager.getSzone()[1][i][3].Pos, disabled_color);
				driver->draw3DLine(matManager.getSzone()[1][i][1].Pos, matManager.getSzone()[1][i][2].Pos, disabled_color);
			}
		}
	}
	auto setAlpha = [](irr::video::SMaterial& material, const irr::video::SColor& color) {
		uint32_t endalpha = std::round(color.getAlpha() * (selFieldAlpha - 5.0) * (0.005));
		material.DiffuseColor = endalpha << 24;
		material.AmbientColor = color;
	};
	//current sel
	if(dField.hovered_location == 0 || dField.hovered_location == LOCATION_HAND || dField.hovered_location == POSITION_HINT)
		return;
	if(!dInfo.HasFieldFlag(DUEL_EMZONE) && dField.hovered_location == LOCATION_MZONE && dField.hovered_sequence > 4)
		return;
	if((!dInfo.HasFieldFlag(DUEL_SEPARATE_PZONE) && dField.hovered_location == LOCATION_SZONE && dField.hovered_sequence > 5))
		return;
	setAlpha(matManager.mLinkedField, skin::DUELFIELD_LINKED_VAL);
	setAlpha(matManager.mMutualLinkedField, skin::DUELFIELD_MUTUAL_LINKED_VAL);
	selFieldAlpha += selFieldDAlpha * (float)delta_time * 60.0f / 1000.0f;
	if(selFieldAlpha <= 5) {
		selFieldAlpha = 5;
		selFieldDAlpha = 10;
	}
	if(selFieldAlpha >= 205) {
		selFieldAlpha = 205;
		selFieldDAlpha = -10;
	}
	setAlpha(matManager.mSelField, skin::DUELFIELD_HOVERED_VAL);
	const irr::video::S3DVertex* vertex = nullptr;
	if(dField.hovered_location == LOCATION_DECK)
		vertex = matManager.getDeck()[dField.hovered_controler];
	else if(dField.hovered_location == LOCATION_MZONE) {
		vertex = matManager.vFieldMzone[dField.hovered_controler][dField.hovered_sequence];
		ClientCard* pcard = dField.mzone[dField.hovered_controler][dField.hovered_sequence];
		if(pcard && (pcard->type & TYPE_LINK) && (pcard->position & POS_FACEUP)) {
			DrawLinkedZones(pcard);
		}
	} else if(dField.hovered_location == LOCATION_SZONE) {
		vertex = matManager.getSzone()[dField.hovered_controler][dField.hovered_sequence];
		ClientCard* pcard = dField.szone[dField.hovered_controler][dField.hovered_sequence];
		if(pcard && (pcard->type & TYPE_LINK) && (pcard->position & POS_FACEUP))
			DrawLinkedZones(pcard);
	} else if(dField.hovered_location == LOCATION_GRAVE)
		vertex = matManager.getGrave()[dField.hovered_controler];
	else if(dField.hovered_location == LOCATION_REMOVED)
		vertex = matManager.getRemove()[dField.hovered_controler];
	else if(dField.hovered_location == LOCATION_EXTRA)
		vertex = matManager.getExtra()[dField.hovered_controler];
	if(!vertex)
		return;
	driver->setMaterial(matManager.mSelField);
	driver->drawVertexPrimitiveList(vertex, 4, matManager.iRectangle, 2);
}
void Game::DrawLinkedZones(ClientCard* pcard) {
	///kdiy////////
	if(videostart) return;
	///kdiy////////
	auto CheckMutual = [&](ClientCard* pcard, int mark)->bool {
		driver->setMaterial(matManager.mLinkedField);
		if(pcard && pcard->type & TYPE_LINK && pcard->link_marker & mark) {
			driver->setMaterial(matManager.mMutualLinkedField);
			return true;
		}
		return false;
	};
	const int mark = pcard->link_marker;
	ClientCard* pcard2;
	const uint32_t three_columns = dInfo.HasFieldFlag(DUEL_3_COLUMNS_FIELD);
	if(dField.hovered_location == LOCATION_SZONE) {
		if(dField.hovered_sequence > 4)
			return;
		if(mark & LINK_MARKER_TOP_LEFT && dField.hovered_sequence > (0 + three_columns)) {
			pcard2 = dField.mzone[dField.hovered_controler][dField.hovered_sequence - 1];
			CheckMutual(pcard2, LINK_MARKER_BOTTOM_RIGHT);
			driver->drawVertexPrimitiveList(&matManager.vFieldMzone[dField.hovered_controler][dField.hovered_sequence - 1], 4, matManager.iRectangle, 2);
		}
		if(mark & LINK_MARKER_TOP) {
			pcard2 = dField.mzone[dField.hovered_controler][dField.hovered_sequence];
			CheckMutual(pcard2, LINK_MARKER_BOTTOM);
			driver->drawVertexPrimitiveList(&matManager.vFieldMzone[dField.hovered_controler][dField.hovered_sequence], 4, matManager.iRectangle, 2);
		}
		if(mark & LINK_MARKER_TOP_RIGHT && dField.hovered_sequence < (4 - three_columns)) {
			pcard2 = dField.mzone[dField.hovered_controler][dField.hovered_sequence + 1];
			CheckMutual(pcard2, LINK_MARKER_BOTTOM_LEFT);
			driver->drawVertexPrimitiveList(&matManager.vFieldMzone[dField.hovered_controler][dField.hovered_sequence + 1], 4, matManager.iRectangle, 2);
		}
		if(mark & LINK_MARKER_LEFT && dField.hovered_sequence >(0 + three_columns)) {
			pcard2 = dField.szone[dField.hovered_controler][dField.hovered_sequence - 1];
			CheckMutual(pcard2, LINK_MARKER_RIGHT);
			driver->drawVertexPrimitiveList(&matManager.getSzone()[dField.hovered_controler][dField.hovered_sequence - 1], 4, matManager.iRectangle, 2);
		}
		if(mark & LINK_MARKER_RIGHT && dField.hovered_sequence < (4 - three_columns)) {
			pcard2 = dField.szone[dField.hovered_controler][dField.hovered_sequence + 1];
			CheckMutual(pcard2, LINK_MARKER_LEFT);
			driver->drawVertexPrimitiveList(&matManager.getSzone()[dField.hovered_controler][dField.hovered_sequence + 1], 4, matManager.iRectangle, 2);
		}
		return;
	}
	if (dField.hovered_sequence < 5) {
		if (mark & LINK_MARKER_LEFT && dField.hovered_sequence > (0 + three_columns)) {
			pcard2 = dField.mzone[dField.hovered_controler][dField.hovered_sequence - 1];
			CheckMutual(pcard2, LINK_MARKER_RIGHT);
			driver->drawVertexPrimitiveList(&matManager.vFieldMzone[dField.hovered_controler][dField.hovered_sequence - 1], 4, matManager.iRectangle, 2);
		}
		if (mark & LINK_MARKER_RIGHT && dField.hovered_sequence < (4 - three_columns)) {
			pcard2 = dField.mzone[dField.hovered_controler][dField.hovered_sequence + 1];
			CheckMutual(pcard2, LINK_MARKER_LEFT);
			driver->drawVertexPrimitiveList(&matManager.vFieldMzone[dField.hovered_controler][dField.hovered_sequence + 1], 4, matManager.iRectangle, 2);
		}
		if(mark & LINK_MARKER_BOTTOM_LEFT && dField.hovered_sequence > (0 + three_columns)) {
			pcard2 = dField.szone[dField.hovered_controler][dField.hovered_sequence - 1];
            /////kdiy//////
			//if(CheckMutual(pcard2, LINK_MARKER_TOP_RIGHT))
			CheckMutual(pcard2, LINK_MARKER_TOP_RIGHT);
            /////kdiy//////
				driver->drawVertexPrimitiveList(&matManager.getSzone()[dField.hovered_controler][dField.hovered_sequence - 1], 4, matManager.iRectangle, 2);
		}
		if(mark & LINK_MARKER_BOTTOM_RIGHT && dField.hovered_sequence < (4 - three_columns)) {
			pcard2 = dField.szone[dField.hovered_controler][dField.hovered_sequence + 1];
            /////kdiy//////
			//if(CheckMutual(pcard2, LINK_MARKER_TOP_LEFT))
            CheckMutual(pcard2, LINK_MARKER_TOP_LEFT);
            /////kdiy//////
				driver->drawVertexPrimitiveList(&matManager.getSzone()[dField.hovered_controler][dField.hovered_sequence + 1], 4, matManager.iRectangle, 2);
		}
		if(mark & LINK_MARKER_BOTTOM) {
			pcard2 = dField.szone[dField.hovered_controler][dField.hovered_sequence];
            /////kdiy//////
			//if(CheckMutual(pcard2, LINK_MARKER_TOP))
            CheckMutual(pcard2, LINK_MARKER_TOP);
            /////kdiy//////
				driver->drawVertexPrimitiveList(&matManager.getSzone()[dField.hovered_controler][dField.hovered_sequence], 4, matManager.iRectangle, 2);
		}
		if(dInfo.HasFieldFlag(DUEL_EMZONE)) {
			if ((mark & LINK_MARKER_TOP_LEFT && dField.hovered_sequence == 2)
				|| (mark & LINK_MARKER_TOP && dField.hovered_sequence == 1)
				|| (mark & LINK_MARKER_TOP_RIGHT && dField.hovered_sequence == 0)) {
				int other_mark = (dField.hovered_sequence == 2) ? LINK_MARKER_BOTTOM_RIGHT : (dField.hovered_sequence == 1) ? LINK_MARKER_BOTTOM : LINK_MARKER_BOTTOM_LEFT;
				pcard2 = dField.mzone[dField.hovered_controler][5];
				if (!pcard2) {
					pcard2 = dField.mzone[1 - dField.hovered_controler][6];
					other_mark = (dField.hovered_sequence == 2) ? LINK_MARKER_TOP_LEFT : (dField.hovered_sequence == 1) ? LINK_MARKER_TOP : LINK_MARKER_TOP_RIGHT;
				}
				CheckMutual(pcard2, other_mark);
				driver->drawVertexPrimitiveList(&matManager.vFieldMzone[dField.hovered_controler][5], 4, matManager.iRectangle, 2);
			}
			if ((mark & LINK_MARKER_TOP_LEFT && dField.hovered_sequence == 4)
				|| (mark & LINK_MARKER_TOP && dField.hovered_sequence == 3)
				|| (mark & LINK_MARKER_TOP_RIGHT && dField.hovered_sequence == 2)) {
				int other_mark = (dField.hovered_sequence == 4) ? LINK_MARKER_BOTTOM_RIGHT : (dField.hovered_sequence == 3) ? LINK_MARKER_BOTTOM : LINK_MARKER_BOTTOM_LEFT;
				pcard2 = dField.mzone[dField.hovered_controler][6];
				if (!pcard2) {
					pcard2 = dField.mzone[1 - dField.hovered_controler][5];
					other_mark = (dField.hovered_sequence == 4) ? LINK_MARKER_TOP_LEFT : (dField.hovered_sequence == 3) ? LINK_MARKER_TOP : LINK_MARKER_TOP_RIGHT;
				}
				CheckMutual(pcard2, other_mark);
				driver->drawVertexPrimitiveList(&matManager.vFieldMzone[dField.hovered_controler][6], 4, matManager.iRectangle, 2);
			}
		}
	} else {
		int swap = (dField.hovered_sequence == 5) ? 0 : 2;
		if (mark & LINK_MARKER_BOTTOM_LEFT && !(three_columns && swap == 0)) {
			pcard2 = dField.mzone[dField.hovered_controler][0 + swap];
			CheckMutual(pcard2, LINK_MARKER_TOP_RIGHT);
			driver->drawVertexPrimitiveList(&matManager.vFieldMzone[dField.hovered_controler][0 + swap], 4, matManager.iRectangle, 2);
		}
		if (mark & LINK_MARKER_BOTTOM) {
			pcard2 = dField.mzone[dField.hovered_controler][1 + swap];
			CheckMutual(pcard2, LINK_MARKER_TOP);
			driver->drawVertexPrimitiveList(&matManager.vFieldMzone[dField.hovered_controler][1 + swap], 4, matManager.iRectangle, 2);
		}
		if (mark & LINK_MARKER_BOTTOM_RIGHT && !(three_columns && swap == 2)) {
			pcard2 = dField.mzone[dField.hovered_controler][2 + swap];
			CheckMutual(pcard2, LINK_MARKER_TOP_LEFT);
			driver->drawVertexPrimitiveList(&matManager.vFieldMzone[dField.hovered_controler][2 + swap], 4, matManager.iRectangle, 2);
		}
		if (mark & LINK_MARKER_TOP_LEFT && !(three_columns && swap == 0)) {
			pcard2 = dField.mzone[1 - dField.hovered_controler][4 - swap];
			CheckMutual(pcard2, LINK_MARKER_TOP_LEFT);
			driver->drawVertexPrimitiveList(&matManager.vFieldMzone[1 - dField.hovered_controler][4 - swap], 4, matManager.iRectangle, 2);
		}
		if (mark & LINK_MARKER_TOP) {
			pcard2 = dField.mzone[1 - dField.hovered_controler][3 - swap];
			CheckMutual(pcard2, LINK_MARKER_TOP);
			driver->drawVertexPrimitiveList(&matManager.vFieldMzone[1 - dField.hovered_controler][3 - swap], 4, matManager.iRectangle, 2);
		}
		if (mark & LINK_MARKER_TOP_RIGHT && !(three_columns && swap == 2)) {
			pcard2 = dField.mzone[1 - dField.hovered_controler][2 - swap];
			CheckMutual(pcard2, LINK_MARKER_TOP_RIGHT);
			driver->drawVertexPrimitiveList(&matManager.vFieldMzone[1 - dField.hovered_controler][2 - swap], 4, matManager.iRectangle, 2);
		}
	}
}
void Game::DrawCards() {
	for(auto& pcard : dField.overlay_cards)
		DrawCard(pcard);
	for(int p = 0; p < 2; ++p) {
		//////kdiy/////////
		for(int i = 0; i < 7; i++) {
			for(int j = 0; j < 10; j++) {
				if(!dField.mzone[p][i] && haloNodeexist[p][i][j]) {
					haloNode[p][i][j].clear();
				    haloNodeexist[p][i][j] = false;
				}
			}
		}
		for(int i = 0; i < 5; i++) {
			for(int j = 0; j < 10; j++) {
				if(!dField.szone[p][i] && haloNodeexist[p][i+7][j]) {
					haloNode[p][i+7][j].clear();
				    haloNodeexist[p][i+7][j] = false;
				}
			}
		}
		//////kdiy/////////
		for(auto& pcard : dField.mzone[p])
			if(pcard)
				DrawCard(pcard);
		for(auto& pcard : dField.szone[p])
			if(pcard)
				DrawCard(pcard);
		for(auto& pcard : dField.deck[p])
			DrawCard(pcard);
		for(auto& pcard : dField.hand[p])
			DrawCard(pcard);
		for(auto& pcard : dField.grave[p])
			DrawCard(pcard);
		for(auto& pcard : dField.remove[p])
			DrawCard(pcard);
		for(auto& pcard : dField.extra[p])
			DrawCard(pcard);
		if(dField.skills[p])
			DrawCard(dField.skills[p]);
	}
}
void Game::DrawCard(ClientCard* pcard) {
	///kdiy////////
	if(videostart && !pcard->is_anime) return;
	if(!videostart) pcard->is_anime = false;
	///kdiy////////
	if (pcard->aniFrame > 0) {
		uint32_t movetime = std::min<uint32_t>(delta_time, pcard->aniFrame);
		if (pcard->is_moving) {
			pcard->curPos += (pcard->dPos * movetime);
			pcard->curRot += (pcard->dRot * movetime);
			pcard->mTransform.setTranslation(pcard->curPos);
            ///kdiy////////
            if(!pcard->is_attacking)
            ///kdiy////////
			pcard->mTransform.setRotationRadians(pcard->curRot);
		}
		if (pcard->is_fading)
			pcard->curAlpha += pcard->dAlpha * movetime;
		pcard->aniFrame -= movetime;
		if (pcard->aniFrame <= 0) {
			pcard->aniFrame = 0;
			pcard->is_moving = false;
			pcard->is_fading = false;
			if (std::exchange(pcard->refresh_on_stop, false))
				pcard->UpdateDrawCoordinates(true);
		}
	}
	///kdiy////////
    auto cd = gDataManager->GetCardData(pcard->code);
	irr::video::ITexture* cardcloseup; irr::video::SColor cardcloseupcolor = irr::video::SColor(255, 255, 255, 0);
	std::tie(cardcloseup, cardcloseupcolor) = imageManager.GetTextureCloseup(pcard->piccode > 0 ? pcard->piccode : pcard->code, pcard->is_change && cd ? cd->alias : pcard->alias);
	matManager.mTexture.AmbientColor = 0xffffffff;
    auto drawLine = [&](const auto& pos0, const auto& pos1, const auto& pos2, const auto& pos3, irr::video::SColor color) -> void {
        driver->draw3DLineW(pos0, pos1, color, (pcard->location & LOCATION_ONFIELD) && (pcard->type & TYPE_MONSTER) && (pcard->position & POS_FACEUP) && cardcloseup ? 10 : 8);
        driver->draw3DLineW(pos1, pos3, color, (pcard->location & LOCATION_ONFIELD) && (pcard->type & TYPE_MONSTER) && (pcard->position & POS_FACEUP) && cardcloseup ? 10 : 8);
        driver->draw3DLineW(pos3, pos2, color, (pcard->location & LOCATION_ONFIELD) && (pcard->type & TYPE_MONSTER) && (pcard->position & POS_FACEUP) && cardcloseup ? 10 : 8);
        driver->draw3DLineW(pos2, pos0, color, (pcard->location & LOCATION_ONFIELD) && (pcard->type & TYPE_MONSTER) && (pcard->position & POS_FACEUP) && cardcloseup ? 10 : 8);
    };
    ///kdiy////////
	if (((pcard->location == LOCATION_HAND && pcard->code) || ((pcard->location & 0xc) && (pcard->position & POS_FACEUP))) && (pcard->status & (STATUS_DISABLED | STATUS_FORBIDDEN)))
	matManager.mCard.AmbientColor = irr::video::SColor(255, 128, 128, 180);
	else
	///kdiy////////
	matManager.mCard.AmbientColor = 0xffffffff;
	matManager.mCard.DiffuseColor = ((int)std::round(pcard->curAlpha) << 24) | 0xffffff;
	driver->setTransform(irr::video::ETS_WORLD, pcard->mTransform);
	auto m22 = pcard->mTransform(2, 2);
	if (m22 > -0.99 || pcard->is_moving) {
        ///kdiy////////
		//matManager.mCard.setTexture(0, imageManager.GetTextureCard(pcard->code, imgType::ART));
		matManager.mCard.setTexture(0, imageManager.GetTextureCard(pcard->piccode > 0 ? pcard->piccode : pcard->code, imgType::ART));
        ///kdiy////////
		driver->setMaterial(matManager.mCard);
		///kdiy////////
		//driver->drawVertexPrimitiveList(matManager.vCardFront, 4, matManager.iRectangle, 2);
		driver->drawVertexPrimitiveList((pcard->location & LOCATION_ONFIELD) ? matManager.vCardFront2 : matManager.vCardFront, 4, matManager.iRectangle, 2);
		///kdiy////////
	}
	if (m22 < 0.99 || pcard->is_moving) {
        ///kdiy////////
		auto txt = imageManager.GetTextureCard(pcard->cover, imgType::COVER);
		if (txt == imageManager.tCover[0]) {
            ///kdiy////////
            if (pcard->location & LOCATION_EXTRA)
			matManager.mCard.setTexture(0, imageManager.tCover[pcard->controler + 2]);
            else
            ///kdiy////////
			matManager.mCard.setTexture(0, imageManager.tCover[pcard->controler]);
		}
		else {
			matManager.mCard.setTexture(0, imageManager.GetTextureCard(pcard->cover, imgType::COVER));
		}
		driver->setMaterial(matManager.mCard);
		///kdiy////////
		//driver->drawVertexPrimitiveList(matManager.vCardBack, 4, matManager.iRectangle, 2);
		driver->drawVertexPrimitiveList((pcard->location & LOCATION_ONFIELD) ? matManager.vCardBack2 : matManager.vCardBack, 4, matManager.iRectangle, 2);
		///kdiy////////
	}
	///kdiy////////
	if (pcard->is_attack && (pcard->location & LOCATION_ONFIELD)) {
		float xa = pcard->attPos.X;
		float ya = pcard->attPos.Y;
		float xd, yd;
		xd = pcard->attdPos.X;
		yd = pcard->attdPos.Y;
		irr::core::vector3df atkr = irr::core::vector3df(0, 0, pcard->controler == 0 ? -std::atan((xd - xa) / (yd - ya)) : irr::core::PI -std::atan((xd - xa) / (yd - ya)));
		pcard->mTransform.setRotationRadians(atkr);
	} else if ((pcard->position & POS_FACEUP) && (pcard->type & TYPE_PENDULUM) && (pcard->location & LOCATION_SZONE) && (pcard->sequence == dInfo.GetPzoneIndex(0)) && !pcard->is_orica && pcard->is_pzone && !pcard->equipTarget
		&& !gGameConfig->topdown_view) {
		pcard->mTransform.setTranslation(pcard->curPos + irr::core::vector3df(pcard->controler == 0 ? -0.32f : 0.32f, pcard->controler == 0 ? 0 : -0.8f, 0));
		pcard->mTransform.setRotationRadians(pcard->curRot + irr::core::vector3df(-irr::core::PI / 3, 0, pcard->controler == 0 ? -irr::core::PI / 5 : -irr::core::PI + irr::core::PI / 5));
	} else if ((pcard->position & POS_FACEUP) && (pcard->type & TYPE_PENDULUM) && (pcard->location & LOCATION_SZONE) && (pcard->sequence == dInfo.GetPzoneIndex(1)) && !pcard->is_orica && pcard->is_pzone && !pcard->equipTarget
		&& !gGameConfig->topdown_view) {
		pcard->mTransform.setTranslation(pcard->curPos + irr::core::vector3df(pcard->controler == 0 ? 0.32f : -0.32f, pcard->controler == 0 ? 0 : -0.8f, 0));
		pcard->mTransform.setRotationRadians(pcard->curRot + irr::core::vector3df(-irr::core::PI / 3, 0, pcard->controler == 0 ? irr::core::PI / 5 : -irr::core::PI - irr::core::PI / 5));
	} else {
		pcard->mTransform.setTranslation(pcard->curPos);
		pcard->mTransform.setRotationRadians(pcard->curRot);
	}
    if((pcard->cmdFlag & COMMAND_ACTIVATE) && pcard->controler == 0 && (pcard->location & (LOCATION_HAND | LOCATION_ONFIELD))
	  && !(pcard->is_selectable && (pcard->location & 0xe))
	  && !(pcard->is_activable) && !(pcard->is_highlighting)) {
        driver->setMaterial(matManager.mOutLine);
        drawLine((pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[0].Pos : matManager.vCardOutliner[0].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[1].Pos : matManager.vCardOutliner[1].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[2].Pos : matManager.vCardOutliner[2].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[3].Pos : matManager.vCardOutliner[3].Pos, 0xff00ff00);
    }
    if((pcard->cmdFlag & COMMAND_SPSUMMON) && (pcard->location & LOCATION_HAND)
	  && !(pcard->is_selectable && (pcard->location & 0xe))
	  && !(pcard->is_activable) && !(pcard->is_highlighting)) {
        driver->setMaterial(matManager.mOutLine);
		drawLine((pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[0].Pos : matManager.vCardOutliner[0].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[1].Pos : matManager.vCardOutliner[1].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[2].Pos : matManager.vCardOutliner[2].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[3].Pos : matManager.vCardOutliner[3].Pos, 0xff0000ff);
    }
	if (pcard->is_activable) {
		irr::video::SColor outline_color = skin::DUELFIELD_HIGHLIGHTING_CARD_OUTLINE_VAL;
		if(gGameConfig->dotted_lines) {
			if ((pcard->location == LOCATION_HAND && pcard->code) || ((pcard->location & 0xc) && (pcard->position & POS_FACEUP)))
				DrawSelectionLine((pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutline2 : matManager.vCardOutline, true, 2, outline_color);
			else
			    DrawSelectionLine((pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2 : matManager.vCardOutliner, true, 2, outline_color);
		} else {
			driver->setMaterial(matManager.mOutLine);
			drawLine((pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[0].Pos : matManager.vCardOutliner[0].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[1].Pos : matManager.vCardOutliner[1].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[2].Pos : matManager.vCardOutliner[2].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[3].Pos : matManager.vCardOutliner[3].Pos, 0xffffff00);
		}
	}
	if((pcard->location & LOCATION_ONFIELD) && (pcard->position & POS_FACEUP) && cardcloseup) {
		if ( ((pcard->location == LOCATION_MZONE && !pcard->is_sanct) || (pcard->location == LOCATION_SZONE && pcard->is_orica)) || (pcard->cmdFlag & COMMAND_ATTACK) || pcard->is_attack || pcard->is_attacked) {
			if ((pcard->status & (STATUS_DISABLED | STATUS_FORBIDDEN)))
				matManager.mTexture.AmbientColor = irr::video::SColor(255, 128, 128, 128);
			else if (pcard->position & POS_DEFENSE)
				matManager.mTexture.AmbientColor = irr::video::SColor(255, 180, 180, 255);
			else
				matManager.mTexture.AmbientColor = 0xffffffff;
			matManager.mTexture.setTexture(0, cardcloseup);
			driver->setMaterial(matManager.mTexture);
			irr::core::matrix4 atk;
			if (!(pcard->cmdFlag & COMMAND_ATTACK))
				atk.setTranslation(pcard->curPos + irr::core::vector3df(0, pcard->controler == 0 ? 0 : 0.2f, 0.2f));
			else
				atk.setTranslation(pcard->curPos + irr::core::vector3df(0, (pcard->controler == 0 ? -0.4f : 0.4f) * (atkdy / 4.0f + 0.35f), 0.05f));
			driver->setTransform(irr::video::ETS_WORLD, atk);
			if (pcard->attack >= 10000 || pcard->defense >= 10000)
				driver->drawVertexPrimitiveList(matManager.vAttack4, 4, matManager.iRectangle, 2);
			else if (pcard->attack >= 4500 && pcard->position == POS_FACEUP_ATTACK)
				driver->drawVertexPrimitiveList(matManager.vAttack3, 4, matManager.iRectangle, 2);
			else if (pcard->attack < 1000 && (((pcard->type & TYPE_LINK) && pcard->link < 3) || ((pcard->type & TYPE_XYZ) && pcard->rank < 4) || (!(pcard->type & (TYPE_LINK | TYPE_XYZ)) && pcard->level < 4)))
				driver->drawVertexPrimitiveList(matManager.vAttack, 4, matManager.iRectangle, 2);
			else
				driver->drawVertexPrimitiveList(matManager.vAttack2, 4, matManager.iRectangle, 2);
		}
	}
	driver->setTransform(irr::video::ETS_WORLD, pcard->mTransform);
	if (pcard->is_attack_disabled) {
		matManager.mTexture.setTexture(0, imageManager.tShield);
		driver->setMaterial(matManager.mTexture);
		driver->drawVertexPrimitiveList(matManager.vSymbol, 4, matManager.iRectangle, 2);
	}
	if (pcard->is_damage) {
		matManager.mTexture.setTexture(0, imageManager.tCrack);
		driver->setMaterial(matManager.mTexture);
		driver->drawVertexPrimitiveList(matManager.vAttack3, 4, matManager.iRectangle, 2);
	}
	///kdiy////////
	if (pcard->is_selectable && (pcard->location & 0xe)) {
		irr::video::SColor outline_color = skin::DUELFIELD_SELECTABLE_CARD_OUTLINE_VAL;
		///kdiy////////
		// if ((pcard->location == LOCATION_HAND && pcard->code) || ((pcard->location & 0xc) && (pcard->position & POS_FACEUP)))
			//DrawSelectionLine(matManager.vCardOutline, !pcard->is_selected, 2, outline_color);
		// else
		// 	DrawSelectionLine(matManager.vCardOutliner, !pcard->is_selected, 2, outline_color);
		if(gGameConfig->dotted_lines) {
			if ((pcard->location == LOCATION_HAND && pcard->code) || ((pcard->location & 0xc) && (pcard->position & POS_FACEUP)))
			    DrawSelectionLine((pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutline2 : matManager.vCardOutline, !pcard->is_selected, 2, outline_color);
			else
			    DrawSelectionLine((pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2 : matManager.vCardOutliner, !pcard->is_selected, 2, outline_color);
		} else {
			if(!pcard->is_selected) {
				driver->setMaterial(matManager.mOutLine);
				drawLine((pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[0].Pos : matManager.vCardOutliner[0].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[1].Pos : matManager.vCardOutliner[1].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[2].Pos : matManager.vCardOutliner[2].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[3].Pos : matManager.vCardOutliner[3].Pos, 0xffff00ff);
			} else {
				driver->setMaterial(matManager.mOutLine);
				drawLine((pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[0].Pos : matManager.vCardOutliner[0].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[1].Pos : matManager.vCardOutliner[1].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[2].Pos : matManager.vCardOutliner[2].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[3].Pos : matManager.vCardOutliner[3].Pos, 0xff808080);
			}
		}
		///kdiy////////
	}
	if (pcard->is_highlighting) {
		irr::video::SColor outline_color = skin::DUELFIELD_HIGHLIGHTING_CARD_OUTLINE_VAL;
		///kdiy////////
		// if ((pcard->location == LOCATION_HAND && pcard->code) || ((pcard->location & 0xc) && (pcard->position & POS_FACEUP)))
			//DrawSelectionLine(matManager.vCardOutline, true, 2, outline_color);
		// else
		// 	DrawSelectionLine(matManager.vCardOutliner, true, 2, outline_color);
		if(gGameConfig->dotted_lines) {
			if ((pcard->location == LOCATION_HAND && pcard->code) || ((pcard->location & 0xc) && (pcard->position & POS_FACEUP)))
			    DrawSelectionLine((pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutline2 : matManager.vCardOutline, true, 2, outline_color);
			else
			    DrawSelectionLine((pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2 : matManager.vCardOutliner, true, 2, outline_color);
		} else {
			driver->setMaterial(matManager.mOutLine);
			drawLine((pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[0].Pos : matManager.vCardOutliner[0].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[1].Pos : matManager.vCardOutliner[1].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[2].Pos : matManager.vCardOutliner[2].Pos, (pcard->location & LOCATION_ONFIELD) ? matManager.vCardOutliner2[3].Pos : matManager.vCardOutliner[3].Pos, 0xff00ffff);
		}
        ///kdiy////////
	}
	if (pcard->is_showequip) {
		matManager.mTexture.setTexture(0, imageManager.tEquip);
		driver->setMaterial(matManager.mTexture);
		driver->drawVertexPrimitiveList(matManager.vSymbol, 4, matManager.iRectangle, 2);
	}
	else if (pcard->is_showtarget) {
		matManager.mTexture.setTexture(0, imageManager.tTarget);
		driver->setMaterial(matManager.mTexture);
		driver->drawVertexPrimitiveList(matManager.vSymbol, 4, matManager.iRectangle, 2);
	}
	else if (pcard->is_showchaintarget) {
		matManager.mTexture.setTexture(0, imageManager.tChainTarget);
		driver->setMaterial(matManager.mTexture);
		driver->drawVertexPrimitiveList(matManager.vSymbol, 4, matManager.iRectangle, 2);
	}
	///kdiy////////
	// else if ((pcard->status & (STATUS_DISABLED | STATUS_FORBIDDEN))
	// 	&& (pcard->location & LOCATION_ONFIELD) && (pcard->position & POS_FACEUP)) {
		// matManager.mTexture.setTexture(0, imageManager.tNegated);
		// driver->setMaterial(matManager.mTexture);
		// driver->drawVertexPrimitiveList(matManager.vNegate, 4, matManager.iRectangle, 2);
	// }
	///kdiy////////
	if (pcard->is_moving)
		return;
	///kdiy////////
	if(pcard->desc_hints.size() > 0 && ((pcard->location == LOCATION_HAND && pcard->code) || (pcard->position & POS_FACEUP))) {
	    matManager.mTexture.setTexture(0, imageManager.tHint);
		driver->setMaterial(matManager.mTexture);
		irr::core::matrix4 atk;
		atk.setTranslation(pcard->curPos + irr::core::vector3df(0, -1.0f, 0));
		driver->setTransform(irr::video::ETS_WORLD, atk);
		driver->drawVertexPrimitiveList(matManager.vHint, 4, matManager.iRectangle, 2);
	}
	driver->setTransform(irr::video::ETS_WORLD, pcard->mTransform);
	if((pcard->type & TYPE_XYZ) && (pcard->location & LOCATION_ONFIELD)) {
		if(cd) {
			auto setcodes = cd->setcodes;
			if(cd->alias && setcodes.empty()) {
				auto data = gDataManager->GetCardData(cd->alias);
				if(data)
					setcodes = data->setcodes;
			}
			if(pcard && pcard->is_change && pcard->rsetnames && pcard->rsetnames > 0) {
				setcodes.clear();
				for(int i = 0; i < 4; i++) {
					uint16_t setcode = (pcard->rsetnames >> (i * 16)) & 0xffff;
					if(setcode)
						setcodes.push_back(setcode);
				}
			}
			if(std::find(setcodes.begin(), setcodes.end(), 0x1073) != setcodes.end() || std::find(setcodes.begin(), setcodes.end(), 0x1048) != setcodes.end()) {
				for(size_t i = 0; i < pcard->overlayed.size(); i++) {
					if(i > 7) break;
					matManager.mTexture.setTexture(0, imageManager.tCXyz);
					driver->setMaterial(matManager.mTexture);
					irr::core::matrix4 atk;
					atk.setTranslation(pcard->curPos + irr::core::vector3df(-0.5f + 0.3f * (i % 4), i < 4 ? 0.38f : 0.58f, 0.25f));
					driver->setTransform(irr::video::ETS_WORLD, atk);
					driver->drawVertexPrimitiveList(matManager.vCXyz, 4, matManager.iRectangle, 2);
				}
			} else {
				int sequence = (pcard->location & LOCATION_SZONE) ? pcard->sequence + 7 : pcard->sequence;
				int incre = 0;
				int incre2 = 0;
                int size = (pcard->overlayed.size() > 10) ? 10 : pcard->overlayed.size();
				for(size_t i = 0; i < size; i++) {
					int neg = 1;
					if(size > 2 && i >= size / 2) neg = -1;
					int power = 1;
					if(size / 4 > 0) power = size / 4;
					if(size > 2 && i % 2 == 0 && i < size / 2) incre += 1;
					if(size > 2 && i % 2 == 0 && i >= size / 2) incre2 += 1;
					matManager.mTexture.setTexture(0, imageManager.tXyz);
					matManager.mTexture.AmbientColor = cardcloseupcolor;
					driver->setMaterial(matManager.mTexture);
					irr::core::matrix4 atk;
					auto pos = irr::core::vector3df((pow(-1, i) * (0.72f + 0.1f * neg / power * (i < size / 2 ? incre : incre2))) * atkdy2, (((0.62f + 0.3f * neg / power * (i < size / 2 ? incre : incre2)))) * atkdy2, pow(-1, i) * 0.2f * atk2dy2);
					atk.setTranslation(pcard->curPos + pos);
					driver->setTransform(irr::video::ETS_WORLD, atk);
					driver->drawVertexPrimitiveList(matManager.vXyz, 4, matManager.iRectangle, 2);
					if(!haloNodeexist[pcard->controler][sequence][i])
						haloNodeexist[pcard->controler][sequence][i] = true;
					else {
						int ptk = 0;
						for(irr::core::vector3df pt : haloNode[pcard->controler][sequence][i]) {
							matManager.mTexture.setTexture(0, imageManager.tXyz);
							float scale = 1.0f - (ptk * 1.06f / 80);
							if(scale < 0.0f)
								scale = 0.0f;
							int alpha = 255 - (ptk * 266 / 80);
							if(alpha < 0)
								alpha = 0;
							matManager.mTexture.AmbientColor = cardcloseupcolor;
							irr::video::SColor color(alpha, 255, 255, 255);
							matManager.mTexture.DiffuseColor = color;
							driver->setMaterial(matManager.mTexture);
							irr::core::matrix4 atk;
							atk.setTranslation(pt);
							atk.setScale(irr::core::vector3df(scale, scale, scale));
							driver->setTransform(irr::video::ETS_WORLD, atk);
							driver->drawVertexPrimitiveList(matManager.vXyztrail, 4, matManager.iRectangle, 2);
                            ptk++;
						}
					}
					haloNode[pcard->controler][sequence][i].insert(haloNode[pcard->controler][sequence][i].begin(), pcard->curPos + irr::core::vector3df((pow(-1, i) * (0.72f + 0.1f * neg / power * (i < size / 2 ? incre : incre2))) * atkdy2, (((0.62f + 0.3f * neg / power * (i < size / 2 ? incre : incre2)))) * atkdy2, pow(-1, i) * 0.2f * atk2dy2));
					if(haloNode[pcard->controler][sequence][i].size() > 80)
						haloNode[pcard->controler][sequence][i].pop_back();
				}
				for(size_t j = size; ; j++) {
					if(!haloNodeexist[pcard->controler][sequence][j]) break;
					haloNode[pcard->controler][sequence][j].clear();
					haloNodeexist[pcard->controler][sequence][j] = false;
				}
				matManager.mTexture.AmbientColor = 0xffffffff;
				matManager.mTexture.DiffuseColor = 0xff000000;
			}
		}
	}
	driver->setTransform(irr::video::ETS_WORLD, pcard->mTransform);
    ////kdiy/////////
    //if(pcard->cmdFlag & COMMAND_ATTACK) {
	if((pcard->cmdFlag & COMMAND_ATTACK) && !cardcloseup) {
    ////kidy/////////
        matManager.mTexture.setTexture(0, imageManager.tAttack);
        driver->setMaterial(matManager.mTexture);
		irr::core::matrix4 atk;
		atk.setTranslation(pcard->curPos + irr::core::vector3df(0, (pcard->controler == 0 ? -1 : 1) * (atkdy / 4.0f + 0.35f), 0.05f));
		atk.setRotationRadians(irr::core::vector3df(0, 0, pcard->controler == 0 ? 0 : irr::core::PI));
		driver->setTransform(irr::video::ETS_WORLD, atk);
		driver->drawVertexPrimitiveList(matManager.vSymbol, 4, matManager.iRectangle, 2);
	}
    ////kdiy/////////
	if((pcard->position & POS_FACEUP) && (pcard->type & TYPE_PENDULUM) && (pcard->location & LOCATION_SZONE) && (pcard->sequence == dInfo.GetPzoneIndex(0)) && !pcard->is_orica && pcard->is_pzone && !pcard->equipTarget) {
		int scale = pcard->lscale;
		if(scale >= 0 && scale <= 13 && imageManager.tLScale[scale]) {
			matManager.mTexture.setTexture(0, imageManager.tLScale[scale]);
			driver->setMaterial(matManager.mTexture);
			driver->drawVertexPrimitiveList(matManager.vPScale, 4, matManager.iRectangle, 2);
		}
	}
	if((pcard->position & POS_FACEUP) && (pcard->type & TYPE_PENDULUM) && (pcard->location & LOCATION_SZONE) && (pcard->sequence == dInfo.GetPzoneIndex(1)) && !pcard->is_orica && pcard->is_pzone && !pcard->equipTarget) {
		int scale2 = pcard->rscale;
		if(scale2 >= 0 && scale2 <= 13 && imageManager.tRScale[scale2]) {
			matManager.mTexture.setTexture(0, imageManager.tRScale[scale2]);
			driver->setMaterial(matManager.mTexture);
			driver->drawVertexPrimitiveList(matManager.vPScale, 4, matManager.iRectangle, 2);
		}
	}
	////kdiy/////////
}
template<typename T>
inline void DrawShadowTextPos(irr::gui::CGUITTFont* font, const T& text, const irr::core::recti& shadowposition, const irr::core::recti& mainposition,
					   irr::video::SColor color = 0xffffffff, irr::video::SColor shadowcolor = 0xff000000, bool hcenter = false, bool vcenter = false, const irr::core::recti* clip = nullptr) {
	font->drawustring(text, shadowposition, shadowcolor, hcenter, vcenter, clip);
	font->drawustring(text, mainposition, color, hcenter, vcenter, clip);
}
//We don't want multiple function signatures per argument combination
template<typename T, typename... Args>
ForceInline void DrawShadowText(irr::gui::CGUITTFont* font, const T& text, const irr::core::recti& shadowposition, const irr::core::recti& padding, Args&&... args) {
	const irr::core::recti position(shadowposition.UpperLeftCorner.X + padding.UpperLeftCorner.X, shadowposition.UpperLeftCorner.Y + padding.UpperLeftCorner.Y,
									shadowposition.LowerRightCorner.X + padding.LowerRightCorner.X, shadowposition.LowerRightCorner.Y + padding.LowerRightCorner.Y);
	DrawShadowTextPos(font, text, shadowposition, position, std::forward<Args>(args)...);
}
void Game::DrawMisc() {
	const float twoPI = 2.0f * irr::core::PI;
	static float act_rot = 0.0f;
	//pre expanded version of setRotationRadians, we're only setting the z value, saves computations
	auto SetZRotation = [](irr::core::matrix4& mat) {
		mat[2] = mat[6] = mat[8] = mat[9] = 0;
		mat[10] = 1;
		const auto _cos = std::cos(act_rot);
		const auto _sin = std::sin(act_rot);
		mat[0] = mat[5] = _cos;
		mat[1] = _sin;
		mat[4] = -_sin;
	};
	const int three_columns = dInfo.HasFieldFlag(DUEL_3_COLUMNS_FIELD);
	irr::core::matrix4 im, ic, it;
	act_rot += (1.2f / 1000.0f) * delta_time;
	if(act_rot >= twoPI) {
		act_rot -= twoPI;
		//double branch to account for random instances where the value increases too much
		if(act_rot >= twoPI) {
			act_rot = fmod(act_rot, twoPI);
		}
	}
	SetZRotation(im);
	matManager.mTexture.setTexture(0, imageManager.tAct);
	driver->setMaterial(matManager.mTexture);

	auto drawact = [this, &im](const Materials::QuadVertex vertex, float zval) {
		im.setTranslation(irr::core::vector3df((vertex[0].Pos.X + vertex[1].Pos.X) / 2,
			(vertex[0].Pos.Y + vertex[2].Pos.Y) / 2, zval));
		driver->setTransform(irr::video::ETS_WORLD, im);
		driver->drawVertexPrimitiveList(matManager.vActivate, 4, matManager.iRectangle, 2);
	};

	for(int p = 0; p < 2; p++) {
		if(dField.deck_act[p])
			drawact(matManager.getDeck()[p], dField.deck[p].size() * 0.01f + 0.02f);
		if(dField.grave_act[p])
			drawact(matManager.getGrave()[p], dField.grave[p].size() * 0.01f + 0.02f);
		if(dField.remove_act[p])
			drawact(matManager.getRemove()[p], dField.remove[p].size() * 0.01f + 0.02f);
		if(dField.extra_act[p])
			drawact(matManager.getExtra()[p], dField.extra[p].size() * 0.01f + 0.02f);
		if(dField.pzone_act[p])
			drawact(matManager.getSzone()[p][dInfo.GetPzoneIndex(0)], 0.03f);
	}

	if(dField.conti_act) {
		im.setTranslation(irr::core::vector3df((matManager.vFieldContiAct[three_columns][0].X + matManager.vFieldContiAct[three_columns][1].X) / 2,
			(matManager.vFieldContiAct[three_columns][0].Y + matManager.vFieldContiAct[three_columns][2].Y) / 2, 0.03f));
		driver->setTransform(irr::video::ETS_WORLD, im);
		driver->drawVertexPrimitiveList(matManager.vActivate, 4, matManager.iRectangle, 2);
	}

	matManager.mTRTexture.AmbientColor = skin::DUELFIELD_CHAIN_COLOR_VAL;
	auto setCoords = [&](size_t i) {
		const auto div = i / 5;
		const auto mod = i % 5;
		matManager.vChainNum[0].TCoords = irr::core::vector2df(0.19375f * mod, 0.2421875f * div);
		matManager.vChainNum[1].TCoords = irr::core::vector2df(0.19375f * (mod + 1), 0.2421875f * div);
		matManager.vChainNum[2].TCoords = irr::core::vector2df(0.19375f * mod, 0.2421875f * (div + 1));
		matManager.vChainNum[3].TCoords = irr::core::vector2df(0.19375f * (mod + 1), 0.2421875f * (div + 1));
	};
	for(size_t i = 0; i < dField.chains.size(); ++i) {
		const auto& chain = dField.chains[i];
		if(chain.solved)
			break;
		matManager.mTRTexture.setTexture(0, imageManager.tChain);
		SetZRotation(ic);
		ic.setTranslation(chain.chain_pos);
		driver->setMaterial(matManager.mTRTexture);
		driver->setTransform(irr::video::ETS_WORLD, ic);
		driver->drawVertexPrimitiveList(matManager.vSymbol, 4, matManager.iRectangle, 2);
		it.setScale(0.6f);
		it.setTranslation(chain.chain_pos);
		matManager.mTRTexture.setTexture(0, imageManager.tNumber);
		setCoords(i);
		driver->setMaterial(matManager.mTRTexture);
		driver->setTransform(irr::video::ETS_WORLD, it);
		driver->drawVertexPrimitiveList(matManager.vChainNum, 4, matManager.iRectangle, 2);
	}
	//lp bar
	const auto& self = dInfo.isTeam1 ? dInfo.selfnames : dInfo.opponames;
	const auto& oppo = dInfo.isTeam1 ? dInfo.opponames : dInfo.selfnames;
	const auto lpframe_pos = ((dInfo.turn % 2 && dInfo.isFirst) || (!(dInfo.turn % 2) && !dInfo.isFirst)) ?
						/////kdiy/////////
						//Resize(327, 8, 630, 51 + static_cast<irr::s32>(23 * (self.size() - 1))) :
						//Resize(689, 8, 991, 51 + static_cast<irr::s32>(23 * (oppo.size() - 1)));
						Resize(15, 551, 218, 594 + static_cast<irr::s32>(23 * (self.size() - 1))) :
						Resize(689, 8, 891, 51 + static_cast<irr::s32>(23 * (oppo.size() - 1)));
						/////kdiy/////////
	// driver->draw2DRectangle(skin::DUELFIELD_TURNPLAYER_COLOR_VAL, lpframe_pos);
	// driver->draw2DRectangleOutline(lpframe_pos, skin::DUELFIELD_TURNPLAYER_OUTLINE_COLOR_VAL);
	/////kdiy/////////
	//driver->draw2DImage(imageManager.tLPFrame, Resize(330, 10, 629, 30), irr::core::recti(0, 0, 200, 20), 0, 0, true);
	//driver->draw2DImage(imageManager.tLPFrame, Resize(691, 10, 990, 30), irr::core::recti(0, 0, 200, 20), 0, 0, true);
	driver->draw2DImage(imageManager.tLPFrame_z4, Resize(161, 553, 350, 640), irr::core::recti(0, 0, 494, 228), 0, 0, true);
	driver->draw2DImage(mainGame->mode->isMode ? imageManager.modeHead[avataricon1] : imageManager.lpicon[gSoundManager->character[avataricon1]][gSoundManager->subcharacter[gSoundManager->character[avataricon1]]][0], Resize(268, 567, 318, 617), mainGame->mode->isMode ? imageManager.modehead_size[avataricon1] : irr::core::recti(0, 0, 240, 240), 0, 0, true);
	if(dField.player_desc_hints[0].size() > 0)
	    driver->draw2DImage(imageManager.tHint, Resize(151, 550, 191, 615), irr::core::recti(0, 0, 532, 649), 0, 0, true);
	driver->draw2DImage(imageManager.tLPFrame2_z4, Resize(811, 18, 1020, 105), irr::core::recti(0, 0, 494, 228), 0, 0, true);
	driver->draw2DImage(mainGame->mode->isMode ? imageManager.modeHead[avataricon2] : imageManager.lpicon[gSoundManager->character[avataricon2]][gSoundManager->subcharacter[gSoundManager->character[avataricon2]]][0], Resize(845, 32, 895, 82), mainGame->mode->isMode ? imageManager.modehead_size[avataricon2] : irr::core::recti(0, 0, 240, 240), 0, 0, true);
	if(dField.player_desc_hints[1].size() > 0)
	    driver->draw2DImage(imageManager.tHint, Resize(801, 15, 841, 80), irr::core::recti(0, 0, 532, 649), 0, 0, true);
	/////kdiy/////////

#define SKCOLOR(what) skin::LPBAR_##what##_VAL
#define RECTCOLOR(what) SKCOLOR(what##_TOP_LEFT), SKCOLOR(what##_TOP_RIGHT), SKCOLOR(what##_BOTTOM_LEFT), SKCOLOR(what##_BOTTOM_RIGHT)
#define	DRAWRECT(rect_pos,what,clip) do { driver->draw2DRectangleClip(rect_pos, RECTCOLOR(what),nullptr,clip); } while(0)
	/////kdiy/////////
	// if(dInfo.lp[0]) {
	// 	/////kdiy/////////
	// 	//const auto lpbar_pos = Resize(335, 12, 625, 28);
	// 	const auto lpbar_pos = Resize(23, 555, 215, 571);
	// 	/////kdiy/////////
	// 	if(dInfo.lp[0] < dInfo.startlp) {
	// 		/////kdiy/////////
	// 		//auto cliprect = Resize(335, 12, 335 + 290 * (dInfo.lp[0] / static_cast<double>(dInfo.startlp)), 28);
	// 		auto cliprect = Resize(23, 555, 23 + (215-23) * (dInfo.lp[0] / static_cast<double>(dInfo.startlp)), 571);
	// 		/////kdiy/////////
	// 		DRAWRECT(lpbar_pos, 1, &cliprect);
	// 	} else {
	// 		DRAWRECT(lpbar_pos, 1, nullptr);
	// 	}
	// }
	// if(dInfo.lp[1] > 0) {
	// 	/////kdiy/////////
	// 	//const auto lpbar_pos = Resize(696, 12, 986, 28);
	// 	const auto lpbar_pos = Resize(696, 12, 886, 28);
	// 	/////kdiy/////////
	// 	if(dInfo.lp[1] < dInfo.startlp) {
	// 		/////kdiy/////////
	// 		//auto cliprect = Resize(986 - 290 * (dInfo.lp[1] / static_cast<double>(dInfo.startlp)), 12, 986, 28);
	// 		auto cliprect = Resize(886 - (886-696) * (dInfo.lp[1] / static_cast<double>(dInfo.startlp)), 12, 886, 28);
	// 		/////kdiy/////////
	// 		DRAWRECT(lpbar_pos, 2, &cliprect);
	// 	} else {
	// 		DRAWRECT(lpbar_pos, 2, nullptr);
	// 	}
	// }
	/////kdiy/////////

	if(lpframe > 0 && delta_frames) {
		dInfo.lp[lpplayer] -= lpd * delta_frames;
		dInfo.strLP[lpplayer] = epro::to_wstring(std::max(0, dInfo.lp[lpplayer]));
		///////////kdiy///////////
		if(dInfo.lp[lpplayer] >= 8888888) {
            dInfo.lp[lpplayer] = 8888888l;
			dInfo.strLP[lpplayer] = L"\u221E";
        }
		///////////kdiy///////////
		lpcalpha -= 0x19 * delta_frames;
		lpframe -= delta_frames;
	}
	if(lpcstring.size()) {
        ///////////kdiy///////////
		// if(lpplayer == 0)
		// 	DrawShadowText(lpcFont, lpcstring, Resize(400, 470, 920, 520), Resize(0, 2, 2, 0), (lpcalpha << 24) | lpccolor, (lpcalpha << 24) | 0x00ffffff, true);
		// else
		// 	DrawShadowText(lpcFont, lpcstring, Resize(400, 160, 920, 210), Resize(0, 2, 2, 0), (lpcalpha << 24) | lpccolor, (lpcalpha << 24) | 0x00ffffff, true);
		if(lpplayer == 0) {
            if(lpcstring == epro::format(L"-\u221E"))
                DrawShadowText(nameFont, lpcstring, Resize(400, 470, 920, 520), Resize(0, 2, 2, 0), (lpcalpha << 24) | lpccolor, (lpcalpha << 24) | 0x00ffffff, true);
			else
                DrawShadowText(lpcFont, lpcstring, Resize(400, 470, 920, 520), Resize(0, 2, 2, 0), (lpcalpha << 24) | lpccolor, (lpcalpha << 24) | 0x00ffffff, true);
        } else {
			if(lpcstring == epro::format(L"-\u221E"))
                DrawShadowText(nameFont, lpcstring, Resize(400, 160, 920, 210), Resize(0, 2, 2, 0), (lpcalpha << 24) | lpccolor, (lpcalpha << 24) | 0x00ffffff, true);
			else
                DrawShadowText(lpcFont, lpcstring, Resize(400, 160, 920, 210), Resize(0, 2, 2, 0), (lpcalpha << 24) | lpccolor, (lpcalpha << 24) | 0x00ffffff, true);
        }
        ///////////kdiy///////////
	}

#undef SKCOLOR
#define SKCOLOR(what) skin::TIMEBAR_##what##_VAL

	if(!dInfo.isReplay && !dInfo.isSingleMode && dInfo.player_type < 7 && dInfo.time_limit) {
		////kdiy////////////
		//auto rectpos = Resize(525, 34, 625, 44);
		//auto cliprect = Resize(525, 34, 525 + dInfo.time_left[0] * 100 / dInfo.time_limit, 44);
		// DRAWRECT(rectpos, 1, &cliprect);
		// driver->draw2DRectangleOutline(rectpos, skin::TIMEBAR_1_OUTLINE_VAL);
		// rectpos = Resize(695, 34, 795, 44);
		// cliprect = Resize(795 - dInfo.time_left[1] * 100 / dInfo.time_limit, 34, 795, 44);
		// DRAWRECT(rectpos, 2, &cliprect);
		// driver->draw2DRectangleOutline(rectpos, skin::TIMEBAR_2_OUTLINE_VAL);
        driver->draw2DImage(imageManager.tTimer, Resize(380, 353, 440, 413), irr::core::recti(0, 0, 333, 332), 0, 0, true);
        DrawShadowText(numFont, epro::to_wstring(dInfo.time_left[0]), Resize(400, 373, 420, 393), Resize(0, 1, 2, 0), dInfo.time_left[0] < 10 ? 0xffff0000 : 0xffffffff, 0xff000000, true, false);
        driver->draw2DImage(imageManager.tTimer, Resize(850, 260, 900, 320), irr::core::recti(0, 0, 333, 332), 0, 0, true);
        DrawShadowText(numFont, epro::to_wstring(dInfo.time_left[1]), Resize(870, 280, 880, 300), Resize(0, 1, 2, 0), dInfo.time_left[1] < 10 ? 0xffff0000 : 0xffffffff, 0xff000000, true, false);
		////kdiy////////////
	}

    ////kdiy////////////
	// DrawShadowText(numFont, dInfo.strLP[0], Resize(330, 11, 629, 29), Resize(0, 1, 2, 0), skin::DUELFIELD_LP_1_VAL, 0xff000000, true, true);
	// DrawShadowText(numFont, dInfo.strLP[1], Resize(691, 11, 990, 29), Resize(0, 1, 2, 0), skin::DUELFIELD_LP_2_VAL, 0xff000000, true, true); 161, 553, 350, 640 691, 48, 900, 135   268, 567, 318, 617 725, 62, 775, 112
	DrawShadowText(numFont, L"LP", Resize(166, 585, 233, 604), Resize(0, 1, 2, 0), skin::DUELFIELD_LP_1_VAL, 0xff000000, true, false);
	DrawShadowText(numFont, L"LP", mainGame->mode->isMode || gSoundManager->character[avataricon1] > 0 ? Resize(875, 50, 942, 69) : Resize(820, 50, 887, 69), Resize(0, 1, 2, 0), skin::DUELFIELD_LP_2_VAL, 0xff000000, true, false);
    if(dInfo.lp[0] >= 8888888)
	    DrawShadowText(nameFont, dInfo.strLP[0], Resize(208, 580, 248, 634), Resize(0, 1, 2, 0), skin::DUELFIELD_LP_1_VAL, 0xff000000, true, true);
    else
	    DrawShadowText(lpFont, dInfo.strLP[0], Resize(208, 600, 268, 624), Resize(0, 1, 2, 0), skin::DUELFIELD_LP_1_VAL, 0xff000000, true, false);
    if(dInfo.lp[1] >= 8888888)
	    DrawShadowText(nameFont, dInfo.strLP[1], mainGame->mode->isMode || gSoundManager->character[avataricon1] > 0 ? Resize(917, 45, 957, 89) : Resize(868, 45, 908, 89), Resize(0, 1, 2, 0), skin::DUELFIELD_LP_2_VAL, 0xff000000, true, true);
    else
	    DrawShadowText(lpFont, dInfo.strLP[1], mainGame->mode->isMode || gSoundManager->character[avataricon1] > 0 ? Resize(917, 65, 977, 89) : Resize(868, 65, 928, 89), Resize(0, 1, 2, 0), skin::DUELFIELD_LP_2_VAL, 0xff000000, true, false);

	//irr::core::recti p1size = Resize(335, 31, 629, 50);
	//irr::core::recti p2size = Resize(986, 31, 986, 50);
	// {
	// 	int i = 0;
	// 	for (const auto& player : self) {
	// 		if (i++ == dInfo.current_player[0])
	// 			textFont->drawustring(player, p1size, 0xffffffff, false, false, 0);
	// 		else
	// 			textFont->drawustring(player, p1size, 0xff808080, false, false, 0);
	// 		p1size += irr::core::vector2di{ 0, p1size.getHeight() + ResizeY(4) };
	// 	}
	// 	i = 0;
	// 	const auto basecorner = p2size.UpperLeftCorner.X;
	// 	for (const auto& player : oppo) {
	// 		const irr::core::ustring utext(player);
	// 		auto cld = textFont->getDimensionustring(utext);
	// 		p2size.UpperLeftCorner.X = basecorner - cld.Width;
	// 		if (i++ == dInfo.current_player[1])
	// 			textFont->drawustring(utext, p2size, 0xffffffff, false, false, 0);
	// 		else
	// 			textFont->drawustring(utext, p2size, 0xff808080, false, false, 0);
	// 		p2size += irr::core::vector2di{ 0, p2size.getHeight() + ResizeY(4) };
	// 	}
	// }691, 48, 900, 135    811, 18, 1020, 105
	irr::core::recti p1size = Resize(355, 578, 375, 597);
	irr::core::recti p2size = Resize(719, 30, 737, 49);
	{
		int i = 0;
		for (const auto& player : self) {
			if (i++ == dInfo.current_player[0])
				textFont->drawustring(player, Resize(196, 569, 226, 580), skin::DUELFIELD_LP_1_VAL, false, true, 0);
			else {
				textFont->drawustring(player, p1size, 0xff00ff00, false, true, 0);
                p1size += irr::core::vector2di{ 0, p1size.getHeight() + ResizeY(4) };
            }
		}
		i = 0;
		for (const auto& player : oppo) {
			if (i++ == dInfo.current_player[1])
				textFont->drawustring(player, mainGame->mode->isMode || gSoundManager->character[avataricon1] > 0 ? Resize(903, 34, 933, 45) : Resize(850, 34, 880, 45), skin::DUELFIELD_LP_2_VAL, false, true, 0);
			else {
				textFont->drawustring(player, p2size, 0xff00ff00, false, true, 0);
                p2size += irr::core::vector2di{ 0, p2size.getHeight() + ResizeY(4) };
            }
		}
	}
	////kdiy////////////
	/*driver->draw2DRectangle(Resize(632, 10, 688, 30), 0x00000000, 0x00000000, 0xffffffff, 0xffffffff);
	driver->draw2DRectangle(Resize(632, 30, 688, 50), 0xffffffff, 0xffffffff, 0x00000000, 0x00000000);*/
	////kdiy////////////
	//DrawShadowText(lpcFont, gDataManager->GetNumString(dInfo.turn), Resize(635, 5, 685, 40), Resize(0, 0, 2, 0), skin::DUELFIELD_TURN_COUNT_VAL, 0x80000000, true);
	DrawShadowText(turnFont, L"TURN " + gDataManager->GetNumString(dInfo.turn), Resize(450, 5, 625, 40), Resize(0, 0, 8, 0), 0xff000000, 0xffff00ff, true);
	////kdiy////////////
#undef DRAWRECT
#undef LPCOLOR
#undef SKCOLOR

	ClientCard* pcard;
	const size_t pzones[]{ dInfo.GetPzoneIndex(0), dInfo.GetPzoneIndex(1) };
	for (size_t p = 0; p < 2; ++p) {
		for (size_t i = 0; i < 7; ++i) {
			pcard = dField.mzone[p][i];
			/////////kdiy////////////
			//if (pcard && pcard->code != 0 && (p == 0 || (pcard->position & POS_FACEUP)))
				// DrawStatus(pcard);
			if(!pcard) continue;
			if(pcard->code != 0 && (p == 0 || (pcard->position & POS_FACEUP)) && (!pcard->is_sanct || (pcard->cmdFlag & COMMAND_ATTACK) || pcard->is_attack || pcard->is_attacked) && !pcard->is_battling)
				DrawStatus(pcard, pcard->is_sanct ? true : false);
			/////////kdiy////////////
		}
		/////////kdiy////////////
		for (int i = 0; i < 5; ++i) {
			pcard = dField.szone[p][i];
			if(!pcard) continue;
			if(pcard->code != 0 && (p == 0 || (pcard->position & POS_FACEUP)) && (pcard->is_orica || (pcard->cmdFlag & COMMAND_ATTACK) || pcard->is_attack || pcard->is_attacked) && !pcard->is_battling)
				DrawStatus(pcard, !pcard->is_orica ? true : false);
		}
		// // Draw pendulum scales
		// for (const auto pzone : pzones) {
		// 	pcard = dField.szone[p][pzone];
		// 	if (pcard && (pcard->type & TYPE_PENDULUM) && !pcard->equipTarget)
		// 		DrawPendScale(pcard);
		// }
        /////////kdiy////////////
		if (dField.extra[p].size()) {
			const auto str = (dField.extra_p_count[p]) ? epro::format(L"{}({})", dField.extra[p].size(), dField.extra_p_count[p]) : epro::format(L"{}", dField.extra[p].size());
			DrawStackIndicator(str, matManager.getExtra()[p], (p == 1));
		}
		if (dField.deck[p].size())
			DrawStackIndicator(gDataManager->GetNumString(dField.deck[p].size()), matManager.getDeck()[p], (p == 1));
		if (dField.grave[p].size())
			DrawStackIndicator(gDataManager->GetNumString(dField.grave[p].size()), matManager.getGrave()[p], (p == 1));
		if (dField.remove[p].size())
			DrawStackIndicator(gDataManager->GetNumString(dField.remove[p].size()), matManager.getRemove()[p], (p == 1));
	}
}
/*
Draws the stats of a card based on its relative position
*/
//kidy///////
//void Game::DrawStatus(ClientCard* pcard) {
void Game::DrawStatus(ClientCard* pcard, bool attackonly) {
//kidy///////
	///kdiy////////
	if(videostart) return;
	///kdiy////////
	auto getcoords = [collisionmanager=device->getSceneManager()->getSceneCollisionManager()](const irr::core::vector3df& pos3d) {
		return collisionmanager->getScreenCoordinatesFrom3DPosition(pos3d);
	};
	int x1, y1, x2, y2;
	////kdiy//////////
	//if (pcard->controler == 0) {
	//	auto coords = getcoords({ pcard->curPos.X, (0.39f + pcard->curPos.Y), pcard->curPos.Z });
	//	x1 = coords.X;
	//	y1 = coords.Y;
	//	coords = getcoords({ (pcard->curPos.X - 0.48f), (pcard->curPos.Y - 0.66f), pcard->curPos.Z });
	//	x2 = coords.X;
	//	y2 = coords.Y;
	//} else {
	//	auto coords = getcoords({ pcard->curPos.X, (pcard->curPos.Y - 0.66f), pcard->curPos.Z });
	//	x1 = coords.X;
	//	y1 = coords.Y;
	//	coords = getcoords({ (pcard->curPos.X - 0.48f), (0.39f + pcard->curPos.Y), pcard->curPos.Z });
	//	x2 = coords.X;
	//	y2 = coords.Y;
	//}
	int x3, y3;
	if (pcard->controler == 0) {
		auto coords = getcoords({ pcard->curPos.X + 0.05f, (0.49f + pcard->curPos.Y), (pcard->curPos.Z + 0.25f) });
		x1 = coords.X;
		y1 = coords.Y;
		coords = getcoords({ pcard->curPos.X, (0.55f + pcard->curPos.Y), (pcard->curPos.Z + 0.25f) });
		y3 = coords.Y;
		coords = getcoords({ (pcard->curPos.X - 0.3f), (pcard->curPos.Y - 0.5f), (pcard->curPos.Z + 0.25f) });
		x2 = coords.X;
		y2 = coords.Y;
		coords = getcoords({ (pcard->curPos.X + 0.45f), (pcard->curPos.Y - 0.5f), (pcard->curPos.Z + 0.25f) });
		x3 = coords.X;
	} else {
		auto coords = getcoords({ pcard->curPos.X + 0.1f, (pcard->curPos.Y - 0.4f), (pcard->curPos.Z + 0.25f) });
		x1 = coords.X;
		y1 = coords.Y;
		coords = getcoords({ pcard->curPos.X, (pcard->curPos.Y - 0.4f), (pcard->curPos.Z + 0.25f) });
		y3 = coords.Y;
		coords = getcoords({ (pcard->curPos.X - 0.3f), (0.59f + pcard->curPos.Y), (pcard->curPos.Z + 0.25f) });
		x2 = coords.X;
		y2 = coords.Y;
		coords = getcoords({ (pcard->curPos.X + 0.45f), (0.59f + pcard->curPos.Y), (pcard->curPos.Z + 0.25f) });
		x3 = coords.X;
	}
	////kdiy//////////

	auto GetAtkColor = [&pcard] {
		////kdiy//////////
		if(pcard->position == POS_FACEDOWN || pcard->position == POS_FACEUP_DEFENSE || pcard->position == POS_FACEDOWN_DEFENSE)
			return irr::video::SColor(255, 200, 200, 200);
		////kdiy//////////
		if(pcard->attack > pcard->base_attack)
			return skin::DUELFIELD_HIGHER_CARD_ATK_VAL;
		if(pcard->attack < pcard->base_attack)
			return skin::DUELFIELD_LOWER_CARD_ATK_VAL;
		return skin::DUELFIELD_UNCHANGED_CARD_ATK_VAL;
	};

	auto GetDefColor = [&pcard] {
		////kdiy//////////
		if(!(pcard->position == POS_FACEDOWN || pcard->position == POS_FACEUP_DEFENSE || pcard->position == POS_FACEDOWN_DEFENSE))
			return irr::video::SColor(255, 200, 200, 200);
		////kdiy//////////
		if(pcard->defense > pcard->base_defense)
			return skin::DUELFIELD_HIGHER_CARD_DEF_VAL;
		if(pcard->defense < pcard->base_defense)
			return skin::DUELFIELD_LOWER_CARD_DEF_VAL;
		return skin::DUELFIELD_UNCHANGED_CARD_DEF_VAL;
	};

	auto GetLevelColor = [&pcard] {
		if(pcard->type & TYPE_TUNER)
			return skin::DUELFIELD_CARD_TUNER_LEVEL_VAL;
		return skin::DUELFIELD_CARD_LEVEL_VAL;
	};

    //////kdiy////////
	//const auto atk = adFont->getDimensionustring(pcard->atkstring);
	//const auto slash = adFont->getDimensionustring(L"/");
	auto lv = atkFont->getDimensionustring(pcard->lvstring);
	auto rk = atkFont->getDimensionustring(pcard->rkstring);
	auto lk = atkFont->getDimensionustring(pcard->linkstring);
	const auto atk = atkFont->getDimensionustring(pcard->atkstring);
	const auto slash = atkFont->getDimensionustring(L"/");
	const auto slash2 = atkFont->getDimensionustring(L"|");
	//////kdiy////////
	const auto half_slash_width = static_cast<int>(std::floor(slash.Width / 2));

	const auto padding_1111 = Resize(1, 1, 1, 1);
	const auto padding_1011 = Resize(1, 0, 1, 1);

	////kdiy//////////
	// if(pcard->type & TYPE_LINK) {
	// 	DrawShadowText(adFont, pcard->atkstring, irr::core::recti(x1 - std::floor(atk.Width / 2), y1, x1 + std::floor(atk.Width / 2), y1 + 1),
	// 				   padding_1111, GetAtkColor(), 0xff000000, true);
	// } else {
	// 	DrawShadowText(adFont, L"/", irr::core::recti(x1 - half_slash_width, y1, x1 + half_slash_width, y1 + 1), padding_1111, 0xffffffff, 0xff000000, true);
	// 	DrawShadowText(adFont, pcard->atkstring, irr::core::recti(x1 - half_slash_width - atk.Width - slash.Width, y1, x1 - half_slash_width, y1 + 1),
	// 				   padding_1111, GetAtkColor(), 0xff000000);
	// 	DrawShadowText(adFont, pcard->defstring, irr::core::recti(x1 + half_slash_width + slash.Width, y1, x1 - half_slash_width, y1 + 1),
	// 				   padding_1111, GetDefColor(), 0xff000000);
	// }
	// if (pcard->level != 0 && pcard->rank != 0) {
	// 	DrawShadowText(adFont, L"/", irr::core::recti(x2 - half_slash_width, y2, x2 + half_slash_width, y2 + 1),
	// 	               padding_1111, 0xffffffff, 0xff000000, true);
		//DrawShadowText(adFont, pcard->lvstring, irr::core::recti(x2 - half_slash_width - atk.Width - slash.Width, y2, x2 - half_slash_width, y2 + 1),
					   //padding_1111, GetLevelColor(), 0xff000000);
		//DrawShadowText(adFont, pcard->rkstring, irr::core::recti(x2 + half_slash_width + slash.Width, y2, x2 - half_slash_width, y2 + 1),
					   //padding_1111, skin::DUELFIELD_CARD_RANK_VAL, 0xff000000);
	// else if (pcard->rank != 0)
	// 	DrawShadowText(adFont, pcard->rkstring, irr::core::recti(x2, y2, x2 + 1, y2 + 1), padding_1011, skin::DUELFIELD_CARD_RANK_VAL, 0xff000000);
	// else if (pcard->level != 0)
	// 	DrawShadowText(adFont, pcard->lvstring, irr::core::recti(x2, y2, x2 + 1, y2 + 1), padding_1011, GetLevelColor(), 0xff000000);
	// else if (pcard->link != 0)
	// 	DrawShadowText(adFont, pcard->linkstring, irr::core::recti(x2, y2, x2 + 1, y2 + 1), padding_1011, skin::DUELFIELD_CARD_LINK_VAL, 0xff000000);
	if(pcard->type & TYPE_LINK) {
        auto font = pcard->attack >= 8888888 ? numFont0 : atkFont;
		DrawShadowText(font, pcard->atkstring, irr::core::recti(x1 - std::floor(atk.Width / 2), y1, x1 + std::floor(atk.Width / 2), y1 + 1),
					   padding_1111, GetAtkColor(), 0xff000000, true);
	} else {
        auto font = pcard->attack >= 8888888 ? (pcard->position == POS_FACEDOWN || pcard->position == POS_FACEUP_DEFENSE || pcard->position == POS_FACEDOWN_DEFENSE) ? adFont0 : numFont0 :  (pcard->position == POS_FACEDOWN || pcard->position == POS_FACEUP_DEFENSE || pcard->position == POS_FACEDOWN_DEFENSE) ? defFont : atkFont;
		DrawShadowText(font, L"/", irr::core::recti(x1 - half_slash_width, y1, x1 + half_slash_width, y1 + 1), padding_1111, 0xffffffff, 0xff000000, true);
		DrawShadowText(font, pcard->atkstring, irr::core::recti(x1 - half_slash_width - atk.Width, (pcard->position == POS_FACEDOWN || pcard->position == POS_FACEUP_DEFENSE || pcard->position == POS_FACEDOWN_DEFENSE) ? y3 : y1, x1 - half_slash_width, ((pcard->position == POS_FACEDOWN || pcard->position == POS_FACEUP_DEFENSE || pcard->position == POS_FACEDOWN_DEFENSE) ? y3 : y1) + 1),
					   padding_1111, GetAtkColor(), 0xff000000);
		DrawShadowText(pcard->defense >= 8888888 ? (pcard->position == POS_FACEDOWN || pcard->position == POS_FACEUP_DEFENSE || pcard->position == POS_FACEDOWN_DEFENSE) ? adFont0 : numFont0 :  (pcard->position == POS_FACEDOWN || pcard->position == POS_FACEUP_DEFENSE || pcard->position == POS_FACEDOWN_DEFENSE) ? atkFont : defFont, pcard->defstring, irr::core::recti(x1 + half_slash_width, (pcard->position == POS_FACEDOWN || pcard->position == POS_FACEUP_DEFENSE || pcard->position == POS_FACEDOWN_DEFENSE) ? y1 : y3, x1 - half_slash_width, ((pcard->position == POS_FACEDOWN || pcard->position == POS_FACEUP_DEFENSE || pcard->position == POS_FACEDOWN_DEFENSE) ? y1 : y3) + 1),
					   padding_1111, GetDefColor(), 0xff000000);
	}

	if(!attackonly) {
	//has lv, rk, lk
	if ((pcard->level != 0 || (pcard->type & (TYPE_FUSION | TYPE_SYNCHRO | TYPE_RITUAL))) && (pcard->rank != 0 || (pcard->type & TYPE_XYZ)) && (pcard->link != 0 || (pcard->type & TYPE_LINK))) {
		//DrawShadowText(adFont, pcard->rkstring, irr::core::recti(x2 - std::floor(rk.Width / 2), y2, x2 + std::floor(rk.Width / 2), y2 + 1),
		//	padding_1111, skin::DUELFIELD_CARD_RANK_VAL, 0xff000000, true);
		//DrawShadowText(adFont, L"/", irr::core::recti(x2 - std::floor(rk.Width / 2) - slash.Width, y2, x2 - std::floor(rk.Width / 2), y2 + 1),
		//	padding_1111, 0xffffffff, 0xff000000);
		//DrawShadowText(adFont, L"/", irr::core::recti(x2 + std::floor(rk.Width / 2), y2, x2 + std::floor(rk.Width / 2) + slash.Width, y2 + 1),
		//	padding_1111, 0xffffffff, 0xff000000);
		//DrawShadowText(adFont, pcard->lvstring, irr::core::recti(x2 - std::floor(rk.Width / 2) - slash.Width - lv.Width, y2, x2 - std::floor(rk.Width / 2) - slash.Width, y2 + 1),
		//	padding_1111, GetLevelColor(), 0xff000000);
		//DrawShadowText(adFont, pcard->linkstring, irr::core::recti(x2 + std::floor(rk.Width / 2) + slash.Width, y2, x2 + std::floor(rk.Width / 2) + slash.Width + lk.Width, y2 + 1),
		//	padding_1111, skin::DUELFIELD_CARD_LINK_VAL, 0xff000000);
		DrawShadowText(atkFont, L"|", irr::core::recti(x2 + lv.Width, y2, x2 + lv.Width + slash2.Width, y2 + 1),
			padding_1111, 0xffffffff, 0xff000000, true);
		DrawShadowText(atkFont, pcard->lvstring, irr::core::recti(x2, y2, x2 + 1, y2 + 1), padding_1111, GetLevelColor(), 0xff000000);
		DrawShadowText(atkFont, pcard->rkstring, irr::core::recti(x2 + lv.Width + slash2.Width, y2, x2 + lv.Width + slash2.Width + rk.Width, y2 + 1),
			padding_1111, skin::DUELFIELD_CARD_RANK_VAL, 0xff000000, true);
		DrawShadowText(atkFont, pcard->linkstring, irr::core::recti(x3, y2, x3 + 1, y2 + 1), padding_1111, skin::DUELFIELD_CARD_LINK_VAL, 0xff000000);

        matManager.mTexture.setTexture(0, imageManager.tLvRank);
        driver->setMaterial(matManager.mTexture);
        irr::core::matrix4 atk;
        atk.setTranslation(pcard->curPos + irr::core::vector3df(-0.4f, pcard->controler == 0 ? -0.385f : 0.705f, 0.25f));
        driver->setTransform(irr::video::ETS_WORLD, atk);
        driver->drawVertexPrimitiveList(matManager.vXyz, 4, matManager.iRectangle, 2);

        matManager.mTexture.setTexture(0, imageManager.tLink);
        driver->setMaterial(matManager.mTexture);
        irr::core::matrix4 atk2;
        atk2.setTranslation(pcard->curPos + irr::core::vector3df(0.35f, pcard->controler == 0 ? -0.385f : 0.755f, 0.25f));
        driver->setTransform(irr::video::ETS_WORLD, atk2);
        driver->drawVertexPrimitiveList(matManager.vXyz, 4, matManager.iRectangle, 2);
	//has lk, rk
	} else if ((pcard->link != 0 || (pcard->type & TYPE_LINK)) && (pcard->rank != 0 || (pcard->type & TYPE_XYZ))) {
		//DrawShadowText(adFont, L"/", irr::core::recti(x2 - std::floor(slash.Width / 2), y2, x2 + std::floor(slash.Width / 2), y2 + 1),
		//	padding_1111, 0xffffffff, 0xff000000, true);
		//DrawShadowText(adFont, pcard->rkstring, irr::core::recti(x2 - std::floor(slash.Width / 2) - rk.Width, y2, x2 - std::floor(slash.Width / 2), y2 + 1),
		//	padding_1111, skin::DUELFIELD_CARD_RANK_VAL, 0xff000000);
		//DrawShadowText(adFont, pcard->linkstring, irr::core::recti(x2 + std::floor(slash.Width / 2), y2, x2 + std::floor(slash.Width / 2) + lk.Width, y2 + 1),
		//	padding_1011, skin::DUELFIELD_CARD_LINK_VAL, 0xff000000);
		DrawShadowText(atkFont, pcard->rkstring, irr::core::recti(x2, y2, x2 + 1, y2 + 1), padding_1111, skin::DUELFIELD_CARD_RANK_VAL, 0xff000000);
		DrawShadowText(atkFont, pcard->linkstring, irr::core::recti(x3, y2, x3 + 1, y2 + 1), padding_1111, skin::DUELFIELD_CARD_LINK_VAL, 0xff000000);

        matManager.mTexture.setTexture(0, imageManager.tRank);
        driver->setMaterial(matManager.mTexture);
        irr::core::matrix4 atk;
        atk.setTranslation(pcard->curPos + irr::core::vector3df(-0.4f, pcard->controler == 0 ? -0.385f : 0.705f, 0.25f));
        driver->setTransform(irr::video::ETS_WORLD, atk);
        driver->drawVertexPrimitiveList(matManager.vXyz, 4, matManager.iRectangle, 2);

        matManager.mTexture.setTexture(0, imageManager.tLink);
        driver->setMaterial(matManager.mTexture);
        irr::core::matrix4 atk2;
        atk2.setTranslation(pcard->curPos + irr::core::vector3df(0.35f, pcard->controler == 0 ? -0.385f : 0.755f, 0.25f));
        driver->setTransform(irr::video::ETS_WORLD, atk2);
        driver->drawVertexPrimitiveList(matManager.vXyz, 4, matManager.iRectangle, 2);
	//has lk, lv
	} else if ((pcard->link != 0 || (pcard->type & TYPE_LINK)) && (pcard->level != 0 || (pcard->type & (TYPE_FUSION | TYPE_SYNCHRO | TYPE_RITUAL)))) {
		//DrawShadowText(adFont, L"/", irr::core::recti(x2 - std::floor(slash.Width / 2), y2, x2 + std::floor(slash.Width / 2), y2 + 1),
		//	padding_1111, 0xffffffff, 0xff000000, true);
		//DrawShadowText(adFont, pcard->lvstring, irr::core::recti(x2 - half_slash_width - lv.Width, y2, x2 - half_slash_width, y2 + 1),
		//	padding_1111, GetLevelColor(), 0xff000000);
		//DrawShadowText(adFont, pcard->linkstring, irr::core::recti(x2 + std::floor(slash.Width / 2), y2, x2 + std::floor(slash.Width / 2) + lk.Width, y2 + 1),
		//	padding_1011, skin::DUELFIELD_CARD_LINK_VAL, 0xff000000);
		DrawShadowText(atkFont, pcard->lvstring, irr::core::recti(x2, y2, x2 + 1, y2 + 1), padding_1111, GetLevelColor(), 0xff000000);
		DrawShadowText(atkFont, pcard->linkstring, irr::core::recti(x3, y2, x3 + 1, y2 + 1), padding_1111, skin::DUELFIELD_CARD_LINK_VAL, 0xff000000);

		matManager.mTexture.setTexture(0, imageManager.tLevel);
        driver->setMaterial(matManager.mTexture);
        irr::core::matrix4 atk;
        atk.setTranslation(pcard->curPos + irr::core::vector3df(-0.4f, pcard->controler == 0 ? -0.385f : 0.705f, 0.25f));
        driver->setTransform(irr::video::ETS_WORLD, atk);
        driver->drawVertexPrimitiveList(matManager.vXyz, 4, matManager.iRectangle, 2);

        matManager.mTexture.setTexture(0, imageManager.tLink);
        driver->setMaterial(matManager.mTexture);
        irr::core::matrix4 atk2;
        atk2.setTranslation(pcard->curPos + irr::core::vector3df(0.35f, pcard->controler == 0 ? -0.385f : 0.755f, 0.25f));
        driver->setTransform(irr::video::ETS_WORLD, atk2);
        driver->drawVertexPrimitiveList(matManager.vXyz, 4, matManager.iRectangle, 2);
	//has lv, rk
	} else if ((pcard->level != 0 || (pcard->type & (TYPE_FUSION | TYPE_SYNCHRO | TYPE_RITUAL))) && (pcard->rank != 0 || (pcard->type & TYPE_XYZ))) {
		//DrawShadowText(adFont, L"/", irr::core::recti(x2 - half_slash_width, y2, x2 + half_slash_width, y2 + 1),
		//	padding_1111, 0xffffffff, 0xff000000, true);
		//DrawShadowText(adFont, pcard->lvstring, irr::core::recti(x2 - half_slash_width - lv.Width, y2, x2 - half_slash_width, y2 + 1),
		//	padding_1111, GetLevelColor(), 0xff000000);
		//DrawShadowText(adFont, pcard->rkstring, irr::core::recti(x2 + half_slash_width, y2, x2 + half_slash_width + rk.Width, y2 + 1),
		//	padding_1111, skin::DUELFIELD_CARD_RANK_VAL, 0xff000000);
		DrawShadowText(atkFont, L"|", irr::core::recti(x2 + lv.Width, y2, x2 + lv.Width + slash2.Width, y2 + 1),
			padding_1111, 0xffffffff, 0xff000000, true);
		DrawShadowText(atkFont, pcard->lvstring, irr::core::recti(x2, y2, x2 + 1, y2 + 1), padding_1111, GetLevelColor(), 0xff000000);
		DrawShadowText(atkFont, pcard->rkstring, irr::core::recti(x2 + lv.Width + slash2.Width, y2, x2 + lv.Width + slash2.Width + rk.Width, y2 + 1),
			padding_1111, skin::DUELFIELD_CARD_RANK_VAL, 0xff000000);

        matManager.mTexture.setTexture(0, imageManager.tLvRank);
        driver->setMaterial(matManager.mTexture);
        irr::core::matrix4 atk;
        atk.setTranslation(pcard->curPos + irr::core::vector3df(-0.4f, pcard->controler == 0 ? -0.385f : 0.705f, 0.25f));
        driver->setTransform(irr::video::ETS_WORLD, atk);
        driver->drawVertexPrimitiveList(matManager.vXyz, 4, matManager.iRectangle, 2);
	//has rk
	} else if (pcard->rank != 0 || (pcard->type & TYPE_XYZ)) {
		DrawShadowText(atkFont, pcard->rkstring, irr::core::recti(x2, y2, x2 + 1, y2 + 1), padding_1011, skin::DUELFIELD_CARD_RANK_VAL, 0xff000000);

        matManager.mTexture.setTexture(0, imageManager.tRank);
        driver->setMaterial(matManager.mTexture);
        irr::core::matrix4 atk;
        atk.setTranslation(pcard->curPos + irr::core::vector3df(-0.4f, pcard->controler == 0 ? -0.385f : 0.705f, 0.25f));
        driver->setTransform(irr::video::ETS_WORLD, atk);
        driver->drawVertexPrimitiveList(matManager.vXyz, 4, matManager.iRectangle, 2);
	//has lv
    } else if (pcard->level != 0 || !(pcard->type & (TYPE_XYZ | TYPE_LINK))) {
		DrawShadowText(atkFont, pcard->lvstring, irr::core::recti(x2, y2, x2 + 1, y2 + 1), padding_1011, GetLevelColor(), 0xff000000);
	
        matManager.mTexture.setTexture(0, imageManager.tLevel);
        driver->setMaterial(matManager.mTexture);
        irr::core::matrix4 atk;
        atk.setTranslation(pcard->curPos + irr::core::vector3df(-0.4f, pcard->controler == 0 ? -0.385f : 0.705f, 0.25f));
        driver->setTransform(irr::video::ETS_WORLD, atk);
        driver->drawVertexPrimitiveList(matManager.vXyz, 4, matManager.iRectangle, 2);
	//has lk
	} else if (pcard->link != 0 || (pcard->type & TYPE_LINK)) {
		DrawShadowText(atkFont, pcard->linkstring, irr::core::recti(x2, y2, x2 + 1, y2 + 1), padding_1011, skin::DUELFIELD_CARD_LINK_VAL, 0xff000000);
	
        matManager.mTexture.setTexture(0, imageManager.tLink);
        driver->setMaterial(matManager.mTexture);
        irr::core::matrix4 atk;
        atk.setTranslation(pcard->curPos + irr::core::vector3df(-0.4f, pcard->controler == 0 ? -0.385f : 0.755f, 0.25f));
        driver->setTransform(irr::video::ETS_WORLD, atk);
        driver->drawVertexPrimitiveList(matManager.vXyz, 4, matManager.iRectangle, 2);
	}
	}
	////kdiy//////////
}
/*
Draws the pendulum scale value of a card in the pendulum zone based on its relative position
*/
void Game::DrawPendScale(ClientCard* pcard) {
	///kdiy////////
	if(videostart) return;
	///kdiy////////
	int swap = (pcard->sequence > 1 && pcard->sequence != 6) ? 1 : 0;
	float x0, y0, reverse = (pcard->controler == 0) ? 1.0f : -1.0f;
	std::wstring scale;
	if (swap) {
		x0 = pcard->curPos.X - 0.35f * reverse;
		scale = pcard->rscstring;
	} else {
		x0 = pcard->curPos.X + 0.35f * reverse;
		scale = pcard->lscstring;
	}
	if (pcard->controler == 0) {
		swap = 1 - swap;
		y0 = pcard->curPos.Y - 0.56f;
	} else
		y0 = pcard->curPos.Y + 0.29f;
	auto coords = device->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition({ x0, y0, pcard->curPos.Z });
	DrawShadowText(adFont, scale, irr::core::recti(coords.X - (12 * swap), coords.Y, coords.X + (12 * (1 - swap)), coords.Y - 800),
				   Resize(1, 1, 1, 1), skin::DUELFIELD_CARD_PSCALE_VAL, 0xff000000, true);
}
/*
Draws the text in the middle of the bottom side of the zone
*/
void Game::DrawStackIndicator(epro::wstringview text, const Materials::QuadVertex v, bool opponent) {
	///kdiy////////
	if(videostart) return;
	///kdiy////////
	const irr::core::ustring utext(text);
	const auto dim = textFont->getDimensionustring(utext) / 2;
	//int width = dim.Width / 2, height = dim.Height / 2;
	float x0 = (v[0].Pos.X + v[1].Pos.X) / 2.0f;
	float y0 = (opponent) ? v[0].Pos.Y : v[2].Pos.Y;
	auto coords = device->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition({ x0, y0, 0 });
	DrawShadowText(numFont, utext, irr::core::recti(coords.X - dim.Width, coords.Y - dim.Height, coords.X + dim.Width, coords.Y + dim.Height),
				   Resize(0, 1, 0, 1), skin::DUELFIELD_STACK_VAL, 0xff000000);
}
void Game::DrawGUI() {
	if(imageLoading.size()) {
		for(auto mit = imageLoading.begin(); mit != imageLoading.end();) {
			int check = 0;
			mit->first->setImage(imageManager.GetTextureCard(mit->second, imgType::ART, false, false, &check));
			if(check != 2)
				mit = imageLoading.erase(mit);
			else
				++mit;
		}
	}
	imageManager.RefreshCachedTextures();
	for(auto fit = fadingList.begin(); fit != fadingList.end();) {
		auto fthis = fit;
		FadingUnit& fu = *fthis;
		if(fu.fadingFrame != 0.0f) {
			fit++;
			float movetime = std::min(fu.fadingFrame, (float)delta_time);
			fu.guiFading->setVisible(true);
			if(fu.isFadein) {
				if(fu.fadingFrame > (int)(5.0f * 1000.0f / 60.0f)) {
					fu.fadingUL.X -= fu.fadingDest.X * movetime;
					fu.fadingLR.X += fu.fadingDest.X * movetime;
					fu.fadingFrame -= movetime;
					if(!fu.fadingFrame)
						fu.fadingFrame += 0.0001f;
					fu.guiFading->setRelativePosition(irr::core::recti(fu.fadingUL, fu.fadingLR));
				} else {
					fu.fadingUL.Y -= fu.fadingDest.Y * movetime;
					fu.fadingLR.Y += fu.fadingDest.Y * movetime;
					fu.fadingFrame -= movetime;
					if(!fu.fadingFrame) {
						fu.guiFading->setRelativePosition(fu.fadingSize);
						if(fu.guiFading == wPosSelect) {
							btnPSAU->setDrawImage(true);
							btnPSAD->setDrawImage(true);
							btnPSDU->setDrawImage(true);
							btnPSDD->setDrawImage(true);
						}
						if(fu.guiFading == wCardSelect) {
							for(int i = 0; i < 5; ++i)
								btnCardSelect[i]->setDrawImage(true);
						}
						if(fu.guiFading == wCardDisplay) {
							for(int i = 0; i < 5; ++i)
								btnCardDisplay[i]->setDrawImage(true);
						}
					} else
						fu.guiFading->setRelativePosition(irr::core::recti(fu.fadingUL, fu.fadingLR));
				}
			} else {
				if(fu.fadingFrame > (5.0f * 1000.0f / 60.0f)) {
					fu.fadingUL.Y += fu.fadingDest.Y * movetime;
					fu.fadingLR.Y -= fu.fadingDest.Y * movetime;
					fu.fadingFrame -= movetime;
					if(!fu.fadingFrame)
						fu.fadingFrame += 0.0001f;
					fu.guiFading->setRelativePosition(irr::core::recti(fu.fadingUL, fu.fadingLR));
				} else {
					fu.fadingUL.X += fu.fadingDest.X * movetime;
					fu.fadingLR.X -= fu.fadingDest.X * movetime;
					fu.fadingFrame -= movetime;
					if(!fu.fadingFrame) {
						fu.guiFading->setVisible(false);
						fu.guiFading->setRelativePosition(fu.fadingSize);
						if(fu.guiFading == wPosSelect) {
							btnPSAU->setDrawImage(true);
							btnPSAD->setDrawImage(true);
							btnPSDU->setDrawImage(true);
							btnPSDD->setDrawImage(true);
						}
						if(fu.guiFading == wCardSelect) {
							for(int i = 0; i < 5; ++i)
								btnCardSelect[i]->setDrawImage(true);
						}
						if(fu.guiFading == wCardDisplay) {
							for(int i = 0; i < 5; ++i)
								btnCardDisplay[i]->setDrawImage(true);
						}
					} else
						fu.guiFading->setRelativePosition(irr::core::recti(fu.fadingUL, fu.fadingLR));
				}
				if(fu.signalAction && !fu.fadingFrame) {
					DuelClient::SendResponse();
					fu.signalAction = false;
				}
			}
		} else if(fu.autoFadeoutFrame) {
			fit++;
			uint32_t movetime = std::min<uint32_t>(delta_time, fu.autoFadeoutFrame);
			fu.autoFadeoutFrame -= movetime;
			fu.guiFading->setEnabled(fu.wasEnabled);
			if(!fu.autoFadeoutFrame)
				HideElement(fu.guiFading);
		} else {
			fu.guiFading->setEnabled(fu.wasEnabled);
			if(fu.wasEnabled){
				const auto prevfocused = env->getFocus();
				env->setFocus(fu.guiFading);
				if(prevfocused && (prevfocused->getType() == irr::gui::EGUIET_EDIT_BOX))
					env->setFocus(prevfocused);
			}
			fu.guiFading->setRelativePosition(fu.fadingSize);
			fit = fadingList.erase(fthis);
		}
	}
	env->drawAll();
}
inline void SetS3DVertex(Materials::QuadVertex v, irr::f32 x1, irr::f32 y1, irr::f32 x2, irr::f32 y2, irr::f32 z, irr::f32 nz, irr::f32 tu1, irr::f32 tv1, irr::f32 tu2, irr::f32 tv2) {
	v[0] = irr::video::S3DVertex(x1, y1, z, 0, 0, nz, irr::video::SColor(255, 255, 255, 255), tu1, tv1);
	v[1] = irr::video::S3DVertex(x2, y1, z, 0, 0, nz, irr::video::SColor(255, 255, 255, 255), tu2, tv1);
	v[2] = irr::video::S3DVertex(x1, y2, z, 0, 0, nz, irr::video::SColor(255, 255, 255, 255), tu1, tv2);
	v[3] = irr::video::S3DVertex(x2, y2, z, 0, 0, nz, irr::video::SColor(255, 255, 255, 255), tu2, tv2);
}
void Game::DrawSpec() {
	const auto drawrect2 = ResizeWin(574, 150, 574 + CARD_IMG_WIDTH, 150 + CARD_IMG_HEIGHT);
    //////kdiy//////////
	const auto drawrect2_hd = ResizeWin(574, 150, 574 + 484, 150 + 700);
	bool hdexist = imageManager.GetTextureCardHD(showcardcode);
	const auto drawrect3 = ResizeWin(594, 150, 594 + 300, 150 + 300);
	auto DrawTextureRect = [this](Materials::QuadVertex vertices, irr::video::ITexture* texture) {
		matManager.mTexture.setTexture(0, texture);
		driver->setMaterial(matManager.mTexture);
		driver->drawVertexPrimitiveList(vertices, 4, matManager.iRectangle, 2);
	};
	irr::video::ITexture* cardcloseup; irr::video::SColor cardcloseupcolor;
	std::tie(cardcloseup, cardcloseupcolor) = imageManager.GetTextureCloseup(showcardcode, showcardalias, true);
    //////kdiy//////////
	if(showcard) {
		switch(showcard) {
		case 1: {
			auto cardtxt = imageManager.GetTextureCard(showcardcode, imgType::ART);
			auto cardrect = irr::core::rect<irr::s32>(irr::core::vector2di(0, 0), irr::core::dimension2di(cardtxt->getOriginalSize()));
            //////kdiy//////////
			//driver->draw2DImage(cardtxt, drawrect2, cardrect);
			if(cardcloseup) {
				matManager.mTexture.setTexture(0, cardtxt);
				driver->setMaterial(matManager.mTexture);
				irr::core::matrix4 atk;
				atk.setTranslation(irr::core::vector3df(-4, -1, 0));
				atk.setRotationRadians(irr::core::vector3df(-irr::core::PI/4, irr::core::PI/8, -irr::core::PI/8));
				driver->setTransform(irr::video::ETS_WORLD, atk);
				driver->drawVertexPrimitiveList(matManager.vCloseup, 4, matManager.iRectangle, 2);
			    auto cardrect2 = irr::core::rect<irr::s32>(irr::core::vector2di(0, 0), irr::core::dimension2di(cardcloseup->getOriginalSize()));
				driver->draw2DImage(cardcloseup, drawrect3, cardrect2, 0, 0, true);
			} else
			    driver->draw2DImage(cardtxt, hdexist ? drawrect2_hd : drawrect2, cardrect);
            //////kdiy//////////
			driver->draw2DImage(imageManager.tMask, ResizeWin(574, 150, 574 + (showcarddif > CARD_IMG_WIDTH ? CARD_IMG_WIDTH : showcarddif), 404),
								Scale<irr::s32>(CARD_IMG_HEIGHT - showcarddif, 0, CARD_IMG_HEIGHT - (showcarddif > CARD_IMG_WIDTH ? showcarddif - CARD_IMG_WIDTH : 0), CARD_IMG_HEIGHT), 0, 0, true);
			showcarddif += (900.0f / 1000.0f) * (float)delta_time;
			if(std::round(showcarddif) >= CARD_IMG_HEIGHT) {
				showcard = 2;
				showcarddif = 0;
			}
			break;
		}
		case 2: {
			auto cardtxt = imageManager.GetTextureCard(showcardcode, imgType::ART);
			auto cardrect = irr::core::rect<irr::s32>(irr::core::vector2di(0, 0), irr::core::dimension2di(cardtxt->getOriginalSize()));
            //////kdiy//////////
			//driver->draw2DImage(cardtxt, drawrect2, cardrect);
			if(cardcloseup) {
				matManager.mTexture.setTexture(0, cardtxt);
				driver->setMaterial(matManager.mTexture);
				irr::core::matrix4 atk;
				atk.setTranslation(irr::core::vector3df(-4, -1, 0));
				atk.setRotationRadians(irr::core::vector3df(-irr::core::PI/4, irr::core::PI/8, -irr::core::PI/8));
				driver->setTransform(irr::video::ETS_WORLD, atk);
				driver->drawVertexPrimitiveList(matManager.vCloseup, 4, matManager.iRectangle, 2);
				auto cardrect2 = irr::core::rect<irr::s32>(irr::core::vector2di(0, 0), irr::core::dimension2di(cardcloseup->getOriginalSize()));
				driver->draw2DImage(cardcloseup, drawrect3, cardrect2, 0, 0, true);
			} else
			    driver->draw2DImage(cardtxt, hdexist ? drawrect2_hd : drawrect2, cardrect);
            //////kdiy//////////
			driver->draw2DImage(imageManager.tMask, ResizeWin(574 + showcarddif, 150, 751, 404), Scale(0, 0, CARD_IMG_WIDTH - showcarddif, 254), 0, 0, true);
			showcarddif += (900.0f / 1000.0f) * (float)delta_time;
			if(showcarddif >= CARD_IMG_WIDTH) {
				showcard = 0;
			}
			break;
		}
		case 3: {
			auto cardtxt = imageManager.GetTextureCard(showcardcode, imgType::ART);
			auto cardrect = irr::core::rect<irr::s32>(irr::core::vector2di(0, 0), irr::core::dimension2di(cardtxt->getOriginalSize()));
			//////kdiy//////////
			//driver->draw2DImage(cardtxt, drawrect2, cardrect);
			driver->draw2DImage(cardtxt, hdexist ? drawrect2_hd : drawrect2, cardrect);
			//////kdiy//////////
			driver->draw2DImage(imageManager.tNegated, ResizeWin(536 + showcarddif, 141 + showcarddif, 793 - showcarddif, 397 - showcarddif), Scale(0, 0, 128, 128), 0, 0, true);
			if(showcarddif < 64)
				showcarddif += (240.0f / 1000.0f) * (float)delta_time;
			break;
		}
		case 4: {
			auto cardtxt = imageManager.GetTextureCard(showcardcode, imgType::ART);
			auto cardrect = irr::core::rect<irr::s32>(irr::core::vector2di(0, 0), irr::core::dimension2di(cardtxt->getOriginalSize()));
			matManager.c2d[0] = ((int)std::round(showcarddif) << 24) | 0xffffff;
			matManager.c2d[1] = ((int)std::round(showcarddif) << 24) | 0xffffff;
			matManager.c2d[2] = ((int)std::round(showcarddif) << 24) | 0xffffff;
			matManager.c2d[3] = ((int)std::round(showcarddif) << 24) | 0xffffff;
			//////kdiy//////////
			//driver->draw2DImage(cardtxt, drawrect2, cardrect, 0, matManager.c2d, true);
			driver->draw2DImage(cardtxt, hdexist ? drawrect2_hd : drawrect2, cardrect, 0, matManager.c2d, true);
			//////kdiy//////////
			if(showcarddif < 255)
				showcarddif += (1020.0f / 1000.0f) * (float)delta_time;
			break;
		}
		case 5: {
			auto cardtxt = imageManager.GetTextureCard(showcardcode, imgType::ART);
			auto cardrect = irr::core::rect<irr::s32>(irr::core::vector2di(0, 0), irr::core::dimension2di(cardtxt->getOriginalSize()));
			matManager.c2d[0] = ((int)std::round(showcarddif) << 25) | 0xffffff;
			matManager.c2d[1] = ((int)std::round(showcarddif) << 25) | 0xffffff;
			matManager.c2d[2] = ((int)std::round(showcarddif) << 25) | 0xffffff;
			matManager.c2d[3] = ((int)std::round(showcarddif) << 25) | 0xffffff;
			auto rect = ResizeWin(662 - showcarddif * (CARD_IMG_WIDTH_F / CARD_IMG_HEIGHT_F), 277 - showcarddif, 662 + showcarddif * (CARD_IMG_WIDTH_F / CARD_IMG_HEIGHT_F), 277 + showcarddif);
			driver->draw2DImage(cardtxt, rect, cardrect, 0, matManager.c2d, true);
            //////kdiy//////////
            if(chklast) {
				irr::video::ITexture* cardcloseup; irr::video::SColor cardcloseupcolor;
				std::tie(cardcloseup, cardcloseupcolor) = imageManager.GetTextureCloseup(showcardcode, showcardalias, true);
                if(cardcloseup)
                    DrawTextureRect(matManager.vCloseup, cardcloseup);
            }
            //////kdiy//////////
			if(showcarddif < 127.0f) {
				showcarddif += (540.0f / 1000.0f) * (float)delta_time;
				if(showcarddif > 127.0f)
					showcarddif = 127.0f;
			}
			break;
		}
		case 6: {
			auto cardtxt = imageManager.GetTextureCard(showcardcode, imgType::ART);
			auto cardrect = irr::core::rect<irr::s32>(irr::core::vector2di(0, 0), irr::core::dimension2di(cardtxt->getOriginalSize()));
			//////kdiy//////////
			//driver->draw2DImage(cardtxt, drawrect2, cardrect);
			driver->draw2DImage(cardtxt, hdexist ? drawrect2_hd : drawrect2, cardrect);
			//////kdiy//////////
			driver->draw2DImage(imageManager.tNumber, ResizeWin(536 + showcarddif, 141 + showcarddif, 793 - showcarddif, 397 - showcarddif),
								Scale(((int)std::round(showcardp) % 5) * 64, ((int)std::round(showcardp) / 5) * 64, ((int)std::round(showcardp) % 5 + 1) * 64, ((int)std::round(showcardp) / 5 + 1) * 64), 0, 0, true);
			if(showcarddif < 64)
				showcarddif += (240.0f / 1000.0f) * (float)delta_time;
			break;
		}
		case 7: {
			auto cardtxt = imageManager.GetTextureCard(showcardcode, imgType::ART);
			auto cardrect = irr::core::rect<irr::s32>(irr::core::vector2di(0, 0), irr::core::dimension2di(cardtxt->getOriginalSize()));
			irr::core::vector2di corner[4];
			float y = sin(showcarddif * irr::core::PI / 180.0f) * CARD_IMG_HEIGHT;
			auto a = ResizeWin(574 - (CARD_IMG_HEIGHT - y) * 0.3f, (150 + CARD_IMG_HEIGHT) - y, 751 + (CARD_IMG_HEIGHT - y) * 0.3f, 150 + CARD_IMG_HEIGHT);
			auto b = ResizeWin(574, 150, 574 + CARD_IMG_WIDTH, 404);
			corner[0] = a.UpperLeftCorner;
			corner[1] = irr::core::vector2di{ a.LowerRightCorner.X, a.UpperLeftCorner.Y };
			corner[2] = irr::core::vector2di{ b.UpperLeftCorner.X, b.LowerRightCorner.Y };
			corner[3] = b.LowerRightCorner;
			irr::gui::Draw2DImageQuad(driver, cardtxt, cardrect, corner);
            //////kdiy//////////
            if(cardcloseup)
                DrawTextureRect(matManager.vCloseup, cardcloseup);
            //////kdiy//////////
			showcardp += (float)delta_time * 60.0f / 1000.0f;
			showcarddif += (540.0f / 1000.0f) * (float)delta_time;
			if(showcarddif >= 90)
				showcarddif = 90;
			if(showcardp >= 60) {
				showcardp = 0;
				showcarddif = 0;
			}
			break;
		}
        //////kdiy//////////
		case 11: {
			if(cutincharacter[0] > 0) {
				auto size = imageManager.cutincharacter_size[gSoundManager->character[showcardcode]][gSoundManager->subcharacter[gSoundManager->character[showcardcode]]][cutincharacter[0]-1];
				auto width = size.getWidth();
				auto height = size.getHeight();
				driver->draw2DImage(imageManager.cutin[gSoundManager->character[showcardcode]][gSoundManager->subcharacter[gSoundManager->character[showcardcode]]][cutincharacter[0]-1], ResizeWin(324, 300, 324 + (showcarddif > width/1.333 ? width/1.333 : showcarddif), 300 + height/1.6),
					imageManager.cutincharacter_size[gSoundManager->character[showcardcode]][gSoundManager->subcharacter[gSoundManager->character[showcardcode]]][cutincharacter[0]-1], 0, 0, true);
			}
			if(cutincharacter[1] > 0) {
				auto size = imageManager.cutincharacter_size[gSoundManager->character[showcardcode]][gSoundManager->subcharacter[gSoundManager->character[showcardcode]]][cutincharacter[1]-1];
				auto width = size.getWidth();
				auto height = size.getHeight();
				driver->draw2DImage(imageManager.cutin[gSoundManager->character[showcardcode]][gSoundManager->subcharacter[gSoundManager->character[showcardcode]]][cutincharacter[1]-1], ResizeWin(924 - (showcarddif > width/1.333 ? width/1.333 : showcarddif), 100, 924, 100 + height/1.6),
					imageManager.cutincharacter_size[gSoundManager->character[showcardcode]][gSoundManager->subcharacter[gSoundManager->character[showcardcode]]][cutincharacter[1]-1], 0, 0, true);
			}
			showcarddif += (2600.0f / 1000.0f) * (float)delta_time;
			// if(showcarddif >= 240 * 3) {
			// 	showcard = 0;
			// }
			break;
		}
        //////kdiy//////////
		case 100: {
			if(showcardp < 60) {
				driver->draw2DImage(imageManager.tHand[(showcardcode >> 16) & 0x3], Resize(615, showcarddif));
				driver->draw2DImage(imageManager.tHand[showcardcode & 0x3], Resize(615, 540 - showcarddif));
				float dy = -0.333333f * showcardp + 10;
				showcardp += (float)delta_time * 60.0f / 1000.0f;
				if(showcardp < 30)
					showcarddif += (dy * 60.f / 1000.0f) * (float)delta_time;
			} else
				showcard = 0;
			break;
		}
		case 101: {
			irr::core::ustring lstr = L"";
			if(1 <= showcardcode && showcardcode <= 14) {
                lstr = gDataManager->GetSysString(1700 + showcardcode);
			}
			auto pos = lpcFont->getDimensionustring(lstr);
			if(showcardp < 10.0f) {
				int alpha = ((int)std::round(showcardp) * 25) << 24;
				DrawShadowText(lpcFont, lstr, ResizePhaseHint(661 - (9 - showcardp) * 40, 291, 960, 370, pos.Width), Resize(-1, -1, 0, 0), alpha | 0xffffff, alpha);
			} else if(showcardp < showcarddif) {
                ////kdiy/////////
				DrawShadowText(lpcFont, lstr, ResizePhaseHint(661, 291, 960, 370, pos.Width), Resize(-1, -1, 0, 0), 0xff000000);
				DrawShadowText(lpcFont, lstr, ResizePhaseHint(661, 291, 960, 370, pos.Width), Resize(-1, -1, 0, 0), 0xffffffff);
                ////kdiy/////////
				if(dInfo.vic_string.size() && (showcardcode == 1 || showcardcode == 2)) {
					auto a = (291 + pos.Height + 2);
					driver->draw2DRectangle(0xa0000000, Resize(540, a, 790, a + 20));
					DrawShadowText(guiFont, dInfo.vic_string, Resize(492, a + 1, 840, a + 20), Resize(-2, -1, 0, 0), 0xffffffff, 0xff000000, true, true);
				}
			} else if(showcardp < showcarddif + 10.0f) {
				int alpha = (int)std::round((((showcarddif + 10.0f - showcardp) * 25.0f) / 1000.0f) * (float)delta_time) << 24;
				DrawShadowText(lpcFont, lstr, ResizePhaseHint(661 + (showcardp - showcarddif) * 40, 291, 960, 370, pos.Width), Resize(-1, -1, 0, 0), alpha | 0xffffff, alpha);
			}
			showcardp += std::min(((float)delta_time * 60.0f / 1000.0f), showcarddif - showcardp);
			break;
		}
		}
	}
	if(is_attacking) {
		irr::core::matrix4 matk;
		matk.setTranslation(atk_t);
		matk.setRotationRadians(atk_r);
		driver->setTransform(irr::video::ETS_WORLD, matk);
		matManager.mATK.AmbientColor = skin::DUELFIELD_ATTACK_ARROW_VAL;
		matManager.mATK.DiffuseColor = (skin::DUELFIELD_ATTACK_ARROW_VAL.getAlpha() << 24) | 0xffffff;
		driver->setMaterial(matManager.mATK);
		driver->drawVertexPrimitiveList(&matManager.vArrow[std::min(static_cast<int>(attack_sv) * 4, 28)], 12, matManager.iArrow, 10, irr::video::EVT_STANDARD, irr::scene::EPT_TRIANGLE_STRIP);
		attack_sv += (60.0f / 1000.0f) * delta_time;
		if (static_cast<int>(attack_sv) > 9)
			attack_sv = 0.0f;
	}
	bool showChat = true;
	if(hideChat) {
		showChat = false;
		hideChatTimer = 10;
	} else if(hideChatTimer > 0) {
		showChat = false;
		hideChatTimer--;
	}
	for(int i = 0; i < 8; ++i) {
		static constexpr uint32_t chatColor[] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xff8080ff, 0xffff4040, 0xffff4040,
										   0xffff4040, 0xff40ff40, 0xff4040ff, 0xff40ffff, 0xffff40ff, 0xffffff40, 0xffffffff, 0xff808080, 0xff404040};
		if(chatTiming[i] > 0.0f) {
			chatTiming[i] -= (float)delta_time * 60.0f / 1000.0f;
			if(dInfo.isStarted && i >= 5)
				continue;
			if(!showChat && i > 2)
				continue;
			int w = textFont->getDimensionustring(chatMsg[i]).Width;
			irr::core::recti chatrect = wChat->getRelativePosition();
			auto rectloc = chatrect;
			rectloc -= irr::core::vector2di(0, (i + 1) * chatrect.getHeight() + Scale(1));
			rectloc.LowerRightCorner.X = rectloc.UpperLeftCorner.X + w + Scale(4);
			auto msgloc = chatrect;
			msgloc -= irr::core::vector2di(Scale(-2), (i + 1) * chatrect.getHeight() + Scale(1));
			auto shadowloc = msgloc + irr::core::vector2di(1, 1);
			driver->draw2DRectangle(rectloc, 0xa0000000, 0xa0000000, 0xa0000000, 0xa0000000);
			textFont->drawustring(chatMsg[i], msgloc, 0xff000000, false, false);
			textFont->drawustring(chatMsg[i], shadowloc, chatColor[chatType[i]], false, false);
		}
	}
}
void Game::DrawBackImage(irr::video::ITexture* texture, bool resized) {
	static irr::video::ITexture* prevbg = nullptr;
	static irr::core::recti dest_size = { 0,0,0,0 };
	static irr::core::recti bg_size = { 0,0,0,0 };
	static bool was_scaled = false;
	if(was_scaled && !gGameConfig->scale_background) {
		was_scaled = false;
		prevbg = nullptr;
	} else if(!was_scaled && gGameConfig->scale_background) {
		was_scaled = true;
		prevbg = nullptr;
	}
	if(resized)
		prevbg = nullptr;
	if(!texture)
		return;
    /////ktemp//////
    // if(texture == imageManager.tBackGround_menu) {
    //     if(openVideo("./movies/SlifervsObelisk.mp4")) {
	// 	    if(PlayVideo(true))
    //             if(videotexture)
	// 		    driver->draw2DImage(videotexture, Resize(0, 0, 1024, 640), irr::core::recti(0, 0, videoFrame->width, videoFrame->height));
	// 		    //driver->draw2DImage(videotexture, Resize(0, 0, 1024, 640), irr::core::recti(0, 0, frame.cols, frame.rows));
    //         return;
	//     }
    // } else {
    //     if(!isAnime && videostart) StopVideo();
    // }
    /////ktemp//////
	if(texture != prevbg) {
		prevbg = texture;
		dest_size = Resize(0, 0, 1024, 640);
		bg_size = irr::core::recti(0, 0, texture->getOriginalSize().Width, texture->getOriginalSize().Height);
		if(!gGameConfig->scale_background) {
			irr::core::rectf dest_sizef(0, 0, dest_size.getWidth(), dest_size.getHeight());
			irr::core::rectf bg_sizef(0, 0, bg_size.getWidth(), bg_size.getHeight());
			float width = ((bg_sizef.getWidth() / bg_sizef.getHeight()) * dest_sizef.getHeight()) - dest_sizef.getWidth();
			float height = ((bg_sizef.getHeight() / bg_sizef.getWidth()) * dest_sizef.getWidth()) - dest_sizef.getHeight();
			if(width > 0) {
				int off = std::ceil(width * 0.5f);
				dest_size = irr::core::recti({ -off, 0, dest_size.getWidth() + off, dest_size.getHeight() });
			} else if(height > 0) {
				int off = std::ceil(height * 0.5f);
				dest_size = irr::core::recti({ 0, -off, dest_size.getWidth(), dest_size.getHeight() + off });
			}
		}
	}
	if(gGameConfig->accurate_bg_resize)
		imageManager.draw2DImageFilterScaled(texture, dest_size, bg_size);
    else
		driver->draw2DImage(texture, dest_size, bg_size);
}
void Game::ShowElement(irr::gui::IGUIElement * win, int autoframe) {
	FadingUnit fu;
	fu.fadingSize = win->getRelativePosition();
	fu.wasEnabled = win->isEnabled();
	win->setEnabled(false);
	for(auto fit = fadingList.begin(); fit != fadingList.end(); ++fit) {
		if(win == fit->guiFading) {
			if(win != wOptions) // the size of wOptions is always set by ClientField::ShowSelectOption before showing it
				fu.fadingSize = fit->fadingSize;
			fu.wasEnabled = fit->wasEnabled;
		}
	}
	irr::core::vector2di center = fu.fadingSize.getCenter();
	fu.fadingFrame = 10.0f * 1000.0f / 60.0f;
	fu.fadingDest.X = fu.fadingSize.getWidth() / fu.fadingFrame;
	fu.fadingDest.Y = (fu.fadingSize.getHeight() - 4) / fu.fadingFrame;
	fu.fadingUL = center;
	fu.fadingLR = center;
	fu.fadingUL.Y -= 2;
	fu.fadingLR.Y += 2;
	fu.guiFading = win;
	fu.isFadein = true;
	fu.autoFadeoutFrame = autoframe * 1000 / 60;
	fu.signalAction = 0;
	if(win == wPosSelect) {
		btnPSAU->setDrawImage(false);
		btnPSAD->setDrawImage(false);
		btnPSDU->setDrawImage(false);
		btnPSDD->setDrawImage(false);
	}
	if(win == wCardSelect) {
		for(int i = 0; i < 5; ++i)
			btnCardSelect[i]->setDrawImage(false);
	}
	if(win == wCardDisplay) {
		for(int i = 0; i < 5; ++i)
			btnCardDisplay[i]->setDrawImage(false);
	}
	win->setRelativePosition(Scale(center.X, center.Y, 0, 0));
	fadingList.push_back(fu);
}
void Game::HideElement(irr::gui::IGUIElement * win, bool set_action) {
	FadingUnit fu;
	fu.fadingSize = win->getRelativePosition();
	fu.wasEnabled = win->isEnabled();
	win->setEnabled(false);
	for(auto fit = fadingList.begin(); fit != fadingList.end(); ++fit) {
		if(win == fit->guiFading) {
			fu.fadingSize = fit->fadingSize;
			fu.wasEnabled = fit->wasEnabled;
		}
	}
	fu.fadingFrame = 10.0f * 1000.0f / 60.0f;
	fu.fadingDest.X = fu.fadingSize.getWidth() / fu.fadingFrame;
	fu.fadingDest.Y = (fu.fadingSize.getHeight() - 4) / fu.fadingFrame;
	fu.fadingUL = fu.fadingSize.UpperLeftCorner;
	fu.fadingLR = fu.fadingSize.LowerRightCorner;
	fu.guiFading = win;
	fu.isFadein = false;
	fu.autoFadeoutFrame = 0;
	fu.signalAction = set_action;
	if(win == wPosSelect) {
		btnPSAU->setDrawImage(false);
		btnPSAD->setDrawImage(false);
		btnPSDU->setDrawImage(false);
		btnPSDD->setDrawImage(false);
	}
	if(win == wCardSelect) {
		for(int i = 0; i < 5; ++i)
			btnCardSelect[i]->setDrawImage(false);
		dField.conti_selecting = false;
	}
	if(win == wCardDisplay) {
		for(int i = 0; i < 5; ++i)
			btnCardDisplay[i]->setDrawImage(false);
	}
	fadingList.push_back(fu);
}
void Game::PopupElement(irr::gui::IGUIElement * element, int hideframe) {
	element->getParent()->bringToFront(element);
	if(!is_building)
		dField.panel = element;
	const auto prevfocused = env->getFocus();
	env->setFocus(element);
	if(prevfocused && (prevfocused->getType() == irr::gui::EGUIET_EDIT_BOX))
		env->setFocus(prevfocused);
	if(!hideframe)
		ShowElement(element);
	else ShowElement(element, hideframe);
}
//kidy///////
//void Game::WaitFrameSignal(int frame, std::unique_lock<epro::mutex>& _lck) {
void Game::WaitFrameSignal(int frame, std::unique_lock<epro::mutex>& _lck, bool forced) {
	//signalFrame = (gGameConfig->quick_animation && frame >= 12) ? 12 * 1000 / 60 : frame * 1000 / 60;
	signalFrame = (gGameConfig->quick_animation && frame >= 12 && !forced) ? (frame == 40 || frame == 120 ? (30 * 1000 / 60) : (12 * 1000 / 60)) : (frame * 1000 / 60);
//kidy///////
	frameSignal.Wait(_lck);
}
void Game::DrawThumb(const CardDataC* cp, irr::core::vector2di pos, LFList* lflist, bool drag, const irr::core::recti* cliprect, bool load_image) {
	auto code = cp->code;
	auto flit = lflist->GetLimitationIterator(cp);
	int count = 3;
	if(flit == lflist->content.end()) {
		if(lflist->whitelist)
			count = -1;
	} else
		count = flit->second;
	irr::video::ITexture* img = load_image ? imageManager.GetTextureCard(code, imgType::THUMB) : imageManager.tUnknown;
	if (!img)
		return;
	irr::core::dimension2du size = img->getOriginalSize();
	irr::core::recti dragloc = Resize(pos.X, pos.Y, pos.X + CARD_THUMB_WIDTH, pos.Y + CARD_THUMB_HEIGHT);
	irr::core::recti limitloc = Resize(pos.X, pos.Y, pos.X + 20, pos.Y + 20);
	irr::core::recti otloc = Resize(pos.X + 7, pos.Y + 50, pos.X + 37, pos.Y + 65);
	if(drag) {
		dragloc = irr::core::recti(pos.X, pos.Y, pos.X + Scale(CARD_THUMB_WIDTH * window_scale.X), pos.Y + Scale(CARD_THUMB_HEIGHT * window_scale.Y));
		limitloc = irr::core::recti(pos.X, pos.Y, pos.X + Scale(20 * window_scale.X), pos.Y + Scale(20 * window_scale.Y));
		otloc = irr::core::recti(pos.X + 7, pos.Y + 50 * window_scale.Y, pos.X + 37 * window_scale.X, pos.Y + 65 * window_scale.Y);
	}
	driver->draw2DImage(img, dragloc, irr::core::recti(0, 0, size.Width, size.Height), cliprect);
	if(!is_siding) {
		switch(count) {
			case -1:
			case 0:
				imageManager.draw2DImageFilterScaled(imageManager.tLim, limitloc, irr::core::recti(0, 0, 64, 64), cliprect, 0, true);
				break;
			case 1:
				imageManager.draw2DImageFilterScaled(imageManager.tLim, limitloc, irr::core::recti(64, 0, 128, 64), cliprect, 0, true);
				break;
			case 2:
				imageManager.draw2DImageFilterScaled(imageManager.tLim, limitloc, irr::core::recti(0, 64, 64, 128), cliprect, 0, true);
				break;
			default:
				if(cp->ot & SCOPE_LEGEND) {
					imageManager.draw2DImageFilterScaled(imageManager.tLim, limitloc, irr::core::recti(64, 64, 128, 128), cliprect, 0, true);
				}
				break;
		}
#define IDX(scope,idx) case SCOPE_##scope:\
							index = idx;\
							goto draw;
		if(gGameConfig->showScopeLabel && !lflist->whitelist) {
			// Label display logic:
			// If it contains exactly one bit between Anime, Illegal, Video Game, Custom, and Prerelease, display that.
			// Else, if it contains exactly one bit between OCG and TCG, display that.
			switch(cp->ot & ~(SCOPE_PRERELEASE | SCOPE_LEGEND)) {
				int index;
				IDX(OCG,0)
				IDX(TCG,1)
				IDX(ILLEGAL,2)
				IDX(ANIME,3)
				IDX(VIDEO_GAME,5)
				IDX(CUSTOM,6)
				IDX(SPEED,8)
				IDX(RUSH,9)
				default: break;
				draw:
				imageManager.draw2DImageFilterScaled(imageManager.tOT, otloc, irr::core::recti(0, index * 64, 128, index * 64 + 64), cliprect, 0, true);
			}
		}
#undef IDX
	}
}
//kdiy//////////
void Game::DrawThumb2(uint32_t code, irr::core::vector2di pos) {
	irr::video::ITexture* img = imageManager.GetTextureCard(code, imgType::THUMB);
	if (!img)
		return;
	irr::core::dimension2du size = img->getOriginalSize();
	irr::core::recti dragloc = Resize(pos.X, pos.Y, pos.X + CARD_THUMB_WIDTH, pos.Y + CARD_THUMB_HEIGHT);
	irr::core::recti limitloc = Resize(pos.X, pos.Y, pos.X + 20, pos.Y + 20);
	irr::core::recti otloc = Resize(pos.X + 7, pos.Y + 50, pos.X + 37, pos.Y + 65);
	driver->draw2DImage(img, dragloc, irr::core::recti(0, 0, size.Width, size.Height));
}
//kdiy//////////
#define SKCOLOR(what) skin::DECK_WINDOW_##what##_VAL
#define DECKCOLOR(what) SKCOLOR(what##_TOP_LEFT), SKCOLOR(what##_TOP_RIGHT), SKCOLOR(what##_BOTTOM_LEFT), SKCOLOR(what##_BOTTOM_RIGHT)
#define DRAWRECT(what,...) do { driver->draw2DRectangle(Resize(__VA_ARGS__), DECKCOLOR(what)); } while(0)
#define DRAWOUTLINE(what,...) do { driver->draw2DRectangleOutline(Resize(__VA_ARGS__), SKCOLOR(what##_OUTLINE)); } while(0)
void Game::DrawDeckBd() {
	const auto GetDeckSizeStr = [&](const Deck::Vector& deck, const Deck::Vector& pre_deck)->std::wstring {
		if(is_siding)
			return epro::format(L"{} ({})", deck.size(), pre_deck.size());
		return epro::to_wstring(deck.size());
	};
	const auto& current_deck = deckBuilder.GetCurrentDeck();

	///kdiy///////
	if(dInfo.isInDuel) {
        if(dInfo.isSingleMode && !dInfo.isHandTest) {
            mainGame->btnLocation[0]->setVisible(false);
            for(int i = 1; i < 6; ++i)
				mainGame->btnLocation[i]->setVisible(true);
            if(mainGame->btnLocation[0]->isPressed()) {
                for(int i = 0; i < 6; ++i) {
					if(i == 1) mainGame->btnLocation[1]->setPressed(true);
					else mainGame->btnLocation[i]->setPressed(false);
				}
            }
        } else {
            for(int i = 0; i < 6; ++i)
				mainGame->btnLocation[i]->setVisible(true);
        }
        
		auto show_deck = deckBuilder.GetCurrentDeck().main;
		const auto& players = ReplayMode::cur_replay.GetPlayerNames();
		if(dInfo.isReplay && players.empty())
			return;
		const auto& decks = ReplayMode::cur_replay.GetPlayerDecks();
		if(dInfo.isReplay && players.size() > decks.size())
			return;
        auto turnplayer = 1 - ((dInfo.turn % 2 && dInfo.isFirst) || (!(dInfo.turn % 2) && !dInfo.isFirst));
		cardlist_type show_deck3;
        if(dInfo.isReplay) show_deck3 = decks[(turnplayer == 0) ? dInfo.current_player[0] : dInfo.current_player[1] + dInfo.team1].main_deck;
		auto show_deck2 = dField.deck[0];
		if(mainGame->btnLocation[2]->isPressed()) show_deck2 = dField.grave[0];
		if(mainGame->btnLocation[3]->isPressed()) show_deck2 = dField.remove[0];
		if(mainGame->btnLocation[4]->isPressed()) show_deck2 = dField.hand[0];
		if(mainGame->btnLocation[5]->isPressed()) show_deck2 = dField.extra[0];
		std::sort(show_deck2.begin(), show_deck2.end());
		auto show_oppdeck2 = dField.deck[1];
		if(mainGame->btnLocation[2]->isPressed()) show_oppdeck2 = dField.grave[1];
		if(mainGame->btnLocation[3]->isPressed()) show_oppdeck2 = dField.remove[1];
		if(mainGame->btnLocation[4]->isPressed()) show_oppdeck2 = dField.hand[1];
		if(mainGame->btnLocation[5]->isPressed()) show_oppdeck2 = dField.extra[1];
		std::sort(show_oppdeck2.begin(), show_oppdeck2.end());

		auto decksize = mainGame->btnLocation[0]->isPressed() ? dInfo.isReplay ? show_deck3.size() : show_deck.size() : show_oppdeck2.size();
		auto decksize2 = show_deck2.size();

		DRAWRECT(MAIN_INFO, 10, 5, 297, 20);
		DRAWOUTLINE(MAIN_INFO, 9, 4, 297, 20);

		DrawShadowText(textFont, gDataManager->GetSysString(mainGame->btnLocation[1]->isPressed() ? 8060 : mainGame->btnLocation[2]->isPressed() ? 8061 : mainGame->btnLocation[3]->isPressed() ? 8062 : mainGame->btnLocation[4]->isPressed() ? 8063 : mainGame->btnLocation[5]->isPressed() ? 8064 : 8059), Resize(14, 4, 109, 19), Resize(1, 1, 1, 1), 0xffffffff, 0xff000000, false, true);

		auto main_deck_size_str = epro::to_wstring(decksize);;
		DrawShadowText(numFont, main_deck_size_str, Resize(79, 5, 139, 20), Resize(1, 1, 1, 1), 0xffffffff, 0xff000000, false, true);

        bool half = !(mainGame->btnLocation[0]->isPressed());
        int monster_count = deckBuilder.main_monster_count;
        int spell_count = deckBuilder.main_spell_count;
        int trap_count = deckBuilder.main_trap_count;
		int oppmonster_count = 0;
        int oppspell_count = 0;
        int opptrap_count = 0;
        if(half) {
            monster_count = 0;
            spell_count = 0;
            trap_count = 0;
            for(const auto& card : show_deck2) {
                if(card->type & TYPE_MONSTER)
                    monster_count++;
                else if(card->type & TYPE_SPELL)
                    spell_count++;
                else if(card->type & TYPE_TRAP)
                    trap_count++;
            }
            for(const auto& card : show_oppdeck2) {
                if(card->type & TYPE_MONSTER)
                    oppmonster_count++;
                else if(card->type & TYPE_SPELL)
                    oppspell_count++;
                else if(card->type & TYPE_TRAP)
                    opptrap_count++;
            }
        }
		auto main_types_count_str = epro::format(L"{} {} {} {} {} {}",
													  gDataManager->GetSysString(1312), oppmonster_count,
													  gDataManager->GetSysString(1313), oppspell_count,
													  gDataManager->GetSysString(1314), opptrap_count);

		auto mainpos = Resize(10, 5, 297, 20);
		auto mainDeckTypeSize = textFont->getDimensionustring(main_types_count_str);
		auto pos = irr::core::recti(mainpos.LowerRightCorner.X - mainDeckTypeSize.Width - 5, mainpos.UpperLeftCorner.Y,
										  mainpos.LowerRightCorner.X, mainpos.LowerRightCorner.Y);

		DrawShadowText(textFont, main_types_count_str, pos, irr::core::recti{ 1, 1, 1, 1 }, 0xffffffff, 0xff000000, false, true);

		DRAWRECT(MAIN, 10, 24, 297, half ? 256 : 505);
		DRAWOUTLINE(MAIN, 9, 22, 297, half ? 256 : 505);

		int cards_per_row = (decksize > 91) ? static_cast<int>((decksize - 92) / 10 + 9) : 8;
        if(half) cards_per_row = (decksize > 46) ? static_cast<int>((decksize - 47) / 5 + 11) : 8;
		float dx = (297.0f-14.0f-47.0f) / (cards_per_row - 1);

		for(int i = 0; i < static_cast<int>(decksize); ++i) {
			if(28 + (i / cards_per_row) * 42 > (half ? 256 : 505)) break;
			DrawThumb2(!mainGame->btnLocation[0]->isPressed() ? show_oppdeck2[i]->code : dInfo.isReplay ? show_deck3[i] : show_deck[i]->code, irr::core::vector2di(14 + (i % cards_per_row) * dx, 28 + (i / cards_per_row) * 42));
		}

		if(half) {
			DRAWRECT(MAIN_INFO, 10, 259, 297, 274);
			DRAWOUTLINE(MAIN_INFO, 9, 257, 297, 274);
			
			DrawShadowText(textFont, gDataManager->GetSysString(mainGame->btnLocation[2]->isPressed() ? 1004 : mainGame->btnLocation[3]->isPressed() ? 1005 : mainGame->btnLocation[4]->isPressed() ? 1001 : mainGame->btnLocation[5]->isPressed() ? 1006 : 1000), Resize(14, 258, 109, 273), Resize(1, 1, 1, 1), 0xffffffff, 0xff000000, false, true);
			
			auto main_deck_size_str = epro::to_wstring(decksize2);;
			DrawShadowText(numFont, main_deck_size_str, Resize(79, 259, 139, 274), Resize(1, 1, 1, 1), 0xffffffff, 0xff000000, false, true);
			auto main_types_count_str = epro::format(L"{} {} {} {} {} {}",
													  gDataManager->GetSysString(1312), monster_count,
													  gDataManager->GetSysString(1313), spell_count,
													  gDataManager->GetSysString(1314), trap_count);
			auto mainpos = Resize(10, 259, 297, 274);
			auto mainDeckTypeSize = textFont->getDimensionustring(main_types_count_str);
            auto pos = irr::core::recti(mainpos.LowerRightCorner.X - mainDeckTypeSize.Width - 5, mainpos.UpperLeftCorner.Y,
										  mainpos.LowerRightCorner.X, mainpos.LowerRightCorner.Y);

		    DrawShadowText(textFont, main_types_count_str, pos, irr::core::recti{ 1, 1, 1, 1 }, 0xffffffff, 0xff000000, false, true);

			DRAWRECT(MAIN, 10, 278, 297, 505);
			DRAWOUTLINE(MAIN, 9, 276, 297, 505);
			
			int cards_per_row = (decksize2 > 46) ? static_cast<int>((decksize2 - 47) / 5 + 11) : 8;
			float dx = (297.0f-14.0f-47.0f) / (cards_per_row - 1);
			
			for(int i = 0; i < static_cast<int>(decksize2); ++i) {
				if(282 + (i / cards_per_row) * 42 > 505) break;
				DrawThumb2(show_deck2[i]->code, irr::core::vector2di(14 + (i % cards_per_row) * dx, 282 + (i / cards_per_row) * 42));
            }
		}
	    return;
	}
	///kdiy///////
	//main deck
	{
		DRAWRECT(MAIN_INFO, 310, 137, 797, 157);
		DRAWOUTLINE(MAIN_INFO, 309, 136, 797, 157);

		DrawShadowText(textFont, gDataManager->GetSysString(1330), Resize(314, 136, 409, 156), Resize(1, 1, 1, 1), 0xffffffff, 0xff000000, false, true);

		const auto main_deck_size_str = GetDeckSizeStr(current_deck.main, gdeckManager->pre_deck.main);
		DrawShadowText(numFont, main_deck_size_str, Resize(379, 137, 439, 157), Resize(1, 1, 1, 1), 0xffffffff, 0xff000000, false, true);

		const auto main_types_count_str = epro::format(L"{} {} {} {} {} {}",
													  gDataManager->GetSysString(1312), deckBuilder.main_monster_count,
													  gDataManager->GetSysString(1313), deckBuilder.main_spell_count,
													  gDataManager->GetSysString(1314), deckBuilder.main_trap_count);

		const auto mainpos = Resize(310, 137, 797, 157);
		const auto mainDeckTypeSize = textFont->getDimensionustring(main_types_count_str);
		const auto pos = irr::core::recti(mainpos.LowerRightCorner.X - mainDeckTypeSize.Width - 5, mainpos.UpperLeftCorner.Y,
										  mainpos.LowerRightCorner.X, mainpos.LowerRightCorner.Y);

		DrawShadowText(textFont, main_types_count_str, pos, irr::core::recti{ 1, 1, 1, 1 }, 0xffffffff, 0xff000000, false, true);

		DRAWRECT(MAIN, 310, 160, 797, 436);
		DRAWOUTLINE(MAIN, 309, 159, 797, 436);

		const int cards_per_row = (current_deck.main.size() > 40) ? static_cast<int>((current_deck.main.size() - 41) / 4 + 11) : 10;
		const float dx = 436.0f / (cards_per_row - 1);

		for(int i = 0; i < static_cast<int>(current_deck.main.size()); ++i) {
			DrawThumb(current_deck.main[i], irr::core::vector2di(314 + (i % cards_per_row) * dx, 164 + (i / cards_per_row) * 68), deckBuilder.filterList);
			if(deckBuilder.hovered_pos == 1 && deckBuilder.hovered_seq == i)
				driver->draw2DRectangleOutline(Resize(313 + (i % cards_per_row) * dx, 163 + (i / cards_per_row) * 68, 359 + (i % cards_per_row) * dx, 228 + (i / cards_per_row) * 68), skin::DECK_WINDOW_HOVERED_CARD_OUTLINE_VAL);
		}
	}
	//extra deck
	{
		DRAWRECT(EXTRA_INFO, 310, 440, 797, 460);
		DRAWOUTLINE(EXTRA_INFO, 309, 439, 797, 460);

		DrawShadowText(textFont, gDataManager->GetSysString(1331), Resize(314, 439, 409, 459), Resize(1, 1, 1, 1), 0xffffffff, 0xff000000, false, true);

		const auto extra_deck_size_str = GetDeckSizeStr(current_deck.extra, gdeckManager->pre_deck.extra);
		DrawShadowText(numFont, extra_deck_size_str, Resize(379, 440, 439, 460), Resize(1, 1, 1, 1), 0xffffffff, 0xff000000, false, true);

		const auto extra_types_count_str = epro::format(L"{} {} {} {} {} {} {} {}",
													   gDataManager->GetSysString(1056), deckBuilder.extra_fusion_count,
													   gDataManager->GetSysString(1073), deckBuilder.extra_xyz_count,
													   gDataManager->GetSysString(1063), deckBuilder.extra_synchro_count,
													   gDataManager->GetSysString(1076), deckBuilder.extra_link_count);

		const auto extrapos = Resize(310, 440, 797, 460);
		const auto extraDeckTypeSize = textFont->getDimensionustring(extra_types_count_str);
		const auto pos = irr::core::recti(extrapos.LowerRightCorner.X - extraDeckTypeSize.Width - 5, extrapos.UpperLeftCorner.Y,
										  extrapos.LowerRightCorner.X, extrapos.LowerRightCorner.Y);

		DrawShadowText(textFont, extra_types_count_str, pos, irr::core::recti{ 1, 1, 1, 1 }, 0xffffffff, 0xff000000, false, true);

		DRAWRECT(EXTRA, 310, 463, 797, 533);
		DRAWOUTLINE(EXTRA, 309, 462, 797, 533);

		const float dx = (current_deck.extra.size() <= 10) ? (436.0f / 9.0f) : (436.0f / (current_deck.extra.size() - 1));

		for(size_t i = 0; i < current_deck.extra.size(); ++i) {
			DrawThumb(current_deck.extra[i], irr::core::vector2di(314 + i * dx, 466), deckBuilder.filterList);
			if(deckBuilder.hovered_pos == 2 && deckBuilder.hovered_seq == (int)i)
				driver->draw2DRectangleOutline(Resize(313 + i * dx, 465, 359 + i * dx, 531), skin::DECK_WINDOW_HOVERED_CARD_OUTLINE_VAL);
		}
	}
	//side deck
	{
		DRAWRECT(SIDE_INFO, 310, 537, 797, 557);
		DRAWOUTLINE(SIDE_INFO, 309, 536, 797, 557);

		DrawShadowText(textFont, gDataManager->GetSysString(1332), Resize(314, 536, 409, 556), Resize(1, 1, 1, 1), 0xffffffff, 0xff000000, false, true);

		const auto side_deck_size_str = GetDeckSizeStr(current_deck.side, gdeckManager->pre_deck.side);
		DrawShadowText(numFont, side_deck_size_str, Resize(379, 536, 439, 556), Resize(1, 1, 1, 1), 0xffffffff, 0xff000000, false, true);

		const auto side_types_count_str = epro::format(L"{} {} {} {} {} {}",
													  gDataManager->GetSysString(1312), deckBuilder.side_monster_count,
													  gDataManager->GetSysString(1313), deckBuilder.side_spell_count,
													  gDataManager->GetSysString(1314), deckBuilder.side_trap_count);

		const auto sidepos = Resize(310, 537, 797, 557);
		const auto sideDeckTypeSize = textFont->getDimensionustring(side_types_count_str);
		const auto pos = irr::core::recti(sidepos.LowerRightCorner.X - sideDeckTypeSize.Width - 5, sidepos.UpperLeftCorner.Y,
										  sidepos.LowerRightCorner.X, sidepos.LowerRightCorner.Y);

		DrawShadowText(textFont, side_types_count_str, pos, irr::core::recti{ 1, 1, 1, 1 }, 0xffffffff, 0xff000000, false, true);
		DRAWRECT(SIDE, 310, 560, 797, 630);
		DRAWOUTLINE(SIDE, 309, 559, 797, 630);

		const float dx = (current_deck.side.size() <= 10) ? (436.0f / 9.0f) : (436.0f / (current_deck.side.size() - 1));

		for(size_t i = 0; i < current_deck.side.size(); ++i) {
			DrawThumb(current_deck.side[i], irr::core::vector2di(314 + i * dx, 564), deckBuilder.filterList);
			if(deckBuilder.hovered_pos == 3 && deckBuilder.hovered_seq == (int)i)
				driver->draw2DRectangleOutline(Resize(313 + i * dx, 563, 359 + i * dx, 629), skin::DECK_WINDOW_HOVERED_CARD_OUTLINE_VAL);
		}
	}
	//search result
	{
		DRAWRECT(SEARCH_RESULT_INFO, 805, 137, 915, 157);
		DRAWOUTLINE(SEARCH_RESULT_INFO, 804, 136, 915, 157);

		DrawShadowText(textFont, gDataManager->GetSysString(1333), Resize(809, 136, 914, 156), Resize(1, 1, 1, 1), 0xffffffff, 0xff000000, false, true);

		const auto tmpstring = gDataManager->GetSysString(1333);
		const auto size = textFont->getDimensionustring(tmpstring).Width + ResizeX(5);
		const auto pos = irr::core::recti(ResizeX(809) + size, ResizeY(136), ResizeX(809) + size + 10, ResizeY(156));
		DrawShadowText(numFont, deckBuilder.result_string, pos, Resize(0, 1, 0, 1), 0xffffffff, 0xff000000, false, true);

		DRAWRECT(SEARCH_RESULT, 805, 160, 1020, 630);
		DRAWOUTLINE(SEARCH_RESULT, 804, 159, 1020, 630);

		const int prev_pos = deckBuilder.scroll_pos;
		deckBuilder.scroll_pos = floor(scrFilter->getPos() / DECK_SEARCH_SCROLL_STEP);

		const bool draw_thumb = std::abs(prev_pos - deckBuilder.scroll_pos) < (10.0f * 60.0f / 1000.0f) * delta_time;
		const int card_position = deckBuilder.scroll_pos;
		const int height_offset = (scrFilter->getPos() % DECK_SEARCH_SCROLL_STEP) * -1.f * 0.65f;
		const irr::core::recti rect = Resize(805, 160, 1020, 630);

		//loads the thumb of one card before and one after to make the scroll smoother
		int i = (card_position > 0) ? -1 : 0;
		for(; i < 9 && (i + card_position) < (int)deckBuilder.results.size(); ++i) {
			auto ptr = deckBuilder.results[i + card_position];
			if(deckBuilder.hovered_pos == 4 && deckBuilder.hovered_seq == i)
				driver->draw2DRectangle(skin::DECK_WINDOW_HOVERED_CARD_RESULT_VAL, Resize(806, height_offset + 164 + i * 66, 1019, height_offset + 230 + i * 66), &rect);
			DrawThumb(ptr, irr::core::vector2di(810, height_offset + 165 + i * 66), deckBuilder.filterList, false, &rect, draw_thumb);
			if(ptr->type & TYPE_MONSTER) {
				DrawShadowTextPos(textFont, gDataManager->GetName(ptr->code), Resize(859, height_offset + 164 + i * 66, 955, height_offset + 185 + i * 66),
								  Resize(860, height_offset + 165 + i * 66, 955, height_offset + 185 + i * 66), 0xffffffff, 0xff000000, false, false, &rect);
				///////kdiy////////////
				// if(ptr->type & TYPE_LINK) {
				// 	DrawShadowTextPos(textFont, epro::format(L"{}/{}", gDataManager->FormatAttribute(ptr->attribute), gDataManager->FormatRace(ptr->race)),
				// 					  Resize(859, height_offset + 186 + i * 66, 955, height_offset + 207 + i * 66),
				// 					  Resize(860, height_offset + 187 + i * 66, 955, height_offset + 207 + i * 66), 0xffffffff, 0xff000000, false, false, &rect);
				// } else {
				// 	const wchar_t* form = L"\u2605";
				// 	if(ptr->type & TYPE_XYZ) form = L"\u2606";
				// 	DrawShadowTextPos(textFont, epro::format(L"{}/{} {}{}", gDataManager->FormatAttribute(ptr->attribute), gDataManager->FormatRace(ptr->race), form, ptr->level),
				// 					  Resize(859, height_offset + 186 + i * 66, 955, height_offset + 207 + i * 66),
				// 					  Resize(860, height_offset + 187 + i * 66, 955, height_offset + 207 + i * 66), 0xffffffff, 0xff000000, false, false, &rect);
				// }
				//has lv, rk
				if ((ptr->type & (TYPE_FUSION | TYPE_SYNCHRO | TYPE_RITUAL)) && (ptr->type & TYPE_XYZ)) {
					DrawShadowTextPos(textFont, epro::format(L"{}/{} {}{}{}", gDataManager->FormatAttribute(ptr->attribute), gDataManager->FormatRace(ptr->race), L"\u2605", L"\u2606", ptr->level),
									  Resize(859, height_offset + 186 + i * 66, 955, height_offset + 207 + i * 66),
									  Resize(860, height_offset + 187 + i * 66, 955, height_offset + 207 + i * 66), 0xffffffff, 0xff000000, false, false, &rect);
				//no link
				} else if(!(ptr->type & TYPE_LINK)){
					const wchar_t* form = L"\u2605";
					if(ptr->type & TYPE_XYZ) form = L"\u2606";
					DrawShadowTextPos(textFont, epro::format(L"{}/{} {}{}", gDataManager->FormatAttribute(ptr->attribute), gDataManager->FormatRace(ptr->race), form, ptr->level),
									  Resize(859, height_offset + 186 + i * 66, 955, height_offset + 207 + i * 66),
									  Resize(860, height_offset + 187 + i * 66, 955, height_offset + 207 + i * 66), 0xffffffff, 0xff000000, false, false, &rect);
				} else {
					DrawShadowTextPos(textFont, epro::format(L"{}/{}", gDataManager->FormatAttribute(ptr->attribute), gDataManager->FormatRace(ptr->race)),
									  Resize(859, height_offset + 186 + i * 66, 955, height_offset + 207 + i * 66),
									  Resize(860, height_offset + 187 + i * 66, 955, height_offset + 207 + i * 66), 0xffffffff, 0xff000000, false, false, &rect);
				}
				///////kdiy////////////
			} else {
				DrawShadowTextPos(textFont, gDataManager->GetName(ptr->code), Resize(859, height_offset + 164 + i * 66, 955, height_offset + 185 + i * 66),
								  Resize(860, height_offset + 165 + i * 66, 955, height_offset + 185 + i * 66), 0xffffffff, 0xff000000, false, false, &rect);
				DrawShadowTextPos(textFont, gDataManager->FormatType(ptr->type), Resize(859, height_offset + 186 + i * 66, 955, height_offset + 207 + i * 66),
								  Resize(860, height_offset + 187 + i * 66, 955, height_offset + 207 + i * 66), 0xffffffff, 0xff000000, false, false, &rect);
			}
			auto GetScopeString = [&ptr]()->std::wstring {
				auto scope = gDataManager->FormatScope(ptr->ot, true);
				if(ptr->type & TYPE_MONSTER) {
					std::wstring buffer;
					///////kdiy////////////
					// if(ptr->type & TYPE_LINK) {
						//if(ptr->attack < 0)
							//buffer = epro::format(L"?/Link {}\t", ptr->level);
						// else
						// 	buffer = epro::format(L"{}/Link {}\t", ptr->attack, ptr->level);
					// } else {
					// 	if(ptr->attack < 0 && ptr->defense < 0)
					// 		buffer = L"?/?";
					// 	else if(ptr->attack < 0)
					// 		buffer = epro::format(L"?/{}", ptr->defense);
					// 	else if(ptr->defense < 0)
					// 		buffer = epro::format(L"{}/?", ptr->attack);
					// 	else
					// 		buffer = epro::format(L"{}/{}", ptr->attack, ptr->defense);
					int link_marker = ptr->link_marker;
					int32_t mixlink = 0;
					if(ptr->type & TYPE_LINK) {
						if(link_marker & LINK_MARKER_BOTTOM_LEFT) mixlink += 1;
						if(link_marker & LINK_MARKER_BOTTOM_RIGHT) mixlink += 1;
						if(link_marker & LINK_MARKER_BOTTOM) mixlink += 1;
						if(link_marker & LINK_MARKER_LEFT) mixlink += 1;
						if(link_marker & LINK_MARKER_RIGHT) mixlink += 1;
						if(link_marker & LINK_MARKER_TOP) mixlink += 1;
						if(link_marker & LINK_MARKER_TOP_LEFT) mixlink += 1;
						if(link_marker & LINK_MARKER_TOP_RIGHT) mixlink += 1;
					} else mixlink = ptr->level;
					std::wstring ltext;
					if (!(ptr->type & TYPE_LINK)) {
						if (ptr->attack < 0 && ptr->defense < 0)
						    buffer = L"?/?";
						else if(ptr->attack >= 9999999 && ptr->defense >= 9999999)
						    buffer = epro::format(L"(\u221E)/(\u221E)");
						else if(ptr->attack >= 8888888 && ptr->defense >= 8888888)
						    buffer = epro::format(L"\u221E/\u221E");
						else if(ptr->attack >= 9999999 && ptr->defense >= 8888888)
						    buffer = epro::format(L"(\u221E)/\u221E");
						else if(ptr->attack >= 8888888 && ptr->defense>= 9999999)
						    buffer = epro::format(L"\u221E/(\u221E)");
						else if(ptr->attack >= 9999999 && ptr->defense >= 0)
						    buffer = epro::format(L"(\u221E)/{}", ptr->defense);
						else if(ptr->defense >= 9999999 && ptr->attack >= 0)
						    buffer = epro::format(L"{}/(\u221E)", ptr->attack);
						else if(ptr->attack >= 8888888 && ptr->defense >= 0)
						    buffer = epro::format(L"\u221E/{}", ptr->defense);
						else if(ptr->defense >= 8888888 && ptr->attack >= 0)
						    buffer = epro::format(L"{}/\u221E", ptr->attack);
						else if(ptr->attack < 0 && ptr->defense >= 9999999)
						    buffer = epro::format(L"?/(\u221E)", ptr->defense);
						else if(ptr->defense < 0 && ptr->attack >= 9999999)
						    buffer = epro::format(L"(\u221E)/?", ptr->attack);
						else if(ptr->attack < 0 && ptr->defense >= 8888888)
						    buffer = epro::format(L"?/\u221E", ptr->defense);
						else if(ptr->defense < 0 && ptr->attack >= 8888888)
						    buffer = epro::format(L"\u221E/?", ptr->attack);
						else if (ptr->attack < 0)
						    buffer = epro::format(L"?/{}", ptr->defense);
						else if (ptr->defense < 0)
						    buffer = epro::format(L"{}/?", ptr->attack);
						else
						    buffer = epro::format(L"{}/{}", ptr->attack, ptr->defense);
					} else {
						if (ptr->attack < 0)
							buffer = epro::format(L"?/LINK {}\t", mixlink);
						else if(ptr->attack >= 9999999)
							buffer = epro::format(L"(\u221E)/LINK {}\t", mixlink);
						else if(ptr->attack >= 8888888)
							buffer = epro::format(L"\u221E/LINK {}\t", mixlink);
						else
							buffer = epro::format(L"{}/LINK {}\t", ptr->attack, mixlink);
					}
					///////kdiy////////////
					if(ptr->type & TYPE_PENDULUM)
						buffer.append(epro::format(L" {}/{}", ptr->lscale, ptr->rscale));
					if(!scope.empty())
						return epro::format(L"{} [{}]", buffer, scope);
					return buffer;
				}
				if(!scope.empty())
					return epro::format(L"[{}]", scope);
				return L"";
			};
			DrawShadowTextPos(textFont, GetScopeString(), Resize(859, height_offset + 208 + i * 66, 955, height_offset + 229 + i * 66),
							  Resize(860, height_offset + 209 + i * 66, 955, height_offset + 229 + i * 66), 0xffffffff, 0xff000000, false, false, &rect);
		}
	}
	if(deckBuilder.is_draging)
		DrawThumb(deckBuilder.dragging_pointer, irr::core::vector2di(deckBuilder.dragx - Scale(CARD_THUMB_WIDTH / 2), deckBuilder.dragy - Scale(CARD_THUMB_HEIGHT / 2)), deckBuilder.filterList, true);
}
#undef DRAWRECT
#undef DECKCOLOR
#undef SKCOLOR
}
