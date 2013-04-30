// _base.h

#pragma once

namespace fw
{
	interface IAction;
	interface IBody;
	interface IKineNode;
	interface IMaterial;
	interface ISceneObject;
	interface ISceneCamera;
	interface IMesh;
};

typedef fw::IAction		* HACTION;
typedef fw::IBody		* HBODY;
typedef fw::IKineNode	* HBONE;
typedef fw::IMaterial	* HMATERIAL;
typedef fw::ISceneObject* HOBJECT;
typedef fw::ISceneCamera* HCAMERA;
typedef fw::IMesh		* HMESH;

interface ILostDeviceObserver
{
	virtual void OnLostDevice() = 0;
	virtual void OnResetDevice() = 0;
};

interface IAnimationListener
{
	virtual int OnAnimationBegin(AVULONG nParam) = 0;
	virtual int OnAnimationTick(AVULONG nParam) = 0;
	virtual int OnAnimationEnd(AVULONG nParam) = 0;
};

