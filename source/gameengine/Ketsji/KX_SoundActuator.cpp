/*
 * KX_SoundActuator.cpp
 *
 *
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
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

/** \file gameengine/Ketsji/KX_SoundActuator.cpp
 *  \ingroup ketsji
 */


#include "KX_SoundActuator.h"

#ifdef WITH_AUDASPACE
#  ifdef WITH_SYSTEM_AUDASPACE
typedef float sample_t;
#    include AUD_PYTHON_H
#  endif
#  include AUD_SOUND_H
#  include AUD_SPECIAL_H
#  include AUD_DEVICE_H
#  include AUD_HANDLE_H
#endif

#include "KX_GameObject.h"
#include "KX_PyMath.h" // needed for PyObjectFrom()
#include "KX_Globals.h"
#include "KX_Camera.h"
#include <iostream>

/* ------------------------------------------------------------------------- */
/* Native functions                                                          */
/* ------------------------------------------------------------------------- */
KX_SoundActuator::KX_SoundActuator(SCA_IObject* gameobj,
#ifdef WITH_AUDASPACE
								   AUD_Sound* sound,
#endif  // WITH_AUDASPACE
								   float volume,
								   float pitch,
								   bool is3d,
								   KX_3DSoundSettings settings,
								   KX_SOUNDACT_TYPE type)//,
								   : SCA_IActuator(gameobj, KX_ACT_SOUND)
{
#ifdef WITH_AUDASPACE
	m_sound = sound ? AUD_Sound_copy(sound) : nullptr;
	m_handle = nullptr;
#endif  // WITH_AUDASPACE
	m_volume = volume;
	m_pitch = pitch;
	m_is3d = is3d;
	m_3d = settings;
	m_type = type;
	m_isplaying = false;
}



KX_SoundActuator::~KX_SoundActuator()
{
#ifdef WITH_AUDASPACE
	if (m_handle) {
		AUD_Handle_stop(m_handle);
	}

	if (m_sound) {
		AUD_Sound_free(m_sound);
	}
#endif  // WITH_AUDASPACE
}

void KX_SoundActuator::play()
{
#ifdef WITH_AUDASPACE
	if (m_handle) {
		AUD_Handle_stop(m_handle);
		m_handle = nullptr;
	}

	if (!m_sound)
		return;

	// this is the sound that will be played and not deleted afterwards
	AUD_Sound* sound = m_sound;

	bool loop = false;

	switch (m_type)
	{
	case KX_SOUNDACT_LOOPBIDIRECTIONAL:
	case KX_SOUNDACT_LOOPBIDIRECTIONAL_STOP:
		sound = AUD_Sound_pingpong(sound);
		ATTR_FALLTHROUGH;
	case KX_SOUNDACT_LOOPEND:
	case KX_SOUNDACT_LOOPSTOP:
		loop = true;
		break;
	case KX_SOUNDACT_PLAYSTOP:
	case KX_SOUNDACT_PLAYEND:
	default:
		break;
	}

	AUD_Device* device = AUD_Device_getCurrent();
	m_handle = AUD_Device_play(device, sound, false);
	AUD_Device_free(device);

	// in case of pingpong, we have to free the sound
	if (sound != m_sound)
		AUD_Sound_free(sound);

	if (m_handle != nullptr) {
		if (m_is3d) {
			AUD_Handle_setRelative(m_handle, true);
			AUD_Handle_setVolumeMaximum(m_handle, m_3d.max_gain);
			AUD_Handle_setVolumeMinimum(m_handle, m_3d.min_gain);
			AUD_Handle_setDistanceReference(m_handle, m_3d.reference_distance);
			AUD_Handle_setDistanceMaximum(m_handle, m_3d.max_distance);
			AUD_Handle_setAttenuation(m_handle, m_3d.rolloff_factor);
			AUD_Handle_setConeAngleInner(m_handle, m_3d.cone_inner_angle);
			AUD_Handle_setConeAngleOuter(m_handle, m_3d.cone_outer_angle);
			AUD_Handle_setConeVolumeOuter(m_handle, m_3d.cone_outer_gain);
		}

		if (loop)
			AUD_Handle_setLoopCount(m_handle, -1);
		AUD_Handle_setPitch(m_handle, m_pitch);
		AUD_Handle_setVolume(m_handle, m_volume);
	}

	m_isplaying = true;
#endif  // WITH_AUDASPACE
}

EXP_Value* KX_SoundActuator::GetReplica()
{
	KX_SoundActuator* replica = new KX_SoundActuator(*this);
	replica->ProcessReplica();
	return replica;
}

