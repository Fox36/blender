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

/** \file KX_KetsjiEngine.h
 *  \ingroup ketsji
 */

#ifndef __KX_KETSJIENGINE_H__
#define __KX_KETSJIENGINE_H__

#include "MT_CmMatrix4x4.h"
#include "MT_Matrix4x4.h"
#include <string>
#include "KX_ISystem.h"
#include "KX_Scene.h"
#include "KX_TimeCategoryLogger.h"
#include "EXP_Python.h"
#include "KX_WorldInfo.h"
#include "RAS_CameraData.h"
#include <vector>

struct TaskScheduler;
class KX_ISystem;
class KX_ISceneConverter;
class KX_NetworkMessageManager;
class CListValue;
class RAS_ICanvas;
class RAS_OffScreen;
class RAS_IRasterizer;
class SCA_IInputDevice;

#define LEFT_EYE  1
#define RIGHT_EYE 2

enum class KX_ExitRequest
{
	NO_REQUEST = 0,
	QUIT_GAME,
	RESTART_GAME,
	START_OTHER_GAME,
	NO_SCENES_LEFT,
	BLENDER_ESC,
	OUTSIDE,
	MAX
};

enum class KX_DebugOption
{
	DISABLE = 0,
	FORCE,
	ALLOW
};

typedef struct {
	short glslflag;
} GlobalSettings;

/**
 * KX_KetsjiEngine is the core game engine class.
 */
class KX_KetsjiEngine
{

private:
	/// 2D Canvas (2D Rendering Device Context)
	RAS_ICanvas *m_canvas;
	/// 3D Rasterizer (3D Rendering)
	RAS_IRasterizer *m_rasterizer;
	KX_ISystem *m_kxsystem;
	KX_ISceneConverter *m_sceneconverter;
	KX_NetworkMessageManager *m_networkMessageManager;
#ifdef WITH_PYTHON
	/// \note borrowed from sys.modules["__main__"], don't manage ref's
	PyObject *m_pythondictionary;
	PyObject *m_pyprofiledict;
#endif
	SCA_IInputDevice *m_inputDevice;

	/// Lists of scenes scheduled to be removed at the end of the frame.
	std::vector<std::string> m_removingScenes;
	/// Lists of overley scenes scheduled to be added at the end of the frame.
	std::vector<std::string> m_addingOverlayScenes;
	/// Lists of background scenes scheduled to be added at the end of the frame.
	std::vector<std::string> m_addingBackgroundScenes;
	/// Lists of scenes scheduled to be replaced at the end of the frame.
	std::vector<std::pair<std::string, std::string> >  m_replace_scenes;

	/// The current list of scenes.
	CListValue *m_scenes;
	/// State variable recording the presence of object debug info in the current scene list.
	bool m_propertiesPresent;

	bool m_bInitialized;
	int m_activecam;
	bool m_fixedFramerate;
	bool m_useExternalClock;

	bool m_firstframe;
	int m_currentFrame;

	/// current logic game time
	double m_frameTime;
	/// game time for the next rendering step
	double m_clockTime;
	/// game time of the previous rendering step
	double m_previousClockTime;
	///game time when the animations were last updated
	double m_previousAnimTime;
	double m_remainingTime;
	/// time scaling parameter. if > 1.0, time goes faster than real-time. If < 1.0, times goes slower than real-time.
	double m_timescale;
	double m_previousRealTime;

	/// maximum number of consecutive logic frame
	static int m_maxLogicFrame;
	/// maximum number of consecutive physics frame
	static int m_maxPhysicsFrame;
	static double m_ticrate;
	/// for animation playback only - ipo and action
	static double m_anim_framerate;

	static bool m_restrict_anim_fps;

	static bool				m_doRender;  /* whether or not the scene should be rendered after the logic frame */

	/// Key used to exit the BGE
	static short m_exitkey;

	KX_ExitRequest m_exitcode;
	std::string m_exitstring;

	float m_cameraZoom;

