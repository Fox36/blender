/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file gameengine/GameLogic/SCA_IScene.cpp
 *  \ingroup gamelogic
 */


#include "SCA_IScene.h"
#include "EXP_Value.h"


SCA_DebugProp::SCA_DebugProp()
	: m_obj(NULL)
{
}

SCA_DebugProp::~SCA_DebugProp()
{
	if (m_obj) 
		m_obj->Release(); 
}


SCA_IScene::SCA_IScene()
{
}

SCA_IScene::~SCA_IScene()
{
	RemoveAllDebugProperties();
}

void SCA_IScene::RemoveAllDebugProperties()
{
	for (std::vector<SCA_DebugProp *>::iterator it = m_debugList.begin();
		!(it == m_debugList.end()); ++it)
	{
		delete (*it);
	}
	m_debugList.clear();
}

std::vector<SCA_DebugProp *> &SCA_IScene::GetDebugProperties() 
{
	return m_debugList;
}

<<<<<<< 5ed285505a760083cca565114e9a4809e7f8d3d0

bool SCA_IScene::PropertyInDebugList( class CValue *gameobj, const std::string &name )
{
	for (std::vector<SCA_DebugProp*>::iterator it = m_debugList.begin();
		!(it==m_debugList.end());++it) {
		std::string debugname = (*it)->m_name;
=======
bool SCA_IScene::PropertyInDebugList(class CValue *gameobj, const STR_String &name)
{
	for (std::vector<SCA_DebugProp *>::iterator it = m_debugList.begin();
		!(it == m_debugList.end()); ++it) 
	{
		STR_String debugname = (*it)->m_name;
>>>>>>> UPBGE: Update files in GameLogic folder
		CValue *debugobj = (*it)->m_obj;

		if (debugobj == gameobj && debugname == name)
			return true;
	}
	return false;
}

bool SCA_IScene::ObjectInDebugList(class CValue *gameobj)
{
	for (std::vector<SCA_DebugProp *>::iterator it = m_debugList.begin();
		!(it == m_debugList.end()); ++it) 
	{
		CValue *debugobj = (*it)->m_obj;

		if (debugobj == gameobj)
			return true;
	}
	return false;
}

<<<<<<< 5ed285505a760083cca565114e9a4809e7f8d3d0

void SCA_IScene::AddDebugProperty(class CValue* debugprop,
								  const std::string &name)
=======
void SCA_IScene::AddDebugProperty(class CValue *debugprop, const STR_String &name)
>>>>>>> UPBGE: Update files in GameLogic folder
{
	if (m_debugList.size() < DEBUG_MAX_DISPLAY) {
		SCA_DebugProp *dprop = new SCA_DebugProp();
		dprop->m_obj = debugprop;
		debugprop->AddRef();
		dprop->m_name = name;
		m_debugList.push_back(dprop);
	}
}

<<<<<<< 5ed285505a760083cca565114e9a4809e7f8d3d0

void SCA_IScene::RemoveDebugProperty(class CValue *gameobj,
								  const std::string &name)
=======
void SCA_IScene::RemoveDebugProperty(class CValue *gameobj, const STR_String &name)
>>>>>>> UPBGE: Update files in GameLogic folder
{
	std::vector<SCA_DebugProp *>::iterator it = m_debugList.begin();
	while (it != m_debugList.end()) {
		std::string debugname = (*it)->m_name;
		CValue *debugobj = (*it)->m_obj;

		if (debugobj == gameobj && debugname == name) {
			delete (*it);
			m_debugList.erase(it);
			break;
		}
		++it;
	}
}

void SCA_IScene::RemoveObjectDebugProperties(class CValue *gameobj)
{	
	std::vector<SCA_DebugProp *>::iterator it = m_debugList.begin();
	while (it != m_debugList.end()) {
		CValue *debugobj = (*it)->m_obj;

		if (debugobj == gameobj) {
			delete (*it);
			m_debugList.erase(it);
			continue;
		}
		++it;
	}
}