void KX_SoundActuator::ProcessReplica()
{
	SCA_IActuator::ProcessReplica();
#ifdef WITH_AUDASPACE
	m_handle = nullptr;
	m_sound = AUD_Sound_copy(m_sound);
#endif  // WITH_AUDASPACE
}

bool KX_SoundActuator::Update(double curtime)
{
	bool result = false;

#ifdef WITH_AUDASPACE
	// do nothing on negative events, otherwise sounds are played twice!
	bool bNegativeEvent = IsNegativeEvent();
	bool bPositiveEvent = m_posevent;
#endif  // WITH_AUDASPACE
	
	RemoveAllEvents();

#ifdef WITH_AUDASPACE
	if (!m_sound)
		return false;

	// actual audio device playing state
	bool isplaying = m_handle ? (AUD_Handle_getStatus(m_handle) == AUD_STATUS_PLAYING) : false;

	if (bNegativeEvent)
	{
		// here must be a check if it is still playing
		if (m_isplaying && isplaying)
		{
			switch (m_type)
			{
			case KX_SOUNDACT_PLAYSTOP:
			case KX_SOUNDACT_LOOPSTOP:
			case KX_SOUNDACT_LOOPBIDIRECTIONAL_STOP:
				{
					// stop immediately
					if (m_handle)
					{
						AUD_Handle_stop(m_handle);
						m_handle = nullptr;
					}
					break;
				}
			case KX_SOUNDACT_PLAYEND:
				{
					// do nothing, sound will stop anyway when it's finished
					break;
				}
			case KX_SOUNDACT_LOOPEND:
			case KX_SOUNDACT_LOOPBIDIRECTIONAL:
				{
					// stop the looping so that the sound stops when it finished
					if (m_handle)
						AUD_Handle_setLoopCount(m_handle, 0);
					break;
				}
			default:
				// implement me !!
				break;
			}
		}
		// remember that we tried to stop the actuator
		m_isplaying = false;
	}
	
#if 1
	// Warning: when de-activating the actuator, after a single negative event this runs again with...
	// m_posevent==false && m_posevent==false, in this case IsNegativeEvent() returns false 
	// and assumes this is a positive event.
	// check that we actually have a positive event so as not to play sounds when being disabled.
	else if (bPositiveEvent)  /* <- added since 2.49 */
#else
	else	// <- works in most cases except a loop-end sound will never stop unless
			// the negative pulse is done continuesly
#endif
	{
		if (!m_isplaying)
			play();
	}
	// verify that the sound is still playing
	isplaying = m_handle ? (AUD_Handle_getStatus(m_handle) == AUD_STATUS_PLAYING) : false;

	if (isplaying)
	{
		if (m_is3d)
		{
			KX_Camera* cam = KX_GetActiveScene()->GetActiveCamera();
			if (cam)
			{
				KX_GameObject* obj = (KX_GameObject*)this->GetParent();
				mt::vec3 p;
				mt::mat3 Mo;
				float data[4];

				Mo = cam->NodeGetWorldOrientation().Inverse();
				p = (obj->NodeGetWorldPosition() - cam->NodeGetWorldPosition());
				p = Mo * p;
				p.Pack(data);
				AUD_Handle_setLocation(m_handle, data);
				p = (obj->GetLinearVelocity() - cam->GetLinearVelocity());
				p = Mo * p;
				p.Pack(data);
				AUD_Handle_setVelocity(m_handle, data);
				mt::quat::FromMatrix(Mo * obj->NodeGetWorldOrientation()).Pack(data);
				AUD_Handle_setOrientation(m_handle, data);
			}
		}
		result = true;
	}
	else
	{
		m_isplaying = false;
		result = false;
	}
#endif  // WITH_AUDASPACE

	return result;
}

#ifdef WITH_PYTHON

/* ------------------------------------------------------------------------- */
/* Python functions                                                          */
/* ------------------------------------------------------------------------- */



/* Integration hooks ------------------------------------------------------- */
PyTypeObject KX_SoundActuator::Type = {
	PyVarObject_HEAD_INIT(nullptr, 0)
	"KX_SoundActuator",
	sizeof(EXP_PyObjectPlus_Proxy),
	0,
	py_base_dealloc,
	0,
	0,
	0,
	0,
	py_base_repr,
	0,0,0,0,0,0,0,0,0,
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	0,0,0,0,0,0,0,
	Methods,
	0,
	0,
	&SCA_IActuator::Type,
	0,0,0,0,0,0,
	py_base_new
};