	bool m_overrideCam;
	std::string m_overrideSceneName;
	RAS_CameraData m_overrideCamData;
	MT_CmMatrix4x4 m_overrideCamProjMat;
	MT_CmMatrix4x4 m_overrideCamViewMat;
	/// Default camera zoom.
	float m_overrideCamZoom;

	bool m_stereo;
	int m_curreye;

	/// Categories for profiling display.
	typedef enum {
		tc_first = 0,
		tc_physics = 0,
		tc_logic,
		tc_animations,
		tc_network,
		tc_scenegraph,
		tc_rasterizer,
		tc_services, // time spent in miscelaneous activities
		tc_overhead, // profile info drawing overhead
		tc_outside, // time spent outside main loop
		tc_latency, // time spent waiting on the gpu
		tc_numCategories
	} KX_TimeCategory;

	/// Time logger.
	KX_TimeCategoryLogger m_logger;

	/// Labels for profiling display.
	static const char m_profileLabels[tc_numCategories][15];
	/// Last estimated framerate
	static double m_average_framerate;
	/// Show the framerate on the game display?
	bool m_show_framerate;
	/// Show profiling info on the game display?
	bool m_show_profile;
	/// Show any debug (scene) object properties on the game display?
	bool m_showProperties;
	/// Show background behind text for readability?
	bool m_showBackground;
	/// Show debug properties on the game display
	bool m_show_debug_properties;
	/// Automatic add debug properties to the debug list
	bool m_autoAddDebugProperties;

	/// Hide cursor every frame?
	bool m_hideCursor;

	/// Enable debug draw of culling bounding boxes.
	KX_DebugOption m_showBoundingBox;
	/// Enable debug draw armatures.
	KX_DebugOption m_showArmature;
	/// Enable debug draw of camera frustum.
	KX_DebugOption m_showCameraFrustum;
	/// Enable debug light shadow frustum.
	KX_DebugOption m_showShadowFrustum;

	/// Settings that doesn't go away with Game Actuator
	GlobalSettings m_globalsettings;

	/// Task scheduler for multi-threading
	TaskScheduler *m_taskscheduler;

	/** Set scene's total pause duration for animations process.
	 * This is done in a separate loop to get the proper state of each scenes.
	 * eg: There's 2 scenes, the first is suspended and the second is active.
	 * If the second scene resume the first, the first scene will be not proceed
	 * in 'NextFrame' for one frame, but set as active.
	 * The render functions, called after and which update animations,
	 * will see the first scene as active and will proceed to it,
	 * but it will cause some negative current frame on actions because of the
	 * total pause duration not set.
	 */
	void UpdateSuspendedScenes(double framestep);

	/// Update and return the projection matrix of a camera depending on the viewport.
	const MT_Matrix4x4& GetCameraProjectionMatrix(KX_Scene *scene, KX_Camera *cam, const RAS_Rect& viewport, const RAS_Rect& area);

	void RenderFrame(KX_Scene *scene, KX_Camera *cam, RAS_OffScreen *offScreen, unsigned short pass);
	RAS_OffScreen *PostRenderScene(KX_Scene *scene, RAS_OffScreen *inputofs, RAS_OffScreen *targetofs);
	void RenderDebugProperties();
	/// Debug draw cameras frustum of a scene.
	void DrawDebugCameraFrustum(KX_Scene *scene, KX_Camera *cam, RAS_DebugDraw& debugDraw, const RAS_Rect& viewport, const RAS_Rect& area);
	/// Debug draw lights shadow frustum of a scene.
	void DrawDebugShadowFrustum(KX_Scene *scene, RAS_DebugDraw& debugDraw);

public:
	KX_KetsjiEngine(KX_ISystem *system);
	virtual ~KX_KetsjiEngine();

	/// set the devices and stuff. the client must take care of creating these
	void SetInputDevice(SCA_IInputDevice *inputDevice);
	void SetCanvas(RAS_ICanvas *canvas);
	void SetRasterizer(RAS_IRasterizer *rasterizer);
	void SetNetworkMessageManager(KX_NetworkMessageManager *manager);
#ifdef WITH_PYTHON
	void SetPyNamespace(PyObject *pythondictionary);
	PyObject *GetPyNamespace()
	{
		return m_pythondictionary;
	}
	PyObject *GetPyProfileDict();
#endif
	void SetSceneConverter(KX_ISceneConverter *sceneconverter);
	KX_ISceneConverter *GetSceneConverter()
	{
		return m_sceneconverter;
	}