PyMethodDef KX_SoundActuator::Methods[] = {
	EXP_PYMETHODTABLE_NOARGS(KX_SoundActuator, startSound),
	EXP_PYMETHODTABLE_NOARGS(KX_SoundActuator, pauseSound),
	EXP_PYMETHODTABLE_NOARGS(KX_SoundActuator, stopSound),
	{nullptr, nullptr} //Sentinel
};

PyAttributeDef KX_SoundActuator::Attributes[] = {
	EXP_PYATTRIBUTE_BOOL_RO("is3D", KX_SoundActuator, m_is3d),
	EXP_PYATTRIBUTE_RW_FUNCTION("volume_maximum", KX_SoundActuator, pyattr_get_3d_property, pyattr_set_3d_property),
	EXP_PYATTRIBUTE_RW_FUNCTION("volume_minimum", KX_SoundActuator, pyattr_get_3d_property, pyattr_set_3d_property),
	EXP_PYATTRIBUTE_RW_FUNCTION("distance_reference", KX_SoundActuator, pyattr_get_3d_property, pyattr_set_3d_property),
	EXP_PYATTRIBUTE_RW_FUNCTION("distance_maximum", KX_SoundActuator, pyattr_get_3d_property, pyattr_set_3d_property),
	EXP_PYATTRIBUTE_RW_FUNCTION("attenuation", KX_SoundActuator, pyattr_get_3d_property, pyattr_set_3d_property),
	EXP_PYATTRIBUTE_RW_FUNCTION("cone_angle_inner", KX_SoundActuator, pyattr_get_3d_property, pyattr_set_3d_property),
	EXP_PYATTRIBUTE_RW_FUNCTION("cone_angle_outer", KX_SoundActuator, pyattr_get_3d_property, pyattr_set_3d_property),
	EXP_PYATTRIBUTE_RW_FUNCTION("cone_volume_outer", KX_SoundActuator, pyattr_get_3d_property, pyattr_set_3d_property),
	EXP_PYATTRIBUTE_RW_FUNCTION("sound", KX_SoundActuator, pyattr_get_sound, pyattr_set_sound),

	EXP_PYATTRIBUTE_RW_FUNCTION("time", KX_SoundActuator, pyattr_get_audposition, pyattr_set_audposition),
	EXP_PYATTRIBUTE_RW_FUNCTION("volume", KX_SoundActuator, pyattr_get_gain, pyattr_set_gain),
	EXP_PYATTRIBUTE_RW_FUNCTION("pitch", KX_SoundActuator, pyattr_get_pitch, pyattr_set_pitch),
	EXP_PYATTRIBUTE_ENUM_RW("mode",KX_SoundActuator::KX_SOUNDACT_NODEF+1,KX_SoundActuator::KX_SOUNDACT_MAX-1,false,KX_SoundActuator,m_type),
	EXP_PYATTRIBUTE_NULL //Sentinel
};

/* Methods ----------------------------------------------------------------- */
EXP_PYMETHODDEF_DOC_NOARGS(KX_SoundActuator, startSound,
"startSound()\n"
"\tStarts the sound.\n")
{
#ifdef WITH_AUDASPACE
	switch (m_handle ? AUD_Handle_getStatus(m_handle) : AUD_STATUS_INVALID) {
		case AUD_STATUS_PLAYING:
			break;
		case AUD_STATUS_PAUSED:
			AUD_Handle_resume(m_handle);
			break;
		default:
			play();
	}
#endif  // WITH_AUDASPACE

	Py_RETURN_NONE;
}

EXP_PYMETHODDEF_DOC_NOARGS(KX_SoundActuator, pauseSound,
"pauseSound()\n"
"\tPauses the sound.\n")
{
#ifdef WITH_AUDASPACE
	if (m_handle)
		AUD_Handle_pause(m_handle);
#endif  // WITH_AUDASPACE

	Py_RETURN_NONE;
}

EXP_PYMETHODDEF_DOC_NOARGS(KX_SoundActuator, stopSound,
"stopSound()\n"
"\tStops the sound.\n")
{
#ifdef WITH_AUDASPACE
	if (m_handle)
	{
		AUD_Handle_stop(m_handle);
		m_handle = nullptr;
	}
#endif  // WITH_AUDASPACE

	Py_RETURN_NONE;
}