	RAS_IRasterizer *GetRasterizer()
	{
		return m_rasterizer;
	}
	RAS_ICanvas *GetCanvas()
	{
		return m_canvas;
	}
	SCA_IInputDevice *GetInputDevice()
	{
		return m_inputDevice;
	}
	KX_NetworkMessageManager *GetNetworkMessageManager() const
	{
		return m_networkMessageManager;
	}

	TaskScheduler *GetTaskScheduler()
	{
		return m_taskscheduler;
	}

	/// returns true if an update happened to indicate -> Render
	bool NextFrame();
	void Render();
	void RenderShadowBuffers(KX_Scene *scene);

	void StartEngine(bool clearIpo);
	void StopEngine();
	void Export(const std::string& filename);

	void RequestExit(KX_ExitRequest exitrequestmode);
	void SetNameNextGame(const std::string& nextgame);
	KX_ExitRequest GetExitCode();
	const std::string& GetExitString();

	CListValue *CurrentScenes();
	KX_Scene *FindScene(const std::string& scenename);
	void AddScene(KX_Scene *scene);
	void ConvertAndAddScene(const std::string& scenename, bool overlay);

	void RemoveScene(const std::string& scenename);
	bool ReplaceScene(const std::string& oldscene, const std::string& newscene);
	void SuspendScene(const std::string& scenename);
	void ResumeScene(const std::string& scenename);

	void GetSceneViewport(KX_Scene *scene, KX_Camera *cam, RAS_Rect& area, RAS_Rect& viewport);

	/// Sets zoom for camera objects, useful only with extend and scale framing mode.
	void SetCameraZoom(float camzoom);
	/// Sets zoom for default camera, = 2 in embedded mode.
	void SetCameraOverrideZoom(float camzoom);
	/// Get the camera zoom for the passed camera.
	float GetCameraZoom(KX_Camera *camera) const;

	void EnableCameraOverride(const std::string& forscene, const MT_CmMatrix4x4& projmat, const MT_CmMatrix4x4& viewmat, const RAS_CameraData& camdata);

	// Update animations for object in this scene
	void UpdateAnimations(KX_Scene *scene);

	/**
	 * Sets display of fixed frames.
	 * \param fixedFramerate New setting for display all frames.
	 */
	void SetUseFixedFramerate(bool fixedFramerate);

	/**
	 * Returns display of fixed frames.
	 * \return Current setting for display all frames.
	 */
	bool GetUseFixedFramerate(void) const;

	/**
	 * Sets if the BGE relies on a external clock or its own internal clock
	 */
	void SetUseExternalClock(bool bUseExternalClock);

	/**
	 * Returns if we rely on an external clock
	 * \return Current setting
	 */
	bool GetUseExternalClock(void) const;

	/**
	 * Returns next render frame game time
	 */
	double GetClockTime(void) const;

	/**
	 * Set the next render frame game time. It will impact also frame time, as
	 * this one is derived from clocktime
	 */
	void SetClockTime(double externalClockTime);

	/**
	 * Returns current logic frame game time
	 */
	double GetFrameTime(void) const;

	/**
	 * Returns the real (system) time
	 */
	double GetRealTime(void) const;

	/**
	 * Gets the number of logic updates per second.
	 */
	static double GetTicRate();
	/**
	 * Sets the number of logic updates per second.
	 */
	static void SetTicRate(double ticrate);
	/**
	 * Gets the maximum number of logic frame before render frame
	 */
	static int GetMaxLogicFrame();
	/**
	 * Sets the maximum number of logic frame before render frame
	 */
	static void SetMaxLogicFrame(int frame);
	/**
	 * Gets the maximum number of physics frame before render frame
	 */
	static int GetMaxPhysicsFrame();
	/**
	 * Sets the maximum number of physics frame before render frame
	 */
	static void SetMaxPhysicsFrame(int frame);

	/**
	 * Gets whether or not to lock animation updates to the animframerate
	 */
	static bool GetRestrictAnimationFPS();

	/**
	 * Sets whether or not to lock animation updates to the animframerate
	 */
	static void SetRestrictAnimationFPS(bool bRestrictAnimFPS);

	/**
	 * Gets the framerate for playing animations. (actions and ipos)
	 */
	static double GetAnimFrameRate();
	/**
	 * Sets the framerate for playing animations. (actions and ipos)
	 */
	static void SetAnimFrameRate(double framerate);

	/**
	 * Gets the last estimated average framerate
	 */
	static double GetAverageFrameRate();

	/**
	 * Gets the time scale multiplier 
	 */
	double GetTimeScale() const;

	/**
	 * Sets the time scale multiplier
	 */
	void SetTimeScale(double scale);

	static void SetExitKey(short key);

	static short GetExitKey();

	/**
	 * Activate or deactivates the render of the scene after the logic frame
	 * \param render	true (render) or false (do not render)
	 */
	static void SetRender(bool render);
	/**
	 * Get the current render flag value
	 */
	static bool GetRender();

	/**
	 * \Sets the display for frame rate on or off.
	 */
	void SetShowFramerate(bool frameRate);

	/**
	 * \Gets the display for frame rate on or off.
	 */
	bool GetShowFramerate();

	/**
	 * \Sets the display for individual components on or off.
	 */
	void SetShowProfile(bool profile);

	/**
	 * \Gets the display for individual components on or off.
	 */
	bool GetShowProfile();

	/**
	 * \Sets the display of scene object debug properties on or off.
	 */
	void SetShowProperties(bool properties);

	/**
	 * \Gets the display of scene object debug properties on or off.
	 */
	bool GetShowProperties();

	/**
	 * \Sets if the auto adding of scene object debug properties on or off.
	 */
	bool GetAutoAddDebugProperties();

	/**
	 * \Sets the auto adding of scene object debug properties on or off.
	 */
	void SetAutoAddDebugProperties(bool add);

	/// Allow debug bounding box debug.
	void SetShowBoundingBox(KX_DebugOption mode);
	/// Returns the current setting for bounding box debug.
	KX_DebugOption GetShowBoundingBox() const;

	/// Allow debug armatures.
	void SetShowArmatures(KX_DebugOption mode);
	/// Returns the current setting for armatures debug.
	KX_DebugOption GetShowArmatures() const;

	/// Allow debug camera frustum.
	void SetShowCameraFrustum(KX_DebugOption mode);
	/// Returns the current setting for camera frustum debug.
	KX_DebugOption GetShowCameraFrustum() const;

	/// Allow debug light shadow frustum.
	void SetShowShadowFrustum(KX_DebugOption mode);
	/// Returns the current setting for light shadow frustum debug.
	KX_DebugOption GetShowShadowFrustum() const;

	KX_Scene *CreateScene(const std::string& scenename);
	KX_Scene *CreateScene(Scene *scene, bool libloading = false);

	GlobalSettings *GetGlobalSettings(void);
	void SetGlobalSettings(GlobalSettings *gs);

	/**
	 * Invalidate all the camera matrices and handle other
	 * needed changes when resized.
	 * It's only called from Blenderplayer.
	 */
	void Resize();

protected:
	/**
	 * Processes all scheduled scene activity.
	 * At the end, if the scene lists have changed,
	 * SceneListsChanged(void) is called.
	 * \see SceneListsChanged(void).
	 */
	void ProcessScheduledScenes(void);

	/**
	 * This method is invoked when the scene lists have changed.
	 */
	void RemoveScheduledScenes(void);
	void AddScheduledScenes(void);
	void ReplaceScheduledScenes(void);
	void PostProcessScene(KX_Scene *scene);

	void BeginFrame();
	void EndFrame();
};

#endif  /* __KX_KETSJIENGINE_H__ */