/* Atribute setting and getting -------------------------------------------- */
PyObject *KX_SoundActuator::pyattr_get_3d_property(EXP_PyObjectPlus *self, const struct EXP_PYATTRIBUTE_DEF *attrdef)
{
	KX_SoundActuator * actuator = static_cast<KX_SoundActuator *> (self);
	const std::string& prop = attrdef->m_name;
	float result_value = 0.0f;

	if (prop == "volume_maximum") {
		result_value = actuator->m_3d.max_gain;
	}
	else if (prop == "volume_minimum") {
		result_value = actuator->m_3d.min_gain;
	}
	else if (prop == "distance_reference") {
		result_value = actuator->m_3d.reference_distance;
	}
	else if (prop == "distance_maximum") {
		result_value = actuator->m_3d.max_distance;
	}
	else if (prop == "attenuation") {
		result_value = actuator->m_3d.rolloff_factor;
	}
	else if (prop == "cone_angle_inner") {
		result_value = actuator->m_3d.cone_inner_angle;
	}
	else if (prop == "cone_angle_outer") {
		result_value = actuator->m_3d.cone_outer_angle;
	}
	else if (prop == "cone_volume_outer") {
		result_value = actuator->m_3d.cone_outer_gain;
	}
	else {
		Py_RETURN_NONE;
	}

	PyObject *result = PyFloat_FromDouble(result_value);
	return result;
}

PyObject *KX_SoundActuator::pyattr_get_audposition(EXP_PyObjectPlus *self, const struct EXP_PYATTRIBUTE_DEF *attrdef)
{
	float position = 0.0f;

#ifdef WITH_AUDASPACE
	KX_SoundActuator * actuator = static_cast<KX_SoundActuator *> (self);

	if (actuator->m_handle)
		position = AUD_Handle_getPosition(actuator->m_handle);
#endif  // WITH_AUDASPACE

	PyObject *result = PyFloat_FromDouble(position);

	return result;
}

PyObject *KX_SoundActuator::pyattr_get_gain(EXP_PyObjectPlus *self, const struct EXP_PYATTRIBUTE_DEF *attrdef)
{
	KX_SoundActuator * actuator = static_cast<KX_SoundActuator *> (self);
	float gain = actuator->m_volume;

	PyObject *result = PyFloat_FromDouble(gain);

	return result;
}

PyObject *KX_SoundActuator::pyattr_get_pitch(EXP_PyObjectPlus *self, const struct EXP_PYATTRIBUTE_DEF *attrdef)
{
	KX_SoundActuator * actuator = static_cast<KX_SoundActuator *> (self);
	float pitch = actuator->m_pitch;

	PyObject *result = PyFloat_FromDouble(pitch);

	return result;
}

PyObject *KX_SoundActuator::pyattr_get_sound(EXP_PyObjectPlus *self, const struct EXP_PYATTRIBUTE_DEF *attrdef)
{
#ifdef WITH_AUDASPACE
	KX_SoundActuator * actuator = static_cast<KX_SoundActuator *> (self);

	if (actuator->m_sound)
		return (PyObject *)AUD_getPythonSound(actuator->m_sound);
#endif  // WITH_AUDASPACE

	Py_RETURN_NONE;

}

int KX_SoundActuator::pyattr_set_3d_property(EXP_PyObjectPlus *self, const struct EXP_PYATTRIBUTE_DEF *attrdef, PyObject *value)
{
	KX_SoundActuator * actuator = static_cast<KX_SoundActuator *> (self);
	const std::string& prop = attrdef->m_name;
	float prop_value = 0.0f;

	if (!PyArg_Parse(value, "f", &prop_value))
		return PY_SET_ATTR_FAIL;

	// if sound is working and 3D, set the new setting
	if (!actuator->m_is3d)
		return PY_SET_ATTR_FAIL;

	if (prop == "volume_maximum") {
		actuator->m_3d.max_gain = prop_value;
#ifdef WITH_AUDASPACE
		if (actuator->m_handle)
			AUD_Handle_setVolumeMaximum(actuator->m_handle, prop_value);
#endif  // WITH_AUDASPACE

	} else if (prop == "volume_minimum") {
		actuator->m_3d.min_gain = prop_value;
#ifdef WITH_AUDASPACE
		if (actuator->m_handle)
			AUD_Handle_setVolumeMinimum(actuator->m_handle, prop_value);
#endif  // WITH_AUDASPACE

	} else if (prop == "distance_reference") {
		actuator->m_3d.reference_distance = prop_value;
#ifdef WITH_AUDASPACE
		if (actuator->m_handle)
			AUD_Handle_setDistanceReference(actuator->m_handle, prop_value);
#endif  // WITH_AUDASPACE

	} else if (prop == "distance_maximum") {
		actuator->m_3d.max_distance = prop_value;
#ifdef WITH_AUDASPACE
		if (actuator->m_handle)
			AUD_Handle_setDistanceMaximum(actuator->m_handle, prop_value);
#endif  // WITH_AUDASPACE

	} else if (prop == "attenuation") {
		actuator->m_3d.rolloff_factor = prop_value;
#ifdef WITH_AUDASPACE
		if (actuator->m_handle)
			AUD_Handle_setAttenuation(actuator->m_handle, prop_value);
#endif  // WITH_AUDASPACE

	} else if (prop == "cone_angle_inner") {
		actuator->m_3d.cone_inner_angle = prop_value;
#ifdef WITH_AUDASPACE
		if (actuator->m_handle)
			AUD_Handle_setConeAngleInner(actuator->m_handle, prop_value);
#endif  // WITH_AUDASPACE

	} else if (prop == "cone_angle_outer") {
		actuator->m_3d.cone_outer_angle = prop_value;
#ifdef WITH_AUDASPACE
		if (actuator->m_handle)
			AUD_Handle_setConeAngleOuter(actuator->m_handle, prop_value);
#endif  // WITH_AUDASPACE

	} else if (prop == "cone_volume_outer") {
		actuator->m_3d.cone_outer_gain = prop_value;
#ifdef WITH_AUDASPACE
		if (actuator->m_handle)
			AUD_Handle_setConeVolumeOuter(actuator->m_handle, prop_value);
#endif  // WITH_AUDASPACE

	} else {
		return PY_SET_ATTR_FAIL;
	}
	
	return PY_SET_ATTR_SUCCESS;
}

int KX_SoundActuator::pyattr_set_audposition(EXP_PyObjectPlus *self, const struct EXP_PYATTRIBUTE_DEF *attrdef, PyObject *value)
{
	float position = 1.0f;
	if (!PyArg_Parse(value, "f", &position))
		return PY_SET_ATTR_FAIL;

#ifdef WITH_AUDASPACE
	KX_SoundActuator * actuator = static_cast<KX_SoundActuator *> (self);

	if (actuator->m_handle)
		AUD_Handle_setPosition(actuator->m_handle, position);
#endif  // WITH_AUDASPACE

	return PY_SET_ATTR_SUCCESS;
}

int KX_SoundActuator::pyattr_set_gain(EXP_PyObjectPlus *self, const struct EXP_PYATTRIBUTE_DEF *attrdef, PyObject *value)
{
	float gain = 1.0f;
	KX_SoundActuator * actuator = static_cast<KX_SoundActuator *> (self);
	if (!PyArg_Parse(value, "f", &gain))
		return PY_SET_ATTR_FAIL;

	actuator->m_volume = gain;

#ifdef WITH_AUDASPACE
	if (actuator->m_handle)
		AUD_Handle_setVolume(actuator->m_handle, gain);
#endif  // WITH_AUDASPACE

	return PY_SET_ATTR_SUCCESS;
}

int KX_SoundActuator::pyattr_set_pitch(EXP_PyObjectPlus *self, const struct EXP_PYATTRIBUTE_DEF *attrdef, PyObject *value)
{
	float pitch = 1.0f;
	KX_SoundActuator * actuator = static_cast<KX_SoundActuator *> (self);
	if (!PyArg_Parse(value, "f", &pitch))
		return PY_SET_ATTR_FAIL;

	actuator->m_pitch = pitch;

#ifdef WITH_AUDASPACE
	if (actuator->m_handle)
		AUD_Handle_setPitch(actuator->m_handle, pitch);
#endif  // WITH_AUDASPACE

	return PY_SET_ATTR_SUCCESS;
}

int KX_SoundActuator::pyattr_set_sound(EXP_PyObjectPlus *self, const struct EXP_PYATTRIBUTE_DEF *attrdef, PyObject *value)
{
	PyObject *sound = nullptr;
	if (!PyArg_Parse(value, "O", &sound))
		return PY_SET_ATTR_FAIL;

#ifdef WITH_AUDASPACE
	KX_SoundActuator * actuator = static_cast<KX_SoundActuator *> (self);

	AUD_Sound *snd = AUD_getSoundFromPython(sound);

	if (!snd) {
		return PY_SET_ATTR_FAIL;
	}

	AUD_Sound_free(actuator->m_sound);
	actuator->m_sound = snd;
#endif  // WITH_AUDASPACE

	return PY_SET_ATTR_SUCCESS;
}

#endif // WITH_PYTHON
